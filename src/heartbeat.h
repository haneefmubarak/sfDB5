#pragma once

//===	Includes

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <arpa/inet.h>

#include "xm.h"
#include "constants.h"
#include "server.h"
#include "crypto.h"

//===	Defines

#define	HEARTBEAT_MAX_LEN	256

//===	Functions

// sends and checks heartbeats
// (pthread)
void *Heartbeat (void *p);
