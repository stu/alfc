#ifndef LOGWRITE_H
#define LOGWRITE_H
#ifdef __cplusplus
extern "C"{
#endif

enum LogWrite_InfoType { LOG_INFO = 0x01, LOG_DEBUG = 0x02, LOG_ERROR = 0x04, LOG_STDERR = 0x08 };
#define FLAG_MASK 	(LOG_INFO | LOG_DEBUG | LOG_ERROR | LOG_STDERR)

extern void LogWrite_Shutdown(void);
extern void LogWrite_Startup(int size, int flags, int LogLineCount);
extern void LogWrite_SetFlags(int flags);
extern int LogWrite_GetFlags(void);

#define	LogDebug(...) _LogWrite(__FILE__, __LINE__, LOG_DEBUG, __VA_ARGS__)
#define	LogError(...) _LogWrite(__FILE__, __LINE__, LOG_ERROR, __VA_ARGS__)
#define	LogInfo(...) _LogWrite(NULL, __LINE__, LOG_INFO, __VA_ARGS__)

extern void _LogWrite(char *file, long line, int flags, char *strX, ...);


#ifdef __cplusplus
}
#endif
#endif
