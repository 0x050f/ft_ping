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
	static int nbr_packet = 0;


	char buffer[20 + ICMP_MINLEN + 56];
	bzero(buffer, sizeof(buffer));
	struct 
	struct icmphdr *hdr;
	hdr = ((struct icmphdr *)(&buffer + 20));
	hdr->type = ICMP_ECHO;
	hdr->code = 0;
	hdr->un.echo.id = getpid();
	hdr->un.echo.sequence = ++nbr_packet;
	hdr->checksum = checksum(&hdr, sizeof(hdr) + 56);
	struct timeval *start = ((void *)(buffer + ICMP_MINLEN + 4));
	g_ping.n_repeat = nbr_packet;
	// sendto ^ | recvmsg v
	if (gettimeofday(start, NULL))
	{
//		dprintf(stderr_fileno, "%s: gettimeofday: %s\n", argv[0], strerror(errno));
	}
	alarm(1);
	if (sendto(sockfd, &buffer, sizeof(buffer), 0, (struct sockaddr *)serv_addr, sizeof(*serv_addr)) <= 0)
		return (1);
	else
		++g_ping.transmitted;
	//msg_control=[{cmsg_len=32, cmsg_level=SOL_SOCKET, cmsg_type=SO_TIMESTAMP_OLD, cmsg_data={tv_sec=1642460328, tv_usec=296924}}], msg_controllen=32, msg_flags=0}
	return (1);
}

void			receive_ping(int sockfd, struct sockaddr_in *serv_addr)
{
	struct timeval end;
	char buf[20 + ICMP_MINLEN + 56];
	struct iovec iov;
	struct msghdr msghdr;
	bzero(&msghdr, sizeof(msghdr));
	msghdr.msg_name = serv_addr;
	msghdr.msg_namelen = sizeof(*serv_addr);
	iov.iov_base = buf;
	iov.iov_len = sizeof(buf);
	msghdr.msg_iov = &iov;
	msghdr.msg_iovlen = 1;
	char ans_data[4096];
	msghdr.msg_control = ans_data;
	msghdr.msg_controllen = sizeof(ans_data);
	msghdr.msg_flags = 0;
	struct timeval tv_out;
	tv_out.tv_sec = 1;
	tv_out.tv_usec = 0;
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,(const char*)&tv_out, sizeof tv_out);// timeout recvmsg
	while (1)
	{
		recvmsg(sockfd, &msghdr, 0);
		struct icmphdr *icmp = (struct icmphdr	*)(msghdr.msg_iov->iov_base + 20);
		struct timeval *start = (void *)(msghdr.msg_iov->iov_base + 20 + ICMP_MINLEN + 4);
		if (icmp->type == ICMP_ECHOREPLY)
		{
			++g_ping.received;
			if (gettimeofday(&end, NULL))
			{
		//		dprintf(STDERR_FILENO, "%s: gettimeofday: %s\n", argv[0], strerror(errno));
			}
			double diff = (float)((end.tv_sec - start->tv_sec) * 1000000 + end.tv_usec - start->tv_usec) / 1000;
			if (diff < g_ping.timer.min)
				g_ping.timer.min = diff;
			if (diff > g_ping.timer.max)
				g_ping.timer.max = diff;
			g_ping.timer.sum += diff;
			g_ping.timer.tsum += (end.tv_sec - start->tv_sec) * 1000000 + end.tv_usec - start->tv_usec;
			g_ping.timer.tsum2 += ((long long)(end.tv_sec - start->tv_sec) * 1000000 + end.tv_usec - start->tv_usec) * ((long long)(end.tv_sec - start->tv_sec) * 1000000 + end.tv_usec - start->tv_usec);
			if (diff < 0.1)
				printf("icmp_seq=%ld time=%.3f ms\n", g_ping.received, diff);
			else
				printf("icmp_seq=%ld time=%.2f ms\n", g_ping.received, diff);
		}
		else if (icmp->type != ICMP_ECHO)
		{
			printf("icmp->type == %d\n", icmp->type);
			/* TODO: From XXXX Destination host unreachable */
			char *error = NULL;
			printf("icmp_seq=%hu %s\n", icmp->un.echo.sequence, error);
			++g_ping.errors;
		}
		else
			printf("icmp_seq=%hu - icmp echo\n", icmp->un.echo.sequence);
	}
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
		printf("%ld packets transmitted, %ld received, ", g_ping.transmitted, g_ping.received);
		if (g_ping.errors)
			printf("+%ld errors, ", g_ping.errors);
		double percentage;
		if (g_ping.received)
			percentage = (1.0 - (double)g_ping.received / (double)g_ping.transmitted) * 100.0;
		else
			percentage = 0;
		printf("%.4g%% packet loss, time %ld ms\n", percentage, time);
		if (g_ping.received)
		{
			g_ping.timer.tsum /= g_ping.received;
			g_ping.timer.tsum2 /= g_ping.received;
			double tmdev;
			if (g_ping.n_repeat != 1)
				tmdev = (long double)ft_sqrt(g_ping.timer.tsum2 - g_ping.timer.tsum * g_ping.timer.tsum, g_ping.timer.max * 1000 - g_ping.timer.min * 1000) / 1000.0;
			else
				tmdev = 0;
			printf("rtt min/avg/max/mdev = %.3f/%.3f/%.3f/%.3f ms\n", g_ping.timer.min, g_ping.timer.sum / g_ping.received, g_ping.timer.max, tmdev);
		}
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
    // set socket options at ip to TTL and value to 64,
    // change to what you want by setting ttl_val
	int ttl_val = 64;
    setsockopt(sockfd, SOL_IP, IP_TTL, &ttl_val, sizeof(ttl_val));
	/* TODO: signal handler ctrl + c */
	g_ping.fd = sockfd;
	g_ping.addr = &serv_addr;
	g_ping.transmitted = 0;
	g_ping.received = 0;
	g_ping.errors = 0;
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
	receive_ping(sockfd, &serv_addr);
	return (0);
}
