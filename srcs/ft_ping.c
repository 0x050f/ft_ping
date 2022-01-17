#include "ft_ping.h"

t_ping g_ping;

long		ft_sqrt(long long nb, long long x)
{
	for (int i = 0; i < 10; i++)
		x -= (x * x - nb) / (2 * x);
	return (x);
}

/* RFC 1071 https://datatracker.ietf.org/doc/html/rfc1071 */
unsigned short checksum(void *addr, size_t count)
{
	unsigned short *ptr;
	unsigned long sum;

	ptr = addr;
	for (sum = 0; count > 1; count -= 2)
		sum += *ptr++;
	if (count > 0)
		sum += *(unsigned char *)ptr;
	while (sum >> 16)
		sum = (sum & 0xffff) + (sum >> 16);
	return (~sum);
}

void	ft_bzero(void *s, size_t n)
{
	char	*tmp;

	while (n--)
	{
		tmp = (char *)s;
		*tmp = 0;
		s++;
	}
}

int				resolve_hostname(char *addr, char *hostname)
{
	int ret;
	struct addrinfo *res;
	struct addrinfo hints;

	ft_bzero(&hints, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	ret = getaddrinfo(hostname, NULL, &hints, &res);
	if (ret)
		return (ret);
	inet_ntop(AF_INET, &((struct sockaddr_in *)res->ai_addr)->sin_addr, addr, 256);
	return (0);
}

void			sig_handler(int sig_num);

int				ping(int sockfd, struct sockaddr_in *serv_addr)
{
	struct timeval start;
	struct timeval end;
	static int nbr_packet = 0;

	char buffer[64];
	bzero(buffer, sizeof(buffer));
	struct icmphdr *hdr;
	hdr = (struct icmphdr *)&buffer;
	hdr->type = ICMP_ECHO;
	hdr->un.echo.id = getpid();
	hdr->un.echo.sequence = ++nbr_packet;
	hdr->checksum = checksum(&buffer, sizeof(buffer));
	g_ping.n_repeat = nbr_packet;
	int sent = 0;
	// sendto ^ | recvmsg v
	char buf[80];
	struct iovec iov;
	struct msghdr msghdr;
	bzero(&msghdr, sizeof(msghdr));
	msghdr.msg_name = serv_addr;
	msghdr.msg_namelen = sizeof(*serv_addr);
	iov.iov_base = buf;
	iov.iov_len = sizeof(buf);
	msghdr.msg_iov = &iov;
	msghdr.msg_iovlen = 1;
	if (gettimeofday(&start, NULL))
	{
//		dprintf(stderr_fileno, "%s: gettimeofday: %s\n", argv[0], strerror(errno));
	}
	alarm(1);
	if (sendto(sockfd, &buffer, sizeof(buffer), 0, (struct sockaddr *)serv_addr, sizeof(*serv_addr)) < 0)
	{
//		dprintf(STDERR_FILENO, "%s: sendto: %s\n", argv[0], strerror(errno));
		return (1);
	}
	else
	{
		sent = 1;
		++g_ping.transmitted;
	}
	if (recvmsg(sockfd, &msghdr, 0) < 0)
	{
//		dprintf(STDERR_FILENO, "%s: recvmsg: %s\n", argv[0], strerror(errno));
		return (1);
	}
	else
		++g_ping.received;
	if (gettimeofday(&end, NULL))
	{
//		dprintf(STDERR_FILENO, "%s: gettimeofday: %s\n", argv[0], strerror(errno));
	}
	if (sent)
	{
		double diff = (float)((end.tv_sec - start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec) / 1000;
		if (diff < g_ping.timer.min)
			g_ping.timer.min = diff;
		if (diff > g_ping.timer.max)
			g_ping.timer.max = diff;
		g_ping.timer.sum += diff;
		g_ping.timer.tsum += (end.tv_sec - start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec;
		g_ping.timer.tsum2 += ((long long)(end.tv_sec - start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec) * ((long long)(end.tv_sec - start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec);
		if (diff < 0.1)
			printf("icmp_seq=%d time=%.3f ms\n", nbr_packet, diff);
		else
			printf("icmp_seq=%d time=%.2f ms\n", nbr_packet, diff);
	}
	else
		printf("ko\n");
	return (0);
}

void			sig_handler(int sig_num)
{
	(void)sig_num;

	if (sig_num == SIGALRM)
	{
		if (gettimeofday(&g_ping.end, NULL))
		{
		}
		ping(g_ping.fd, g_ping.addr);
	}
	else if (sig_num == SIGINT)
	{
		/* TODO: print stats */
		printf("\n--- %s ping statistics ---\n", g_ping.hostname);
		/* TODO: packet loss */
		long time;
		if (g_ping.end.tv_sec && g_ping.end.tv_usec)
			time = ((g_ping.end.tv_sec - g_ping.start.tv_sec) * 1000000 + g_ping.end.tv_usec - g_ping.start.tv_usec) / 1000;
		else
			time = 0;
		printf("%ld packets transmitted, %ld received, 0%% packet loss, time %ld ms\n", g_ping.transmitted, g_ping.received, time);
		g_ping.timer.tsum /= g_ping.received;
		g_ping.timer.tsum2 /= g_ping.received;
		double tmdev;
		if (g_ping.n_repeat != 1)
			tmdev = (long double)ft_sqrt(g_ping.timer.tsum2 - g_ping.timer.tsum * g_ping.timer.tsum, g_ping.timer.max * 1000 - g_ping.timer.min * 1000) / 1000.0;
		else
			tmdev = 0;
		printf("rtt min/avg/max/mdev = %.3f/%.3f/%.3f/%.3f ms\n", g_ping.timer.min, g_ping.timer.sum / g_ping.received, g_ping.timer.max, tmdev);
		exit(0);
	}
}

int				main(int argc, char *argv[])
{
	int ret;
	int sockfd;

	if (argc < 2)
	{
		dprintf(STDERR_FILENO, "%s: usage error: Destination address required\n", argv[0]);
		return (1);
	}
	/* TODO: Parse options */
	g_ping.hostname = argv[1];
	/* Resolve hostname */
	char addr[256];
	ret = resolve_hostname(addr, g_ping.hostname);
	if (ret)
	{
/*
		if (ret == EAI_AGAIN)
			dprintf(STDERR_FILENO, "%s: %s: Temporary failure in name resolution\n", argv[0], g_ping.hostname);
		else*/
			dprintf(STDERR_FILENO, "%s: %s: Name or service not known\n", argv[0], g_ping.hostname);
		return (1);
	}
	printf("%s\n", addr);
	struct sockaddr_in serv_addr;
	bzero(&serv_addr, sizeof(serv_addr));
	ret = inet_pton(AF_INET, addr, &serv_addr.sin_addr.s_addr);
	if (!ret)
	{
		printf("not valid\n");
	}
	else if (ret == -1)
	{
		dprintf(STDERR_FILENO, "%s: inet_pton: %s\n", argv[0], strerror(errno));
	}
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = 0;
	sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sockfd < 0)
	{
		dprintf(STDERR_FILENO, "%s: socket: Operation not permitted\n", argv[0]);
		return (1);
	}
	/* TODO: signal handler ctrl + c */
	g_ping.fd = sockfd;
	g_ping.addr = &serv_addr;
	g_ping.transmitted = 0;
	g_ping.received = 0;
	g_ping.timer.min = DBL_MAX;
	g_ping.timer.max = 0;
	g_ping.timer.sum = 0;
	g_ping.timer.tsum = 0;
	g_ping.timer.tsum2 = 0;
	signal(SIGALRM, sig_handler); // Register signal handler
	signal(SIGINT, sig_handler); // Register signal handler
	g_ping.end.tv_sec = 0;
	g_ping.end.tv_usec = 0;
	if (gettimeofday(&g_ping.start, NULL))
	{
	}
	ping(sockfd, &serv_addr);
	while (1)
		;
	return (0);
}
