#include "headers.h"

uint16_t VersionMajor(void)
{
	return VMAJ;
}

uint16_t VersionMinor(void)
{
	return VMIN;
}

uint16_t VersionBuild(void)
{
	return VBUILD;
}

char* VersionDate(void)
{
	return __DATE__;
}

char* VersionTime(void)
{
	return __TIME__;
}


