#pragma once

/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Mon Oct 15 15:29:08 CST 2012
 */

#include <cavan.h>

#define CAVAN_XML_FLAG_NAME_ALLOC			(1 << 0)
#define CAVAN_XML_FLAG_VALUE_ALLOC			(1 << 1)
#define CAVAN_XML_FLAG_CONTENT_ALLOC		(1 << 2)
#define CAVAN_XML_FLAG_CONTENT_MULTI_LINE	(1 << 2)

#define pr_parser_error_info(lineno, fmt, args ...) \
	pr_red_info("(" fmt ") at line[%d]", ##args, lineno)

typedef enum cavan_xml_token
{
	CAVAN_XML_TOKEN_NONE,
	CAVAN_XML_TOKEN_TAG_ATTR,
	CAVAN_XML_TOKEN_TAG_SINGLE,
	CAVAN_XML_TOKEN_TAG_BEGIN,
	CAVAN_XML_TOKEN_TAG_END,
	CAVAN_XML_TOKEN_COMMENT,
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
	struct cavan_xml_tag *attr;
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

	cavan_xml_token_t token;
	cavan_xml_token_t prev_token;
	cavan_xml_token_t next_token;
};

struct cavan_xml_attribute *cavan_xml_attribute_alloc(char *name, char *value, int flags);
void cavan_xml_attribute_free(struct cavan_xml_attribute *attr);
struct cavan_xml_attribute *cavan_xml_attribute_find(struct cavan_xml_attribute *head, const char *name);
bool cavan_xml_attribute_set_name(struct cavan_xml_attribute *attr, char *name, int flags);
bool cavan_xml_attribute_set_value(struct cavan_xml_attribute *attr, char *value, int flags);
bool cavan_xml_attribute_set(struct cavan_xml_attribute **head, char *name, char *value, int flags);
bool cavan_xml_attribute_remove(struct cavan_xml_attribute **head, struct cavan_xml_attribute *attr);

struct cavan_xml_tag *cavan_xml_tag_alloc(char *name);
void cavan_xml_tag_free(struct cavan_xml_tag *tag);
struct cavan_xml_tag *cavan_xml_tag_find(struct cavan_xml_tag *head, const char *name);
bool cavan_xml_tag_remove(struct cavan_xml_tag **head, struct cavan_xml_tag *tag);
int cavan_xml_tag_remove_all_by_name(struct cavan_xml_tag **head, const char *name, bool recursion);

struct cavan_xml_document *cavan_xml_document_alloc(void);
void cavan_xml_document_free(struct cavan_xml_document *doc);
void cavan_xml_document_invert(struct cavan_xml_document *doc);
struct cavan_xml_document *cavan_xml_document_parse(const char *pathname);
char *cavan_xml_document_tostring(struct cavan_xml_document *doc, char *buff, size_t size);
