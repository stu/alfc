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
	HLP_F_LINK1 = 0x02,
	HLP_F_LINK2 = 0x04,
	HLP_F_BOLD = 0x08
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

typedef struct udtHelpLink
{
	int id;
	int row;
	int col;
	int length;
} uHelpLink, *pHelpLink;

typedef struct udtHelpPage
{
	char *name;

	int width;
	int line_count;
	uint16_t **lines;

	int displayed_page_link_count;
	int highlight_link;

	int link_count;
	uHelpLink **_links;
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
