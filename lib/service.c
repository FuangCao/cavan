/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Mon Apr 16 10:03:27 CST 2012
 */

#include <cavan.h>
#include <cavan/service.h>
#include <cavan/process.h>
#include <cavan/permission.h>

#define CAVAN_SERVICE_DEBUG		0

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

int cavan_daemon_permission_clear(u32 permission)
{
	int ret;
	struct __user_cap_data_struct data;
	struct __user_cap_header_struct header =
	{
		.pid = 0,
		.version = _LINUX_CAPABILITY_VERSION,
	};

	ret = capget(&header, &data);
	if (ret < 0)
	{
		pr_error_info("capget");
		return ret;
	}

	data.permitted &= ~permission;
	data.effective = data.permitted;

	return capset(&header, &data);
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

	if (desc->super_permission)
	{
		ret = check_super_permission(true, 5000);
		if (ret < 0)
		{
			return ret;
		}
	}
	else
	{
		ret = setuid(getuid());
		ret = setgid(getgid());
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

// ================================================================================

int cavan_dynamic_service_init(struct cavan_dynamic_service *service)
{
	int ret;

	ret = pthread_mutex_init(&service->lock, NULL);
	if (ret < 0)
	{
		pr_error_info("pthread_mutex_init");
		return ret;
	}

	return 0;
}

void cavan_dynamic_service_deinit(struct cavan_dynamic_service *service)
{
	pthread_mutex_destroy(&service->lock);
}

struct cavan_dynamic_service *cavan_dynamic_service_create(size_t size)
{
	struct cavan_dynamic_service *service;

	service = malloc(sizeof(*service) + size);
	if (service == NULL)
	{
		pr_error_info("malloc");
		return NULL;
	}

	if (cavan_dynamic_service_init(service) < 0)
	{
		pr_red_info("cavan_dynamic_service_init");
		goto out_free_service;
	}

	service->private_data = service + 1;

	return service;

out_free_service:
	free(service);
	return NULL;
}

void cavan_dynamic_service_destroy(struct cavan_dynamic_service *service)
{
	cavan_dynamic_service_deinit(service);
	free(service);
}

static void *cavan_dynamic_service_handler(void *data)
{
	int ret;
	int index;
	void *conn;
	pthread_t thread;
	struct cavan_dynamic_service *service = data;

	pthread_mutex_lock(&service->lock);
	index = ++service->count;

	while (service->state == CAVAN_SERVICE_STATE_RUNNING)
	{
		pr_bold_info("service %s daemon %d ready (%d/%d)", service->name, index, service->used, service->count);

		pthread_mutex_unlock(&service->lock);
		conn = service->open_connect(service);
		pthread_mutex_lock(&service->lock);

		if (conn == NULL)
		{
			pr_red_info("conn == NULL");
			continue;
		}

		service->used++;

		if (service->used < service->count)
		{
			pr_green_info("don't need create daemon");
		}
		else
		{
			if (service->count < service->max)
			{
				int ret;

				ret = pthread_create(&thread, NULL, cavan_dynamic_service_handler, service);
				if (ret < 0)
				{
					pr_red_info("create daemon faild");
				}
				else
				{
					pr_green_info("create daemon successfully");
				}
			}
			else
			{
				pr_red_info("too match deamon count = %d", service->count);
			}
		}

		pr_bold_info("service %s daemon %d busy (%d/%d)", service->name, index, service->used, service->count);

		pthread_mutex_unlock(&service->lock);
		ret = service->run(service, conn);
		service->close_connect(service, conn);
		if (ret < 0)
		{
			pr_red_info("service %s daemon %d fault", service->name, index);
		}
		else
		{
			pr_green_info("service %s daemon %d complete", service->name, index);
		}

		pthread_mutex_lock(&service->lock);
		service->used--;

		if (index > service->min && service->count - service->used > 1)
		{
			break;
		}
	}

	service->count--;
	pr_green_info("service %s daemon %d exit (%d/%d)", service->name, index, service->used, service->count);

	pthread_mutex_unlock(&service->lock);

	return NULL;
}

int cavan_dynamic_service_run(struct cavan_dynamic_service *service)
{
	int ret;

	if (service == NULL)
	{
		pr_red_info("service == NULL");
		return -EINVAL;
	}

	if (service->name == NULL)
	{
		pr_red_info("service->name == NULL");
		return -EINVAL;
	}

	if (service->open_connect == NULL)
	{
		pr_red_info("service->open_connect == NULL");
		return -EINVAL;
	}

	if (service->close_connect == NULL)
	{
		pr_red_info("service->close_connect == NULL");
		return -EINVAL;
	}

	if (service->start == NULL)
	{
		pr_red_info("service->start == NULL");
		return -EINVAL;
	}

	if (service->stop == NULL)
	{
		pr_red_info("service->stop == NULL");
		return -EINVAL;
	}

	if (service->run == NULL)
	{
		pr_red_info("service->run == NULL");
		return -EINVAL;
	}

	pr_bold_info("daemon min = %d, max = %d", service->min, service->max);

	if (service->min <= 0 || service->max < service->min)
	{
		pr_red_info("invalid min or max");
		return -EINVAL;
	}

	if (service->super_permission && (ret = check_super_permission(true, 5000)) < 0)
	{
		return ret;
	}

	if (service->as_daemon)
	{
		pr_blue_info("Run %s as daemon", service->name);

		ret = daemon(0, service->show_verbose);
		if (ret < 0)
		{
			pr_red_info("daemon");
			return ret;
		}
	}

	service->count = 0;
	service->used = 0;
	service->state = CAVAN_SERVICE_STATE_RUNNING;

	ret = service->start(service);
	if (ret < 0)
	{
		pr_red_info("service->start");
		return ret;
	}

	cavan_dynamic_service_handler(service);

	pthread_mutex_lock(&service->lock);

	if (service->state == CAVAN_SERVICE_STATE_RUNNING)
	{
		service->stop(service);
	}

	service->state = CAVAN_SERVICE_STATE_STOPPED;

	pthread_mutex_unlock(&service->lock);

	pr_bold_info("service %s stopped", service->name);

	return 0;
}

int cavan_dynamic_service_stop(struct cavan_dynamic_service *service)
{
	pthread_mutex_lock(&service->lock);

	if (service->state == CAVAN_SERVICE_STATE_RUNNING)
	{
		int i;

		service->state = CAVAN_SERVICE_STATE_STOPPING;
		service->stop(service);

		for (i = 0; i < 100; i++)
		{
			pthread_mutex_unlock(&service->lock);
			msleep(100);
			pthread_mutex_lock(&service->lock);

			if (service->state == CAVAN_SERVICE_STATE_STOPPED)
			{
				break;
			}

			pr_red_info("wait service stop %d", i);
		}
	}

	pthread_mutex_unlock(&service->lock);

	return 0;
}
