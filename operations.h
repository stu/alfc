#ifndef _OPERATIONS_H
#define _OPERATIONS_H
#ifdef __cplusplus
extern "C"{
#endif

extern int CopyFile(uGlobalData *gd, uFileOperation *op);
extern int MoveFile(uGlobalData *gd, uFileOperation *op);
extern int DeleteFile(uGlobalData *gd, uFileOperation *op);

#ifdef __cplusplus
}
#endif
#endif // _OPERATIONS_H
