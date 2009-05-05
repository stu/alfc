#include "headers.h"
#include <unistd.h>
#include <utime.h>

#define BLOCK_SIZE		(1024*16)

static void OpWindowPercentageBuffer(uWindow *w, uint64_t total, uint64_t readin)
{
	char buffx[128]; // never exceeds 80
	int l = w->width - 2;
	int pct;
	int q;

	memset(buffx, '-', l);

	if (total == 0)
		total = 1;

	// 10 in 200
	pct = (readin * 100) / total;
	q = (pct * l) / 100;

	memset(buffx, 'X', q);
	buffx[l] = 0;

	w->gd->screen->set_style(STYLE_HIGHLIGHT);
	w->gd->screen->set_cursor(3 + w->offset_row, 2 + w->offset_col);
	w->gd->screen->print_abs(buffx);
	w->gd->screen->set_style(STYLE_TITLE);
	w->gd->screen->update_window();
}

static char* build_fn(char *p, char *f)
{
	char *z;

	z = malloc( strlen(p) + strlen(f) + 8);
	assert(z != NULL);

	strcpy(z, p);
	if (z[strlen(z)] != '/')
		strcat(z, "/");
	strcat(z, f);

	return z;
}

int Ops_Symlink(uGlobalData *gd, uFileOperation *x, uWindow *w)
{
	struct stat statbuff;
	char *src;
	char *dst;

	src = build_fn(x->op.udtSymlink.source_path, x->op.udtSymlink.source_filename);
	dst = build_fn(x->op.udtSymlink.dest_path, x->op.udtSymlink.dest_filename);


	// make sure we are in the path of the file to copy and its valid
	if (ALFC_stat(src, &statbuff) == -1)
	{
		x->result_code = -1;
		x->result_msg = strdup(strerror(errno));
		free(src);
		return x->result_code;
	}

#ifndef __WIN32__
	// parse through the symlink?
	if (S_ISLNK(statbuff.st_mode))
	{
		x->result_code = -1;
		x->result_msg = strdup("Symlinks not yet handled.");
		free(src);
		return x->result_code;
	}

	x->op.udtSymlink.source_length = statbuff.st_size;


	// GO!
	if (symlink(src, dst) != 0)
	{
		x->result_code = -1;
		x->result_msg = strdup(strerror(errno));
		free(src);
		return x->result_code;
	}

	x->result_code = 0;
	x->result_msg = strdup("OK");
#else
	x->result_code = -1;
	x->result_msg = strdup("UNSUPPORTED ON WIN32");
#endif

	free(src);
	free(dst);

	return x->result_code;
}

