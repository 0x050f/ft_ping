#include "ft_ping.h"

int			getaddrinfo_error(int error, char *str)
{
	dprintf(STDERR_FILENO, "%s: %s: ", g_ping.prg_name, str);
	if (error == EAI_AGAIN || error == EAI_FAIL)
		dprintf(STDERR_FILENO, "Temporary failure in name resolution\n");
	else if (error == EAI_NONAME)
		dprintf(STDERR_FILENO, "Name or service not known\n");
	else
		dprintf(STDERR_FILENO, "Error\n");
	return (2);
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
		{"-v", "verbose output"},
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

int			usage_error(int error)
{
	dprintf(STDERR_FILENO, "usage error: ");
	if (error == ERR_NO_ARGS)
		dprintf(STDERR_FILENO, "Destination address required\n");
	else if (error == ERR_NB_DEST)
		dprintf(STDERR_FILENO, "Can't handle multiple destination\n");
	return (1);
}

int			args_error(int error, char *str)
{
	dprintf(STDERR_FILENO, "%s: ", g_ping.prg_name);
	if (error == ERR_NO_ARGS || error == ERR_NB_DEST)
		return(usage_error(error));
	else if (error == ERR_INV_OPT)
		dprintf(STDERR_FILENO, "invalid option -- '%s'\n", str);
	else if (error == ERR_INV_ARG || error == ERR_OOR_ARG)
	{
		dprintf(STDERR_FILENO, "invalid argument: '%s'", str);
		if (error == ERR_OOR_ARG)
			dprintf(STDERR_FILENO, ": out of range: 0 <= value <= 255");
		dprintf(STDERR_FILENO, "\n");
	}
	else if (error == ERR_REQ_ARG)
		dprintf(STDERR_FILENO, "option requires an argument -- '%s'\n", str);
	return (2);
}
