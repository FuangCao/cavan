/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Sat Nov 24 18:07:06 CST 2012
 */

#include <cavan.h>
#include <cavan/timer.h>

int cavan_time_diff(const struct timespec *t1, const struct timespec *t2)
{
	return (t1->tv_sec - t2->tv_sec) * 1000 + (t1->tv_nsec - t2->tv_nsec) / 1000000;
}

void cavan_timer_set_time(struct timespec *time, u32 timeout)
{
	long tmp;

	clock_gettime(CLOCK_REALTIME, time);

	tmp = time->tv_nsec / 1000000 + timeout;
	time->tv_sec += tmp / 1000;
	time->tv_nsec = (tmp % 1000) * 1000000;
}

bool cavan_timer_exists(struct cavan_timer *head, struct cavan_timer *node)
{
	struct cavan_timer *p;

	for (p = head->next; p != head; p = p->next)
	{
		if (p == node)
		{
			return true;
		}
	}

	return false;
}

void cavan_timer_remove_node(struct cavan_timer *node)
{
	node->prev->next = node->next;
	node->next->prev = node->prev;
}

void cavan_timer_add_node_base(struct cavan_timer *head, struct cavan_timer *node)
{
	struct cavan_timer *p;

	for (p = head->next; p != head; p = p->next)
	{
		if (cavan_time_diff(&p->time, &node->time) > 0)
		{
			break;
		}
	}

	p->prev->next = node;
	node->next = p;

	node->prev = p->prev;
	p->prev = node;
}

int cavan_timer_add_node(struct cavan_timer_service *service, struct cavan_timer *node, u32 timeout)
{
	struct cavan_timer *head;

	if (node->handler == NULL)
	{
		pr_red_info("p->handler == NULL");

		return -EINVAL;
	}

	pthread_mutex_lock(&service->lock);

	cavan_timer_set_time(&node->time, timeout);

	head = &service->head_node;

	if (cavan_timer_exists(head, node))
	{
		cavan_timer_remove_node(node);
		cavan_timer_add_node_base(head, node);
	}
	else
	{
		cavan_timer_add_node_base(head, node);
		sem_post(&service->sem);
	}

	pthread_cond_broadcast(&service->cond);

	pthread_mutex_unlock(&service->lock);

	return 0;
}

static void *cavan_timer_service_handler(void *data)
{
	int ret;
	struct cavan_timer *timer;
	struct cavan_timer_service *service = data;
	struct cavan_timer *head = &service->head_node;

	pthread_mutex_lock(&service->lock);

	service->state = CAVAN_TIMER_STATE_RUNNING;

	while (service->state == CAVAN_TIMER_STATE_RUNNING)
	{
		pthread_mutex_unlock(&service->lock);

		pr_bold_info("Wait for timer");
		sem_wait(&service->sem);
		pr_bold_info("Timer running");

		while (service->state == CAVAN_TIMER_STATE_RUNNING)
		{
			timer = head->next;
			ret = pthread_cond_timedwait(&service->cond, &service->lock, &timer->time);
			if (ret == 0)
			{
				continue;
			}

			cavan_timer_remove_node(timer);

			pthread_mutex_unlock(&service->lock);
			timer->handler(timer, timer->private_data);
			pthread_mutex_lock(&service->lock);

			break;
		}
	}

	service->state = CAVAN_TIMER_STATE_STOPPED;
	pthread_mutex_unlock(&service->lock);

	pr_red_info("Timer service exit!");

	return NULL;
}

int cavan_timer_service_start(struct cavan_timer_service *service)
{
	int ret;

	ret = sem_init(&service->sem, 0, 0);
	if (ret < 0)
	{
		pr_error_info("sem_init");
		return ret;
	}

	service->head_node.next = &service->head_node;
	service->head_node.prev = &service->head_node;

	pthread_mutex_init(&service->lock, NULL);
	pthread_cond_init(&service->cond, NULL);

	ret = pthread_create(&service->thread, NULL, cavan_timer_service_handler, service);
	if (ret < 0)
	{
		pr_error_info("pthread_create");
		pthread_mutex_destroy(&service->lock);
		pthread_cond_destroy(&service->cond);
		return ret;
	}

	return 0;
}

int cavan_timer_service_stop(struct cavan_timer_service *service)
{
	pthread_mutex_lock(&service->lock);

	service->state = CAVAN_TIMER_STATE_STOPPING;

	while (service->state != CAVAN_TIMER_STATE_STOPPED)
	{
		sem_post(&service->sem);
		pthread_cond_broadcast(&service->cond);

		pthread_mutex_unlock(&service->lock);
		msleep(100);
		pthread_mutex_lock(&service->lock);
	}

	pthread_mutex_unlock(&service->lock);

	pthread_mutex_destroy(&service->lock);
	pthread_cond_destroy(&service->cond);

	return 0;
}
