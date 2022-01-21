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

int	is_num(const char *str)
{
	int i;

	i = 0;
	while (str[i])
	{
		if (str[i] < '0' || str[i] > '9')
			return (0);
		i++;
	}
	return (1);
}

int	ft_atoi(const char *str)
{
	int i;
	int num;
	int sign;

	i = 0;
	num = 0;
	sign = 1;
	while (str[i] == '\t' || str[i] == '\v' || str[i] == '\n' ||
		str[i] == '\f' || str[i] == '\r' || str[i] == ' ')
		i++;
	if (str[i] == '-' || str[i] == '+')
		sign *= (str[i++] == '-') ? -1 : 1;
	while (str[i] >= '0' && str[i] <= '9')
		num = num * 10 + str[i++] - '0';
	return (num * sign);
}

int			ft_strcmp(const char *s1, const char *s2)
{
	int i;

	i = 0;
	while (s1[i] && s2[i] && s1[i] == s2[i])
		i++;
	return ((unsigned char)s1[i] - (unsigned char)s2[i]);
}

size_t		ft_strlen(const char *s)
{
	const char *ptr;

	ptr = s;
	while (*ptr)
		++ptr;
	return (ptr - s);
}
