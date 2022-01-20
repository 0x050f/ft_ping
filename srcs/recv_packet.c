#include "ft_ping.h"

void	fill_msg_header(struct msghdr *msghdr, struct iovec *iov, t_icmp_packet *packet, char buffer[512])
{
	ft_bzero(msghdr, sizeof(struct msghdr));
	msghdr->msg_name = &g_ping.sockaddr;
	msghdr->msg_namelen = sizeof(struct sockaddr);
	iov->iov_base = packet;
	iov->iov_len = sizeof(t_icmp_packet);
	msghdr->msg_iov = iov;
	msghdr->msg_iovlen = 1;
	msghdr->msg_control = buffer;
	msghdr->msg_controllen = 512;
	msghdr->msg_flags = 0;
}

void	recv_packet(void)
{
	char			buffer[512];
	struct msghdr	msghdr;
	struct iovec	iov;
	t_icmp_packet	packet;

	while (true)
	{
		fill_msg_header(&msghdr, &iov, &packet, buffer);
		if (recvmsg(g_ping.sockfd, &msghdr, 0) < 0)
			dprintf(STDERR_FILENO, "%s: recvmsg: Error\n", g_ping.prg_name);
		if (packet.icmphdr.type == ICMP_ECHOREPLY)
		{
			++g_ping.stats.received;
			if (gettimeofday(&g_ping.stats.end, NULL)) // TODO: error gettimeofday
				dprintf(STDERR_FILENO, "%s: gettimeofday: Error\n", g_ping.prg_name);
			double diff = get_diff_ms(&packet.start, &g_ping.stats.end);
			if (diff < g_ping.stats.timer.min)
				g_ping.stats.timer.min = diff;
			if (diff > g_ping.stats.timer.max)
				g_ping.stats.timer.max = diff;
			g_ping.stats.timer.sum += diff;
			g_ping.stats.timer.tsum += diff * 1000;
			g_ping.stats.timer.tsum2 +=  diff * 1000 * diff * 1000;
			if (diff < 0.1)
				printf("icmp_seq=%ld time=%.3f ms\n", g_ping.stats.received, diff);
			else
				printf("icmp_seq=%ld time=%.2f ms\n", g_ping.stats.received, diff);
		}
		else if (packet.icmphdr.type != ICMP_ECHO)
		{
			printf("icmp->type == %d\n", packet.icmphdr.type);
			/* TODO: From XXXX Destination host unreachable */
			char *error = NULL;
			printf("icmp_seq=%hu %s\n", packet.icmphdr.un.echo.sequence, error);
			++g_ping.stats.errors;
		}
	}
}
