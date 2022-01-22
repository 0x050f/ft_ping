#include "ft_ping.h"

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

int			handle_options(int argc, char *argv[], int *i)
{
	int		j;
	int		k;
	char	options[NB_OPTIONS] = {'f', 'h', 'I', 'l', 'm', 'M', 'n', 'p', 'Q',
									'S', 't', 'T', 'w', 'W'};

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
			else if (options[k] == 'f')
				g_ping.options.f = 1;
			else if (options[k] == 't')
			{
				char *str;

				g_ping.options.t = 1;
				if (ft_strlen(&argv[*i][j + 1]))
					str = &argv[*i][j + 1];
				else if (argc  - 1 < *i + 1)
					return (args_error(ERR_REQ_ARG, "t"));
				else
				{
					*i += 1;
					str = argv[*i];
				}
				g_ping.ttl_val = ft_atoi(str);
				if (g_ping.ttl_val < 0 || g_ping.ttl_val > 255)
					return (args_error(ERR_OOR_ARG, str));
				if (!is_num(str))
					return (args_error(ERR_INV_ARG, str));
				return (0);
			}
		}
		j++;
	}
	/* Weird when `ping - localhost` != `ping -` */
	if ((j == 1 && (argc == *i + 1 || !ft_strcmp(argv[*i + 1], "localhost"))) || argv[*i][j])
	{
		if (k == NB_OPTIONS)
		{
			char option[2] = {0, 0};
			option[0] = argv[*i][j];
			args_error(ERR_INV_OPT, option);
		}
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
		return (args_error(ERR_NO_ARGS, NULL));
	for (int i = 1; i < argc; i++)
	{
		/* Weird when `ping - localhost` != `ping -` */
		if (*argv[i] != '-' || (ft_strlen(argv[i]) == 1 && (argc == i + 1 || ft_strcmp(argv[i + 1], "localhost"))))
		{
			if (ping->hostname)
				return (args_error(ERR_NB_DEST, NULL));
			ping->hostname = argv[i];
			ret = resolve_hostname(g_ping.address, g_ping.hostname);
			if (ret)
				return (getaddrinfo_error(ret, g_ping.hostname));
			else if (!g_ping.address)
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
	if (!ping->hostname)
		return (args_error(ERR_NO_ARGS, NULL));
	return (0);
}
