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
	iphdr->check = 0;
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
	ft_memset(packet.payload + sizeof(struct timeval), 42, PAYLOAD_SIZE);
	fill_icmp_header(&packet.icmphdr); // checksum after fill everything in payload
	if (sendto(g_ping.sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)&g_ping.sockaddr, sizeof(struct sockaddr)) < 0)
		dprintf(STDERR_FILENO, "%s: sendto: Error\n", g_ping.prg_name);
	alarm(SEND_DELAY);
}
