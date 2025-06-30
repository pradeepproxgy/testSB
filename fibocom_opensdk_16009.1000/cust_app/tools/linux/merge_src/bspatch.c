/*-
 * Copyright 2003-2005 Colin Percival
 * All rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted providing that the following conditions 
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "bzlib.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <err.h>
#include <unistd.h>
#include <fcntl.h>
#include "bzlib_private.h"

static off_t offtin(u_char *buf)
{
	off_t y;

	y=buf[7]&0x7F;
	y=y*256;y+=buf[6];
	y=y*256;y+=buf[5];
	y=y*256;y+=buf[4];
	y=y*256;y+=buf[3];
	y=y*256;y+=buf[2];
	y=y*256;y+=buf[1];
	y=y*256;y+=buf[0];

	if(buf[7]&0x80) y=-y;

	return y;
}
int seg_block_size = 0;

void check_ver_and_crc(char *patch_file, char* obin_file, int blocksize)
{
	FILE *pf = NULL;
	FILE *pVer = NULL;
	unsigned int patch_len = 0;
	unsigned char *patch_data = NULL;
	unsigned int obin_len = 0;
	unsigned char *obin_data = NULL;
	unsigned int total_crc = 0;
	unsigned int obin_crc = 0;
	int i;
	int fd;
	unsigned int total_len = 0;
	int blocksize_file = 0;

	if(((fd=open(patch_file,O_RDONLY,0))<0) ||
		((patch_len=lseek(fd,0,SEEK_END))==-1) ||
		((patch_data=malloc(patch_len+1))==NULL) ||
		(lseek(fd,0,SEEK_SET)!=0) ||
		(read(fd,patch_data,patch_len)!=patch_len) ||
		(close(fd)==-1)) err(1,"%s",patch_file);
	printf("patch_len = %d\n", patch_len);

	if(((fd=open(obin_file,O_RDONLY,0))<0) ||
		((obin_len=lseek(fd,0,SEEK_END))==-1) ||
		((obin_data=malloc(obin_len+1))==NULL) ||
		(lseek(fd,0,SEEK_SET)!=0) ||
		(read(fd,obin_data,obin_len)!=obin_len) ||
		(close(fd)==-1)) err(1,"%s",obin_file);
	printf("obin_len = %d\n", obin_len);


	if ((pf = fopen(patch_file, "r")) == NULL)
		err(1, "%s", patch_file);
	if (fread((char *)&total_len, 1, 4, pf) != 4)
		err(1, "fread");
	if (fread((char *)&total_crc, 1, 4, pf) != 4)
		err(1, "fread");
	if (fread((char *)&obin_len, 1, 4, pf) != 4)
		err(1, "fread");
	if (fread((char *)&obin_crc, 1, 4, pf) != 4)
		err(1, "fread");
	if (fread((char *)&blocksize_file, 1, 4, pf) != 4)
		err(1, "fread");

	if(blocksize_file != blocksize){
		printf("blocksize_file != seg_block_size, error...\n");
		exit(1);
	}
	printf("seg_block_size = %d check right ! GOOD ...\n", blocksize);

	if(total_len != patch_len){
		printf("total_len != patch_len, error...\n");
		exit(1);
	}
	printf("patch len check right ! GOOD ...\n");
	

	/* check old.bin crc value */
	printf("obin_crc = %u, obin_len = %u\n", obin_crc, obin_len);
	for(i=0; i<obin_len; i++)
		obin_crc -= *(obin_data+i);
	if(obin_crc != 0){
		printf("obin_crc != 0, error...\n");
		exit(1);
	}
	printf("obin_crc check right ! GOOD ...\n");

	/* check total crc value */
	printf("total_crc = %u\n", total_crc);
	for(i=30; i<patch_len; i++)
		total_crc -= *(patch_data+i);
	
	if(total_crc != 0){
		printf("total_crc != 0, error..., %u\n", total_crc);
		exit(1);
	}
	printf("total_crc check right ! GOOD ...\n");

	fclose(pf);
	return;
}