int Ops_CopyFile(uGlobalData *gd, uFileOperation *x, uWindow *w)
{
	struct stat statbuff;
	struct stat ostatbuff;
	struct utimbuf timebuff;

	uint8_t *buff;
	uint8_t *cbuff;

	char *src;
	char *dst;


#ifndef __WIN32__
	int ifd;
	int ofd;
#else
	FILE *ifp;
	FILE *ofp;
#endif

	int flag;
	size_t offset;

	buff = malloc(16+BLOCK_SIZE);
	if (buff == NULL)
	{
		x->result_code = -1;
		x->result_msg = strdup("Not enough memory for buffer");
		return x->result_code;
	}

	cbuff = malloc(16+BLOCK_SIZE);
	if (cbuff == NULL)
	{
		free(buff);
		x->result_code = -1;
		x->result_msg = strdup("Not enough memory for buffer");
		return x->result_code;
	}

	src = build_fn(x->op.udtCopy.source_path, x->op.udtCopy.source_filename);
	dst = build_fn(x->op.udtCopy.dest_path, x->op.udtCopy.dest_filename);


	//LogInfo("Copy %s to %s\n", src, dst);

	// same?
	if (strcmp(src, dst) == 0)
	{
		x->result_code = -1;
		x->result_msg = strdup("Source and Destination are the same");
		free(src);
		free(dst);
		free(buff);
		free(cbuff);
		return x->result_code;
	}

	// make sure we are in the path of the file to copy and its valid
	if (ALFC_stat(src, &statbuff) == -1)
	{
		x->result_code = -1;
		x->result_msg = strdup(strerror(errno));
		free(src);
		free(dst);
		free(buff);
		free(cbuff);
		return x->result_code;
	}

#ifndef __WIN32__
	// parse through the symlink?
	if (S_ISLNK(statbuff.st_mode))
	{
		x->result_code = -1;
		x->result_msg = strdup("Symlinks not yet handled.");
		free(src);
		free(dst);
		free(buff);
		free(cbuff);
		return x->result_code;
	}
#endif


	// directory copy
	if (S_ISDIR(statbuff.st_mode))
	{
		if (strncmp(src, dst, strlen(src)) == 0)
		{
			x->result_code = -1;
			x->result_msg = strdup("Recursive copies are not supported.");
			free(src);
			free(dst);
			free(buff);
			free(cbuff);
			return x->result_code;
		}

		x->result_code = -1;
		x->result_msg = strdup("Directory copies not yet handled.");
		free(src);
		free(dst);
		free(buff);
		free(cbuff);
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
		free(cbuff);
		return x->result_code;
	}

	// test if destination already exists
	if (ALFC_stat(dst, &ostatbuff) == 0)
	{
		if (IsTrue(INI_get(gd->optfile, "copy_move", "overwrite")) == 0)
		{
			if (ALFC_unlink(dst) != 0)
			{
				x->result_code = -1;
				x->result_msg = strdup(ALFC_get_last_error(errno));
				free(src);
				return x->result_code;
			}


			// make sure its gone!
			if (ALFC_stat(dst, &ostatbuff) == 0)
			{
				x->result_code = -1;
				x->result_msg = strdup("Destination failed to delete with overwrite");
				free(src);
				free(dst);
				free(buff);
				free(cbuff);
				return x->result_code;
			}
		}
		else
		{
			x->result_code = -1;
			x->result_msg = strdup("Destination already exists");
			free(src);
			free(dst);
			free(buff);
			free(cbuff);
			return x->result_code;
		}
	}

#ifdef __WIN32__
	if((ifp = fopen(src, "rb")) == NULL)
#else
	if ((ifd = open(src, O_RDONLY)) < 0)
#endif
	{
		x->result_code = -1;
		x->result_msg = strdup(strerror(errno));
		free(src);
		free(dst);
		free(buff);
		free(cbuff);
		return x->result_code;
	}

#ifdef __WIN32__
	if((ofp = fopen(dst, "wb+")) == NULL)
#else
	if ((ofd = open(dst, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | statbuff.st_mode)) < 0)
#endif
	{
		x->result_code = -1;
		x->result_msg = strdup(strerror(errno));
#ifdef __WIN32__
		fclose(ifp);
#else
		close(ifd);
#endif
		free(src);
		free(dst);
		free(buff);
		free(cbuff);
		return x->result_code;
	}

	x->op.udtCopy.source_length = statbuff.st_size;

	flag = 0;
	offset = 0;
	while (flag == 0)
	{
		int size;
		int size2;

		uint32_t input_crc;
		uint32_t output_crc;

		if (statbuff.st_size > BLOCK_SIZE)
			size = BLOCK_SIZE;
		else
			size = statbuff.st_size;

		memset(buff, 0x0, BLOCK_SIZE);
		memset(cbuff, 0x0, BLOCK_SIZE);


#ifdef __WIN32__
		//fseek(ifp, offset, SEEK_SET);
		lseek64(ifp->_file, offset, SEEK_SET);
		if( (size2 = fread(buff, 1, size, ifp)) != size )
#else
		lseek(ifd, offset, SEEK_SET);
		if ((size2 = read(ifd, buff, size)) == -1)
#endif
		{
			x->result_code = -1;
			x->result_msg = strdup("Error in reading input file");


#ifdef __WIN32__
			fclose(ifp);
			fclose(ofp);
#else
			close(ifd);
			close(ofd);
#endif
			ALFC_unlink(dst);

			free(src);
			free(dst);
			free(buff);
			free(cbuff);
			return x->result_code;
		}
		size = size2;


#ifdef __WIN32__
		fseek(ofp, offset, SEEK_SET);
		if((size2 = fwrite(buff, 1, size, ofp)) != size)
#else
		lseek(ofd, offset, SEEK_SET);
		if ((size2 = write(ofd, buff, size)) != size)
#endif
		{
			x->result_code = -1;
			x->result_msg = strdup("Error in writing to output file");


#ifdef __WIN32__
			fclose(ifp);
			fclose(ofp);
#else
			close(ifd);
			close(ofd);
#endif
			ALFC_unlink(dst);
			free(src);
			free(dst);
			free(buff);
			free(cbuff);
			return x->result_code;
		}
		size = size2;


#ifdef __WIN32__
		fseek(ofp, offset, SEEK_SET);
		if((size2 = fread(cbuff, 1, size, ofp)) != size)
#else
		lseek(ofd, offset, SEEK_SET);
		if ((size2 = read(ofd, cbuff, size)) == -1)
#endif
		{
			x->result_code = -1;
			x->result_msg = strdup("Error in writing to output file");


#ifdef __WIN32__
			fclose(ifp);
			fclose(ofp);
#else
			close(ifd);
			close(ofd);
#endif
			ALFC_unlink(dst);
			free(src);
			free(dst);
			free(buff);
			free(cbuff);
			return x->result_code;
		}
		size = size2;

		buff[size] = 0;
		cbuff[size] = 0;

		input_crc = fletcher32((uint16_t*) buff, (1 + size) / 2);
		output_crc = fletcher32((uint16_t*) cbuff, (1 + size) / 2);

		if (input_crc != output_crc)
		{
			x->result_code = -1;
			x->result_msg = strdup("Failed CRC Match");


#ifdef __WIN32__
			fclose(ifp);
			fclose(ofp);
#else
			close(ifd);
			close(ofd);
#endif
			ALFC_unlink(dst);
			free(src);
			free(dst);
			free(buff);
			free(cbuff);
			return x->result_code;
		}

		/*
		 sprintf((char*)buff, "Written CRC %04X, %i bytes", output_crc, size);
		 gd->screen->set_style(STYLE_TITLE);
		 gd->screen->set_cursor(gd->screen->get_screen_height(), 1);
		 gd->screen->print((char*)buff);
		 gd->screen->get_keypress();
		 */

		offset += size;
		statbuff.st_size -= size;

		OpWindowPercentageBuffer(w, x->op.udtCopy.source_length, offset);

		if (size == 0 || statbuff.st_size == 0)
			flag = 1;
	}

#ifndef __WIN32__
	close(ifd);
	fchmod(ofd, statbuff.st_mode);
	fchown(ofd, statbuff.st_uid, statbuff.st_gid);
	close(ofd);
#else
	fclose(ifp);
	fclose(ofp);
#endif


#ifdef __WIN32__
	// mingw does not have fchmod (which I'd prefer over chmod).
	// also does not have any chown variants.
	chmod(dst, statbuff.st_mode);
#endif

	timebuff.actime = statbuff.st_atime;
	timebuff.modtime = statbuff.st_mtime;
	utime(dst, &timebuff);

	free(src);
	free(dst);
	free(buff);
	free(cbuff);

	x->result_code = 0;
	x->result_msg = strdup("OK");

	return x->result_code;
}

