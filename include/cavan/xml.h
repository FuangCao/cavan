#pragma once

/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Mon Oct 15 15:29:08 CST 2012
 */

#include <cavan.h>

#define CAVAN_XML_FLAG_NAME_ALLOC		(1 << 0)
#define CAVAN_XML_FLAG_VALUE_ALLOC		(1 << 1)
#define CAVAN_XML_FLAG_CONTENT_ALLOC	(1 << 2)

struct cavan_xml_attribute
{
	int flags;
	char *name;
	char *value;
	struct cavan_xml_attribute *next;
};

struct cavan_xml_tag
{
	int flags;
	char *name;
	char *content;
	struct cavan_xml_attribute *attr;
	struct cavan_xml_tag *next;
};

struct cavan_xml_document
{
	size_t size;
	char *content;
	struct cavan_xml_attribute *attr;
	struct cavan_xml_tag *tag;
};

struct cavan_xml_attribute *cavan_xml_attribute_alloc(char *name, char *value);
void cavan_xml_attribute_free(struct cavan_xml_attribute *attr);
struct cavan_xml_tag *cavan_xml_tag_alloc(char *name);
void cavan_xml_tag_free(struct cavan_xml_tag *tag);
struct cavan_xml_document *cavan_xml_document_alloc(void);
void cavan_xml_document_free(struct cavan_xml_document *doc);
struct cavan_xml_document *cavan_xml_parse(const char *pathname);
