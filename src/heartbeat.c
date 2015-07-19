#include "heartbeat.h"

static void internal__heartbeat_send (int s) {
	pthread_rwlock_rdlock (&server_list_lock);	// this lock will hold for a while

	int x;
	for (x = 0; x < server_alive_count; x++) {
		kv_string counter, payload;
		server_peer[x].heartbeat.ctr.out++;
		counter.data = (void *) &server_peer[x].heartbeat.ctr.out;
		counter.len = sizeof (uint64_t);

		if (CryptoAEADServerSend (server_peer[x], counter, &payload)) {
			perror ("sfDB5::heartbeat::internal__heartbeat_send {\n"
				"CryptoAEADServerSend couldn't allocate memory:\n"
				"received error:\t");
			fputs ("} sfDB5::heartbeat::internal__heartbeat\n", stderr);
			exit (EXIT_FAILURE);
		}

		// reuse existing buffer when doing this
		struct msghdr msg = { 0 };
		struct iovec iov = { 0 };
		msg.msg_name = &server_peer[x].addr;
		msg.msg_namelen = sizeof (server_peer[x].addr);
		msg.msg_iov = &iov;
		msg.msg_iovlen = 1;
		msg.msg_iov->iov_base = payload.data;
		msg.msg_iov->iov_len = payload.len;

		sendmsg (s, &msg, 0);

		// cleanup
		free (payload.data);
	}

	pthread_rwlock_unlock (&server_list_lock);
}

static void *internal__heartbeat_wake (void *p) {
	// force bind
	int s;
	do {
		s = socket (AF_INET, SOCK_STREAM, IPPROTO_SCTP);
	} while (s == -1);

	struct sockaddr_in addr = { 0 };
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr ("127.0.0.1");
	addr.sin_port = htons (6493);

	struct msghdr m = { 0 };
	struct iovec iovc = { 0 };
	uint8_t *string = "WAKE UP DAMMIT!";
	m.msg_name = &addr;
	m.msg_namelen = sizeof (addr);
	m.msg_iov = &iovc;
	m.msg_iovlen = 1;
	m.msg_iov->iov_base = string;
	m.msg_iov->iov_len = strlen (string);

	// get time and calculate offsets
	struct timespec t = { 0 };
	// do not under any circumstances use CLOCK_MONOTONIC_COARSE
	clock_gettime (CLOCK_MONOTONIC, &t);
	double next = ServerTimeToFloat64 (t);

	while (1) {
		int err;
		do {
			err = sendmsg (s, &m, 0);
		} while (err == -1);

		next += HEARTBEAT_PERIOD / SI_G;	// ns --> s
		clock_gettime (CLOCK_MONOTONIC, &t);
		const double now = ServerTimeToFloat64 (t);
		const double until = next - now;

		if (until < 0)	// catch up if we need to
			continue;
		else {		// we're safe - relinquish the cpu
			t = ServerTimeFromFloat64 (until);
			clock_nanosleep (CLOCK_MONOTONIC, 0, &t, NULL);
		}
	}

	return NULL;
}



