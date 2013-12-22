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

#define pr_parser_error_info(parser, fmt, args ...) \
	pr_red_info("(" fmt ") at line[%d]", ##args, (parser)->lineno)

typedef enum cavan_xml_token
{
	CAVAN_XML_TOKEN_ERROR = -1,
	CAVAN_XML_TOKEN_NONE,
	CAVAN_XML_TOKEN_TAG_SINGLE,
	CAVAN_XML_TOKEN_TAG_BEGIN,
	CAVAN_XML_TOKEN_TAG_END,
	CAVAN_XML_TOKEN_CONTENT,
	CAVAN_XML_TOKEN_EOF,
} cavan_xml_token_t;

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
	struct cavan_xml_tag *child;
};

struct cavan_xml_document
{
	char *content;
	const char *newline;
	const char *line_prefix;
	const char *word_sep;
	struct cavan_xml_attribute *attr;
	struct cavan_xml_tag *tag;
};

struct cavan_xml_parser
{
	int lineno;
	char *pos;
	char *pos_end;
	char *name;
	char *content;
	struct cavan_xml_attribute *attr;

	cavan_xml_token_t prev_token;
	cavan_xml_token_t next_token;
};

struct cavan_xml_attribute *cavan_xml_attribute_alloc(char *name, char *value);
void cavan_xml_attribute_free(struct cavan_xml_attribute *attr);
struct cavan_xml_tag *cavan_xml_tag_alloc(char *name);
void cavan_xml_tag_free(struct cavan_xml_tag *tag);
struct cavan_xml_document *cavan_xml_document_alloc(void);
void cavan_xml_document_free(struct cavan_xml_document *doc);
struct cavan_xml_document *cavan_xml_parse(const char *pathname);
void cavan_xml_document_invert(struct cavan_xml_document *doc);
char *cavan_xml_tostring(struct cavan_xml_document *doc, char *buff, size_t size);
struct cavan_xml_tag *cavan_xml_find_tag(struct cavan_xml_tag *head, const char *name);
struct cavan_xml_attribute *cavan_xml_find_attribute(struct cavan_xml_attribute *attr, const char *name);
bool cavan_xml_remove_tag(struct cavan_xml_tag **head, struct cavan_xml_tag *tag);
int cavan_xml_remove_all_tag_by_name(struct cavan_xml_tag **head, const char *name, bool recursion);
bool cavan_xml_remove_attribute(struct cavan_xml_attribute **head, struct cavan_xml_attribute *attr);
