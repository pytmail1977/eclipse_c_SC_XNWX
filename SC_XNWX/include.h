/*
 * include.h
 *
 *  Created on: 2015年1月25日
 *      Author: pyt64
 */

#ifndef INCLUDE_H_
#define INCLUDE_H_

#include "complier.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>

#ifdef _BUILD_ON_MAC
#include <sys/malloc.h>
#else
#include <malloc.h>
#endif
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <pthread.h>
#include <sys/time.h>
#include <semaphore.h>
#include <signal.h>
#include <mysql.h>

#include <net/if.h>

//#include <time.h>
//#include "/usr/libevent2.0.21/include/event2/event.h"

#endif /* INCLUDE_H_ *///#include <iostream.h>
//#include <fstream.h>
