#include "ft_ping.h"

t_ping		g_ping;

int		init_ping(t_ping *ping)
{
	int ret;

	ret = inet_pton(AF_INET, ping->address, &ping->ip_addr);
	if (ret <= 0)// TODO: inet_pton error
	{
		dprintf(STDERR_FILENO, "%s: inet_pton: Error\n", ping->prg_name);
		return (1);
	}
	ping->sockaddr.sin_addr.s_addr = ping->ip_addr;
	ping->sockaddr.sin_family = AF_INET;
	ping->sockaddr.sin_port = 0;
	ping->sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (ping->sockfd < 0)// TODO: socket error
	{
		dprintf(STDERR_FILENO, "%s: socket: Operation not permitted\n", ping->prg_name);
		return (1);
	}
	ping->ttl_val = 64;
	int on = 1;
	/* Set socket options at ip to TTL and value to ttl_val */
	setsockopt(ping->sockfd, IPPROTO_IP, IP_HDRINCL, (const char *)&on, sizeof(on));
//	setsockopt(ping->sockfd, SOL_IP, IP_TTL, &ping->ttl_val, sizeof(ping->ttl_val));
	ping->stats.timer.min = DBL_MAX;
	if (gettimeofday(&ping->stats.start, NULL))
	{
		dprintf(STDERR_FILENO, "%s: gettimeofday: Error\n", ping->prg_name);
		return (1);
	}
	return (0);
}

/*
	getaddr and fill addr
*/
int			resolve_hostname(char *addr, char *hostname)
{
	int ret;
	struct addrinfo *res;
	struct addrinfo hints;

	ft_bzero(&hints, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	ret = getaddrinfo(hostname, NULL, &hints, &res);
	if (!ret)
	{
		inet_ntop(AF_INET, &((struct sockaddr_in *)res->ai_addr)->sin_addr, addr, ADDR_SIZE);
		freeaddrinfo(res);
	}
	return (ret);
}

int			check_args(int argc, char *argv[], t_ping *ping)
{
	char options[2] = {'f', 'm'};

	ft_bzero(ping, sizeof(t_ping));
	ping->prg_name = argv[0];
	if (argc < 2)
	{
		dprintf(STDERR_FILENO, "%s: usage error: Destination address required\n", ping->prg_name);
		return (1);
	}
	/* TODO: options */
	/* TODO: help -h */
	for (int i = 0; i < argc; i++)
	{
		if (*argv[i] != '-')
			ping->hostname = argv[i];
		if (*argv[i] == '-')
		{
			int j = 0;
			while (argv[i][j])
			{
				int k = 0;
				while (options[k])
				{
					if (argv[i][j] == options[k])
						break ;
					k++;
				}
				if (k != sizeof(options))
					return (1);
				j++;
			}
		}
	}
	return (0);
}

int			main(int argc, char *argv[])
{
	int		ret;

	ret = check_args(argc, argv, &g_ping); 
	if (ret)
		return (ret);
	ret = resolve_hostname(g_ping.address, g_ping.hostname);
	if (ret) // TODO: getaddrinfo error
	{
		dprintf(STDERR_FILENO, "%s: %s: Name or service not known\n", g_ping.prg_name, g_ping.hostname);
		return (2);
	}
	else if (!g_ping.address) // TODO: inet_ntop error
	{
		dprintf(STDERR_FILENO, "%s: inet_ntop: Error\n", g_ping.prg_name);
		return (2);
	}
	if (init_ping(&g_ping))
		return (2);
	signal(SIGALRM, &send_packet);
	signal(SIGINT, &ping_stats);
	printf("PING %s (%s) %d(%ld) bytes of data.\n", g_ping.hostname, g_ping.address, PAYLOAD_SIZE, sizeof(t_icmp_packet));
	send_packet(0);
	recv_packet();
	return (0);
}
