#include <cavan.h>
#include <cavan/dd.h>
#include <cavan/parse.h>
#include <cavan/text.h>

int main(int argc, char *argv[])
{
	int i;
	int ret;
	char input_file[100];
	char output_file[100];
	off_t bs = 1, seek = 0, skip = 0, count = 0;

	assert(argc >= 3);

	for (i = 1; i < argc; i++)
	{
		parse_parameter(argv[i]);

		if (strcmp(para_option, "if") == 0)
		{
			strcpy(input_file, para_value);
		}
		else if (strcmp(para_option, "of") == 0)
		{
			strcpy(output_file, para_value);
		}
		else if (strcmp(para_option, "bs") == 0)
		{
			bs = text2size(para_value);
		}
		else if (strcmp(para_option, "seek") == 0)
		{
			seek = text2size(para_value);
		}
		else if (strcmp(para_option, "skip") == 0)
		{
			skip = text2size(para_value);
		}
		else if (strcmp(para_option, "count") == 0)
		{
			count = text2size(para_value);
		}
		else
		{
			error_msg("unknown option \"%s\"", para_option);
			return -1;
		}
	}

	ret = cavan_dd(input_file, output_file, skip * bs, seek * bs, count * bs);
	if (ret < 0)
	{
		error_msg("cavan_dd");
		return ret;
	}

	return ret;
}
