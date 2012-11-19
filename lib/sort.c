#include <cavan.h>
#include <cavan/sort.h>

// Fuang.Cao <cavan.cfa@gmail.com> 2011-12-05 14:38:03

static void cavan_quick_sort_body(int *start, int *end)
{
	int middle;
	int *start_bak, *end_bak;

	middle = *start;
	start_bak = start;
	end_bak = end;

	while (1)
	{
		for (; *end > middle && start < end; end--);
		if (start < end)
		{
			*start++ = *end;
		}
		else
		{
			break;
		}

		for (; *start < middle && start < end; start++);
		if (start < end)
		{
			*end-- = *start;
		}
		else
		{
			break;
		}
	}

	*start = middle;

	if (start - start_bak > 1)
	{
		cavan_quick_sort_body(start_bak, start - 1);
	}

	if (end_bak - end > 1)
	{
		cavan_quick_sort_body(end + 1, end_bak);
	}
}

void cavan_quick_sort(int *a, size_t size)
{
	if (size > 1)
	{
		cavan_quick_sort_body(a, a + size - 1);
	}
}

void cavan_select_sort(int *a, size_t size)
{
	int tmp;
	int *p, *q, *max;

	if (size < 2)
	{
		return;
	}

	for (p = a + size - 1; p > a; p--)
	{
		for (max = p, q = a; q < p; q++)
		{
			if (*q > *max)
			{
				max = q;
			}
		}

		if (p == max)
		{
			continue;
		}

		tmp = *max;
		*max = *p;
		*p = tmp;
	}
}

void cavan_insert_sort(int *a, size_t size)
{
	int tmp;
	int *p, *q, *end;

	if (size < 2)
	{
		return;
	}

	for (p = a + 1, end = a + size; p < end; p++)
	{
		tmp = *p;

		for (q = p - 1; *q > tmp && q >= a; q--)
		{
			*(q + 1) = *q;
		}

		*(q + 1) = tmp;
	}
}

static void cavan_shell_insert(int *a, int *end, int step)
{
	int tmp;
	int *p, *q;

	for (p = a + step; p < end; p += step)
	{
		tmp = *p;

		for (q = p - step; *q > tmp && q >= a; q -= step)
		{
			*(q + step) = *q;
		}

		*(q + step) = tmp;
	}
}

void cavan_shell_short(int *a, size_t size, int *steps, size_t step_size)
{
	int *end;
	size_t i;
	size_t step;

	if (size < 2)
	{
		return;
	}

	end = a + size;

	if (steps == NULL || step_size == 0)
	{
		for (step = size / 2; step > 0; step--)
		{
			for (i = 0; i < step; i++)
			{
				cavan_shell_insert(a + i, end, step);
			}
		}
	}
	else
	{
		size_t j;

		for (i = 0; i < step_size; i++)
		{
			step = steps[i];

			for (j = 0; j < step; j++)
			{
				cavan_shell_insert(a + j, end, step);
			}
		}
	}
}

static void cavan_build_heap(int *a, size_t size)
{
	int *root, *p;
	int i;
	int tmp;

	for (i = size - 1; i > 0; i--)
	{
		root = a + ((i - 1) >> 1);
		p = a + i;

		if (*root < *p)
		{
			tmp = *p;
			*p = *root;
			*root = tmp;
		}
	}
}

void cavan_heap_sort(int *a, size_t size)
{
	int i, j, k;
	int tmp;

	if (size < 2)
	{
		return;
	}

	cavan_build_heap(a, size);

	for (i = size - 1; i > 0; i--)
	{
		tmp = a[0];
		j = 0;

		while (1)
		{
			k = (j << 1) + 1;

			if (k >= i)
			{
				break;
			}

			if (k + 1 < i && a[k] < a[k + 1])
			{
				a[j] = a[k + 1];
				j = k + 1;
			}
			else
			{
				a[j] = a[k];
				j = k;
			}
		}

		a[j] = a[i];
		a[i] = tmp;
	}
}
