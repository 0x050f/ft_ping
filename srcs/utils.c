#include "ft_ping.h"

double		get_diff_ms(struct timeval *start, struct timeval *end)
{
	return ((double)((end->tv_sec - start->tv_sec) * 1000000 + end->tv_usec - start->tv_usec) / 1000);
}

long		ft_sqrt(long long nb, long long x)
{
	for (int i = 0; i < 10; i++)
		x -= (x * x - nb) / (2 * x);
	return (x);
}

void		*ft_memset(void *b, int c, size_t len)
{
	unsigned char	*pt;

	pt = (unsigned char *)b;
	while (len--)
		*pt++ = (unsigned char)c;
	return (b);
}

void		ft_bzero(void *s, size_t n)
{
	char	*tmp;

	while (n--)
	{
		tmp = (char *)s;
		*tmp = 0;
		s++;
	}
}
