/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Sat Apr 20 14:01:25 CST 2013
 */

#include <cavan.h>
#include <cavan/timer.h>
#include <cavan/thread.h>

#define THREAD_EPOLL_SIZE	8

#ifndef CAVAN_THREAD_DEBUG
#define CAVAN_THREAD_DEBUG	0
#endif

int cavan_thread_send_event(struct cavan_thread *thread, u32 event)
{
#if CAVAN_THREAD_DEBUG
	pr_bold_info("send event %d", event);
#endif

	return write(thread->wr_event_fd, &event, sizeof(event));
}

int cavan_thread_recv_event(struct cavan_thread *thread, u32 *event)
{
	int ret;

	ret = read(thread->rd_event_fd, event, sizeof(*event));
	if (ret < 0)
	{
		pr_red_info("read");
		return ret;
	}

#if CAVAN_THREAD_DEBUG
	pr_bold_info("receive event %d", *event);
#endif

	return ret;
}

int cavan_thread_recv_event_timeout(struct cavan_thread *thread, u32 *event, u32 msec)
{
	int ret;

	ret = cavan_thread_epoll_wait_event(thread, msec);
	if (ret < 0)
	{
		pr_error_info("poll");
		return ret;
	}

	if (ret < 1)
	{
		return 0;
	}

	return cavan_thread_recv_event(thread, event);
}

int cavan_thread_wait_event(struct cavan_thread *thread, u32 msec)
{
	int ret;

	pthread_mutex_lock(&thread->lock);

	if (thread->pending)
	{
		pthread_mutex_unlock(&thread->lock);
		return 0;
	}

	thread->state = CAVAN_THREAD_STATE_WAIT_EVENT;

	pthread_mutex_unlock(&thread->lock);
	ret = cavan_thread_epoll_wait_event(thread, msec);
	pthread_mutex_lock(&thread->lock);

	if (thread->state == CAVAN_THREAD_STATE_WAIT_EVENT)
	{
		thread->state = CAVAN_THREAD_STATE_RUNNING;
	}

	pthread_mutex_unlock(&thread->lock);

	return ret;
}

int cavan_thread_msleep_until(struct cavan_thread *thread, struct timespec *time)
{
	int ret;

	pthread_mutex_lock(&thread->lock);

	if (thread->pending)
	{
		thread->pending = false;
		pthread_mutex_unlock(&thread->lock);
		return 0;
	}

	thread->state = CAVAN_THREAD_STATE_SLEEP;

	ret = pthread_cond_timedwait(&thread->cond, &thread->lock, time);
	if (thread->state == CAVAN_THREAD_STATE_SLEEP)
	{
		thread->state = CAVAN_THREAD_STATE_RUNNING;
	}

	pthread_mutex_unlock(&thread->lock);

	return ret;
}

int cavan_thread_msleep(struct cavan_thread *thread, u32 msec)
{
	struct timespec time;

	cavan_timer_set_timespec(&time, msec);

	return cavan_thread_msleep_until(thread, &time);
}

int cavan_thread_epoll_add(struct cavan_thread *thread, int fd, u32 events)
{
	int ret;
    struct epoll_event event =
    {
		.events = events,
		.data.fd = fd
    };

	ret = epoll_ctl(thread->epoll_fd, EPOLL_CTL_ADD, fd, &event);
	if (ret < 0)
	{
		pr_err_info("epoll_ctl EPOLL_CTL_ADD");
		return ret;
	}

	return 0;
}

int cavan_thread_epoll_remove(struct cavan_thread *thread, int fd)
{
	int ret;

	ret = epoll_ctl(thread->epoll_fd, EPOLL_CTL_DEL, fd, NULL);
	if (ret < 0)
	{
		pr_err_info("epoll_ctl EPOLL_CTL_DEL");
		return ret;
	}

	return 0;
}

int cavan_thread_epoll_modify(struct cavan_thread *thread, int fd, u32 events)
{
	int ret;
    struct epoll_event event =
    {
		.events = events,
		.data.fd = fd
    };

	ret = epoll_ctl(thread->epoll_fd, EPOLL_CTL_MOD, fd, &event);
	if (ret < 0)
	{
		pr_err_info("epoll_ctl EPOLL_CTL_MOD");
		return ret;
	}

	return 0;
}

int cavan_thread_epoll_wait(struct cavan_thread *thread, struct epoll_event *events, int count, int timeout)
{
	int ret;

	ret = epoll_wait(thread->epoll_fd, events, count, timeout);
	if (ret < 0)
	{
		pr_err_info("epoll_wait");
		return ret;
	}

	return 0;
}

