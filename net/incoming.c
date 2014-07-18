#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <uv.h>

uv_loop_t *newTCPHandlerThread (uint16_t port, ) {
	uv_loop_t *loop = uv_loop_new ();
	if (!loop)
		return NULL;	// allocation failed

	// initialize libuv tcp loop
	uv_tcp_t server;
	uv_tcp_init (loop, &server);

	// bind to port on all interfaces
	struct sockaddr_in bind = uv_ip4_addr ("0.0.0.0", port);
	uv_tcp_bind (&server, bind);

	if (uv_listen ((uv_stream_t *) &server, 256, incomingTCPconnection
