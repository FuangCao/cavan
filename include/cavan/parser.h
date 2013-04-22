#pragma once

#include <cavan.h>

struct buffer
{
	int size;
	int cursor;
	char space[0];
};

struct equation
{
	char option[64];
	char value[1024];
};

extern char para_option[128];
extern char para_value[128];

char *get_parameter(char *argument, char *parameter);
void parse_parameter_base(const char *parameter, char *option, char *value);

struct buffer *malloc_buffer(int size);
void free_buffer(struct buffer *buff);
struct buffer *read_lines(const char *file_path);
int write_lines(const char *file_path, struct buffer *buff);
int get_next_line(struct buffer *buff, char *line);
int get_index_line(struct buffer *buff, int index, char *line);

char *find_prefix_line(struct buffer *buff, const char *prefix, int prefix_size);
int get_prefix_line(struct buffer *buff, const char *prefix, int prefix_size, char *line);
char *find_next_line(struct buffer *buff, char *line);
char *mem_area_copy(char *dest, const char *src, const char *src_end);
struct buffer *replace_prefix_line(struct buffer *buff, const char *prefix, int prefix_size, const char *new_line, int new_line_size);

ssize_t parse_config_file(const char *buff, size_t bufflen, char sep, struct equation *line, size_t count);
ssize_t parse_config_file2(const char *filepath, size_t max_size, char sep, struct equation *lines, size_t count);

static inline void parse_parameter(const char *parameter)
{
	parse_parameter_base(parameter, para_option, para_value);
}