int cavan_thread_epoll_wait_event(struct cavan_thread *thread, int timeout)
{
	while (1)
	{
		int ret;
		struct epoll_event *p, *p_end, events[10];

		ret = cavan_thread_epoll_wait(thread, events, NELEM(events), timeout);
		if (ret <= 0)
		{
			if (ret < 0)
			{
				pr_red_info("cavan_thread_epoll_wait");
			}

			return ret;
		}

		for (p = events, p_end = p + ret; p < p_end; p++)
		{
			if (p->data.fd == thread->rd_event_fd)
			{
				return 1;
			}
		}
	}

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
		pr_error_info("pthread_mutex_init");
		return ret;
	}

	ret = pthread_cond_init(&thread->cond, NULL);
	if (ret < 0)
	{
		pr_error_info("pthread_cond_init");
		goto out_pthread_mutex_destroy;
	}

	ret = pipe(thread->pipefd);
	if (ret < 0)
	{
		pr_red_info("pipe");
		goto out_pthread_cond_destroy;
	}

	ret = fcntl(thread->pipefd[0], F_SETFL, O_NONBLOCK);
	if (ret < 0)
	{
		pr_err_info("fcntl");
		goto out_close_pipe;
	}

	ret = fcntl(thread->pipefd[1], F_SETFL, O_NONBLOCK);
	if (ret < 0)
	{
		pr_err_info("fcntl");
		goto out_close_pipe;
	}

	thread->epoll_fd = epoll_create(THREAD_EPOLL_SIZE);
	if (thread->epoll_fd < 0)
	{
		pr_err_info("epoll_create");
		ret = thread->epoll_fd;
		goto out_close_pipe;
	}

	ret = cavan_thread_epoll_add(thread, thread->pipefd[0], EPOLLIN);
	if (ret < 0)
	{
		pr_err_info("cavan_thread_epoll_add");
		goto out_close_epoll;
	}

	thread->state = CAVAN_THREAD_STATE_NONE;
	thread->private_data = data;

	if (thread->wake_handker == NULL)
	{
		thread->wake_handker = cavan_thread_wake_handler_none;
	}

	return 0;

out_close_epoll:
	close(thread->epoll_fd);
out_close_pipe:
	close(thread->pipefd[0]);
	close(thread->pipefd[1]);
out_pthread_cond_destroy:
	pthread_cond_destroy(&thread->cond);
out_pthread_mutex_destroy:
	pthread_mutex_destroy(&thread->lock);
	return ret;
}

void cavan_thread_deinit(struct cavan_thread *thread)
{
	close(thread->epoll_fd);

	close(thread->pipefd[0]);
	close(thread->pipefd[1]);

	pthread_cond_destroy(&thread->cond);
	pthread_mutex_destroy(&thread->lock);
}

static void cavan_thread_sighandler(int signum)
{
#if CAVAN_THREAD_DEBUG
	pr_bold_info("signum = %d", signum);
#endif

	if (signum == SIGUSR1)
	{
		pthread_exit(0);
	}
}

static void *cavan_thread_main_loop(void *data)
{
	int ret;
	struct cavan_thread *thread = data;

	signal(SIGUSR1, cavan_thread_sighandler);

	pthread_mutex_lock(&thread->lock);

	data = thread->private_data;
	thread->pending = false;
	thread->state = CAVAN_THREAD_STATE_RUNNING;

	while (1)
	{
		switch (thread->state)
		{
		case CAVAN_THREAD_STATE_SLEEP:
		case CAVAN_THREAD_STATE_WAIT_EVENT:
			thread->state = CAVAN_THREAD_STATE_RUNNING;
		case CAVAN_THREAD_STATE_RUNNING:
#if CAVAN_THREAD_DEBUG
			pr_bold_info("Thread %s running", thread->name);
#endif
			while (thread->state == CAVAN_THREAD_STATE_RUNNING)
			{
				pthread_mutex_unlock(&thread->lock);
				ret = thread->handler(thread, data);
				pthread_mutex_lock(&thread->lock);
				if (ret < 0)
				{
					pr_red_info("thread->handler");
					goto out_thread_exit;
				}
			}
			break;

		case CAVAN_THREAD_STATE_STOPPPING:
#if CAVAN_THREAD_DEBUG
			pr_bold_info("Thread %s stopping", thread->name);
#endif
		case CAVAN_THREAD_STATE_STOPPED:
			goto out_thread_exit;

		case CAVAN_THREAD_STATE_SUSPEND:
			if (thread->pending)
			{
#if CAVAN_THREAD_DEBUG
				pr_bold_info("Thread %s pending", thread->name);
#endif
				thread->state = CAVAN_THREAD_STATE_RUNNING;
			}

			while (thread->state == CAVAN_THREAD_STATE_SUSPEND)
			{
#if CAVAN_THREAD_DEBUG
				pr_bold_info("Thread %s suspend", thread->name);
#endif
				ret = pthread_cond_wait(&thread->cond, &thread->lock);
				if (ret < 0)
				{
					pr_error_info("pthread_cond_wait");
					goto out_thread_exit;
				}
			}

			thread->pending = false;
			break;

		default:
#if CAVAN_THREAD_DEBUG
			pr_red_info("Thread %s invalid state %d", thread->name, thread->state);
#endif
			goto out_thread_exit;
		}
	}

out_thread_exit:
	thread->state = CAVAN_THREAD_STATE_STOPPED;
#if CAVAN_THREAD_DEBUG
	pr_bold_info("Thread %s soppped", thread->name);
#endif

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
			thread->state = CAVAN_THREAD_STATE_IDLE;
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
	int i;

	pthread_mutex_lock(&thread->lock);

	for (i = 1; i < 200; i++)
	{
		switch (thread->state)
		{
		case CAVAN_THREAD_STATE_STOPPPING:
			break;

		case CAVAN_THREAD_STATE_WAIT_EVENT:
			cavan_thread_send_event(thread, 0);
			thread->state = CAVAN_THREAD_STATE_STOPPPING;
			break;

		case CAVAN_THREAD_STATE_SLEEP:
		case CAVAN_THREAD_STATE_SUSPEND:
		case CAVAN_THREAD_STATE_RUNNING:
			thread->state = CAVAN_THREAD_STATE_STOPPPING;
			break;

		default:
			thread->state = CAVAN_THREAD_STATE_STOPPED;
			goto out_pthread_mutex_unlock;
		}

		pthread_cond_broadcast(&thread->cond);

		pthread_mutex_unlock(&thread->lock);
		thread->wake_handker(thread, thread->private_data);
		msleep(i);
		pthread_mutex_lock(&thread->lock);
	}

	if (thread->state != CAVAN_THREAD_STATE_STOPPED)
	{
#if CAVAN_THREAD_DEBUG
		pr_red_info("stop timeout kill it now");
#endif
		pthread_kill(thread->id, SIGUSR1);
	}

	thread->state = CAVAN_THREAD_STATE_STOPPED;

out_pthread_mutex_unlock:
	pthread_mutex_unlock(&thread->lock);
}

