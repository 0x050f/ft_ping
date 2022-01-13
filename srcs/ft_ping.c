#include "ft_ping.h"

typedef struct		s_ping
{
	int fd;
	struct sockaddr_in *addr;
}					t_ping;

t_ping g_ping;

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
	static nbr_packet = 0;

	alarm(1);
	if (gettimeofday(&start, NULL))
	{
//		dprintf(stderr_fileno, "%s: gettimeofday: %s\n", argv[0], strerror(errno));
	}
	char buffer[64];
	bzero(buffer, sizeof(buffer));
	struct icmphdr *hdr;
	hdr = (struct icmphdr *)&buffer;
	hdr->type = ICMP_ECHO;
	hdr->un.echo.id = getpid();
	hdr->un.echo.sequence = ++nbr_packet;
	hdr->checksum = checksum(&buffer, sizeof(buffer));
	int sent = 0;
	if (sendto(sockfd, &buffer, sizeof(buffer), 0, (struct sockaddr *)serv_addr, sizeof(*serv_addr)) < 0)
	{
//		dprintf(stderr_fileno, "%s: sendto: %s\n", argv[0], strerror(errno));
	}
	else
		sent = 1;
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
	if (recvmsg(sockfd, &msghdr, 0) < 0)
	{
//		dprintf(stderr_fileno, "%s: recvmsg: %s\n", argv[0], strerror(errno));
	}
	if (gettimeofday(&end, NULL))
	{
//		dprintf(stderr_fileno, "%s: gettimeofday: %s\n", argv[0], strerror(errno));
	}
	if (sent)
	{
		double diff = ((end.tv_sec - start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec);
		if (diff < 1000)
			printf("icmp_seq=%d time=%.3f ms\n", , (float)((end.tv_sec - start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec) / 1000);
		else
			printf("icmp_seq=%d time=%.2f ms\n", , (float)((end.tv_sec - start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec) / 1000);
	}
	else
		printf("ko\n");
	return (0);
}

void			sig_handler(int sig_num)
{
	(void)sig_num;
	if (sig_num == SIGALRM)
		ping(g_ping.fd, g_ping.addr);
	else if (sig_num == SIGINT)
	{
		/* TODO: print stats */
		printf("hello\n");
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

	char *hostname = argv[1];
	/* Resolve hostname */
	char addr[256];
	ret = resolve_hostname(addr, hostname);
	if (ret)
	{
/*
		if (ret == EAI_AGAIN)
			dprintf(STDERR_FILENO, "%s: %s: Temporary failure in name resolution\n", argv[0], hostname);
		else*/
			dprintf(STDERR_FILENO, "%s: %s: Name or service not known\n", argv[0], hostname);
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
	/* TODO: gettimeofday */
	g_ping.fd = sockfd;
	g_ping.addr = &serv_addr;
	signal(SIGALRM, sig_handler); // Register signal handler
	signal(SIGINT, sig_handler); // Register signal handler
	ping(sockfd, &serv_addr);
	while (1)
		;
	return (0);
}
