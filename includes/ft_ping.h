#ifndef FT_PING_H
# define FT_PING_H

# include <arpa/inet.h>
# include <float.h>
# include <errno.h>
# include <netdb.h>
# include <netinet/ip.h>
# include <netinet/ip_icmp.h>
# include <signal.h>
# include <stdbool.h>
# include <stdio.h>
# include <stdlib.h>
# include <sys/time.h>
# include <unistd.h>

# define SEND_DELAY	1
# define ADDR_SIZE	64

typedef struct		s_timer
{
	double			min;
	double			max;
	double			sum;
	long long		tsum;
	long long		tsum2;
}					t_timer;

typedef struct		s_stats
{
	size_t			n_repeat;
	size_t			transmitted;
	size_t			received;
	size_t			errors;
	t_timer			timer;
	struct timeval	start;
	struct timeval	end;
}					t_stats;

typedef struct		s_ping
{
	char				*prg_name;
	char				*hostname;
	char				address[ADDR_SIZE];
	struct sockaddr_in	sockaddr;
	int					sockfd;
	size_t				ttl_val;
	t_stats				stats;

}					t_ping;

t_ping				g_ping;

#endif
