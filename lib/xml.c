/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Mon Oct 15 15:29:08 CST 2012
 */

#include <cavan.h>
#include <cavan/xml.h>
#include <cavan/stack.h>

#define CONFIG_CAVAN_XML_DEBUG		0

struct cavan_xml_attribute *cavan_xml_attribute_alloc(char *name, char *value, int flags)
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
		free(name);
	}
out_free_attr:
	free(attr);
	return NULL;
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

bool cavan_xml_attribute_set_name(struct cavan_xml_attribute *attr, char *name, int flags)
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
		free(attr->name);
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

bool cavan_xml_attribute_set_value(struct cavan_xml_attribute *attr, char *value, int flags)
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
		free(attr->value);
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

bool cavan_xml_attribute_set(struct cavan_xml_attribute **head, char *name, char *value, int flags)
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

// ============================================================

struct cavan_xml_tag *cavan_xml_tag_alloc(char *name)
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
	tag->name = name;
	tag->child = NULL;
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

static char *cavan_xml_tag_get_line_prefix(const char *prefix, int level, char *buff, size_t size)
{
	char *buff_end = buff + sizeof(buff);

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

			q = tag->content;
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

static int cavan_xml_document_get_next_token(struct cavan_xml_parser *parser, bool verbose)
{
	int ret;
	int lineno;
	char *name = NULL;
	char *value = NULL;
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

	ret = -EFAULT;
	lineno = 0;
	parser->name = NULL;
	parser->attr = NULL;
	parser->content = NULL;
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
			pr_green_info("lineno = %d", parser->lineno + lineno);
#endif
		case ' ':
		case '\t':
		case '\f':
		case '\r':
			if (parser->token == CAVAN_XML_TOKEN_NONE)
			{
				break;
			}

			*tail = 0;
			head = tail = p + 1;

			if (name && value)
			{
				if (cavan_xml_attribute_set(&parser->attr, name, value, 0) == false)
				{
					if (verbose)
					{
						pr_parser_error_info(parser->lineno + lineno, "cavan_xml_attribute_alloc");
					}

					goto out_cavan_xml_token_error;
				}

				name = NULL;
				value = NULL;
			}
			break;

		case '<':
			if (parser->token != CAVAN_XML_TOKEN_NONE)
			{
				if (verbose)
				{
					pr_parser_error_info(parser->lineno + lineno, "token = %d", parser->token);
				}

				goto out_cavan_xml_token_error;
			}

			switch (p[1])
			{
			case '/':
				p++;
				parser->token = CAVAN_XML_TOKEN_TAG_END;
				break;

			case '!':
				if (text_lhcmp("--", p + 2) == 0)
				{
					parser->token = CAVAN_XML_TOKEN_COMMENT;
					parser->pos = p + 4;
					parser->lineno += lineno;
					return 0;
				}
			default:
				parser->token = CAVAN_XML_TOKEN_TAG_BEGIN;
			}

			parser->name = head = tail = p + 1;
			break;

		case '?':
			if (parser->token == CAVAN_XML_TOKEN_NONE)
			{
				if (verbose)
				{
					pr_parser_error_info(parser->lineno + lineno, "invalid '?'");
				}

				goto out_cavan_xml_token_error;
			}

			if (parser->name < p)
			{
				goto label_tag_single;
			}

			parser->token = CAVAN_XML_TOKEN_TAG_ATTR;
			break;

		case '/':
			if (parser->token == CAVAN_XML_TOKEN_NONE)
			{
				if (verbose)
				{
					pr_parser_error_info(parser->lineno + lineno, "invalid '/'");
				}

				goto out_cavan_xml_token_error;
			}

			parser->token = CAVAN_XML_TOKEN_TAG_SINGLE;

label_tag_single:
			if (*++p != '>')
			{
				if (verbose)
				{
					pr_parser_error_info(parser->lineno + lineno, "need '>' by has '%c'", *p);
				}

				goto out_cavan_xml_token_error;
			}

			goto label_tag_end;

		case '>':
			if (parser->token == CAVAN_XML_TOKEN_NONE)
			{
				if (verbose)
				{
					pr_parser_error_info(parser->lineno + lineno, "invalid '>'");
				}

				goto out_cavan_xml_token_error;
			}

label_tag_end:
			*tail = 0;

			if (name && value)
			{
				if (cavan_xml_attribute_set(&parser->attr, name, value, 0) == false)
				{
					if (verbose)
					{
						pr_parser_error_info(parser->lineno + lineno, "cavan_xml_attribute_alloc");
					}

					goto out_cavan_xml_token_error;
				}
			}

			parser->pos = ++p;
			parser->lineno += lineno;
			return 0;

		case '=':
			*tail = 0;
			name = head;
			head = tail = p + 1;
			break;

		case '"':
			if (name == NULL)
			{
				if (verbose)
				{
					pr_parser_error_info(parser->lineno + lineno, "name is null");
				}

				goto out_cavan_xml_token_error;
			}

			for (p++, value = tail = p; p < p_end; p++)
			{
				switch (*p)
				{
				case '\\':
					p++;
					if (p < p_end)
					{
						*tail++ = text_get_escope_letter(*p);
					}
					else
					{
						if (verbose)
						{
							pr_parser_error_info(parser->lineno + lineno, "\" is not pair");
						}

						goto out_cavan_xml_token_error;
					}
					break;

				case '\n':
					lineno++;
#if CONFIG_CAVAN_XML_DEBUG
					pr_green_info("lineno = %d", parser->lineno + lineno);
#endif
				case '\r':
				case '\f':
					break;

				case '"':
					goto label_value_end;

				default:
					*tail++ = *p;
				}
			}
label_value_end:
			*tail = 0;
			head = tail = p + 1;
			break;

		default:
			if (parser->token == CAVAN_XML_TOKEN_NONE)
			{
				if (verbose)
				{
					pr_parser_error_info(parser->lineno + lineno, "invalid content");
				}

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

static int cavan_xml_document_get_comment(struct cavan_xml_parser *parser, char *buff, size_t size)
{
	int lineno;
	char *p, *p_end;

	for (p = parser->pos, p_end = parser->pos_end, lineno = 0; p < p_end; p++)
	{
		switch (*p)
		{
		case '\n':
			lineno++;
			break;

		case '-':
			if (text_lhcmp("->", p + 1) == 0)
			{
				*p = 0;

				if (buff && size > 0)
				{
					text_ncopy(buff, parser->pos, size);
				}

				parser->pos = p + 3;
				parser->lineno += lineno;
				return 0;
			}
		}
	}

	return -1;
}

static struct cavan_xml_document *cavan_xml_document_parse_base(char *content, size_t size)
{
	int ret;
	struct cavan_xml_parser parser;
	struct general_stack stack;
	struct cavan_xml_document *doc;
	struct cavan_xml_tag *parent, *tag;

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
			pr_parser_error_info(parser.lineno, "cavan_xml_get_next_token");
			goto out_cavan_xml_document_free;
		}
#if CONFIG_CAVAN_XML_DEBUG
		pr_green_info("token = %d", parser.token);
#endif
		switch (parser.token)
		{
		case CAVAN_XML_TOKEN_NONE:
			break;

		case CAVAN_XML_TOKEN_EOF:
			goto out_parse_complete;

		case CAVAN_XML_TOKEN_TAG_ATTR:
		case CAVAN_XML_TOKEN_TAG_BEGIN:
		case CAVAN_XML_TOKEN_TAG_SINGLE:
#if CONFIG_CAVAN_XML_DEBUG
			pr_green_info("name = %s", parser.name);
#endif
			tag = cavan_xml_tag_alloc(parser.name);
			if (tag == NULL)
			{
				pr_red_info("cavan_xml_tag_alloc");
				goto out_cavan_xml_document_free;
			}

			tag->attr = parser.attr;

			if (parser.token == CAVAN_XML_TOKEN_TAG_ATTR)
			{
				tag->next = doc->attr;
				doc->attr = tag;
			}
			else
			{
				parent = general_stack_get_top_fd(&stack);
				if (parent)
				{
					if (parent->content)
					{
						pr_parser_error_info(parser.lineno, "tag %s has content %s", parent->name, parent->content);
						goto out_cavan_xml_document_free;
					}

					tag->next = parent->child;
					parent->child = tag;
				}
				else
				{
					tag->next = doc->tag;
					doc->tag = tag;
				}

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

						tag->content = parser.pos;
						parser.pos = p + length;
					}
					else
					{
						general_stack_push_fd(&stack, tag);
						parser.next_token = parser.token;
					}
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

		case CAVAN_XML_TOKEN_COMMENT:
			ret = cavan_xml_document_get_comment(&parser, NULL, 0);
			if (ret < 0)
			{
				pr_parser_error_info(parser.lineno, "cavan_xml_get_comment");
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
