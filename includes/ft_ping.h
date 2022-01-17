#ifndef FT_PING_H
# define FT_PING_H

# include <stdlib.h>
# include <stdio.h>
# include <sys/time.h>
# include <sys/socket.h>
# include <signal.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <netdb.h>
# include <netinet/ip_icmp.h>
# include <limits.h>
# include <float.h>

# include <string.h>
# include <errno.h>
# include <unistd.h>

typedef struct			s_timer
{
	double				min;
	double				max;
	double				sum;
	long long			tsum;
	long long			tsum2;
}						t_timer;

typedef struct			s_ping
{
	int					fd;
	struct sockaddr_in	*addr;
	char				*hostname;
	size_t				n_repeat;
	size_t				transmitted;
	size_t				received;
	t_timer				timer;
	struct timeval		start;
	struct timeval		end;
}						t_ping;

/*
       EAI_ADDRFAMILY
              The specified network host does not have any network addresses in the requested address family.

       EAI_AGAIN
              The name server returned a temporary failure indication.  Try again later.

       EAI_BADFLAGS
              hints.ai_flags contains invalid flags; or, hints.ai_flags included AI_CANONNAME and name was NULL.

       EAI_FAIL
              The name server returned a permanent failure indication.

       EAI_FAMILY
              The requested address family is not supported.

       EAI_MEMORY
              Out of memory.

       EAI_NODATA
              The specified network host exists, but does not have any network addresses defined.

       EAI_NONAME
              The node or service is not known; or both node and service are NULL; or AI_NUMERICSERV was specified in hints.ai_flags and service was not a numeric port-number string.

       EAI_SERVICE
              The  requested  service is not available for the requested socket type.  It may be available through another socket type.  For example, this error could occur if service was "shell" (a service available only on stream sockets), and either hints.ai_protocol was
              IPPROTO_UDP, or hints.ai_socktype was SOCK_DGRAM; or the error could occur if service was not NULL, and hints.ai_socktype was SOCK_RAW (a socket type that does not support the concept of services).

       EAI_SOCKTYPE
              The requested socket type is not supported.  This could occur, for example, if hints.ai_socktype and hints.ai_protocol are inconsistent (e.g., SOCK_DGRAM and IPPROTO_TCP, respectively).

       EAI_SYSTEM
              Other system error, check errno for details.
*/


#endif
