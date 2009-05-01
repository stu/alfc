/****h* ALFC/LuaAPIHash
 * FUNCTION
 *   The C<>Lua interface functions used in the Lua scripts. These functions can
 *   be used by any other lua script regardless of mode.
 *****
 */
#include "headers.h"
#include "md5.h"
#include "rmd160.h"
#include "sha1.h"


/****f* LuaAPIHash/MD5Sum
* FUNCTION
*	This function will MD5 hash a string or file and return a digest.
*
* SYNOPSIS
MD5Sum("string")
* INPUTS
*	o msg (stirng) -- The string (or file) to hash
* RESULTS
*	o digest (string) -- The resulting digest
* AUTHOR
*	Stu George
******
*/
int gme_MD5Sum(lua_State *L)
{
	struct lstr fstring;
	uGlobalData *gd;
	uMD5State state;

	char buff[128];

	gd = GetGlobalData(L);
	assert(gd != NULL);

	GET_LUA_STRING(fstring, 1);


	md5_init(&state);
	md5_update(&state, (uint8_t*)fstring.data, fstring.length);
	md5_final(&state);


	sprintf(buff, "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",

		(state.state[0] >> 0) & 0xFF, (state.state[0] >> 8) & 0xFF, (state.state[0] >> 16) & 0xFF, (state.state[0] >> 24) & 0xFF,
		(state.state[1] >> 0) & 0xFF, (state.state[1] >> 8) & 0xFF, (state.state[1] >> 16) & 0xFF, (state.state[1] >> 24) & 0xFF,
		(state.state[2] >> 0) & 0xFF, (state.state[2] >> 8) & 0xFF, (state.state[2] >> 16) & 0xFF, (state.state[2] >> 24) & 0xFF,
		(state.state[3] >> 0) & 0xFF, (state.state[3] >> 8) & 0xFF, (state.state[3] >> 16) & 0xFF, (state.state[3] >> 24) & 0xFF
		);
		lua_pushstring(L, buff);

	return 1;
}

/****f* LuaAPIHash/RIPEMD160Sum
* FUNCTION
*	This function will RIPEMD-160 hash a string or file and return a digest.
*
* SYNOPSIS
RIPEMD160Sum("string")
* INPUTS
*	o msg (stirng) -- The string (or file) to hash
* RESULTS
*	o digest (string) -- The resulting digest
* AUTHOR
*	Stu George
******
*/
int gme_RIPEMD160Sum(lua_State *L)
{
	struct lstr fstring;
	uGlobalData *gd;

	uint8_t *msg;
	uint32_t nbytes;
	uint32_t X[16];
	uint32_t MDbuf[160/32];
	char buff[512];
	int i;

	gd = GetGlobalData(L);
	assert(gd != NULL);

	GET_LUA_STRING(fstring, 1);
	msg = (uint8_t*)fstring.data;

	MDinit(MDbuf);

	for (nbytes=fstring.length; nbytes > 63; nbytes-=64)
	{
		for (i=0; i<16; i++)
		{
			X[i] = BYTES_TO_DWORD(msg);
			msg += 4;
		}
		compress(MDbuf, X);
	}
	MDfinish(MDbuf, msg, fstring.length, 0);


	sprintf(buff, "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
		(MDbuf[0] >> 0) & 0xFF, (MDbuf[0] >> 8) & 0xFF, (MDbuf[0] >> 16) & 0xFF, (MDbuf[0] >> 24) & 0xFF,
		(MDbuf[1] >> 0) & 0xFF, (MDbuf[1] >> 8) & 0xFF, (MDbuf[1] >> 16) & 0xFF, (MDbuf[1] >> 24) & 0xFF,
		(MDbuf[2] >> 0) & 0xFF, (MDbuf[2] >> 8) & 0xFF, (MDbuf[2] >> 16) & 0xFF, (MDbuf[2] >> 24) & 0xFF,
		(MDbuf[3] >> 0) & 0xFF, (MDbuf[3] >> 8) & 0xFF, (MDbuf[3] >> 16) & 0xFF, (MDbuf[3] >> 24) & 0xFF,
		(MDbuf[4] >> 0) & 0xFF, (MDbuf[4] >> 8) & 0xFF, (MDbuf[4] >> 16) & 0xFF, (MDbuf[4] >> 24) & 0xFF
		);
		lua_pushstring(L, buff);

	return 1;
}

/****f* LuaAPIHash/SHA1Sum
* FUNCTION
*	This function will SHA1 hash a string or file and return a digest.
*
* SYNOPSIS
SHA1Sum("string")
* INPUTS
*	o msg (stirng) -- The string (or file) to hash
* RESULTS
*	o digest (string) -- The resulting digest
* AUTHOR
*	Stu George
******
*/
int gme_SHA1Sum(lua_State *L)
{
	struct lstr fstring;
	uGlobalData *gd;
	SHA1Context sha;

	char buff[512];

	gd = GetGlobalData(L);
	assert(gd != NULL);

	GET_LUA_STRING(fstring, 1);

	SHA1Reset(&sha);
	SHA1Input(&sha, (const uint8_t*)fstring.data, fstring.length);
	SHA1Result(&sha);

	sprintf(buff, "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
		(sha.Message_Digest[0] >> 24) & 0xFF, (sha.Message_Digest[0] >> 16) & 0xFF, (sha.Message_Digest[0] >> 8) & 0xFF, (sha.Message_Digest[0] >> 0) & 0xFF,
		(sha.Message_Digest[1] >> 24) & 0xFF, (sha.Message_Digest[1] >> 16) & 0xFF, (sha.Message_Digest[1] >> 8) & 0xFF, (sha.Message_Digest[1] >> 0) & 0xFF,
		(sha.Message_Digest[2] >> 24) & 0xFF, (sha.Message_Digest[2] >> 16) & 0xFF, (sha.Message_Digest[2] >> 8) & 0xFF, (sha.Message_Digest[2] >> 0) & 0xFF,
		(sha.Message_Digest[3] >> 24) & 0xFF, (sha.Message_Digest[3] >> 16) & 0xFF, (sha.Message_Digest[3] >> 8) & 0xFF, (sha.Message_Digest[3] >> 0) & 0xFF,
		(sha.Message_Digest[4] >> 24) & 0xFF, (sha.Message_Digest[4] >> 16) & 0xFF, (sha.Message_Digest[4] >> 8) & 0xFF, (sha.Message_Digest[4] >> 0) & 0xFF
		);
		lua_pushstring(L, buff);

	return 1;
}

