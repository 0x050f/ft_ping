#include "ft_ping.h"

void	ping_stats(int signum)
{
	t_stats	*stats;
	double	percentage;
	double	tmdev;
	long	time;

	(void)signum;
	stats = &g_ping.stats;
	printf("\n--- %s ping statistics ---\n", g_ping.hostname);
	printf("%d packets transmitted, %d received, ", stats->transmitted, stats->received);
	if (stats->errors)
		printf("+%d errors, ", stats->errors);
	percentage = 0;
	if (stats->received)
		percentage = (1.0 - (double)stats->received / (double)stats->transmitted) * 100.0;
	time = 0;
	if (stats->end.tv_sec && stats->end.tv_usec && stats->transmitted > 1)
		time = get_diff_ms(&stats->start, &stats->end);
	printf("%.4g%% packet loss, time %ld ms\n", percentage, time);
	if (stats->received)
	{
		stats->timer.tsum /= stats->received;
		stats->timer.tsum2 /= stats->received;
		tmdev = 0;
		if (stats->transmitted != 1)
			tmdev = (long double)ft_sqrt(stats->timer.tsum2 - stats->timer.tsum * stats->timer.tsum, stats->timer.max * 1000 - stats->timer.min * 1000) / 1000.0;
		printf("rtt min/avg/max/mdev = %.3f/%.3f/%.3f/%.3f ms", stats->timer.min, stats->timer.sum / stats->received, stats->timer.max, tmdev);
		printf("\n");
	}
	exit(0);
}
