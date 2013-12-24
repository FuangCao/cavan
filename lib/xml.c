/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Mon Oct 15 15:29:08 CST 2012
 */

#include <cavan.h>
#include <cavan/xml.h>
#include <cavan/stack.h>

#define CONFIG_CAVAN_XML_DEBUG		0

struct cavan_xml_attribute *cavan_xml_attribute_alloc(const char *name, const char *value, int flags)
{
	struct cavan_xml_attribute *attr;

#if CONFIG_CAVAN_XML_DEBUG
	pr_green_info("attr: name = %s, value = %s", name, value);
#endif

	attr = malloc(sizeof(*attr));
	if (attr == NULL)
	{
		pr_error_info("malloc");
		return NULL;
	}

	attr->flags = 0;

	if (name && (flags & CAVAN_XML_FLAG_NAME_ALLOC))
	{
		name = strdup(name);
		if (name == NULL)
		{
			goto out_free_attr;
		}

		attr->flags |= CAVAN_XML_FLAG_NAME_ALLOC;
	}

	if (value && (flags & CAVAN_XML_FLAG_VALUE_ALLOC))
	{
		value = strdup(value);
		if (value == NULL)
		{
			goto out_free_name;
		}

		attr->flags |= CAVAN_XML_FLAG_VALUE_ALLOC;
	}

	attr->name = name;
	attr->value = value;
	attr->next = NULL;

	return attr;

out_free_name:
	if ((flags & CAVAN_XML_FLAG_NAME_ALLOC))
	{
		free((char *)name);
	}
out_free_attr:
	free(attr);
	return NULL;
}

void cavan_xml_attribute_free(struct cavan_xml_attribute *attr)
{
	if (attr->name && (attr->flags & CAVAN_XML_FLAG_NAME_ALLOC))
	{
		free((char *)attr->name);
	}

	if (attr->value && (attr->flags & CAVAN_XML_FLAG_VALUE_ALLOC))
	{
		free((char *)attr->value);
	}

	free(attr);
}

static struct cavan_xml_attribute *cavan_xml_attribute_list_invert(struct cavan_xml_attribute *head)
{
	struct cavan_xml_attribute *next, *prev = NULL;

	while (head)
	{
		next = head->next;
		head->next = prev;
		prev = head;
		head = next;
	}

	return prev;
}

static char *cavan_xml_attribute_tostring(struct cavan_xml_attribute *attr, char *buff, char *buff_end)
{
	return buff + snprintf(buff, buff_end - buff, "%s=\"%s\"", attr->name, attr->value);
}

struct cavan_xml_attribute *cavan_xml_attribute_find(struct cavan_xml_attribute *head, const char *name)
{
	while (head)
	{
		if (text_cmp(head->name, name) == 0)
		{
			return head;
		}

		head = head->next;
	}

	return NULL;
}

bool cavan_xml_attribute_set_name(struct cavan_xml_attribute *attr, const char *name, int flags)
{
	if (name && (flags & CAVAN_XML_FLAG_NAME_ALLOC))
	{
		name = strdup(name);
		if (name == NULL)
		{
			return false;
		}
	}

	if (attr->name && (attr->flags & CAVAN_XML_FLAG_NAME_ALLOC))
	{
		free((char *)attr->name);
	}

	attr->name = name;

	if ((flags & CAVAN_XML_FLAG_NAME_ALLOC))
	{
		attr->flags |= CAVAN_XML_FLAG_NAME_ALLOC;
	}
	else
	{
		attr->flags &= ~CAVAN_XML_FLAG_NAME_ALLOC;
	}

	return true;
}

bool cavan_xml_attribute_set_value(struct cavan_xml_attribute *attr, const char *value, int flags)
{
	if (value && (flags & CAVAN_XML_FLAG_VALUE_ALLOC))
	{
		value = strdup(value);
		if (value == NULL)
		{
			return false;
		}
	}

	if (attr->value && (attr->flags & CAVAN_XML_FLAG_VALUE_ALLOC))
	{
		free((char *)attr->value);
	}

	attr->value = value;

	if ((flags & CAVAN_XML_FLAG_VALUE_ALLOC))
	{
		attr->flags |= CAVAN_XML_FLAG_VALUE_ALLOC;
	}
	else
	{
		attr->flags &= ~CAVAN_XML_FLAG_VALUE_ALLOC;
	}

	return true;
}

