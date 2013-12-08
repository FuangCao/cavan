/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Mon Oct 15 15:29:08 CST 2012
 */

#include <cavan.h>
#include <cavan/xml.h>

struct cavan_xml_attribute *cavan_xml_attribute_alloc(char *name, char *value)
{
	struct cavan_xml_attribute *attr;

	attr = malloc(sizeof(*attr));
	if (attr == NULL)
	{
		pr_error_info("malloc");
		return NULL;
	}

	attr->flags = 0;
	attr->name = name;
	attr->value = value;
	attr->next = NULL;

	return attr;
}

void cavan_xml_attribute_free(struct cavan_xml_attribute *attr)
{
	if (attr->name && (attr->flags & CAVAN_XML_FLAG_NAME_ALLOC))
	{
		free(attr->name);
	}

	if (attr->value && (attr->flags & CAVAN_XML_FLAG_VALUE_ALLOC))
	{
		free(attr->value);
	}

	free(attr);
}

struct cavan_xml_tag *cavan_xml_tag_alloc(char *name)
{
	struct cavan_xml_tag *tag;

	tag = malloc(sizeof(*tag));
	if (tag == NULL)
	{
		pr_error_info("malloc");
		return NULL;
	}

	tag->flags = 0;
	tag->name = name;
	tag->next = NULL;
	tag->attr = NULL;
	tag->content = NULL;

	return tag;
}

void cavan_xml_tag_free(struct cavan_xml_tag *tag)
{
	struct cavan_xml_attribute *attr = tag->attr;

	while (attr)
	{
		struct cavan_xml_attribute *next = attr->next;

		cavan_xml_attribute_free(attr);
		attr = next;
	}

	if (tag->content && (tag->flags & CAVAN_XML_FLAG_CONTENT_ALLOC))
	{
		free(tag->content);
	}

	free(tag);
}

struct cavan_xml_document *cavan_xml_document_alloc(void)
{
	struct cavan_xml_document *doc;

	doc = malloc(sizeof(*doc));
	if (doc == NULL)
	{
		pr_error_info("malloc");
		return NULL;
	}

	doc->attr = NULL;
	doc->tag = NULL;

	return doc;
}

void cavan_xml_document_free(struct cavan_xml_document *doc)
{
	struct cavan_xml_attribute *attr = doc->attr;
	struct cavan_xml_tag *tag = doc->tag;

	while (attr)
	{
		struct cavan_xml_attribute *next = attr->next;

		cavan_xml_attribute_free(attr);
		attr = next;
	}

	while (tag)
	{
		struct cavan_xml_tag *next = tag->next;

		cavan_xml_tag_free(tag);
		tag = next;
	}

	if (doc->content)
	{
		free(doc->content);
	}

	free(doc);
}

static struct cavan_xml_attribute *cavan_xml_attribute_parse(char **content, char *content_end)
{
	return NULL;
}

static struct cavan_xml_tag *cavan_xml_tag_parse(char **content, char *content_end)
{
	struct cavan_xml_attribute *tail = NULL, *attr;
	struct cavan_xml_tag *tag = cavan_xml_tag_alloc(NULL);

	if (tag == NULL)
	{
		return NULL;
	}

	while (1)
	{
		attr = cavan_xml_attribute_parse(content, content_end);
		if (attr == NULL)
		{
			break;
		}

		if (tail == NULL)
		{
			tag->attr = attr;
		}

		tail = attr;
	}

	return tag;
}

static struct cavan_xml_document *cavan_xml_document_parse_base(char *content, size_t size)
{
	char *content_end = content + size;
	struct cavan_xml_tag *tail = NULL, *tag;
	struct cavan_xml_document *doc = cavan_xml_document_alloc();

	if (doc == NULL)
	{
		return NULL;
	}

	doc->content = content;

	while (1)
	{
		tag = cavan_xml_tag_parse(&content, content_end);
		if (tag == NULL)
		{
			break;
		}

		if (tail == NULL)
		{
			doc->tag = tag;
		}

		tail = tag;
	}

	return doc;
}

struct cavan_xml_document *cavan_xml_parse(const char *pathname)
{
	size_t size;
	char *content;
	struct cavan_xml_document *doc;

	content = file_read_all(pathname, 0, &size);
	if (content == NULL)
	{
		return NULL;
	}

	doc = cavan_xml_document_parse_base(content, size);
	if (doc == NULL)
	{
		goto out_free_content;
	}

	return doc;

out_free_content:
	free(content);
	return NULL;
}
