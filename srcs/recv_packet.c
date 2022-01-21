#include "ft_ping.h"

void	fill_msg_header(struct msghdr *msghdr, struct iovec *iov, char buffer[MSG_CONTROL_SIZE], struct sockaddr sockaddr)
{
	ft_bzero(msghdr, sizeof(struct msghdr));
	msghdr->msg_name = &sockaddr;
	msghdr->msg_namelen = sizeof(struct sockaddr);
	msghdr->msg_iov = iov;
	msghdr->msg_iovlen = 1;
	msghdr->msg_control = buffer;
	msghdr->msg_controllen = MSG_CONTROL_SIZE;
	msghdr->msg_flags = 0;
}

void	update_stats(double diff)
{
	if (diff < g_ping.stats.timer.min)
		g_ping.stats.timer.min = diff;
	if (diff > g_ping.stats.timer.max)
		g_ping.stats.timer.max = diff;
	g_ping.stats.timer.sum += diff;
	g_ping.stats.timer.tsum += diff * 1000;
	g_ping.stats.timer.tsum2 +=  diff * 1000 * diff * 1000;
}

const char	*dest_unreach[] =
{
	[ICMP_NET_UNREACH]		= "Destination Network Unreachable",
	[ICMP_HOST_UNREACH]		= "Destination Host Unreachable",
	[ICMP_PROT_UNREACH]		= "Destination Protocol Unreachable",
	[ICMP_PORT_UNREACH]		= "Destination Port Unreachable"
};

void	print_recv_packet(t_icmp_packet *packet)
{
	if (packet->icmphdr.type == ICMP_ECHOREPLY)
	{
		++g_ping.stats.received;
		if (gettimeofday(&g_ping.stats.end, NULL)) // TODO: error gettimeofday
			dprintf(STDERR_FILENO, "%s: gettimeofday: Error\n", g_ping.prg_name);
		double diff = get_diff_ms((void *)&packet->payload, &g_ping.stats.end);
		update_stats(diff);
		printf("%ld bytes ", PAYLOAD_SIZE + sizeof(struct icmphdr));
		if (ft_strcmp(g_ping.hostname, g_ping.address))
			printf("from %s (%s): ", g_ping.hostname, g_ping.address);
		else
			printf("from %s: ", g_ping.address);
		printf("icmp_seq=%d ttl=%d ", packet->icmphdr.un.echo.sequence, packet->iphdr.ttl);
		if (diff < 0.1)
			printf("time=%.3f ms\n", diff);
		else
			printf("time=%.2f ms\n", diff);
	}
	else if (packet->icmphdr.type != ICMP_ECHO)
	{
		
		t_icmp_packet *sent_packet = (t_icmp_packet *)((unsigned long)packet + sizeof(struct iphdr) + sizeof(struct icmphdr));
		char *error;
		/* TODO: From XXXX Destination host unreachable */
		if (packet->icmphdr.type == ICMP_DEST_UNREACH)
		{
			if (packet->icmphdr.code < sizeof(dest_unreach))
				error = (char *)dest_unreach[packet->icmphdr.code];
			else
				error = "Destination Unreachable";
		}
		else
			error = "Error";
		printf("icmp_seq=%d %s\n", sent_packet->icmphdr.un.echo.sequence, error);
		++g_ping.stats.errors;
	}
}

void	recv_packet(void)
{
	char			buffer[MSG_CONTROL_SIZE];
	struct msghdr	msghdr;
	struct iovec	iov;
	t_icmp_packet	packet;

	while (true)
	{
		iov.iov_base = &packet;
		iov.iov_len = sizeof(t_icmp_packet);
		fill_msg_header(&msghdr, &iov, buffer, *((struct sockaddr *)(&g_ping.sockaddr)));
		if (recvmsg(g_ping.sockfd, &msghdr, 0) < 0)
			dprintf(STDERR_FILENO, "%s: recvmsg: Error\n", g_ping.prg_name);
		print_recv_packet(&packet);
	}
}
