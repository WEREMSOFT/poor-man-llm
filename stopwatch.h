#ifndef __STOPWATCH_H__
#define __STOPWATCH_H__

#include <time.h>
#include <unistd.h>
#include <stdio.h>

clock_t time_start;
char *stopwatch_label;
void stopwatch_start(char *label)
{
	stopwatch_label = label;
	printf("[%s]::Start", label);
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


#endif