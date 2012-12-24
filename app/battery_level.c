// Fuang.Cao <cavan.cfa@gmail.com> Thu Feb 24 18:50:40 CST 2011

#include <cavan.h>
#include <cavan/text.h>

#define MAX_VOLTAGE		4120
#define MIN_VOLTAGE		3400

int main(int argc, char *argv[])
{
	int voltage;

	assert(argc == 2);

	voltage = text2value_unsigned(argv[1], NULL, 10);

	println("Voltage = %d, Level = %d", voltage, (voltage - MIN_VOLTAGE) * 100 / (MAX_VOLTAGE - MIN_VOLTAGE));

	return 0;
}
