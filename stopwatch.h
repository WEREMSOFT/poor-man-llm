#ifndef __STOPWATCH_H__
#define __STOPWATCH_H__

#include <time.h>
#include <unistd.h>
#include <stdio.h>

clock_t time_start;

void stopwatch_start()
{
	time_start = clock();
}

void stopwatch_stop()
{
	double elapsed_time_in_secs = 0;
	clock_t time_end;

	time_end = clock();

	elapsed_time_in_secs = (double)(time_end - time_start) / CLOCKS_PER_SEC;

	printf("Elapsed time: %f secs.\n", elapsed_time_in_secs);
}

void stopwatch_restart()
{
	stopwatch_stop();
	stopwatch_start();
}

#endif