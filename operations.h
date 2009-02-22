#ifndef _OPERATIONS_H
#define _OPERATIONS_H
#ifdef __cplusplus
extern "C"{
#endif

extern int Ops_CopyFile(uGlobalData *gd, uFileOperation *op);
extern int Ops_MoveFile(uGlobalData *gd, uFileOperation *op);
extern int Ops_DeleteFile(uGlobalData *gd, uFileOperation *op);
extern int Ops_Symlink(uGlobalData *gd, uFileOperation *op);

#ifdef __cplusplus
}
#endif
#endif // _OPERATIONS_H
