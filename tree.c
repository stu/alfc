#include "headers.h"

static void FreeTreeChildNode(void *data)
{
	uTree *node = data;

	if (node->strNode != NULL)
		free(node->strNode);

	if (node->strAbsolutePath != NULL)
		free(node->strAbsolutePath);

	dlist_destroy(node->lstChildNodes);
	free(node->lstChildNodes);

	memset(node, 0x0, sizeof(uTree));
	free(node);
}

static uTree* RecursiveBuildTree(uTree *parent, char *src, int intIgnoreHidden)
{
	DIR *d;
	struct dirent *dr;
	struct stat sbuff;
	uTree *tree = NULL;
	uDirEntry *de;
	uint32_t attrs;

	de = malloc(sizeof(uDirEntry));

	if( chdir(src) == 0)
	{
		tree = calloc(1, sizeof(uTree));
		tree->lstChildNodes = malloc(sizeof(DList));
		dlist_init(tree->lstChildNodes, FreeTreeChildNode);

		tree->strAbsolutePath = GetCurrentWorkingDirectory();
		tree->strNode = strdup(src);
		tree->inode = sbuff.st_ino;
		tree->dev = sbuff.st_dev;
		tree->modtime = sbuff.st_mtime;

		LogInfo("%s : %i-%i\n", tree->strNode, tree->dev, tree->inode);

		d = opendir(".");
		if (d != NULL)
		{
			dr = readdir(d);
			while (dr != NULL)
			{
				ALFC_stat(dr->d_name, &sbuff);

				memset(de, 0x0, sizeof(uDirEntry));
				de->attrs = sbuff.st_mode;
				de->path = strdup("");
				de->name = strdup(dr->d_name);
				attrs = ALFC_GetFileAttrs(de);
				free(de->name);
				free(de->path);

				if ((strcmp(dr->d_name, ".") != 0 && strcmp(dr->d_name, "..") != 0))
				{

					// test link to see if its a directory...
#ifndef __WIN32__
					if (ALFC_IsDir(attrs) == 0 && S_ISLNK(attrs) == 0 && (S_ISBLK(attrs) == 0 && S_ISCHR(attrs) == 0) && S_ISFIFO(attrs) == 0 && S_ISSOCK(attrs) == 0)
#else
					if (ALFC_IsDir(attrs) == 0 && S_ISREG(attrs) == 0)
#endif
					{
						uTree *cnode;
						char *cdir;

						if(intIgnoreHidden == 0 || ALFC_IsHidden(dr->d_name, attrs) != 0)
						{
							cdir = GetCurrentWorkingDirectory();
							cnode = RecursiveBuildTree(tree, dr->d_name, intIgnoreHidden);
							chdir(cdir);
							free(cdir);

							if (cnode != NULL)
								dlist_ins(parent->lstChildNodes, cnode);
						}
					}
				}
				dr = readdir(d);
			}
		}
		else
			LogInfo("could not do opendir in %s\n", src);

		closedir(d);
	}
	else
		LogInfo("Not a directory %s, or no valid read rights\n", src);

	free(de);

	return parent;
}

//BuildTree("/home/sgeorge", IsTrue(INI_get(gdata->optfile, "options", "show_hidden")));
/* pass in with base entry, eg "sgeorge" and not "/home/sgeorge" */
uTree* BuildTree(char *src, int intIgnoreHidden)
{
	char *xdir;
	uTree *tree;

	// save current directory state
	xdir = GetCurrentWorkingDirectory();

	tree = calloc(1, sizeof(uTree));
	tree->lstChildNodes = malloc(sizeof(DList));
	dlist_init(tree->lstChildNodes, FreeTreeChildNode);
	tree->strAbsolutePath = strdup(xdir);
	tree->inode = 0;
	tree->dev = 0;

	tree = RecursiveBuildTree(tree, src, intIgnoreHidden);

	// return to original state
	chdir(xdir);

	return tree;
}
