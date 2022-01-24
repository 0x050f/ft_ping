#include "ft_ping.h"

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

void	fill_ip_header(struct iphdr *iphdr)
{
	iphdr->version = 4; // ipv4
	iphdr->ihl = sizeof(struct iphdr) / 4; // 5 = 20 / 32 bits
	iphdr->tos = 0;
	iphdr->tot_len = sizeof(t_icmp_packet);
	iphdr->id = 0;
	iphdr->frag_off = 0;
	iphdr->ttl = g_ping.ttl_val;
	iphdr->protocol = IPPROTO_ICMP;
	iphdr->check = 0; // filled by kernel
	iphdr->saddr = INADDR_ANY;
	iphdr->daddr = g_ping.ip_addr;
}

void	fill_icmp_header(struct icmphdr *icmphdr)
{
	icmphdr->type = ICMP_ECHO;
	icmphdr->code = 0;
	icmphdr->un.echo.id = getpid();
	icmphdr->un.echo.sequence = ++g_ping.stats.transmitted;
	icmphdr->checksum = checksum((unsigned short *)icmphdr, sizeof(t_icmp_packet) - sizeof(struct iphdr));
}

void	send_packet(int signum)
{
	t_icmp_packet	packet;

	(void)signum;
	ft_bzero(&packet, sizeof(packet));
	fill_ip_header(&packet.iphdr);
	if (gettimeofday((void *)&packet.payload, NULL))
		dprintf(STDERR_FILENO, "%s: gettimeofday: Error\n", g_ping.prg_name);
	if (g_ping.options.w && (((struct timeval *)&packet.payload)->tv_sec - g_ping.stats.start.tv_sec) * 1000 + (((struct timeval *)&packet.payload)->tv_usec - g_ping.stats.start.tv_usec) / 1000 >= g_ping.time_max * 1000)
		ping_stats(0);
	ft_memset(packet.payload + sizeof(struct timeval), 42, PAYLOAD_SIZE - sizeof(struct timeval));
	fill_icmp_header(&packet.icmphdr); // checksum after fill everything in payload
	if (sendto(g_ping.sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)&g_ping.sockaddr, sizeof(struct sockaddr)) < 0)
		dprintf(STDERR_FILENO, "%s: sendto: Error\n", g_ping.prg_name);
	if (!g_ping.options.f)
		alarm(SEND_DELAY);
	else
	{
		write(STDOUT_FILENO, ".", 1); // printf might bug if too fast
		struct itimerval it_val;

		it_val.it_value.tv_sec = 0;
		if (g_ping.stats.timer.sum)
			it_val.it_value.tv_usec = (g_ping.stats.timer.sum / g_ping.stats.received) * 1000;
		else
			it_val.it_value.tv_usec = 50;
		it_val.it_interval = it_val.it_value;
		if (setitimer(ITIMER_REAL, &it_val, NULL) < 0)
			dprintf(STDERR_FILENO, "%s: setittimer: Error\n", g_ping.prg_name);
	}
}
