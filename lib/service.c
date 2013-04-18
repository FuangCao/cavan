/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Mon Apr 16 10:03:27 CST 2012
 */

#include <cavan.h>
#include <cavan/service.h>
#include <cavan/process.h>
#include <cavan/permission.h>

static int cavan_thread_wait_handler_dummy(struct cavan_thread *thread, void *data)
{
	return 0;
}

static int cavan_thread_wake_handler_dummy(struct cavan_thread *thread, void *data)
{
	return 0;
}

int cavan_thread_init(struct cavan_thread *thread, void *data)
{
	int ret;

	if (thread->handler == NULL)
	{
		pr_red_info("thread->handler == NULL");
		return -EINVAL;
	}

	ret = pthread_mutex_init(&thread->lock, NULL);
	if (ret < 0)
	{
		pr_red_info("pthread_mutex_init");
		return ret;
	}

	thread->state = CAVAN_THREAD_STATE_NONE;
	thread->private_data = data;

	if (thread->wait_handler == NULL)
	{
		thread->wait_handler = cavan_thread_wait_handler_dummy;
	}

	if (thread->wake_handker == NULL)
	{
		thread->wake_handker = cavan_thread_wake_handler_dummy;
	}

	return 0;
}

void cavan_thread_deinit(struct cavan_thread *thread)
{
	pthread_mutex_destroy(&thread->lock);
}

static void cavan_thread_sighandler(int signum)
{
	pthread_exit(0);
}

static void *cavan_thread_main_loop(void *data)
{
	int ret;
	struct cavan_thread *thread = data;

	signal(SIGUSR1, cavan_thread_sighandler);

	pthread_mutex_lock(&thread->lock);

	data = thread->private_data;
	thread->state = CAVAN_THREAD_STATE_RUNNING;

	while (1)
	{
		pthread_mutex_unlock(&thread->lock);
		ret = thread->wait_handler(thread, data);
		pthread_mutex_lock(&thread->lock);
		if (ret < 0)
		{
			pr_red_info("thread->wait_handler");
			goto out_thread_exit;
		}

		switch (thread->state)
		{
		case CAVAN_THREAD_STATE_RUNNING:
			pthread_mutex_unlock(&thread->lock);
			ret = thread->handler(thread, data);
			pthread_mutex_lock(&thread->lock);
			if (ret < 0)
			{
				pr_red_info("thread->handler");
				goto out_thread_exit;
			}
			break;

		case CAVAN_THREAD_STATE_STOPPPING:
			pr_bold_info("Thread %s stopping", thread->name);
			goto out_thread_exit;

		case CAVAN_THREAD_STATE_SUSPEND:
			pr_bold_info("Thread %s suspend", thread->name);
			break;

		default:
			pr_red_info("Thread %s invalid state %d", thread->name, thread->state);
		}
	}

out_thread_exit:
	thread->state = CAVAN_THREAD_STATE_STOPPED;
	pr_bold_info("Thread %s soppped", thread->name);

	pthread_mutex_unlock(&thread->lock);

	return NULL;
}

int cavan_thread_start(struct cavan_thread *thread)
{
	int ret;

	pthread_mutex_lock(&thread->lock);

	if (thread->state == CAVAN_THREAD_STATE_NONE)
	{
		ret = pthread_create(&thread->id, NULL, cavan_thread_main_loop, thread);
		if (ret < 0)
		{
			pr_red_info("pthread_create");
		}
		else
		{
			thread->state = CAVAN_THREAD_STATE_IDEL;
		}
	}
	else
	{
		ret = 0;
	}

	pthread_mutex_unlock(&thread->lock);

	return ret;
}

void cavan_thread_stop(struct cavan_thread *thread)
{
	pthread_mutex_lock(&thread->lock);

	if (thread->state == CAVAN_THREAD_STATE_RUNNING || thread->state == CAVAN_THREAD_STATE_SUSPEND)
	{
		int i;

		for (i = 0; i < 10; i++)
		{
			thread->state = CAVAN_THREAD_STATE_STOPPPING;

			pthread_mutex_unlock(&thread->lock);
			thread->wake_handker(thread, thread->private_data);
			msleep(1);
			pthread_mutex_lock(&thread->lock);

			if (thread->state == CAVAN_THREAD_STATE_STOPPED)
			{
				break;
			}
		}
	}

	if (thread->state != CAVAN_THREAD_STATE_NONE && thread->state != CAVAN_THREAD_STATE_STOPPED)
	{
		pthread_kill(thread->id, SIGUSR1);
	}

	pthread_mutex_unlock(&thread->lock);
}

void cavan_thread_suspend(struct cavan_thread *thread)
{
	pthread_mutex_lock(&thread->lock);

	if (thread->state == CAVAN_THREAD_STATE_RUNNING)
	{
		thread->state = CAVAN_THREAD_STATE_SUSPEND;
	}

	pthread_mutex_unlock(&thread->lock);
}

void cavan_thread_resume(struct cavan_thread *thread)
{
	pthread_mutex_lock(&thread->lock);

	if (thread->state == CAVAN_THREAD_STATE_SUSPEND)
	{
		thread->state = CAVAN_THREAD_STATE_RUNNING;
	}

	pthread_mutex_unlock(&thread->lock);

	thread->wake_handker(thread, thread->private_data);
}

// ================================================================================

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

	if (desc->as_daemon)
	{
		ret = daemon(1, desc->verbose);
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
