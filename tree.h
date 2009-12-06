#ifndef TREE_H_
#define TREE_H_
#ifdef __cplusplus
extern "C"
{
#endif

typedef struct udtTree
{
	char *strAbsolutePath;
	char *strNode;

	ino_t inode;
	dev_t dev;
	time_t modtime;

	DList *lstChildNodes;
} uTree;

extern uTree* BuildTree(char *src, int intIgnoreHidden);

#ifdef __cplusplus
}
#endif
#endif /* TREE_H_ */
