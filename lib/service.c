/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Mon Apr 16 10:03:27 CST 2012
 */

#include <cavan.h>
#include <cavan/service.h>
#include <cavan/process.h>
#include <cavan/permission.h>

static void cavan_service_sighandler(int signum)
{
	pr_bold_info("signum = %d", signum);

	if (signum == SIGUSR1)
	{
		pthread_exit(0);
	}
}

static void *cavan_service_handler(void *data)
{
	struct cavan_service_description *desc = data;
	static int count = 0;
	int index;

	signal(SIGUSR1, cavan_service_sighandler);

	pthread_mutex_lock(&desc->mutex_lock);
	index = ++count;
	pthread_mutex_unlock(&desc->mutex_lock);

	while (1)
	{
		int ret;

		pr_bold_info("%s daemon %d ready", desc->name, index);

		ret = desc->handler(desc, index, desc->data);
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

void cavan_service_set_busy(struct cavan_service_description *desc, int index, bool busy)
{
	pthread_mutex_lock(&desc->mutex_lock);

	if (busy)
	{
		desc->used_count++;
	}
	else
	{
		desc->used_count--;
	}

	pr_green_info("%s daemon %d %s [%d/%d]", desc->name, index, busy ? "busy" : "idle", desc->used_count, desc->daemon_count);

	pthread_mutex_unlock(&desc->mutex_lock);
}

int cavan_service_start(struct cavan_service_description *desc)
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

	count = desc->daemon_count;
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
		goto out_free_threads;
	}

	homepath = getenv("HOME");
	if (homepath)
	{
		ret = chdir(homepath);
	}

	umask(0);
	desc->used_count = 0;

	for (i = 0; i < count; i++)
	{
		ret = pthread_create(threads + i, NULL, cavan_service_handler, desc);
		if (ret < 0)
		{
			pr_red_info("pthread_create");

			while (i-- > 0)
			{
#if CONFIG_BUILD_FOR_ANDROID == 0
				pthread_cancel(threads[i]);
#endif
				free(threads);
				return ret;
			}
		}
	}

	desc->threads = threads;

	return 0;

out_free_threads:
	free(threads);
	return ret;
}

int cavan_service_main_loop(struct cavan_service_description *desc)
{
	int i;
	pthread_t *threads;

	for (i = desc->daemon_count - 1, threads = desc->threads; i >= 0; i--)
	{
		pthread_join(threads[i], NULL);
	}

	return 0;
}

int cavan_service_run(struct cavan_service_description *desc)
{
	int ret;

	ret = cavan_service_start(desc);
	if (ret < 0)
	{
		pr_red_info("cavan_service_start");
		return ret;
	}

	return cavan_service_main_loop(desc);
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
#if CONFIG_BUILD_FOR_ANDROID == 0
		pthread_cancel(threads[i]);
#endif
		pthread_kill(threads[i], SIGUSR1);
	}

	free(threads);
	desc->threads = NULL;

	return 0;
}

int cavan_daemon_run(struct cavan_daemon_description *desc)
{
	int ret;
	const char *shell_command = "sh";

	if (desc == NULL || desc->command == NULL)
	{
		pr_red_info("desc == NULL || desc->command == NULL");
		ERROR_RETURN(EINVAL);
	}

	pr_bold_info("command = %s", desc->command);

	if (desc->super_permission && (ret = check_super_permission(true, 5000)) < 0)
	{
		return ret;
	}

	if (desc->logfile)
	{
		int fd = open(desc->logfile, O_WRONLY | O_CREAT | O_TRUNC, 0777);
		if (fd < 0)
		{
			pr_error_info("open file %s failed", desc->logfile);
			return fd;
		}

		ret = dup2(fd, fileno(stdout));
		if (ret < 0)
		{
			pr_error_info("dup2 stdout");
		}
		else
		{
			ret = dup2(fd, fileno(stderr));
		}

		close(fd);

		if (ret < 0)
		{
			return ret;
		}
	}

	if (desc->as_daemon)
	{
		ret = daemon(1, desc->verbose || desc->logfile);
		if (ret < 0)
		{
			pr_error_info("daemon");
			return ret;
		}

		if (desc->pidfile)
		{
			file_printf(desc->pidfile, "%d", getpid());
		}
	}

	return execlp(shell_command, shell_command, "-c", desc->command, NULL);
}

int cavan_daemon_stop(struct cavan_daemon_description *desc)
{
	pid_t pid;

	if (desc == NULL)
	{
		pr_red_info("desc == NULL");
		return -EINVAL;
	}

	if (desc->pidfile)
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
	else if (desc->command)
	{
		pid = process_find_by_cmdline(NULL, desc->command);
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
