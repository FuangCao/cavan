/*
 * File:		test_bcrypt.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2017-05-08 10:30:56
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
#include <cavan/bcrypt.h>

int main(int argc, char *argv[])
{
	bool success;
	char saltb[64], *salt;
	char hashb[64], *hash;

	assert(argc > 1);

	if (argc > 2) {
		salt = argv[2];
	} else {
		salt = bcrypt_gensalt3(saltb, sizeof(saltb));
		if (salt == NULL) {
			pr_red_info("bcrypt_gensalt3");
			return -EFAULT;
		}

		*salt = 0;
		salt = saltb;
	}

	pr_info("salt = %s", salt);

	hash = bcrypt_hashpw(argv[1], salt, hashb, sizeof(hashb));
	if (hash == NULL) {
		pr_red_info("bcrypt_hashpw");
		return -EFAULT;
	}

	*hash = 0;
	hash = hashb;

	pr_info("hash = %s", hash);

	success = bcrypt_checkpw(argv[1], hash);
	pr_info("success = %d", success);

	return 0;
}