int cavan_thread_run(struct cavan_thread *thread, void *data)
{
	int ret;

	ret = cavan_thread_init(thread, data);
	if (ret < 0)
	{
		pr_red_info("cavan_thread_init");
		return ret;
	}

	return cavan_thread_start(thread);
}

int cavan_thread_run_self(struct cavan_thread *thread, void *data)
{
	int ret;

	ret = cavan_thread_init(thread, data);
	if (ret < 0)
	{
		pr_red_info("cavan_thread_init");
		return ret;
	}

	cavan_thread_main_loop(thread);
	cavan_thread_deinit(thread);

	return 0;
}

void cavan_thread_suspend(struct cavan_thread *thread)
{
	pthread_mutex_lock(&thread->lock);

	switch (thread->state)
	{
	case CAVAN_THREAD_STATE_WAIT_EVENT:
	case CAVAN_THREAD_STATE_RUNNING:
	case CAVAN_THREAD_STATE_SLEEP:
		thread->state = CAVAN_THREAD_STATE_SUSPEND;
	case CAVAN_THREAD_STATE_SUSPEND:
	case CAVAN_THREAD_STATE_STOPPPING:
	case CAVAN_THREAD_STATE_STOPPED:
		break;

	default:
		pr_red_info("Invalid state %d", thread->state);
		break;
	}

	pthread_mutex_unlock(&thread->lock);
}

void cavan_thread_resume(struct cavan_thread *thread)
{
	pthread_mutex_lock(&thread->lock);

	thread->pending = true;

	switch (thread->state)
	{
	case CAVAN_THREAD_STATE_WAIT_EVENT:
		cavan_thread_send_event(thread, 0);
	case CAVAN_THREAD_STATE_SUSPEND:
	case CAVAN_THREAD_STATE_SLEEP:
		thread->state = CAVAN_THREAD_STATE_RUNNING;
		pthread_cond_signal(&thread->cond);
	case CAVAN_THREAD_STATE_RUNNING:
		thread->wake_handker(thread, thread->private_data);
	case CAVAN_THREAD_STATE_IDLE:
	case CAVAN_THREAD_STATE_STOPPPING:
	case CAVAN_THREAD_STATE_STOPPED:
		break;

	default:
		pr_red_info("Invalid state %d", thread->state);
		break;
	}

	pthread_mutex_unlock(&thread->lock);
}

void cavan_lock_init(struct cavan_lock *lock, bool acquire)
{
	pthread_mutex_init(&lock->mutex, NULL);

	if (acquire)
	{
		pthread_mutex_lock(&lock->mutex);
		lock->owner = pthread_self();
		lock->held_count = 1;
	}
	else
	{
		lock->owner = 0;
		lock->held_count = 0;
	}
}

void cavan_lock_deinit(struct cavan_lock *lock)
{
	pthread_mutex_destroy(&lock->mutex);
}

void cavan_lock_acquire(struct cavan_lock *lock)
{
	pthread_t owner = pthread_self();

	if (pthread_equal(owner, lock->owner))
	{
		lock->held_count++;
	}
	else
	{
		pthread_mutex_lock(&lock->mutex);
		lock->owner = owner;
		lock->held_count = 1;
	}
}

void cavan_lock_release(struct cavan_lock *lock)
{
	pthread_t owner = pthread_self();

	if (!pthread_equal(owner, lock->owner) || --lock->held_count > 0)
	{
		return;
	}

	if (lock->held_count < 0)
	{
		pr_red_info("unbalanced %s %d", __FUNCTION__, lock->held_count);
		lock->held_count = 0;
	}

	lock->owner = 0;
	pthread_mutex_unlock(&lock->mutex);
}
