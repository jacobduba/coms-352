#include <stdio.h>
#include "encrypt-module.h"

void reset_requested()
{
	log_counts();
}

void reset_finished()
{
}

int main(int argc, char *argv[])
{
	int input_buffer_size, output_buffer_size;
	char c;

	if (argc != 4)
	{
		printf("Error: incorrect number of params.\nUsage: ./encrypt <input> <output> <log>\n");
		return 1;
	}

	printf("Enter input buffer size: ");
	scanf("%d", &input_buffer_size);

	printf("Enter output buffer size: ");
	scanf("%d", &output_buffer_size);

	if (input_buffer_size < 2 || output_buffer_size < 2)
	{
		printf("Error: input and output buffers must be >1.\n");
		return 1;
	}

	init(argv[1], argv[2], argv[3]);

	while ((c = read_input()) != EOF)
	{
		count_input(c);
		c = encrypt(c);
		count_output(c);
		write_output(c);
	}

	printf("End of file reached.\n");

	log_counts();

	return 0;
}
