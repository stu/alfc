#include "headers.h"
#include <utime.h>

int CopyFile(uGlobalData *gd, uFileOperation *x)
{
	struct stat statbuff;
	struct stat ostatbuff;
	struct utimbuf timebuff;

	char *buff;

	char *src;
	char *dst;

	int ifd;
	int ofd;

	int flag;
	size_t offset;

	buff = malloc(16 * 1024);
	if(buff == NULL)
	{
		x->result_code = -1;
		x->result_msg = strdup("Not enough memory for buffer");
		return x->result_code;
	}

	src = malloc( strlen(x->op.udtCopy.source_path) + strlen(x->op.udtCopy.source_filename) + 8);
	strcpy(src, x->op.udtCopy.source_path);
	strcat(src, "/");
	strcat(src, x->op.udtCopy.source_filename);

	dst = malloc( strlen(x->op.udtCopy.dest_path) + strlen(x->op.udtCopy.dest_filename) + 8);
	strcpy(dst, x->op.udtCopy.dest_path);
	strcat(dst, "/");
	strcat(dst, x->op.udtCopy.dest_filename);

	// same?
	if(strcmp(src, dst) == 0)
	{
		x->result_code = -1;
		x->result_msg = strdup("Source and Destination are the same");
		free(src);
		free(dst);
		free(buff);
		return x->result_code;
	}

	// make sure we are in the path of the file to copy and its valid
	if(lstat(src ,&statbuff) == -1)
	{
		x->result_code = errno;
		x->result_msg = strdup(strerror(x->result_code));
		free(src);
		free(dst);
		free(buff);
		return x->result_code;
	}

	// parse through the symlink?
	if (S_ISLNK(statbuff.st_mode))
	{
		x->result_code = -1;
		x->result_msg = strdup("Symlinks not yet handled.");
		free(src);
		free(dst);
		free(buff);
		return x->result_code;
	}

	// directory copy
	if (S_ISDIR(statbuff.st_mode))
	{
		x->result_code = -1;
		x->result_msg = strdup("Directory copies not yet handled.");
		free(src);
		free(dst);
		free(buff);
		return x->result_code;
	}

	// non-regular copy
	if (!S_ISREG(statbuff.st_mode))
	{
		x->result_code = -1;
		x->result_msg = strdup("File is not a regular file.");
		free(src);
		free(dst);
		free(buff);
		return x->result_code;
	}

	// test if destination already exists
	if(lstat(dst ,&ostatbuff) == 0)
	{
		x->result_code = -1;
		x->result_msg = strdup("Destination already exists");
		free(src);
		free(dst);
		free(buff);
		return x->result_code;
	}

	if((ifd = open(src, O_RDONLY)) < 0)
	{
		x->result_code = errno;
		x->result_msg = strdup(strerror(x->result_code));
		free(src);
		free(dst);
		free(buff);
		return x->result_code;
	}

	//if((ofd = open(dst, O_WRONLY | O_CREAT | O_TRUNC, statbuff.st_mode)) < 0)
	if((ofd = open(dst, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | statbuff.st_mode)) < 0)
	{
		x->result_code = errno;
		x->result_msg = strdup(strerror(x->result_code));
		close(ifd);
		free(src);
		free(dst);
		free(buff);
		return x->result_code;
	}


	flag = 0;
	offset = 0;
	while(flag == 0)
	{
		int size;

		uint32_t input_crc;
		uint32_t output_crc;

		if(statbuff.st_size > 16*1024)
			size = 16*1024;
		else
			size = statbuff.st_size;

		memset(buff, 0x0, 16*1024);

		if( read(ifd, buff, size) != size )
		{
			x->result_code = -1;
			x->result_msg = strdup("Error in reading input file");

			close(ifd);
			close(ofd);
			unlink(dst);
			free(src);
			free(dst);
			free(buff);
			return x->result_code;
		}

		input_crc = fletcher32((uint16_t*)buff, (1+size) / 2);

		if( write(ofd, buff, size) != size)
		{
			x->result_code = -1;
			x->result_msg = strdup("Error in writing to output file");

			close(ifd);
			close(ofd);
			unlink(dst);
			free(src);
			free(dst);
			free(buff);
			return x->result_code;
		}

		lseek(ofd, offset, SEEK_SET);
		memset(buff, 0x0, 16*1024);

		if(read(ofd, buff, size) != size)
		{
			x->result_code = -1;
			x->result_msg = strdup("Error in writing to output file");

			close(ifd);
			close(ofd);
			unlink(dst);
			free(src);
			free(dst);
			free(buff);
			return x->result_code;
		}

		output_crc = fletcher32((uint16_t*)buff, (size+1) / 2);

		if(input_crc != output_crc)
		{
			x->result_code = -1;
			x->result_msg = strdup("Failed CRC Match");

			close(ifd);
			close(ofd);
			unlink(dst);
			free(src);
			free(dst);
			free(buff);
			return x->result_code;
		}

		offset += size;
		statbuff.st_size -= size;
		if(statbuff.st_size == 0)
			flag = 1;
	}

	close(ifd);

	fchmod(ofd, statbuff.st_mode);
	fchown(ofd, statbuff.st_uid, statbuff.st_gid);
	close(ofd);

	timebuff.actime = statbuff.st_atime;
	timebuff.modtime = statbuff.st_mtime;
	utime(dst, &timebuff);

	free(src);
	free(dst);
	free(buff);

	x->result_code = 0;
	x->result_msg = strdup("OK");

	return x->result_code;
}

int MoveFile(uGlobalData *gd, uFileOperation *x)
{
	return 0;
}

int DeleteFile(uGlobalData *gd, uFileOperation *x)
{
	return 0;
}
