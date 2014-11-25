#pragma once

#include <cavan.h>
#include <cavan/speed_detector.h>

#define HALF_LENGTH		30
#define FREE_CHAR		'='
#define FULL_CHAR		'H'

struct progress_bar
{
	struct speed_detector detector;

	double total;
	double current;

	int length;
	int progress;
	int percent;
	u32 speed;
};

void progress_bar_update(struct progress_bar *bar);
void progress_bar_init(struct progress_bar *bar, double total);
void progress_bar_add(struct progress_bar *bar, double val);
void progress_bar_set(struct progress_bar *bar, double val);
void progress_bar_finish(struct progress_bar *bar);
