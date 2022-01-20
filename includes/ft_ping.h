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
# define DATA_SIZE 16

/*
	Documentation: 
	https://en.wikipedia.org/wiki/Ping_(networking_utility)
	https://en.wikipedia.org/wiki/IPv4#Header
	https://en.wikipedia.org/wiki/Internet_Control_Message_Protocol
*/

typedef struct		s_icmp_packet
{
	struct iphdr	iphdr; // 20 bytes
	struct icmphdr	icmphdr; // 8 bytes
	char			payload[4];
	struct timeval	start; // 16 bytes
	char			data[DATA_SIZE]; // 16 bytes
}					t_icmp_packet; // 64 bytes in ipv4

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

extern t_ping		g_ping;

/* send_packet.c */
void	send_packet(int signum);

/* utils.c */
double		get_diff_ms(struct timeval *start, struct timeval *end);
long		ft_sqrt(long long nb, long long x);
void		*ft_memset(void *b, int c, size_t len);
void		ft_bzero(void *s, size_t n);

#endif
