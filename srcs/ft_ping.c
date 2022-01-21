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
	if (!g_ping.options.t)
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

void		print_help_menu(void)
{
	char *options[NB_OPTIONS + 1 - NB_IPV4_OPTIONS][2] =
	{
		{"<destination>", "dns name or ip address"},
		{"-f", "flood ping"},
		{"-h", "print help and exit"},
		{"-I <interface>", "either interface name or address"},
		{"-l <preload>", "send <preload> number of packages while waiting replies"},
		{"-m <mark>", "tag the packets going out"},
		{"-M <pmtud opt>", "define mtu discovery, can be one of <do|dont|want>"},
		{"-n", "no dns name resolution"},
		{"-p <pattern>", "contents of padding byte"}, 
		{"-Q <tclass>", "use quality of service <tclass> bits"},
		{"-S <size>", "use <size> as SO_SNDBUF socket option value"},
		{"-t <ttl>", "define time to live"},
		{"-w <deadline>", "reply wait <deadline> in seconds"},
		{"-W <timeout>", "time to wait for response"}
	};
	char *ipv4_options[NB_IPV4_OPTIONS][2] = {
		{"-T <timestamp>", "define timestamp, can be one of <tsonly|tsandaddr|tsprespec>"}
	};
	printf("\nUsage\n  ping [options] <destination>\n\nOptions:\n");
	for (size_t i = 0; i < NB_OPTIONS + 1 - NB_IPV4_OPTIONS; i++)
		printf("  %-18s %s\n", options[i][0], options[i][1]);
	printf("\nIPv4 options:\n");
	for (size_t i = 0; i < NB_IPV4_OPTIONS; i++)
		printf("  %-18s %s\n", ipv4_options[i][0], ipv4_options[i][1]);
}

int			handle_options(int argc, char *argv[], int *i)
{
	int		j;
	int		k;
	char	options[NB_OPTIONS] = {'f', 'h', 'I', 'l', 'm', 'M', 'n', 'p', 'Q', 'S', 't', 'T', 'w', 'W'};

	k = 0;
	j = 1;
	while (argv[*i][j])
	{
		k = 0;
		while (k < NB_OPTIONS)
		{
			if (argv[*i][j] == options[k])
				break ;
			k++;
		}
		if (k == NB_OPTIONS)
			break ;
		else
		{
			if (options[k] == 'h')
			{
				print_help_menu();
				return (2);
			}
			if (options[k] == 't')
			{
				g_ping.options.t = 1;
				if (ft_strlen(&argv[*i][j + 1]))
				{
					g_ping.ttl_val = ft_atoi(&argv[*i][j + 1]);
					if (g_ping.ttl_val < 0 || g_ping.ttl_val > 255)
					{
						dprintf(STDERR_FILENO, "%s: invalid argument: '%d': out of range: 0 <= value <= 255\n", g_ping.prg_name, g_ping.ttl_val);
						return (2);
					}
					if (!is_num(&argv[*i][j + 1]))
					{
						dprintf(STDERR_FILENO, "%s: invalid argument: '%s'\n", g_ping.prg_name, &argv[*i][j + 1]);
						return (2);
					}
					return (0);
				}
				else
				{
					if (argc  - 1 < *i + 1)
					{
						dprintf(STDERR_FILENO, "%s: option requires an argument -- '%c'\n", g_ping.prg_name, 't');
						return (2);
					}
					else
					{
						*i += 1;
						g_ping.ttl_val = ft_atoi(argv[*i]);
						if (g_ping.ttl_val < 0 || g_ping.ttl_val > 255)
						{
							dprintf(STDERR_FILENO, "%s: invalid argument: '%d': out of range: 0 <= value <= 255\n", g_ping.prg_name, g_ping.ttl_val);
							return (2);
						}
						if (!is_num(argv[*i]))
						{
							dprintf(STDERR_FILENO, "%s: invalid argument: '%s'\n", g_ping.prg_name, argv[*i]);
							return (2);
						}
					}
					return (0);
				}
			}
		}
		j++;
	}
	if ((j == 1 && (argc == *i + 1 || !ft_strcmp(argv[*i + 1], "localhost"))) || argv[*i][j])
	{
		if (k == NB_OPTIONS)
			printf("%s: invalid option -- '%c'\n", g_ping.prg_name, argv[*i][j]);
		print_help_menu();
		return (2);
	}
	return (0);
}

int			check_args(int argc, char *argv[], t_ping *ping)
{
	int		ret;

	ft_bzero(ping, sizeof(t_ping));
	ping->prg_name = argv[0];
	ping->hostname = NULL;
	if (argc < 2)
	{
		dprintf(STDERR_FILENO, "%s: usage error: Destination address required\n", ping->prg_name);
		return (1);
	}
	/* TODO: options */
	/* TODO: help -h */
	for (int i = 1; i < argc; i++)
	{
		if (*argv[i] != '-' || (ft_strlen(argv[i]) == 1 && (argc == i + 1 || ft_strcmp(argv[i + 1], "localhost"))))
		{
			if (ping->hostname)
			{
				dprintf(STDERR_FILENO, "%s: usage error: Can't handle multiple destination\n", ping->prg_name);
				return (1);
			}
			ping->hostname = argv[i];
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
		}
		if (*argv[i] == '-')
		{
			ret = handle_options(argc, argv, &i);
			if (ret)
				return (ret);
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
	if (init_ping(&g_ping))
		return (2);
	signal(SIGALRM, &send_packet);
	signal(SIGINT, &ping_stats);
	printf("PING %s (%s) %d(%ld) bytes of data.\n", g_ping.hostname, g_ping.address, PAYLOAD_SIZE, sizeof(t_icmp_packet));
	send_packet(0);
	recv_packet();
	return (0);
}
