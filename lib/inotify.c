#include <cavan.h>
#include <cavan/inotify.h>

// Fuang.Cao <cavan.cfa@gmail.com> 2011-12-22 12:20:56

int cavan_inotify_init(struct cavan_inotify_descriptor *desc)
{
	int fd;
	struct cavan_inotify_watch *p, *p_end;

	fd = inotify_init();
	if (fd < 0)
	{
		print_error("inotify_init");
		return fd;
	}

	desc->fd = fd;

	for (p = desc->watchs, p_end = p + NELEM(desc->watchs); p < p_end; p++)
	{
		p->wd = -1;
	}

	desc->watch_count = 0;

	return 0;
}

void cavan_inotify_deinit(struct cavan_inotify_descriptor *desc)
{
	struct cavan_inotify_watch *p, *p_end;

	for (p = desc->watchs, p_end = p + NELEM(desc->watchs); p < p_end; p++)
	{
		if (p->wd >= 0)
		{
			inotify_rm_watch(desc->fd, p->wd);
		}
	}

	if (desc->fd >= 0)
	{
		close(desc->fd);
	}
}

int cavan_inotify_register_watch(struct cavan_inotify_descriptor *desc, const char *pathname, uint32_t mask, void *data)
{
	int wd;
	struct cavan_inotify_watch *p;

	if (desc->watch_count >= NELEM(desc->watchs))
	{
		error_msg("Too match watch");
		return -EFAULT;
	}

	if (pathname == NULL || *pathname == 0)
	{
		error_msg("pathname == NULL || *pathname == 0");
		return -EINVAL;
	}

	wd = inotify_add_watch(desc->fd, pathname, mask);
	if (wd < 0)
	{
		print_error("inotify_add_watch");
		return wd;
	}

	for (p = desc->watchs; p->wd >= 0; p++);

	p->wd = wd;
	p->data = data;
	text_copy(p->pathname, pathname);
	desc->watch_count++;

	pr_green_info("Add watch: %s", pathname);

	return 0;
}

int cavan_inotify_unregister_watch(struct cavan_inotify_descriptor *desc, const char *pathname)
{
	struct cavan_inotify_watch *p, *p_end;

	for (p = desc->watchs, p_end = p + NELEM(desc->watchs); p < p_end; p++)
	{
		if (text_cmp(p->pathname, pathname) == 0)
		{
			inotify_rm_watch(desc->fd, p->wd);
			p->wd = -1;
			desc->watch_count--;

			return 0;
		}
	}

	return -ENOENT;
}

struct cavan_inotify_watch *cavan_inotify_find_watch(int wd, struct cavan_inotify_watch *watchs, size_t count)
{
	struct cavan_inotify_watch *watch_end;

	for (watch_end = watchs + count; watchs < watch_end; watchs++)
	{
		if (wd == watchs->wd)
		{
			return watchs;
		}
	}

	return NULL;
}

int cavan_inotify_event_loop(struct cavan_inotify_descriptor *desc, int (*handle)(const char *pathname, struct inotify_event *event, void *data))
{
	int fd;
	char buff[(sizeof(struct inotify_event) + 256) * 10];
	ssize_t readlen;
	struct inotify_event *p, *p_end;
	struct cavan_inotify_watch *watch;

	if (handle == NULL)
	{
		error_msg("handle == NULL");
		return -EINVAL;
	}

	if (desc->watch_count <= 0)
	{
		error_msg("Please register some watch");
		return -EINVAL;
	}

	fd = desc->fd;

	while (1)
	{
		readlen = read(fd, buff, sizeof(buff));
		if (readlen < 0)
		{
			print_error("read");
			return readlen;
		}

		p = (struct inotify_event *) buff;
		p_end = (struct inotify_event *)(buff + readlen);

		while (p < p_end)
		{
			watch = cavan_inotify_find_watch(p->wd, desc->watchs, NELEM(desc->watchs));
			if (watch)
			{
				handle(watch->pathname, p, watch->data);
			}

			p = (struct inotify_event *)((char *)(p + 1) + p->len);
		}
	}

	return 0;
}
