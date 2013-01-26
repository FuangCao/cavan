/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Mon Apr 16 10:03:27 CST 2012
 */

#include <cavan.h>
#include <cavan/service.h>
#include <cavan/permission.h>
#include <cavan/process.h>

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
	const char *homepath;

	if (desc == NULL)
	{
		pr_red_info("desc == NULL");
		ERROR_RETURN(EINVAL);
	}

	if (desc->handler == NULL || desc->name == NULL || desc->daemon_count < 1)
	{
		pr_red_info("desc->handler == NULL || desc->name == NULL || desc->daemon_count < 1");
		ERROR_RETURN(EINVAL);
	}

	if (desc->super_permission && (ret = check_super_permission(true, 5000)) < 0)
	{
		return ret;
	}

	if (desc->as_daemon)
	{
		pr_blue_info("Run %s as daemon", desc->name);
		ret = daemon(0, desc->show_verbose);
		if (ret < 0)
		{
			print_error("daemon");
			return ret;
		}
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

	homepath = getenv("HOME");
	if (homepath)
	{
		ret = chdir(homepath);
	}

	for (i = 0; i < count; i++)
	{
		ret = pthread_create(threads + i, NULL, cavan_service_handler, desc);
		if (ret < 0)
		{
			pr_red_info("pthread_create");
			while (i-- > 0)
			{
#ifndef CONFIG_BUILD_FOR_ANDROID
				pthread_cancel(threads[i]);
#endif
				goto out_free_threads;
			}
		}
	}

	desc->threads = threads;

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
	if (threads == NULL)
	{
		return 0;
	}

	for (i = desc->daemon_count - 1; i >= 0; i--)
	{
#ifndef CONFIG_BUILD_FOR_ANDROID
		pthread_cancel(threads[i]);
#endif
	}

	free(threads);
	desc->threads = NULL;

	return 0;
}

int cavan_daemon_run(struct cavan_daemon_description *desc)
{
	int ret;
	pid_t pid;

	if (desc == NULL || desc->cmdfile[0] == 0)
	{
		pr_red_info("Daemon description fault");
		ERROR_RETURN(EINVAL);
	}

	if (desc->super_permission && (ret = check_super_permission(false, 5000)) < 0)
	{
		return ret;
	}

	if (desc->as_daemon)
	{
		pid = fork();
		if (pid < 0)
		{
			pr_red_info("fork failed");
			return pid;
		}

		if (pid)
		{
			if (desc->pidfile[0])
			{
				file_printf(desc->pidfile, "%d", pid);
			}

			return 0;
		}
	}

	return execv(desc->cmdfile, desc->argv);
}

int cavan_daemon_stop(struct cavan_daemon_description *desc)
{
	pid_t pid;

	if (desc == NULL)
	{
		pr_red_info("Daemon description fault");
		return -EINVAL;
	}

	if (desc->pidfile[0])
	{
		char buff[1024];
		ssize_t readlen;

		readlen = file_read(desc->pidfile, buff, sizeof(buff));
		if (readlen < 0)
		{
			pr_red_info("Read file %s failed", desc->pidfile);
			return readlen;
		}

		remove(desc->pidfile);

		pid = text2value_unsigned(buff, NULL, 10);
	}
	else if (desc->cmdfile[0])
	{
		pid = process_find_by_cmdline(NULL, desc->cmdfile);
		if (pid < 0)
		{
			pr_red_info("process_find_by_cmdline failed");
			return pid;
		}
	}
	else
	{
		pr_red_info("Please give command pathname or pid filename");
		return -EINVAL;
	}

	return kill(pid, SIGTERM);
}
