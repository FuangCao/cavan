/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Sat Apr 20 14:01:25 CST 2013
 */

#include <cavan.h>
#include <cavan/thread.h>

int cavan_thread_send_event(struct cavan_thread *thread, u32 event)
{
	pr_bold_info("send event %d", event);

	return write(thread->pipefd[1], &event, sizeof(event));
}

int cavan_thread_recv_event(struct cavan_thread *thread, u32 *event)
{
	int ret;

	ret = read(thread->pipefd[0], event, sizeof(*event));
	if (ret < 0)
	{
		pr_red_info("read");
		return ret;
	}

	pr_bold_info("receive event %d", *event);

	return 0;
}

static int cavan_thread_wait_handler_dummy(struct cavan_thread *thread, u32 *event, void *data)
{
	return cavan_thread_recv_event(thread, event);
}

static int cavan_thread_wake_handler_dummy(struct cavan_thread *thread, u32 event, void *data)
{
	return cavan_thread_send_event(thread, event);
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

	ret = pipe(thread->pipefd);
	if (ret < 0)
	{
		pr_red_info("pipe");
		goto out_pthread_mutex_destroy;
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

out_pthread_mutex_destroy:
	pthread_mutex_destroy(&thread->lock);
	return ret;
}

void cavan_thread_deinit(struct cavan_thread *thread)
{
	pthread_mutex_destroy(&thread->lock);
}

static void cavan_thread_sighandler(int signum)
{
	pr_bold_info("signum = %d", signum);

	if (signum == SIGUSR1)
	{
		pthread_exit(0);
	}
}

static void *cavan_thread_main_loop(void *data)
{
	int ret;
	u32 event;
	struct cavan_thread *thread = data;

	signal(SIGUSR1, cavan_thread_sighandler);

	pthread_mutex_lock(&thread->lock);

	data = thread->private_data;
	thread->state = CAVAN_THREAD_STATE_RUNNING;

	while (1)
	{
		pthread_mutex_unlock(&thread->lock);
		ret = thread->wait_handler(thread, &event, data);
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
			ret = thread->handler(thread, event, data);
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
			thread->wake_handker(thread, 0, thread->private_data);
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

	thread->wake_handker(thread, 0, thread->private_data);
}
