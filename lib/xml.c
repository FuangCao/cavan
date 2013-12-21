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

	pr_green_info("attr: name = %s, value = %s", name, value);

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

	pr_green_info("tag: name = %s", name);

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

#if 0
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

static char *cavan_xml_attribute_parse(struct cavan_xml_document *doc)
{
	return NULL;
}

static char *cavan_xml_tag_parse(struct cavan_xml_document *doc)
{
	return NULL;
}
#endif

static cavan_xml_token_t cavan_xml_get_next_token(struct cavan_xml_parser *parser)
{
	char *name = NULL;
	char *value = NULL;
	char *p, *p_end;
	char *head, *tail;
	cavan_xml_token_t token;
	struct cavan_xml_attribute *attr;

	if (parser->next_token != CAVAN_XML_TOKEN_NONE)
	{
		pr_pos_info();
		token = parser->next_token;
		parser->next_token = CAVAN_XML_TOKEN_NONE;
		return token;
	}

	parser->name = NULL;
	parser->attr = NULL;
	token = CAVAN_XML_TOKEN_NONE;

	for (p = parser->pos, p_end = parser->pos_end, head = tail = p; p < p_end; p++)
	{
		switch (*p)
		{
		case ' ':
		case '\t':
		case '\f':
		case '\r':
		case '\n':
			*tail = 0;
			head = tail = p + 1;
			if (token != CAVAN_XML_TOKEN_NONE)
			{
				if (name && value)
				{
					attr = cavan_xml_attribute_alloc(name, value);
					if (attr == NULL)
					{
						pr_pos_info();
						token = CAVAN_XML_TOKEN_ERROR;
						goto out_cavan_xml_token_error;
					}

					attr->next = parser->attr;
					parser->attr = attr;

					name = NULL;
					value = NULL;
				}
			}
			break;

		case '<':
			if (token != CAVAN_XML_TOKEN_NONE)
			{
				pr_pos_info();
				token = CAVAN_XML_TOKEN_ERROR;
				goto out_cavan_xml_token_error;
			}

			if (p[1] == '/')
			{
				p++;
				token = CAVAN_XML_TOKEN_TAG_END;
			}
			else
			{
				token = CAVAN_XML_TOKEN_TAG_BEGIN;
			}

			parser->name = head = tail = p + 1;
			break;

		case '?':
			if (parser->name == p)
			{
				*tail++ = *p;
				continue;
			}

		case '/':
			p++;
			if (*p != '>')
			{
				pr_pos_info();
				token = CAVAN_XML_TOKEN_ERROR;
				goto out_cavan_xml_token_error;
			}

			token = CAVAN_XML_TOKEN_TAG_SINGLE;
		case '>':
			*tail = 0;

			if (name && value)
			{
				attr = cavan_xml_attribute_alloc(name, value);
				if (attr == NULL)
				{
					pr_pos_info();
					token = CAVAN_XML_TOKEN_ERROR;
					goto out_cavan_xml_token_error;
				}

				attr->next = parser->attr;
				parser->attr = attr;
			}

			parser->pos = p + 1;
			return token;

		case '=':
			*tail = 0;
			name = head;
			head = tail = p + 1;
			break;

		case '"':
			if (name == NULL)
			{
				pr_pos_info();
				token = CAVAN_XML_TOKEN_ERROR;
				goto out_cavan_xml_token_error;
			}

			p++;
			value = tail = p;

			while (p < p_end)
			{
				switch (*p)
				{
				case '\\':
					p++;
					if (p < p_end)
					{
						*tail++ = *p++;
					}
					else
					{
						pr_pos_info();
						token = CAVAN_XML_TOKEN_ERROR;
						goto out_cavan_xml_token_error;
					}
					break;

				case '"':
					goto label_value_end;

				default:
					*tail++ = *p++;
				}
			}
label_value_end:
			*tail = 0;
			head = tail = p + 1;
			break;

		default:
			*tail++ = *p;
		}
	}

	token = CAVAN_XML_TOKEN_EOF;

out_cavan_xml_token_error:
	attr = parser->attr;
	while (attr)
	{
		struct cavan_xml_attribute *next = attr->next;
		cavan_xml_attribute_free(attr);
		attr = next;
	}

	return token;
}

static struct cavan_xml_document *cavan_xml_document_parse_base(char *content, size_t size)
{
	cavan_xml_token_t token;
	struct cavan_xml_parser parser;

	parser.pos = content;
	parser.pos_end = content + size;
	parser.next_token = CAVAN_XML_TOKEN_NONE;

	while (1)
	{
		token = cavan_xml_get_next_token(&parser);
		pr_green_info("token = %d", token);
		switch (token)
		{
		case CAVAN_XML_TOKEN_EOF:
			return NULL;

		case CAVAN_XML_TOKEN_TAG_BEGIN:
			pr_green_info("name = %s", parser.name);
			break;

		case CAVAN_XML_TOKEN_TAG_END:
			pr_green_info("name = %s", parser.name);
			break;

		case CAVAN_XML_TOKEN_TAG_SINGLE:
			pr_green_info("name = %s", parser.name);
			break;

		default:
			pr_red_info("unknown token %d", token);
			return NULL;
		}
	}

	return NULL;
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
