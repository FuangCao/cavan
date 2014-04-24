#include <huamobile/hua_thread.h>
#include <huamobile/hua_input.h>

static int hua_input_thread_handler(void *data)
{
	int err_count;
	struct hua_input_thread *thread = data;

	pr_pos_info();

	mutex_lock(&thread->lock);

	while (1)
	{
		while (thread->state == HUA_INPUT_THREAD_STATE_SUSPEND)
		{
			pr_green_info("huamobile input thread %s suspend", thread->name);

			set_current_state(TASK_UNINTERRUPTIBLE);
			mutex_unlock(&thread->lock);
			schedule();
			mutex_lock(&thread->lock);
		}

		if (thread->state == HUA_INPUT_THREAD_STATE_STOPPING)
		{
			break;
		}

		thread->state = HUA_INPUT_THREAD_STATE_RUNNING;
		pr_green_info("huamobile input thread %s running", thread->name);

		if (thread->prepare)
		{
			thread->prepare(thread, true);
		}

		err_count = 0;

		while (1)
		{
			mutex_unlock(&thread->lock);
			thread->wait_for_event(thread);
			mutex_lock(&thread->lock);

			if (unlikely(thread->state != HUA_INPUT_THREAD_STATE_RUNNING))
			{
				break;
			}

			if (unlikely(thread->event_handle(thread) < 0))
			{
				err_count++;
				pr_red_info("err_count = %d", err_count);

				if (err_count < 20)
				{
					continue;
				}

				if (thread->error_handle)
				{
					thread->error_handle(thread);
				}

				thread->state = HUA_INPUT_THREAD_STATE_SUSPEND;
				break;
			}
		}

		if (thread->prepare)
		{
			thread->prepare(thread, false);
		}
	}

	thread->task = NULL;
	thread->state = HUA_INPUT_THREAD_STATE_STOPPED;
	pr_green_info("huamobile input thread %s stoped", thread->name);

	mutex_unlock(&thread->lock);

	return 0;
}

static int hua_input_thread_prepare(struct hua_input_thread *thread, enum hua_input_thread_state state)
{
	pr_pos_info();

	mutex_lock(&thread->lock);

	if (thread->task == NULL)
	{
		thread->task = kthread_create(hua_input_thread_handler, thread, thread->name);
		if (thread->task == NULL)
		{
			pr_red_info("kthread_create");
			mutex_unlock(&thread->lock);
			return -EFAULT;
		}

		if (thread->priority > 0)
		{
			struct sched_param param =
			{
				.sched_priority = thread->priority
			};

			sched_setscheduler(thread->task, SCHED_FIFO, &param);
		}
	}

	thread->state = state;

	mutex_unlock(&thread->lock);

	return 0;
}

void hua_input_thread_stop(struct hua_input_thread *thread)
{
	mutex_lock(&thread->lock);

	pr_pos_info();

	if (thread->task && thread->state != HUA_INPUT_THREAD_STATE_STOPPED)
	{
		thread->state = HUA_INPUT_THREAD_STATE_STOPPING;

		while (thread->state != HUA_INPUT_THREAD_STATE_STOPPED)
		{
			pr_func_info("thread name = %s", thread->name);

			if (thread->stop)
			{
				mutex_unlock(&thread->lock);
				thread->stop(thread);
				mutex_lock(&thread->lock);
			}

			wake_up_process(thread->task);

			mutex_unlock(&thread->lock);
			msleep(1);
			mutex_lock(&thread->lock);
		}
	}
	else
	{
		thread->state = HUA_INPUT_THREAD_STATE_STOPPED;
	}

	mutex_unlock(&thread->lock);
}

EXPORT_SYMBOL_GPL(hua_input_thread_stop);

int hua_input_thread_set_state(struct hua_input_thread *thread, enum hua_input_thread_state state)
{
	if (state == HUA_INPUT_THREAD_STATE_RUNNING)
	{
		int ret;

		ret = hua_input_thread_prepare(thread, state);
		if (ret < 0)
		{
			pr_red_info("hua_input_thread_prepare");
			return ret;
		}

		wake_up_process(thread->task);
	}
	else
	{
		int locked = mutex_trylock(&thread->lock);

		thread->state = state;

		if (locked)
		{
			mutex_unlock(&thread->lock);
		}
	}

	return 0;
}

EXPORT_SYMBOL_GPL(hua_input_thread_set_state);

int hua_input_thread_init(struct hua_input_thread *thread, const char *format, ...)
{
	va_list ap;

	if (thread->event_handle == NULL || thread->wait_for_event == NULL)
	{
		pr_red_info("thread->event_handle == NULL || thread->wait_for_event == NULL");
		return -EINVAL;
	}

	va_start(ap, format);
	thread->name = kvasprintf(GFP_KERNEL, format, ap);
	va_end(ap);

	if (thread->name == NULL)
	{
		pr_red_info("kvasprintf");
		return -ENOMEM;
	}

	thread->task = NULL;
	thread->state = HUA_INPUT_THREAD_STATE_STOPPED;

	mutex_init(&thread->lock);

	return 0;
}

EXPORT_SYMBOL_GPL(hua_input_thread_init);

void hua_input_thread_destroy(struct hua_input_thread *thread)
{
	kfree(thread->name);
	mutex_destroy(&thread->lock);
}

EXPORT_SYMBOL_GPL(hua_input_thread_destroy);

MODULE_AUTHOR("Fuang.Cao <cavan.cfa@gmail.com>");
MODULE_DESCRIPTION("Huamobile Thread");
MODULE_LICENSE("GPL");
