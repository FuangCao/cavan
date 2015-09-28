#include <cavan.h>
#include <cavan/dd.h>
#include <cavan/parser.h>
#include <cavan/text.h>

int main(int argc, char *argv[])
{
	int i;
	int ret;
	char input_file[100];
	char output_file[100];
	off_t bs = 1, seek = 0, skip = 0, count = 0;

	assert(argc >= 3);

	for (i = 1; i < argc; i++) {
		char c, *p;

		parse_parameter(argv[i]);

		c = para_option[0];
		p = para_option + 1;

		switch (c) {
		case 'i':
			if (strcmp(p, "f") == 0) {
				strcpy(input_file, para_value);
			} else {
				goto out_unknown_option;
			}
			break;
		case 'o':
			if (strcmp(p, "f") == 0) {
				strcpy(output_file, para_value);
			} else {
				goto out_unknown_option;
			}
			break;
		case 'b':
			if (strcmp(p, "s") == 0) {
				bs = text2size(para_value, NULL);
			} else {
				goto out_unknown_option;
			}
			break;
		case 's':
			if (strcmp(p, "kip") == 0) {
				skip = text2size(para_value, NULL);
			} else if (strcmp(p, "eek") == 0) {
				seek = text2size(para_value, NULL);
			} else {
				goto out_unknown_option;
			}
			break;
		case 'c':
			if (strcmp(p, "ount") == 0) {
				count = text2size(para_value, NULL);
			} else {
				goto out_unknown_option;
			}
			break;
		default:
			goto out_unknown_option;
		}
	}

	ret = cavan_dd(input_file, output_file, skip * bs, seek * bs, count * bs);
	if (ret < 0) {
		error_msg("cavan_dd");
		return ret;
	}

	return 0;

out_unknown_option:
	error_msg("unknown option \"%s\"", para_option);
	return -EINVAL;
}
