#include "headers.h"

static int			intLogFlags;
static char		*strLogBuffer;
static DList		*lstLog;
static int			intLineCount;

void LogWrite_SetFlags(int flags)
{
	intLogFlags = flags & FLAG_MASK;
}

int LogWrite_GetFlags(void)
{
	return intLogFlags;
}

static void FreeLog(void *x)
{
	free(x);
}

void LogWrite_Shutdown(void)
{
	DLElement *e;

	if(strLogBuffer  != NULL)
		free(strLogBuffer);

	strLogBuffer = NULL;

	e = dlist_head(lstLog);
	while(e != NULL)
	{
		char *x;

		x = dlist_data(e);
		e = dlist_next(e);
		fprintf(stderr, "%s", x);
	}

	dlist_destroy(lstLog);
	free(lstLog);
}

void LogWrite_Startup(int size, int flags, int LogLineCount)
{
	if(size < 1024)
		size = 1024;

	intLogFlags = flags;

	strLogBuffer = malloc(size);
	assert(strLogBuffer!=NULL);

	lstLog = malloc(sizeof(DList));
	dlist_init(lstLog, FreeLog);

	intLineCount = LogLineCount;
}

static void DropLogLine(DList *lst)
{
	char *x;

	dlist_remove(lst, dlist_head(lst), (void*)&x);
	free(x);
}

void _LogWrite(char *file, long line, int flags, char *strX, ...)
{
	va_list args;
	char *x;

	if(strLogBuffer == NULL)
		return;

	if( ((intLogFlags&~LOG_STDERR) & flags) != flags)
		return;

	if(strX != NULL)
	{
		va_start(args, strX);
		vsprintf(strLogBuffer, strX, args);
		va_end(args);
	}
	else
	{
		strLogBuffer = "";
	}

	x = malloc(4096);

	if(file != NULL)
	{
		sprintf(x, "%s(%li) : %s", file, line, strLogBuffer);
	}
	else
	{
		sprintf(x, "%s", strLogBuffer);
	}

	if( (flags & LOG_STDERR) == LOG_STDERR)
		fprintf(stderr, "%s", x);

	if(intLineCount > 0 && lstLog != NULL)
	{
		x = realloc(x, strlen(x) + 4);
		dlist_ins(lstLog, x);
		if(dlist_size(lstLog) > intLineCount)
			DropLogLine(lstLog);
	}

	fflush(stderr);
}
