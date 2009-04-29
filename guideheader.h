#ifndef GUIDEHEADER_H_
#define GUIDEHEADER_H_
#ifdef __cplusplus
extern "C"
{
#endif

#define GUIDE_MAGIC				0x284C

#define F_ENCRYPTED				0x1000

enum F_COMP
{
	F_NONE = 0,
	F_RLE_COMPRESSION,
	F_LZSS_COMPRESSION,
	F_GZIP_COMPRESSION,
	F_BZIP2_COMPRESSION,
	F_LZMA_COMPRESSION
};

#define F_COMPRESSION_MASK		0x0007

#pragma pack(1)
struct udtODS_GuideEntry
{
	uint8_t header_count;
	uint16_t length;
	uint16_t orig_length;
};

struct udtODS_GuideHeader
{
	uint16_t magic;
	uint16_t version;
	uint16_t flags;


	// flat pointers to null terminated utf-8 strings
	uint16_t meta_size;
	uint32_t meta_offset;

	uint16_t node_count;
	uint32_t node_offset;

	uint16_t header_size;
};
#pragma pack()

struct udtIM_GuideLink
{
	int row;
	int col;

	int	node_count;
	char **nodes;

	char *canonical_link;
	char *link;
};
typedef struct udtIM_GuideLink uIM_GuideLink;

struct udtIM_GuideHeader
{
	char *title;
	char *author;
	char *revision;
	int version;			// only valid when loaded from disk
	int flags;				// only valid when loaded from disk
	DList *lstNodes;
};

typedef struct udtIM_GuideHeader uIM_GuideHeader;

struct udtIM_Node
{
	int node_count;
	char **nodes;

	int length;
	uint8_t *data;
};

typedef struct udtIM_Node uIM_Node;

#define eLF_None 		0x00
#define eLF_Bold		0x01
#define eLF_Underline 	0x02
#define eLF_Reverse		0x04
#define eLF_Fixed		0x08

#define eLF_FIXMASK	0xFF00

struct udtIM_GuideLine
{
	int flags;
	int length;
	uint8_t *text;
};
typedef struct udtIM_GuideLine uIM_GuideLine;

struct udtIM_GuidePage
{
	DList *lstLines;
	DList *lstLinks;
};
typedef struct udtIM_GuidePage uIM_GuidePage;


#ifdef __cplusplus
}
#endif
#endif /* GUIDEHEADER_H_ */