bool cavan_xml_attribute_set(struct cavan_xml_attribute **head, const char *name, const char *value, int flags)
{
	struct cavan_xml_attribute *attr;

	attr = cavan_xml_attribute_find(*head, name);
	if (attr)
	{
		return cavan_xml_attribute_set_value(attr, value, flags);
	}

	attr = cavan_xml_attribute_alloc(name, value, flags);
	if (attr == NULL)
	{
		return false;
	}

	attr->next = *head;
	*head = attr;

	return true;
}

bool cavan_xml_attribute_remove(struct cavan_xml_attribute **head, struct cavan_xml_attribute *attr)
{
	while (*head)
	{
		if (*head == attr)
		{
			*head = attr->next;
			return true;
		}

		head = &(*head)->next;
	}

	return false;
}

static int cavan_xml_attribute_parse_value(char **ptext, char *text_end)
{
	int lineno;
	char *value, *text;

	for (value = text = *ptext, lineno = 0; text < text_end; text++)
	{
		switch (*text)
		{
		case '"':
			*value = 0;
			*ptext = text;
			return lineno;

		case '\n':
			lineno++;
			text = text_skip_space(text + 1, text_end);
			if (text < text_end)
			{
				*value++ = *text;
			}
		case '\r':
		case '\f':
			break;

		case '\\':
			if (++text < text_end)
			{
				*value++ = text_get_escope_letter(*text);
			}
			break;

		default:
			*value++ = *text;
		}
	}

	return -1;
}

void cavan_xml_attribute_extend(struct cavan_xml_attribute **phead, struct cavan_xml_attribute *extra)
{
	while (extra)
	{
		struct cavan_xml_attribute **phead_bak = phead;
		struct cavan_xml_attribute *next = extra->next;

		while (*phead)
		{
			if (strcmp((*phead)->name, extra->name) == 0)
			{
				struct cavan_xml_attribute *attr;

				attr = *phead;
				*phead = attr->next;
				cavan_xml_attribute_free(attr);
			}
			else
			{
				phead = &(*phead)->next;
			}
		}

		extra->next = NULL;
		*phead = extra;

		extra = next;
		phead = phead_bak;
	}
}

// ============================================================

struct cavan_xml_tag *cavan_xml_tag_alloc(const char *name, const char *content, int flags)
{
	struct cavan_xml_tag *tag;

#if CONFIG_CAVAN_XML_DEBUG
	pr_green_info("tag: name = %s", name);
#endif

	tag = malloc(sizeof(*tag));
	if (tag == NULL)
	{
		pr_error_info("malloc");
		return NULL;
	}

	tag->flags = 0;

	if (name && (flags & CAVAN_XML_FLAG_NAME_ALLOC))
	{
		name = strdup(name);
		if (name == NULL)
		{
			goto out_free_tag;
		}

		tag->flags |= CAVAN_XML_FLAG_NAME_ALLOC;
	}

	if (content && (flags & CAVAN_XML_FLAG_CONTENT_ALLOC))
	{
		content = strdup(content);
		if (content == NULL)
		{
			goto out_free_name;
		}

		tag->flags |= CAVAN_XML_FLAG_CONTENT_ALLOC;
	}

	tag->name = name;
	tag->content = content;
	tag->child = NULL;
	tag->next = NULL;
	tag->attr = NULL;

	return tag;

out_free_name:
	if ((flags & CAVAN_XML_FLAG_NAME_ALLOC))
	{
		free((char *)name);
	}
out_free_tag:
	free(tag);
	return NULL;
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

	if (tag->name && (tag->flags & CAVAN_XML_FLAG_NAME_ALLOC))
	{
		free((char *)tag->name);
	}

	if (tag->content && (tag->flags & CAVAN_XML_FLAG_CONTENT_ALLOC))
	{
		free((char *)tag->content);
	}

	free(tag);
}

