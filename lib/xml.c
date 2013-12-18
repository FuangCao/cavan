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

	pr_std_info("attr: name = %s, value = %s", name, value);

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

	pr_std_info("tag: name = %s", name);

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

static char *cavan_xml_skip_space(char *content, char *content_end)
{
	while (content < content_end)
	{
		switch (*content)
		{
		case ' ':
		case '\t':
		case '\r':
		case '\n':
		case '\f':
			break;

		default:
			return content;
		}

		content++;
	}

	return NULL;
}

static char *cavan_xml_find_space(char *content, char *content_end)
{
	while (content < content_end)
	{
		switch (*content)
		{
		case ' ':
		case '\t':
		case '\r':
		case '\n':
		case '\f':
			return content;
		}

		content++;
	}

	return NULL;
}

static struct cavan_xml_attribute *cavan_xml_attribute_parse(char **content, char *content_end)
{
	char quote = 0;
	int quote_count;
	char *p;
	char *name;
	char *value;

	name = cavan_xml_skip_space(*content, content_end);
	if (name == NULL)
	{
		pr_red_info("cavan_xml_skip_space");
		return NULL;
	}

	for (p = name, value = NULL, quote_count = 0; p < content_end; p++)
	{
		switch (*p)
		{
		case '\n':
		case '\f':
		case '\t':
			pr_pos_info();
			return NULL;

		case '=':
			*p = 0;
			value = p + 1;
			break;

		case '"':
		case '\'':
			if (quote_count == 0)
			{
				if (value == NULL)
				{
					pr_pos_info();
					return NULL;
				}

				quote = *p;
				value = p + 1;
			}
			else if (quote_count == 1)
			{
				if (quote != *p)
				{
					pr_pos_info();
					return NULL;
				}

				*p = 0;
			}
			else
			{
				pr_pos_info();
				return NULL;
			}

			quote_count++;
			break;

		case ' ':
			if (quote_count == 0)
			{
				pr_pos_info();
				return NULL;
			}

			if (quote_count == 2)
			{
				struct cavan_xml_attribute *attr = cavan_xml_attribute_alloc(name, value);
				if (attr == NULL)
				{
					pr_pos_info();
					return NULL;
				}

				*content = p + 1;
				return attr;
			}
		}
	}

	return NULL;
}

static struct cavan_xml_tag *cavan_xml_tag_parse(char **content, char *content_end)
{
	char *p;
	char *name;
	int namelen;
	struct cavan_xml_attribute *tail = NULL, *attr;
	struct cavan_xml_tag *tag;

	name = cavan_xml_skip_space(*content, content_end);
	if (name == NULL || *name != '<')
	{
		pr_pos_info();
		return NULL;
	}

	p = cavan_xml_find_space(++name, content_end);
	if (p == NULL || p == name)
	{
		pr_pos_info();
		return NULL;
	}

	if (*(p - 1) == '>')
	{
		p--;
	}

	namelen = p - name;
	*p++ = 0;

	tag = cavan_xml_tag_alloc(name);
	if (tag == NULL)
	{
		pr_pos_info();
		return NULL;
	}

	while (1)
	{
		attr = cavan_xml_attribute_parse(&p, content_end);
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

	p = cavan_xml_skip_space(p, content_end);
	if (p == NULL)
	{
		pr_pos_info();
		return NULL;
	}

	if (*p == '>')
	{
	}
	else if (text_lhcmp("/>", p) == 0)
	{
		p += 2;
	}
	else if (text_lhcmp(name, p) == 0)
	{
		p += namelen;

		if (*p != '>')
		{
			pr_pos_info();
			goto out_cavan_xml_tag_free;
		}

		p++;
	}
	else
	{
		pr_pos_info();
		goto out_cavan_xml_tag_free;
	}

	*content = p;

	return tag;

out_cavan_xml_tag_free:
	cavan_xml_tag_free(tag);
	return NULL;
}

static struct cavan_xml_document *cavan_xml_document_parse_base(char *content, size_t size)
{
	char *content_end = content + size;
	struct cavan_xml_tag *tail = NULL, *tag;
	struct cavan_xml_document *doc;

	doc = cavan_xml_document_alloc();
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