int main(int argc,char * argv[])
{
	FILE * f, * cpf, * dpf, * epf;
	BZFILE * cpfbz2, * dpfbz2, * epfbz2;
	int cbz2err, dbz2err, ebz2err;
	FILE *fp_old, *fp_new;
	ssize_t oldsize,newsize,patchsize;
	ssize_t bzctrllen,bzdatalen,bzextralen;
	u_char header[48],buf[8];
	u_char *old, *new;
	off_t oldpos,newpos;
	off_t ctrl[3];
	off_t lenread;
	off_t i;
	off_t patch_pos = 0;
	off_t patch_seg_len = 0;
	int exit_flag = 0;
	int blocksize_file = 0;

	/*
	File format:
		0		8	"FIBOFOTA"
		8		8	X
		16		8	Y
		24 		8	Z
		32		8	sizeof(oldfile)
		40		8	sizeof(newfile)
		48		X	bzip2(control block)
		48+X		Y	bzip2(diff block)
		48+X+Y	Z	bzip2(extra block)
	with control block a set of triples (x,y,z) meaning "add x bytes
	from oldfile to x bytes from the diff block; copy y bytes from the
	extra block; seek forwards in oldfile by z bytes".
	*/
	if(argc!=5) errx(1,"usage: %s oldfile newfile patchfile segBlockSize\n",argv[0]);
	seg_block_size = atoi(argv[4]);

	
	check_ver_and_crc(argv[3], argv[1], seg_block_size);
	patch_pos = 22 + 4 + 4;

	/* Open patch file */
	if ((f = fopen(argv[3], "r")) == NULL)
		err(1, "fopen(%s)", argv[3]);
	if ((cpf = fopen(argv[3], "r")) == NULL)
			err(1, "fopen(%s)", argv[3]);
	if ((dpf = fopen(argv[3], "r")) == NULL)
			err(1, "fopen(%s)", argv[3]);
	if ((epf = fopen(argv[3], "r")) == NULL)
			err(1, "fopen(%s)", argv[3]);
	if ((fp_old = fopen(argv[1], "r")) == NULL)
			err(1, "fopen(%s)", argv[3]);
	if ((fp_new = fopen(argv[2], "w")) == NULL)
			err(1, "fopen(%s)", argv[3]);


	while(1){
		if(exit_flag == 0xff)
			break;
		
		if (fseeko(f, patch_pos, SEEK_SET))
			err(1, "fseeko line 103");
		
		/* Read header */
		if (fread(header, 1, 48, f) < 48) {
			if (feof(f)){
				printf("line : %d\n", __LINE__);
				errx(1, "Corrupt patch\n");
			}
			err(1, "fread(%s)", argv[3]);
		}

		/* Check for appropriate magic */
		if (memcmp(header, "FIBOFOTA", 8) != 0){
			printf("line : %d\n", __LINE__);
			errx(1, "Corrupt patch\n");
		}

		/* Read lengths from header */
		bzctrllen=offtin(header+8);
		bzdatalen=offtin(header+16);
		bzextralen=offtin(header+24);
		oldsize=offtin(header+32);
		newsize=offtin(header+40);
		if((bzctrllen<0) || (bzdatalen<0) || (newsize<0)
			|| (oldsize<0) || (newsize<0)){
			printf("line : %d\n", __LINE__);
			errx(1,"Corrupt patch\n");
		}
		
		/* last segment patch old.bin or new.bin size is equal to 108*1024 */
		if(oldsize != seg_block_size*1024 || (newsize != seg_block_size*1024))
			exit_flag = 0xff;

		/* every patch segment total length */
		patch_seg_len = bzctrllen + bzdatalen + bzextralen + 48;

		/* Close patch file and re-open it via libbzip2 at the right places */
		
		if (fseeko(cpf, patch_pos + 48, SEEK_SET))
			err(1, "fseeko(%s, %lld)", argv[3],
			    (long long)48);
		if ((cpfbz2 = BZ2_bzReadOpen(&cbz2err, cpf, 0, 0, NULL, 0)) == NULL)
			errx(1, "BZ2_bzReadOpen, bz2err = %d", cbz2err);


		if (fseeko(dpf, patch_pos + 48 + bzctrllen, SEEK_SET))
			err(1, "fseeko(%s, %lld)", argv[3],
			    (long long)(48 + bzctrllen));
		if ((dpfbz2 = BZ2_bzReadOpen(&dbz2err, dpf, 0, 0, NULL, 0)) == NULL)
			errx(1, "BZ2_bzReadOpen, bz2err = %d", dbz2err);


		if (fseeko(epf, patch_pos + 48 + bzctrllen + bzdatalen, SEEK_SET))
			err(1, "fseeko(%s, %lld)", argv[3],
			    (long long)(48 + bzctrllen + bzdatalen));
		if ((epfbz2 = BZ2_bzReadOpen(&ebz2err, epf, 0, 0, NULL, 0)) == NULL)
			errx(1, "BZ2_bzReadOpen, bz2err = %d", ebz2err);
		
		
		if((old=malloc(oldsize+1))==NULL) err(1,NULL);
		if (fread(old, 1, oldsize, fp_old) != oldsize) {
			printf("fread error, line 163\n");
			exit(1);
		}
		if((new=malloc(newsize+1))==NULL) err(1,NULL);

		oldpos=0;newpos=0;
		while(newpos<newsize) {
			/* Read control data */
			for(i=0;i<=2;i++) {
				lenread = BZ2_bzRead(&cbz2err, cpfbz2, buf, 8);
				if ((lenread < 8) || ((cbz2err != BZ_OK) &&
				    (cbz2err != BZ_STREAM_END)))
					errx(1, "Corrupt patch\n");
				ctrl[i]=offtin(buf);
			};

			/* Sanity-check */
			if(newpos+ctrl[0]>newsize)
				errx(1,"Corrupt patch\n");

			/* Read diff string */
			lenread = BZ2_bzRead(&dbz2err, dpfbz2, new + newpos, ctrl[0]);
			if ((lenread < ctrl[0]) ||
			    ((dbz2err != BZ_OK) && (dbz2err != BZ_STREAM_END)))
				errx(1, "Corrupt patch\n");

			/* Add old data to diff string */
			for(i=0;i<ctrl[0];i++)
				if((oldpos+i>=0) && (oldpos+i<oldsize))
					new[newpos+i]+=old[oldpos+i];

			/* Adjust pointers */
			newpos+=ctrl[0];
			oldpos+=ctrl[0];

			/* Sanity-check */
			if(newpos+ctrl[1]>newsize)
				errx(1,"Corrupt patch\n");

			/* Read extra string */
			lenread = BZ2_bzRead(&ebz2err, epfbz2, new + newpos, ctrl[1]);
			if ((lenread < ctrl[1]) ||
			    ((ebz2err != BZ_OK) && (ebz2err != BZ_STREAM_END)))
				errx(1, "Corrupt patch\n");

			/* Adjust pointers */
			newpos+=ctrl[1];
			oldpos+=ctrl[2];
		};

		/* Clean up the bzip2 reads */
		BZ2_bzReadClose(&cbz2err, cpfbz2);
		BZ2_bzReadClose(&dbz2err, dpfbz2);
		BZ2_bzReadClose(&ebz2err, epfbz2);

		if (fwrite(new, 1, newsize, fp_new) != newsize)
			err(1, "fwrite(%s)", argv[3]);

		patch_pos += patch_seg_len;

		free(new);new = NULL;
		free(old);old = NULL;
	}

	if(fclose(f) || fclose(cpf) || fclose(dpf) || fclose(epf)
		|| fclose(fp_old) || fclose(fp_new))
		printf("fclose error, line 240\n");

	return 0;
}