void *Heartbeat (void *p) {
	//struct timespec t = { 0 };
	// do not under any circumstances use CLOCK_MONOTONIC_COARSE
	//clock_gettime (CLOCK_MONOTONIC, &t);
	//double t = ServerTimeToFloat64 (t);

	// background thread to force wakeup timeout on recvmmsg()
	{	// allow stack deallocation of pthread_t
		pthread_t wake;
		pthread_create (&wake, NULL, internal__heartbeat_wake, NULL);
		pthread_detach (wake);
	}

	// setup
	int s;
	s = socket (AF_INET, SOCK_STREAM, IPPROTO_SCTP);
	if (s == -1) {
		perror ("sfDB5::heartbeat::Heartbeat {\n"
			"socket() failed while trying to acquire a socket:\n"
			"received error:\t");
		fputs ("} sfDB5::heartbeat::Heartbeat\n", stderr);
		exit (EXIT_FAILURE);
	}

	struct sockaddr_in l = { 0 };
	l.sin_family = AF_INET;
	l.sin_addr.s_addr = htonl (INADDR_ANY);
	l.sin_port = htons (6493);

	if (bind (s, (struct sockaddr *) &l, sizeof (l))) {
		perror ("sfDB5::heartbeat::Heartbeat {\n"
			"bind failed while trying to bind a socket:\n"
			"received error:\t");
		fputs ("} sfDB5::heartbeat::Heartbeat\n", stderr);
		exit (EXIT_FAILURE);
	}

	if (listen (s, 2 * MAX_SERVERS)) {
		perror ("sfDB5::heartbeat::Heartbeat {\n"
			"listen() failed while trying to listen on a socket:\n"
			"received error:\t");
		fputs ("} sfDB5::heartbeat::Heartbeat\n", stderr);
		exit (EXIT_FAILURE);
	}

	// we can boo hoo early SEGFAULT if insufficient RAM
	struct mmsghdr	*msgs = calloc (MAX_SERVERS, sizeof (struct mmsghdr));
	struct iovec	*iovs = calloc (MAX_SERVERS, sizeof (struct iovec));
	uint8_t *iobuf = calloc (MAX_SERVERS, HEARTBEAT_MAX_LEN);

	int x;
	for (x = 0; x < MAX_SERVERS; x++) {
		iovs[x].iov_base	= &iobuf[x * HEARTBEAT_MAX_LEN];
		iovs[x].iov_len		= HEARTBEAT_MAX_LEN;
		msgs[x].msg_hdr.msg_iov		= &iovs[x];
		msgs[x].msg_hdr.msg_iovlen	= 1;
	}

	// get time and calculate offsets
	struct timespec t = { 0 };
	// do not under any circumstances use CLOCK_MONOTONIC_COARSE
	clock_gettime (CLOCK_MONOTONIC, &t);
	double next = ServerTimeToFloat64 (t);

	while (1) {
		internal__heartbeat_send (s);

		// convert from ns to s
		const struct timespec timeout = ServerTimeFromFloat64 (HEARTBEAT_PERIOD / SI_G);

		int rcvd = recvmmsg (s, msgs, MAX_SERVERS, MSG_DONTWAIT, &timeout);
		if (rcvd == -1) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				errno = 0;	// eh we'll check again
			} else {
				perror ("sfDB5::heartbeat::Heartbeat {\n"
					"recvmmsg() failed while listening for interserver heartbeat:\n"
					"received error:\t");
				fputs ("} sfDB5::heartbeat::Heartbeat\n", stderr);
				exit (EXIT_FAILURE);
			}
		} else {	// we got some packets and shit
			pthread_rwlock_rdlock (&server_list_lock);

			struct timespec ct = { 0 };
			clock_gettime (CLOCK_MONOTONIC, &ct);
			const double curtime = ServerTimeToFloat64 (ct);

			for (x = 0; x < rcvd; x++) {
				__uint128_t id;

				int minlen = 0;
				minlen += sizeof (id);
				minlen += NONCE_LEN;
				minlen += sizeof (uint64_t);
				minlen += AEAD_LEN;

				if (msgs[x].msg_len < minlen || msgs[x].msg_len > HEARTBEAT_MAX_LEN)
					continue;

				// we need to find the correct AEAD interserver key
				memcpy (&id, &iobuf[x * HEARTBEAT_MAX_LEN], sizeof (id));
				// the trick below is posible because of the server list struct layout
				server *srv = bsearch (&id, server_peer, server_alive_count,
						sizeof (*server_peer), ServerPoolIDCompare);

				if (!srv)	// we don't have this server in our list (yet)
					continue;

				// decrypt and verify the payload
				kv_string raw, payload;
				raw.data = &iobuf[x * HEARTBEAT_MAX_LEN + sizeof (id)];
				raw.len = msgs[x].msg_len - sizeof (id);

				if (CryptoAEADServerRecv (*srv, raw, &payload))	// authentication failed
					continue;

				uint64_t ctr;
				memcpy (&ctr, payload.data, sizeof (ctr));

				// verify counter to prevent repeated message attacks
				if (ctr <= srv->heartbeat.ctr.in)
					goto CLEANUP;

				// if we've gotten this far, we're good
				srv->heartbeat.ctr.in = ctr;
				if (srv->heartbeat.in < curtime)	// another thread might have a newer packet
					srv->heartbeat.in = curtime;

				CLEANUP:
				free (payload.data);
			}

			pthread_rwlock_unlock (&server_list_lock);
		}

		next += HEARTBEAT_PERIOD / SI_G;	// ns --> s
		clock_gettime (CLOCK_MONOTONIC, &t);
		const double now = ServerTimeToFloat64 (t);
		const double until = next - now;

		if (until < 0)	// catch up if we need to
			continue;
		else {		// we're safe - relinquish the cpu
			t = ServerTimeFromFloat64 (until);
			clock_nanosleep (CLOCK_MONOTONIC, 0, &t, NULL);
		}
	}
}
