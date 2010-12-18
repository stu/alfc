#include "headers.h"

static void header(void)
{
	fprintf(stderr, "ALFC v%i.%02i/%04i :: Built %s\n", VersionMajor(), VersionMinor(), VersionBuild(), VersionDate());
}

int rlmain(int argc, char *argv[])
{
	int i;
	int start_mode = eMode_Directory;
	char *view_file = NULL;


	for(i=0; i < argc; i += 1)
	{
		if(argv[i][0] == '-')
		{
			if(1+i < argc && strcmp("-l", argv[i]) == 0)
			{
				start_left = argv[i+1];
				i += 1;
			}
			else if(1+i < argc && strcmp("-r", argv[i]) == 0)
			{
				start_right = argv[i+1];
				i += 1;
			}
			else if(strcmp("-?", argv[i]) == 0 || strcmp("--help", argv[i]) == 0)
			{
				header();
				fprintf(stderr, "\n-v\t\tVersion\n"
								"-?\t\tHelp\n"
								"-l DIR\t\tStart left side in directory DIR\n"
								"-r DIR\t\tStart right side in directory DIR\n"
								"-view FILE\tStart in viewer mode\n"
								"-edit FILE\tStart in edit mode\n"
								);
				exit(0);
			}
			else if(strcmp("-v", argv[i]) == 0 || strcmp("--version", argv[i]) == 0)
			{
				header();
				exit(0);
			}
			else if(strcmp("-view", argv[i]) == 0 && 1 + i < argc)
			{
				view_file = argv[1+i];
				i += 1;
				start_mode = eMode_Viewer;
			}
			else if (strcmp("-edit", argv[i]) == 0 && 1 + i < argc)
			{
				view_file = argv[1 + i];
				i += 1;
				start_mode = eMode_Editor;
			}
		}
	}

	return ALFC_main(start_mode, view_file);
}
