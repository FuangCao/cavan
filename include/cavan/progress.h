#pragma once

#define HALF_LENGTH		30
#define FREE_CHAR		'='
#define FULL_CHAR		'H'

#include <cavan.h>

#pragma pack(1)
struct progress_bar_body
{
	char head;
	char content1[HALF_LENGTH];
	char percent[6];
	char content2[HALF_LENGTH];
	char tail;
};
#pragma pack()

struct progress_bar
{
	u64 total;
	u64 current;
	int percent;
	int length;
	struct progress_bar_body body;
};

void progress_bar_update(struct progress_bar *bar);
void progress_bar_init(struct progress_bar *bar, u64 total);
void progress_bar_add(struct progress_bar *bar, u64 val);
void progress_bar_set(struct progress_bar *bar, u64 val);
void progress_bar_finish(struct progress_bar *bar);
