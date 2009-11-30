#ifndef GUIDEHEADER_H_
#define GUIDEHEADER_H_
#ifdef __cplusplus
extern "C"
{
#endif

#define GUIDE_MAGIC				0x284C

enum HELP_F_ATTRS
{
	HLP_F_EMPH = 0x01,
	HLP_F_LINK
};

enum F_RECORD
{
	HLP_TITLE = 0x01,
	HLP_AUTHOR,
	HLP_VERSION,

	HLP_SECTION,
	HLP_LINE,
	HLP_LINE_GZ = HLP_LINE + 0x20,
};

typedef struct udtHelpPage
{
	char *name;

	int width;
	int line_count;
	uint16_t **lines;

	int link_count;

	struct udtHelpLink
	{
		int id;
		int row;
		int col;
		int length;
	} **_links;

} uHelpPage;

typedef struct udtHelpSection
{
	char *name;
	DList *lstLines;
} uHelpSection;

typedef struct udtHelpFile
{
	char *title;
	char *author;
	char *revision;

	DList *lstSections;
} uHelpFile;

#ifdef __cplusplus
}
#endif
#endif /* GUIDEHEADER_H_ */
