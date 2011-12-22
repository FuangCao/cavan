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

void cavan_inotify_uninit(struct cavan_inotify_descriptor *desc)
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

int cavan_inotify_register_watch(struct cavan_inotify_descriptor *desc, const char *pathname, int (*handle)(const char *, struct inotify_event *, void *), uint32_t mask, void *data)
{
	int wd;
	struct cavan_inotify_watch *p;

	if (desc->watch_count >= NELEM(desc->watchs))
	{
		error_msg("Too match watch");
		return -EFAULT;
	}

	if (handle == NULL)
	{
		error_msg("handle == NULL");
		return -EINVAL;
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
	p->handle = handle;
	text_copy(p->pathname, pathname);
	desc->watch_count++;

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

static int cavan_inotify_handle_event(struct inotify_event *event, struct cavan_inotify_watch *watchs, size_t count)
{
	struct cavan_inotify_watch *watch_end;

	for (watch_end = watchs + count; watchs < watch_end; watchs++)
	{
		if (event->wd == watchs->wd)
		{
			return watchs->handle(watchs->pathname, event, watchs->data);
		}
	}

	return -ENOENT;
}

int cavan_inotify_event_loop(struct cavan_inotify_descriptor *desc)
{
	int fd;
	char buff[(sizeof(struct inotify_event) + 256) * 10];
	void *p, *p_end;
	ssize_t readlen;

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

		p = buff;
		p_end = buff + readlen;

		while (p < p_end)
		{
			cavan_inotify_handle_event(p, desc->watchs, NELEM(desc->watchs));

			p += sizeof(struct inotify_event) + ((struct inotify_event *)p)->len;
		}
	}

	return 0;
}
