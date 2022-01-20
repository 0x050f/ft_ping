#include "ft_ping.h"

/* RFC 1071 https://datatracker.ietf.org/doc/html/rfc1071 */
/*
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
*/
uint16_t checksum(uint16_t *addr, int len)
{
  int nleft = len;
  uint32_t sum = 0;
  uint16_t *w = addr;
  uint16_t answer = 0;

  // Adding 16 bits sequentially in sum
  while (nleft > 1) {
    sum += *w;
    nleft -= 2;
    w++;
  }

  // If an odd byte is left
  if (nleft == 1) {
    *(unsigned char *) (&answer) = *(unsigned char *) w;
    sum += answer;
  }

  sum = (sum >> 16) + (sum & 0xffff);
  sum += (sum >> 16);
  answer = ~sum;

  return answer;
}

void	fill_ip_header(struct iphdr *iphdr)
{
	iphdr->version = 4; // ipv4
	iphdr->ihl = 5; //sizeof(struct iphdr) / 4; // 5 = 20 / 32 bits
	iphdr->tos = 0;
	iphdr->tot_len = htons(sizeof(t_icmp_packet));
	iphdr->id = htons(321);
	iphdr->frag_off = htons(0);
	iphdr->ttl = g_ping.ttl_val;
	iphdr->protocol = IPPROTO_ICMP;
	iphdr->check = 0;
	iphdr->saddr = INADDR_ANY;
	iphdr->daddr = g_ping.sockaddr.sin_addr.s_addr;
}

void	fill_icmp_header(struct icmphdr *icmphdr)
{
	icmphdr->type = ICMP_ECHO;
	icmphdr->code = 0;
	icmphdr->un.echo.id = htons(getpid());
	icmphdr->un.echo.sequence = htons(++g_ping.stats.transmitted);
	icmphdr->checksum = 0;
	icmphdr->checksum = checksum((unsigned short *)icmphdr, sizeof(t_icmp_packet) - sizeof(struct iphdr));
}

void	send_packet(int signum)
{
	t_icmp_packet	packet;

	(void)signum;
	ft_bzero(&packet, sizeof(packet));
	fill_ip_header(&packet.iphdr);
	if (gettimeofday(&packet.start, NULL))// TODO: error gettimeofday
		dprintf(STDERR_FILENO, "%s: gettimeofday: Error\n", g_ping.prg_name);
	ft_memset(packet.payload, 42, 4);
	ft_memset(packet.data, 42, DATA_SIZE);
	fill_icmp_header(&packet.icmphdr); // checksum after fill everything in payload
	if (sendto(g_ping.sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)&g_ping.sockaddr, sizeof(struct sockaddr)) < 0) // TODO: error sendto
		dprintf(STDERR_FILENO, "%s: sendto: Error\n", g_ping.prg_name);
	alarm(SEND_DELAY);
}