bool cavan_xml_tag_set_name(struct cavan_xml_tag *tag, const char *name, int flags)
{
	if (name && (flags & CAVAN_XML_FLAG_NAME_ALLOC))
	{
		name = strdup(name);
		if (name == NULL)
		{
			return false;
		}
	}

	if (tag->name && (tag->flags & CAVAN_XML_FLAG_NAME_ALLOC))
	{
		free((char *)tag->name);
	}

	tag->name = name;

	if ((flags & CAVAN_XML_FLAG_NAME_ALLOC))
	{
		tag->flags |= CAVAN_XML_FLAG_NAME_ALLOC;
	}
	else
	{
		tag->flags &= ~CAVAN_XML_FLAG_NAME_ALLOC;
	}

	return true;
}

bool cavan_xml_tag_set_content(struct cavan_xml_tag *tag, const char *content, int flags)
{
	if (content && (flags & CAVAN_XML_FLAG_CONTENT_ALLOC))
	{
		content = strdup(content);
		if (content == NULL)
		{
			return false;
		}
	}

	if (tag->content && (tag->flags & CAVAN_XML_FLAG_CONTENT_ALLOC))
	{
		free((char *)tag->content);
	}

	tag->content = content;

	if ((flags & CAVAN_XML_FLAG_CONTENT_ALLOC))
	{
		tag->flags |= CAVAN_XML_FLAG_CONTENT_ALLOC;
	}
	else
	{
		tag->flags &= ~CAVAN_XML_FLAG_CONTENT_ALLOC;
	}

	return true;
}

struct cavan_xml_tag *cavan_xml_tag_create(struct cavan_xml_tag **head, const char *name, int flags)
{
	struct cavan_xml_tag *tag;

	if ((flags & CAVAN_XML_FLAG_UNIQUE))
	{
		tag = cavan_xml_tag_find(*head, name);
		if (tag)
		{
			return tag;
		}
	}

	tag = cavan_xml_tag_alloc(name, NULL, flags);
	if (tag == NULL)
	{
		return NULL;
	}

	tag->next = *head;
	*head = tag;

	return tag;
}

static char *cavan_xml_tag_get_line_prefix(const char *prefix, int level, char *buff, size_t size)
{
	char *buff_end;

	for (buff_end = buff + size - 1; level > 0; level--)
	{
		buff = text_ncopy(buff, prefix, buff_end - buff);
	}

	*buff = 0;

	return buff;
}

static char *cavan_xml_tag_tostring(struct cavan_xml_document *doc, struct cavan_xml_tag *tag, int level, char *buff, char *buff_end)
{
	char prefix[32];
	struct cavan_xml_attribute *attr;

	cavan_xml_tag_get_line_prefix(doc->line_prefix, level, prefix, sizeof(prefix));

	buff += snprintf(buff, buff_end - buff, "%s<%s", prefix, tag->name);

	for (attr = tag->attr; attr; attr = attr->next)
	{
		buff = text_ncopy(buff, doc->word_sep, buff_end - buff);
		buff = cavan_xml_attribute_tostring(attr, buff, buff_end);
	}

	if (tag->child)
	{
		struct cavan_xml_tag *child;

		buff += snprintf(buff, buff_end - buff, ">%s", doc->newline);

		for (child = tag->child; child; child = child->next)
		{
			buff = cavan_xml_tag_tostring(doc, child, level + 1, buff, buff_end);
			buff = text_ncopy(buff, doc->newline, buff_end - buff);
		}

		buff += snprintf(buff, buff_end - buff, "%s</%s>", prefix, tag->name);
	}
	else if (tag->content)
	{
		if ((tag->flags & CAVAN_XML_FLAG_CONTENT_MULTI_LINE))
		{
			char *content, *p, *p_end;
			char *q, *q_end;
			size_t length;

			length = strlen(tag->content);
			content = alloca(length + 100);
			if (content == NULL)
			{
				pr_error_info("alloca");
				return buff;
			}

			p = content;
			p_end = p + length + 100;

			q = (char *)tag->content;
			q_end = q + length;
			q = text_skip_space_and_lf(q, q_end);

			while (q < q_end && p < p_end)
			{
				switch (*q)
				{
				case '\n':
					q = text_skip_space(q + 1, q_end);
					p += snprintf(p, p_end - p, "%s%s%s", doc->newline, prefix, doc->line_prefix);
					break;

				default:
					*p++ = *q++;
				}
			}

			p = text_skip_space_and_lf_invert(p - 1, content);
			p[1] = 0;

			buff += snprintf(buff, buff_end - buff, ">%s%s%s%s%s%s</%s>", doc->newline, prefix, doc->line_prefix, content, doc->newline, prefix, tag->name);
		}
		else
		{
			buff += snprintf(buff, buff_end - buff, ">%s</%s>", tag->content, tag->name);
		}
	}
	else
	{
		buff = text_ncopy(buff, "/>", buff_end - buff);
	}

	return buff;
}

