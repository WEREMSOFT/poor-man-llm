#ifndef __STOPWATCH_H__
#define __STOPWATCH_H__

#include <time.h>
#include <unistd.h>
#include <stdio.h>

clock_t time_start;
time_t wall_clock_start;
char *stopwatch_label;
char *wall_clock_label;

void stopwatch_start(char *label)
{
	stopwatch_label = label;
	printf("[%s]::Start\n", label);
	time_start = clock();
}

void stopwatch_stop()
{
	double elapsed_time_in_secs = 0;
	clock_t time_end;

	time_end = clock();

	elapsed_time_in_secs = (double)(time_end - time_start) / CLOCKS_PER_SEC;
	printf("[%s]::Elapsed time: %f secs.\n", stopwatch_label, elapsed_time_in_secs);
}

void stopwatch_reset(char *label)
{
	stopwatch_stop();
	stopwatch_start(label);
}

void stopwatch_wall_clock_start(char* label)
{
	wall_clock_label = label;
	printf("[%s]::WC-Start\n", wall_clock_label);
	time(&wall_clock_start);
}

void stopwatch_wall_clock_stop()
{
	time_t wall_clock_end;
	double elapsed;

	time(&wall_clock_end);
	elapsed = difftime(wall_clock_end, wall_clock_start);
	printf("[%s]::WC-End\n", wall_clock_label);
	printf("Wall clock elapsed time in secconds %.00f\n", elapsed);
}
#endif