/*
 * File:		testXml.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2013-12-18 20:07:08
 *
 * Copyright (c) 2013 Fuang.Cao <cavan.cfa@gmail.com>
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
#include <cavan/xml.h>

int main(int argc, char *argv[])
{
	char buff[MB(1)];
	struct cavan_xml_document *doc;

	assert(argc > 1);

	doc = cavan_xml_parse(argv[1]);
	if (doc == NULL)
	{
		pr_red_info("cavan_xml_parse");
		return -EFAULT;
	}

	if (argc > 2)
	{
		cavan_xml_remove_all_tag_by_name(&doc->tag, argv[2], true);
	}

	print_ntext(buff, cavan_xml_tostring(doc, buff, sizeof(buff)) - buff);

	cavan_xml_document_free(doc);

	return 0;
}