struct cavan_xml_tag *cavan_xml_tag_find(struct cavan_xml_tag *head, const char *name)
{
	while (head)
	{
		if (text_cmp(head->name, name) == 0)
		{
			return head;
		}

		head = head->next;
	}

	return NULL;
}

bool cavan_xml_tag_remove(struct cavan_xml_tag **head, struct cavan_xml_tag *tag)
{
	while (*head)
	{
		if (*head == tag)
		{
			*head = tag->next;
			return true;
		}

		head = &(*head)->next;
	}

	return false;
}

int cavan_xml_tag_remove_all_by_name(struct cavan_xml_tag **head, const char *name, bool recursion)
{
	int count = 0;

	while (*head)
	{
		if (text_cmp((*head)->name, name) == 0)
		{
			struct cavan_xml_tag *tag;

			tag = *head;
			*head = (*head)->next;

			cavan_xml_tag_free(tag);
			count++;
		}
		else
		{
			if (recursion)
			{
				count += cavan_xml_tag_remove_all_by_name(&(*head)->child, name, recursion);
			}

			head = &(*head)->next;
		}
	}

	return count;
}

// ============================================================

struct cavan_xml_document *cavan_xml_document_alloc(void)
{
	struct cavan_xml_document *doc;

	doc = malloc(sizeof(*doc));
	if (doc == NULL)
	{
		pr_error_info("malloc");
		return NULL;
	}

	doc->newline = "\n";
	doc->word_sep = " ";
	doc->line_prefix = "\t";
	doc->attr = NULL;
	doc->tag = NULL;

	return doc;
}

void cavan_xml_document_free(struct cavan_xml_document *doc)
{
	int i;
	struct cavan_xml_tag *tags[] = {doc->attr, doc->tag};

	for (i = 0; i < NELEM(tags); i++)
	{
		struct cavan_xml_tag *tag = tags[i];

		while (tag)
		{
			struct cavan_xml_tag *next = tag->next;
			cavan_xml_tag_free(tag);
			tag = next;
		}
	}

	if (doc->content)
	{
		free(doc->content);
	}

	free(doc);
}

static struct cavan_xml_tag *cavan_xml_tag_list_invert(struct cavan_xml_tag *head)
{
	struct cavan_xml_tag *next, *prev = NULL;

	while (head)
	{
		next = head->next;
		head->next = prev;
		head->attr = cavan_xml_attribute_list_invert(head->attr);
		head->child = cavan_xml_tag_list_invert(head->child);

		prev = head;
		head = next;
	}

	return prev;
}

void cavan_xml_document_invert(struct cavan_xml_document *doc)
{
	doc->attr = cavan_xml_tag_list_invert(doc->attr);
	doc->tag = cavan_xml_tag_list_invert(doc->tag);
}

static int cavan_xml_document_parse_comment(char **ptext, char *text_end)
{
	int lineno;
	char *text;

	for (text = *ptext, lineno = 0; text < text_end; text++)
	{
		switch (*text)
		{
		case '\n':
			lineno++;
			break;

		case '-':
			if (text_lhcmp("->", text + 1) == 0)
			{
				*text = 0;
				*ptext = text + 2;
				return lineno;
			}
			break;
		}
	}

	return -1;
}

