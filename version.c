#include <stdint.h>
#include "version.h"

const char* VersionTime(void)
{
return __TIME__;
}
const char* VersionDate(void)
{
return __DATE__;
}
uint16_t VersionMajor(void)
{
	return 0;
}

uint16_t VersionMinor(void)
{
	return 1;
}

uint16_t VersionBuild(void)
{
	return 9158;
}

const char* VersionString(void)
{
	return (char*)"v0.01/9158";
}

