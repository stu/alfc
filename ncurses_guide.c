#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern int ALFC_main(int, char *s);

int main(int argc, char *argv[])
{
	int i;

	char *view_file = calloc(1, 4096);

	for(i=1; i<argc; i++)
	{
		strcat(view_file, " ");
		strcat(view_file, argv[i]);
	}

	i = ALFC_main(0, view_file);

	free(view_file);

	return i;
}