static int cavan_xml_document_get_next_token(struct cavan_xml_parser *parser, bool verbose)
{
	int ret;
	int lineno;
	char *p, *p_end;
	char *head, *tail;
	struct cavan_xml_attribute *attr;

	parser->prev_token = parser->token;

	if (parser->next_token != CAVAN_XML_TOKEN_NONE)
	{
		parser->token = parser->next_token;
		parser->next_token = CAVAN_XML_TOKEN_NONE;
		return 0;
	}

	lineno = parser->lineno;;
	parser->name = NULL;
	parser->attr = NULL;
	parser->content = NULL;
	parser->comment = NULL;
	parser->token = CAVAN_XML_TOKEN_NONE;

	for (p = parser->pos, p_end = parser->pos_end, head = tail = p; p < p_end; p++)
	{
		switch (*p)
		{
		case 0:
		case '\\':
			break;

		case '\n':
			lineno++;
#if CONFIG_CAVAN_XML_DEBUG
			pr_green_info("lineno = %d", lineno);
#endif
		case ' ':
		case '\t':
		case '\f':
		case '\r':
			if (parser->token == CAVAN_XML_TOKEN_NONE)
			{
				break;
			}

			if (byte_is_space_or_lf(*head))
			{
				head = p + 1;
			}

			*tail = 0;
			tail = p + 1;

			break;

		case '<':
			if (parser->token != CAVAN_XML_TOKEN_NONE)
			{
				if (verbose)
				{
					pr_parser_error_info(lineno, "token = %d", parser->token);
				}

				ret = -EFAULT;
				goto out_cavan_xml_token_error;
			}

			switch (p[1])
			{
			case '/':
				p++;
				parser->token = CAVAN_XML_TOKEN_TAG_END;
				break;

			case '?':
				p++;
				parser->token = CAVAN_XML_TOKEN_TAG_ATTR;
				break;

			case '!':
				if (text_lhcmp("--", p + 2) == 0)
				{
					p += 4;
					parser->comment = p;
					ret = cavan_xml_document_parse_comment(&p, p_end);
					if (ret < 0)
					{
						if (verbose)
						{
							pr_parser_error_info(lineno, "cavan_xml_document_parse_comment");
						}

						ret = -EFAULT;
						goto out_cavan_xml_token_error;
					}

					parser->token = CAVAN_XML_TOKEN_COMMENT;
					parser->pos = p + 1;
					parser->lineno = lineno + ret;
					return 0;
				}
			default:
				parser->token = CAVAN_XML_TOKEN_TAG_BEGIN;
			}

			if (byte_is_named(*++p) == false)
			{
				if (verbose)
				{
					pr_parser_error_info(lineno, "'%c' is not a named letter", *p);
				}

				ret = -EFAULT;
				goto out_cavan_xml_token_error;
			}

			head = p;

			p = text_skip_name(p, p_end);
			if (tail == NULL)
			{
				if (verbose)
				{
					pr_parser_error_info(lineno, "text_skip_name");
				}

				ret = -EFAULT;
				goto out_cavan_xml_token_error;
			}

			parser->name = head;
			ret = *p;
			*p = 0;

			switch (ret)
			{
			case '?':
				goto label_tag_attr;

			case '/':
				goto label_tag_single1;

			case '>':
				goto label_tag_end;
			}

			head = tail = p + 1;
			break;

		case '?':
			if (parser->token == CAVAN_XML_TOKEN_NONE)
			{
				if (verbose)
				{
					pr_parser_error_info(lineno, "invalid '?'");
				}

				ret = -EFAULT;
				goto out_cavan_xml_token_error;
			}

label_tag_attr:
			parser->token = CAVAN_XML_TOKEN_TAG_ATTR;
			goto label_tag_single2;

		case '/':
			if (parser->token == CAVAN_XML_TOKEN_NONE)
			{
				if (verbose)
				{
					pr_parser_error_info(lineno, "invalid '/'");
				}

				ret = -EFAULT;
				goto out_cavan_xml_token_error;
			}
label_tag_single1:
			parser->token = CAVAN_XML_TOKEN_TAG_SINGLE;
label_tag_single2:
			if (*++p != '>')
			{
				if (verbose)
				{
					pr_parser_error_info(lineno, "need '>' by has '%c'", *p);
				}

				ret = -EFAULT;
				goto out_cavan_xml_token_error;
			}

			goto label_tag_end;

		case '>':
			if (parser->token == CAVAN_XML_TOKEN_NONE)
			{
				if (verbose)
				{
					pr_parser_error_info(lineno, "invalid '>'");
				}

				ret = -EFAULT;
				goto out_cavan_xml_token_error;
			}

label_tag_end:
			*tail = 0;

			if (parser->name[0] == 0)
			{
				if (verbose)
				{
					pr_parser_error_info(lineno, "tag name is empty");
				}

				ret = -EFAULT;
				goto out_cavan_xml_token_error;
			}

			parser->pos = ++p;
			parser->lineno = lineno;
			return 0;

		case '=':
			*tail = 0;

			p = text_skip_space_and_lf(p + 1, p_end);
			if (*p != '"')
			{
				if (verbose)
				{
					pr_parser_error_info(lineno, "need a '\"'");
				}

				ret = -EFAULT;
				goto out_cavan_xml_token_error;
			}

			tail = ++p;

			ret = cavan_xml_attribute_parse_value(&p, p_end);
			if (ret < 0)
			{
				if (verbose)
				{
					pr_parser_error_info(lineno, "cavan_xml_attribute_parse_value");
				}

				ret = -EFAULT;
				goto out_cavan_xml_token_error;
			}

			lineno += ret;

			if (cavan_xml_attribute_set(&parser->attr, head, tail, 0) == false)
			{
				if (verbose)
				{
					pr_parser_error_info(lineno, "cavan_xml_attribute_alloc");
				}

				ret = -EFAULT;
				goto out_cavan_xml_token_error;
			}

			head = tail = p + 1;
			break;

		default:
			if (parser->token == CAVAN_XML_TOKEN_NONE)
			{
				if (verbose)
				{
					pr_parser_error_info(lineno, "invalid content");
				}

				ret = -EFAULT;
				goto out_cavan_xml_token_error;
			}

			*tail++ = *p;
		}
	}

	ret = 0;
	parser->token = CAVAN_XML_TOKEN_EOF;

out_cavan_xml_token_error:
	attr = parser->attr;
	while (attr)
	{
		struct cavan_xml_attribute *next = attr->next;
		cavan_xml_attribute_free(attr);
		attr = next;
	}

	return ret;
}

