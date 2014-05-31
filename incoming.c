#include "sfDB5.h"

void incoming_client_con (struct evconnlistener *client_lner, evutil_socket_t fd,
				struct sockaddr *client_socket, int socklen, void *ctx) {

	struct event_base *evBase = evconnlistener_get_base (client_lner);
	struct bufferevent *bufEv = bufferevent_socket_new (evBase, fd, BEV_OPT_CLOSE_ON_FREE);

	bufferevent_setcb (bufEv, 
