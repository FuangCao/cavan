/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Mon Apr 16 10:03:27 CST 2012
 */

#include <cavan.h>
#include <cavan/service.h>

static void *cavan_service_handler(void *data)
{
	struct cavan_service_description *desc = data;
	static int count = 0;
	int index;

	pthread_mutex_lock(&desc->mutex_lock);
	index = ++count;
	pthread_mutex_unlock(&desc->mutex_lock);

	while (1)
	{
		int ret;

		pr_bold_info("%s daemon %d ready", desc->name, index);
		ret = desc->handler(index, desc->data);
		if (ret < 0)
		{
			pr_red_info("%s daemon %d fault", desc->name, index);
		}
		else
		{
			pr_green_info("%s daemon %d complete", desc->name, index);
		}
	}

	return NULL;
}

int cavan_service_run(struct cavan_service_description *desc)
{
	int ret;
	int i;
	int count;
	pthread_t *threads;

	if (desc == NULL || desc->handler == NULL || desc->name == NULL || desc->daemon_count < 1)
	{
		pr_red_info("desc == NULL || desc->handler == NULL || desc->name == NULL || desc->daemon_count < 1");
		return -EINVAL;
	}

	count = desc->daemon_count - 1;
	threads = (pthread_t *)malloc(sizeof(pthread_t) * count);
	if (threads == NULL)
	{
		pr_red_info("malloc");
		return -ENOMEM;
	}

	ret = pthread_mutex_init(&desc->mutex_lock, NULL);
	if (ret < 0)
	{
		pr_red_info("pthread_mutex_init");
		return ret;
	}

	for (i = 0; i < count; i++)
	{
		ret = pthread_create(threads + i, NULL, cavan_service_handler, desc);
		if (ret < 0)
		{
			pr_red_info("pthread_create");
			while (i-- > 0)
			{
				pthread_cancel(threads[i]);
				goto out_free_threads;
			}
		}
	}

	desc->threads = threads;

	if (desc->as_daemon)
	{
		pr_blue_info("Run %s as daemon", desc->name);
		ret = daemon(0, desc->show_verbose);
		if (ret < 0)
		{
			print_error("daemon");
			goto out_free_threads;
		}
	}

	cavan_service_handler(desc);

	for (i = 0; i < count; i++)
	{
		pthread_join(threads[i], NULL);
	}

out_free_threads:
	free(threads);

	return ret;
}

int cavan_service_stop(struct cavan_service_description *desc)
{
	int i;
	pthread_t *threads;

	if (desc == NULL)
	{
		pr_red_info("desc == NULL");
		return -EINVAL;
	}

	threads = desc->threads;

	for (i = desc->daemon_count - 1; i >= 0; i--)
	{
		pthread_cancel(threads[i]);
	}

	free(threads);
	desc->threads = NULL;

	return 0;
}
