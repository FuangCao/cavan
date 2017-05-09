/*
 * File:		http_client.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2017-05-09 15:37:48
 *
 * Copyright (c) 2017 Fuang.Cao <cavan.cfa@gmail.com>
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
#include <cavan/http.h>

int main(int argc, char *argv[])
{
	int length;
	char buff[1024];

	assert(argc > 1);

	if (argc > 2) {
		const char *headers[] = {
			"Content-Type: application/json",
		};

		length = http_client_send_request(argv[1], argv[2], headers, NELEM(headers), buff, sizeof(buff));
		if (length < 0) {
			pr_red_info("http_client_post: %d", length);
			return length;
		}

		buff[length] = 0;
		println("POST[%d] = %s", length, buff);
	} else {
		const char *headers[] = {
			"x-auth-token: 3048f9298c34848f103e17ed2bf8ba592fd",
		};

		length = http_client_send_request(argv[1], NULL, headers, NELEM(headers), buff, sizeof(buff));
		if (length < 0) {
			pr_red_info("http_client_get: %d", length);
			return length;
		}

		buff[length] = 0;
		println("GET[%d] = %s", length, buff);
	}

	return 0;
}
