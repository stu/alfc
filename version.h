#ifndef VERSION_H
#define VERSION_H

#ifdef __cplusplus
extern "C"{
#endif

extern uint16_t VersionMajor(void);
extern uint16_t VersionMinor(void);
extern uint16_t VersionBuild(void);
extern const char* VersionString(void);
extern const char* VersionTime(void);
extern const char* VersionDate(void);

#ifdef __cplusplus
};
#endif
#endif

