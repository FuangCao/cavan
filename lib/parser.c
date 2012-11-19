#include <cavan.h>
#include <cavan/parser.h>
#include <cavan/file.h>

char para_option[128];
char para_value[128];

char *get_parameter(char *argument, char *parameter)
{
	while (1)
	{
		switch (argument[0])
		{
		case ' ':
		case '\0':
		case '\t':
		case '\n':
			parameter[0] = '\0';
			return argument;

		default:
			*parameter++ = *argument++;
		}
	}

	return argument;
}

void parse_parameter_base(const char *parameter, char *option, char *value)
{
	value[0] = '\0';

	while (1)
	{
		switch (parameter[0])
		{
		case '=':
			option[0] = '\0';
			option = value;
			break;

		case ' ':
		case '\0':
		case '\t':
		case '\n':
			option[0] = '\0';
			return;

		default:
			*option++ = *parameter;
		}

		parameter++;
	}
}

struct buffer *malloc_buffer(int size)
{
	struct buffer *buff;

	buff = malloc(sizeof(*buff) + size);
	if (buff == NULL)
	{
		return NULL;
	}

	buff->size = size;
	buff->cursor = 0;

	return buff;
}

void free_buffer(struct buffer *buff)
{
	free(buff);
}

struct buffer *read_lines(const char *file_path)
{
	struct stat st;
	struct buffer *buff;
	int ret;
	int fd;

	fd = open(file_path, O_RDONLY | O_BINARY);
	if (fd < 0)
	{
		perror("open");
		return NULL;
	}

	ret = fstat(fd, &st);
	if (ret < 0)
	{
		perror("fstat");
		goto out_close_file;
	}

	buff = malloc_buffer(st.st_size);
	if (buff == NULL)
	{
		perror("malloc_buffer");
		goto out_close_file;
	}

	ret = ffile_read(fd, buff->space, buff->size);
	if (ret < 0)
	{
		perror("ffile_read");
		goto out_free_buff;
	}

	close(fd);

	return buff;

out_free_buff:
	free_buffer(buff);
out_close_file:
	close(fd);

	return NULL;
}

int write_lines(const char *file_path, struct buffer *buff)
{
	return file_writeto(file_path, buff->space, buff->size, 0, O_TRUNC);
}

char *find_next_line(struct buffer *buff, char *line)
{
	char *end_line = buff->space + buff->size;

	while (line < end_line && *line++ != '\n');

	if (line < end_line)
	{
		return line;
	}
	else
	{
		return NULL;
	}
}

int get_next_line(struct buffer *buff, char *line)
{
	char *p, *end_p;
	int count;

	if (buff->cursor >= buff->size)
	{
		return -1;
	}

	p = buff->space + buff->cursor;
	end_p = buff->space + buff->size;

	while (p < end_p && *p != '\n')
	{
		*line++ = *p++;
	}

	count =  p - buff->space - buff->cursor;
	buff->cursor += count + 1;

	*line = 0;

	return count;
}

int get_index_line(struct buffer *buff, int index, char *line)
{
	char *p = buff->space;
	char *q = line;
	char *end_p = buff->space + buff->size;

	index--;

	while (p < end_p && index > 0)
	{
		if (*p == '\n')
		{
			index--;
		}

		p++;
	}

	while (p < end_p && *p != '\n')
	{
		*q++ = *p++;

	}

	return q - line;
}

char *find_prefix_line(struct buffer *buff, const char *prefix, int prefix_size)
{
	int count;
	char *p = buff->space;
	char *end_p = p + buff->size;

	while (is_empty_character(*prefix) && prefix_size)
	{
		prefix++;
		prefix_size--;
	}

	if (prefix_size == 0)
	{
		return NULL;
	}

	end_p -= prefix_size;

	while (1)
	{
		for (count = 0; p < end_p && is_empty_character(*p); p++, count++);

		if (p >= end_p)
		{
			return NULL;
		}

		if (memcmp(p, prefix, prefix_size) == 0)
		{
			return p - count;
		}

		p = find_next_line(buff, p);
		if (p == NULL)
		{
			return NULL;
		}
	}

	return NULL;
}

int get_prefix_line(struct buffer *buff, const char *prefix, int prefix_size, char *line)
{
	char *p, *end_p, *q;

	p = find_prefix_line(buff, prefix, prefix_size);
	if (p == NULL)
	{
		return -1;
	}

	end_p = buff->space + buff->size;
	q = line;

	while (p < end_p && *p != '\n')
	{
		*q++ = *p++;
	}

	return q - line;
}

char *mem_area_copy(char *dest, const char *src, const char *src_end)
{
	while (src <= src_end)
	{
		*(char *)dest++ = *(char *)src++;
	}

	return dest;
}

struct buffer *replace_prefix_line(struct buffer *buff, const char *prefix, int prefix_size, const char *new_line, int new_line_size)
{
	char *p, *end_p, *q;
	struct buffer *new_buff;

	p = find_prefix_line(buff, prefix, prefix_size);
	if (p == NULL)
	{
		return NULL;
	}

	end_p = find_next_line(buff, p);
	if (end_p == NULL)
	{
		new_buff = malloc_buffer(buff->size + new_line_size + 1);
	}
	else
	{
		new_buff = malloc_buffer(buff->size - (end_p - p) + new_line_size + 1);
	}

	if (new_buff == NULL)
	{
		return NULL;
	}

	q = mem_area_copy(mem_area_copy(new_buff->space, buff->space, p - 1), new_line, new_line + new_line_size - 1);

	if (end_p == NULL)
	{
		return new_buff;
	}

	*q++ = '\n';
	mem_area_copy(q, end_p, buff->space + buff->size - 1);

	return new_buff;
}

ssize_t parse_config_file(const char *buff, size_t bufflen, char sep, struct equation *lines, size_t count)
{
	const char *buff_tmp, *buff_end = buff + bufflen;
	struct equation *line_end = lines + count, *line_bak = lines;
	char *p, *p_bak;

	while (buff < buff_end && lines < line_end)
	{
		buff_tmp = buff;

		while (buff_tmp < buff_end && *buff_tmp != '\n')
		{
			buff_tmp++;
		}

		while (buff < buff_tmp && text_has_char(" \t\r", *buff))
		{
			buff++;
		}

		if (*buff == '#')
		{
			goto label_goto_next_line;
		}

		p_bak = p = lines->option;

		while (buff < buff_tmp && *buff != sep)
		{
			*p++ = *buff++;
		}

		if (p == p_bak)
		{
			goto label_goto_next_line;
		}

		while (--p >= p_bak && text_has_char(" \t\r", *p));

		p[1] = 0;
		buff++;

		while (buff < buff_tmp && text_has_char(" \t\r", *buff))
		{
			buff++;
		}

		p_bak = p = lines->value;

		while (buff < buff_tmp && *buff != '\n')
		{
			*p++ = *buff++;
		}

		if (p == p_bak)
		{
			goto label_goto_next_line;
		}

		while (--p >= p_bak && text_has_char(" \t\r", *p));

		p[1] = 0;

		// println("%s = %s", lines->option, lines->value);
		lines++;

label_goto_next_line:
		buff = buff_tmp + 1;
	}

	return lines - line_bak;
}

ssize_t parse_config_file2(const char *filepath, size_t max_size, char sep, struct equation *lines, size_t count)
{
	char buff[max_size];
	ssize_t readlen;

	readlen = file_read(filepath, buff, sizeof(buff));
	if (readlen < 0)
	{
		print_error("read file \"%s\"", filepath);
		return readlen;
	}

	return parse_config_file(buff, readlen, sep, lines, count);
}