static struct cavan_xml_document *cavan_xml_document_parse_base(char *content, size_t size)
{
	int ret;
	struct cavan_xml_parser parser;
	struct general_stack stack;
	struct cavan_xml_document *doc;
	struct cavan_xml_tag *tag, *parent, **pparent;

	ret = general_stack_init_fd(&stack, 10);
	if (ret < 0)
	{
		pr_red_info("general_stack_init_fd");
		return NULL;
	}

	doc = cavan_xml_document_alloc();
	if (doc == NULL)
	{
		pr_red_info("cavan_xml_document_alloc");
		goto out_general_stack_free;
	}

	parser.lineno = 1;
	parser.pos = content;
	parser.pos_end = content + size;
	parser.prev_token = parser.next_token = parser.token = CAVAN_XML_TOKEN_NONE;

	while (1)
	{
		ret = cavan_xml_document_get_next_token(&parser, true);
		if (ret < 0)
		{
			goto out_cavan_xml_document_free;
		}
#if CONFIG_CAVAN_XML_DEBUG
		pr_green_info("token = %d", parser.token);
#endif
		switch (parser.token)
		{
		case CAVAN_XML_TOKEN_COMMENT:
		case CAVAN_XML_TOKEN_NONE:
			break;

		case CAVAN_XML_TOKEN_EOF:
			goto out_parse_complete;

		case CAVAN_XML_TOKEN_TAG_ATTR:
			tag = cavan_xml_tag_create(&doc->attr, parser.name, CAVAN_XML_FLAG_UNIQUE);
			if (tag == NULL)
			{
				pr_red_info("cavan_xml_tag_create");
				goto out_cavan_xml_document_free;
			}

			cavan_xml_attribute_extend(&tag->attr, parser.attr);
			break;

		case CAVAN_XML_TOKEN_TAG_BEGIN:
		case CAVAN_XML_TOKEN_TAG_SINGLE:
			parent = general_stack_get_top_fd(&stack);
			if (parent)
			{
				if (parent->content)
				{
					pr_parser_error_info(parser.lineno, "tag %s has content %s", parent->name, parent->content);
					goto out_cavan_xml_document_free;
				}

				pparent = &parent->child;
			}
			else
			{
				pparent = &doc->tag;
			}

			tag = cavan_xml_tag_create(pparent, parser.name, 0);
			if (tag == NULL)
			{
				pr_red_info("cavan_xml_tag_alloc");
				goto out_cavan_xml_document_free;
			}

			cavan_xml_attribute_extend(&tag->attr, parser.attr);

			if (parser.token == CAVAN_XML_TOKEN_TAG_BEGIN)
			{
				ret = cavan_xml_document_get_next_token(&parser, false);
				if (ret < 0)
				{
					int lineno;
					size_t length;
					char *pos, *p;
					char buff[512];

					pos = parser.pos;
					length = snprintf(buff, sizeof(buff), "</%s>", tag->name);

					p = mem_kmp_find(pos, buff, parser.pos_end - pos, length);
					if (p == NULL)
					{
						pr_parser_error_info(parser.lineno, "tag %s is not pair", tag->name);
						goto out_cavan_xml_document_free;
					}

					*p = 0;

					for (lineno = 0; pos < p; pos++)
					{
						if (*pos == '\n')
						{
							lineno++;
						}
					}

					if (lineno)
					{
						tag->flags |= CAVAN_XML_FLAG_CONTENT_MULTI_LINE;
						parser.lineno += lineno;
					}

					cavan_xml_tag_set_content(tag, parser.pos, 0);
					parser.pos = p + length;
				}
				else
				{
					general_stack_push_fd(&stack, tag);
					parser.next_token = parser.token;
				}
			}
			break;

		case CAVAN_XML_TOKEN_TAG_END:
			tag = general_stack_pop_fd(&stack);
			if (tag == NULL || strcmp(parser.name, tag->name))
			{
				pr_parser_error_info(parser.lineno, "tag (%s <> %s) is not pair", tag ? tag->name : "null", parser.name);
				goto out_cavan_xml_document_free;
			}
			break;

		default:
			pr_red_info("unknown token %d", parser.token);
			goto out_cavan_xml_document_free;
		}
	}

out_parse_complete:
	if (general_stack_get_top_fd(&stack))
	{
		pr_red_info("tag stack not empty");
		goto out_cavan_xml_document_free;
	}