int Ops_DeleteFile(uGlobalData *gd, uFileOperation *x, uWindow *w)
{
	struct stat statbuff;
	struct stat dbuff;
	char *src;

	src = build_fn(x->op.udtDelete.source_path, x->op.udtDelete.source_filename);

	stat(x->op.udtDelete.source_path, &dbuff);


	// make sure we are in the path of the file to copy and its valid
	if (stat(src, &statbuff) == -1)
	{
		x->result_code = -1;
		x->result_msg = strdup(strerror(errno));
		free(src);
		return x->result_code;
	}

#ifndef __WIN32__
	// parse through the symlink?
	if (S_ISLNK(statbuff.st_mode) != 0)
	{
		x->result_code = -1;
		x->result_msg = strdup("Symlinks not yet handled.");
		free(src);
		return x->result_code;
	}
#endif


	// non-regular copy
	if (!S_ISREG(statbuff.st_mode) != 0 && S_ISDIR(statbuff.st_mode) == 0)
	{
		x->result_code = -1;
		x->result_msg = strdup("File is not a regular file.");
		free(src);
		return x->result_code;
	}

	x->op.udtDelete.source_length = statbuff.st_size;


#ifndef __WIN32__
	chmod(x->op.udtDelete.source_path, dbuff.st_mode | S_IWUSR | S_IWGRP | S_IWOTH | S_IRGRP | S_IROTH | S_IRUSR);
	chmod(src, statbuff.st_mode | S_IWUSR | S_IWGRP | S_IWOTH | S_IRGRP | S_IROTH | S_IRUSR);
#endif


	// GO!
	if (ALFC_unlink(src) != 0)
	{
#ifndef __WIN32__
		chmod(x->op.udtDelete.source_path, dbuff.st_mode);
		chmod(src, statbuff.st_mode);
#endif
		x->result_code = -1;
		x->result_msg = strdup(ALFC_get_last_error(errno));
		free(src);
		return x->result_code;
	}

	chmod(x->op.udtDelete.source_path, dbuff.st_mode);

	free(src);

	x->result_code = 0;
	x->result_msg = strdup("OK");

	return x->result_code;
}

int Ops_MoveFile(uGlobalData *gd, uFileOperation *x, uWindow *w)
{
	uFileOperation *z;

	z = calloc(1, sizeof(uFileOperation));

	z->type = eOp_Copy;
	z->op.udtCopy.source_path = x->op.udtMove.source_path;
	z->op.udtCopy.source_filename = x->op.udtMove.source_filename;
	z->op.udtCopy.dest_path = x->op.udtMove.dest_path;
	z->op.udtCopy.dest_filename = x->op.udtMove.dest_filename;

	Ops_CopyFile(gd, z, w);

	x->result_code = z->result_code;
	x->result_msg = z->result_msg;

	if (z->result_code == 0)
	{
		x->op.udtMove.source_length = z->op.udtCopy.source_length;

		if (x->result_msg != NULL)
			free(x->result_msg);

		memset(z, 0x0, sizeof(uFileOperation));

		z->type = eOp_Delete;
		z->op.udtDelete.source_path = x->op.udtMove.source_path;
		z->op.udtDelete.source_filename = x->op.udtMove.source_filename;

		Ops_DeleteFile(gd, z, w);

		x->result_code = z->result_code;
		x->result_msg = z->result_msg;
	}

	free(z);

	return x->result_code;
}

int ops_MakeDirectory(uGlobalData *gd, uFileOperation *x)
{
	return 0;
}

