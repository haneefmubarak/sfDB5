#include "sfDB5.h"


int main (int argc, char **argv) {
	struct event_base *eventBase;
	struct evconnlistener *client_lner;
	struct sockaddr_in *client_socket;

	eventBase = event_base_new();
	assert (eventBase);

	client_socket = calloc (1, sizeof (client_socket));
	assert (client_socket);
	client_socket.sin_family	= AF_INET;
	client_socket.sin_addr.s_addr	= htonl (INADDR_ANY);
	client_socket.sin_port		= htons (6446);

	client_lner = evconnlistener_new_bind (eventBase, incoming_client_con, NULL,
						LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, -1,
						(struct sockaddr *) client_socket,
						sizeof (struct sockaddr));
	assert (client_lner);
	evconnlistener_set_error_cb (client_lner, incoming_client_err);

	event_base_dispatch (base);

	return 0;
}
