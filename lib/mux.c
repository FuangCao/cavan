/*
 * File:		mux.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2015-07-28 11:43:37
 *
 * Copyright (c) 2015 Fuang.Cao <cavan.cfa@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <cavan.h>
#include <cavan/mux.h>

int cavan_mux_init(struct cavan_mux *mux)
{
	cavan_lock_init(&mux->lock, false);
	mux->packages = NULL;

	return 0;
}

void cavan_mux_deinit(struct cavan_mux *mux)
{
	struct cavan_mux_package *head;

	cavan_lock_acquire(&mux->lock);

	head = mux->packages;
	while (head)
	{
		struct cavan_mux_package *next = head->next;

		free(head);
		head = next;
	}

	cavan_lock_release(&mux->lock);

	cavan_lock_deinit(&mux->lock);
}

void cavan_mux_append_package(struct cavan_mux *mux, struct cavan_mux_package *package)
{
	struct cavan_mux_package **head;

	cavan_lock_acquire(&mux->lock);

	for (head = &mux->packages; *head && (*head)->length < package->length; head = &(*head)->next);

	package->next = *head;
	*head = package;

	cavan_lock_release(&mux->lock);
}

struct cavan_mux_package *cavan_mux_dequeue_package(struct cavan_mux *mux, size_t length)
{
	struct cavan_mux_package **head;
	struct cavan_mux_package *package;

	cavan_lock_acquire(&mux->lock);

	head = &mux->packages;

	while (*head && (*head)->length < length)
	{
		head = &(*head)->next;
	}

	package = *head;
	if (package)
	{
		*head = package->next;
	}

	cavan_lock_release(&mux->lock);

	return package;
}

void cavan_mux_show_packages(struct cavan_mux *mux)
{
	struct cavan_mux_package *package;

	cavan_lock_acquire(&mux->lock);


	for (package = mux->packages; package; package = package->next)
	{
		println("length = %d", package->length);
	}

	cavan_lock_release(&mux->lock);
}

struct cavan_mux_package *cavan_mux_package_alloc(struct cavan_mux *mux, size_t length)
{
	struct cavan_mux_package *package;

	package = cavan_mux_dequeue_package(mux, length);
	if (package)
	{
		return package;
	}

	package = malloc(sizeof(struct cavan_mux_package) + length);
	if (package == NULL)
	{
		pr_error_info("malloc");
		return NULL;
	}

	package->length = length;

	return package;
}

void cavan_mux_package_free(struct cavan_mux *mux, struct cavan_mux_package *package)
{
	cavan_mux_append_package(mux, package);
}