	doc->content = content;
	general_stack_free(&stack);

	return doc;

out_cavan_xml_document_free:
	cavan_xml_document_free(doc);
out_general_stack_free:
	general_stack_free(&stack);
	return NULL;
}

struct cavan_xml_document *cavan_xml_document_parse(const char *pathname)
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

	cavan_xml_document_invert(doc);

	return doc;

out_free_content:
	free(content);
	return NULL;
}

char *cavan_xml_document_tostring(struct cavan_xml_document *doc, char *buff, size_t size)
{
	char *buff_end = buff + size - 1;
	struct cavan_xml_tag *tag;
	struct cavan_xml_attribute *attr;

	if (doc->attr != NULL)
	{
		for (tag = doc->attr; tag; tag = tag->next)
		{
			buff += snprintf(buff, buff_end - buff, "<?%s", tag->name);

			for (attr = tag->attr; attr; attr = attr->next)
			{
				buff = text_ncopy(buff, doc->word_sep, buff_end - buff);
				buff = cavan_xml_attribute_tostring(attr, buff, buff_end);
			}

			buff += snprintf(buff, buff_end - buff, "?>%s", doc->newline);
		}

		buff = text_ncopy(buff, doc->newline, buff_end - buff);
	}

	for (tag = doc->tag; tag; tag = tag->next)
	{
		buff = cavan_xml_tag_tostring(doc, tag, 0, buff, buff_end);
		buff = text_ncopy(buff, doc->newline, buff_end - buff);
	}

	*buff = 0;

	return buff;
}
