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
#include <sys/types.h>

#include "bzlib.h"
#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MIN(x,y) (((x)<(y)) ? (x) : (y))

static void split(off_t *I,off_t *V,off_t start,off_t len,off_t h)
{
	off_t i,j,k,x,tmp,jj,kk;

	if(len<16) {
		for(k=start;k<start+len;k+=j) {
			j=1;x=V[I[k]+h];
			for(i=1;k+i<start+len;i++) {
				if(V[I[k+i]+h]<x) {
					x=V[I[k+i]+h];
					j=0;
				};
				if(V[I[k+i]+h]==x) {
					tmp=I[k+j];I[k+j]=I[k+i];I[k+i]=tmp;
					j++;
				};
			};
			for(i=0;i<j;i++) V[I[k+i]]=k+j-1;
			if(j==1) I[k]=-1;
		};
		return;
	};

	x=V[I[start+len/2]+h];
	jj=0;kk=0;
	for(i=start;i<start+len;i++) {
		if(V[I[i]+h]<x) jj++;
		if(V[I[i]+h]==x) kk++;
	};
	jj+=start;kk+=jj;

	i=start;j=0;k=0;
	while(i<jj) {
		if(V[I[i]+h]<x) {
			i++;
		} else if(V[I[i]+h]==x) {
			tmp=I[i];I[i]=I[jj+j];I[jj+j]=tmp;
			j++;
		} else {
			tmp=I[i];I[i]=I[kk+k];I[kk+k]=tmp;
			k++;
		};
	};

	while(jj+j<kk) {
		if(V[I[jj+j]+h]==x) {
			j++;
		} else {
			tmp=I[jj+j];I[jj+j]=I[kk+k];I[kk+k]=tmp;
			k++;
		};
	};

	if(jj>start) split(I,V,start,jj-start,h);

	for(i=0;i<kk-jj;i++) V[I[jj+i]]=kk-1;
	if(jj==kk-1) I[jj]=-1;

	if(start+len>kk) split(I,V,kk,start+len-kk,h);
}

static void qsufsort(off_t *I,off_t *V,u_char *old,off_t oldsize)
{
	off_t buckets[256];
	off_t i,h,len;

	for(i=0;i<256;i++) buckets[i]=0;
	for(i=0;i<oldsize;i++) buckets[old[i]]++;
	for(i=1;i<256;i++) buckets[i]+=buckets[i-1];
	for(i=255;i>0;i--) buckets[i]=buckets[i-1];
	buckets[0]=0;

	for(i=0;i<oldsize;i++) I[++buckets[old[i]]]=i;
	I[0]=oldsize;
	for(i=0;i<oldsize;i++) V[i]=buckets[old[i]];
	V[oldsize]=0;
	for(i=1;i<256;i++) if(buckets[i]==buckets[i-1]+1) I[buckets[i]]=-1;
	I[0]=-1;

	for(h=1;I[0]!=-(oldsize+1);h+=h) {
		len=0;
		for(i=0;i<oldsize+1;) {
			if(I[i]<0) {
				len-=I[i];
				i-=I[i];
			} else {
				if(len) I[i-len]=-len;
				len=V[I[i]]+1-i;
				split(I,V,i,len,h);
				i+=len;
				len=0;
			};
		};
		if(len) I[i-len]=-len;
	};

	for(i=0;i<oldsize+1;i++) I[V[i]]=i;
}

static off_t matchlen(u_char *old,off_t oldsize,u_char *new,off_t newsize)
{
	off_t i;

	for(i=0;(i<oldsize)&&(i<newsize);i++)
		if(old[i]!=new[i]) break;

	return i;
}

static off_t search(off_t *I,u_char *old,off_t oldsize,
		u_char *new,off_t newsize,off_t st,off_t en,off_t *pos)
{
	off_t x,y;

	if(en-st<2) {
		x=matchlen(old+I[st],oldsize-I[st],new,newsize);
		y=matchlen(old+I[en],oldsize-I[en],new,newsize);

		if(x>y) {
			*pos=I[st];
			return x;
		} else {
			*pos=I[en];
			return y;
		}
	};

	x=st+(en-st)/2;
	if(memcmp(old+I[x],new,MIN(oldsize-I[x],newsize))<0) {
		return search(I,old,oldsize,new,newsize,x,en,pos);
	} else {
		return search(I,old,oldsize,new,newsize,st,x,pos);
	};
}

static void offtout(off_t x,u_char *buf)
{
	off_t y;

	if(x<0) y=-x; else y=x;

		buf[0]=y%256;y-=buf[0];
	y=y/256;buf[1]=y%256;y-=buf[1];
	y=y/256;buf[2]=y%256;y-=buf[2];
	y=y/256;buf[3]=y%256;y-=buf[3];
	y=y/256;buf[4]=y%256;y-=buf[4];
	y=y/256;buf[5]=y%256;y-=buf[5];
	y=y/256;buf[6]=y%256;y-=buf[6];
	y=y/256;buf[7]=y%256;

	if(x<0) buf[7]|=0x80;
}
int seg_block_size = 0;

void get_seg_info(off_t file_size, int *seg_nr, int *seg_plus)
{
	*seg_nr = file_size / (seg_block_size*1024);
	*seg_plus = file_size % (seg_block_size*1024);
}

void add_ver_and_crc(char *patch_file, char *obin_file, int blocksize)
{
	FILE *pf = NULL;
	FILE *pVer = NULL;
	unsigned int patch_len = 0;
	unsigned char *patch_data = NULL;
	unsigned char *obin_data = NULL;
	unsigned int obin_len = 0;
	unsigned int crc_total = 0;
	unsigned int crc_obin = 0;
	unsigned char version[32] = {0};
	int i;
	int fd;
	unsigned int total_len = 0;
	char reserved[14] = {0};

	memset(reserved, 0, sizeof(reserved));

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

	for(i=0; i<obin_len; i++)
		crc_obin += *(obin_data + i);

	for(i=0; i<patch_len; i++)
		crc_total += *(patch_data + i);

	total_len = 30 + patch_len;
	
	/* put fp to begin for write all data */
	if ((pf = fopen(patch_file, "w")) == NULL)
		err(1, "%s", patch_file);
	if (fseeko(pf, 0, SEEK_SET))
		err(1, "fseeko");
	if (fwrite((char *)&total_len, 1, 4, pf) != 4)
		err(1, "fwrite");
	if (fwrite((char *)&crc_total, 1, 4, pf) != 4)
		err(1, "fwrite");
	if (fwrite((char *)&obin_len, 1, 4, pf) != 4)
		err(1, "fwrite");
	if (fwrite((char *)&crc_obin, 1, 4, pf) != 4)
		err(1, "fwrite");
	if (fwrite((char *)&blocksize, 1, 4, pf) != 4)
		err(1, "fwrite");
	if (fwrite((char *)reserved, 1, 10, pf) != 10)
		err(1, "fwrite");

	if (fwrite(patch_data, patch_len, 1, pf) != 1)
		err(1, "fwrite");
	fclose(pf);
	return;
}

int main(int argc,char *argv[])
{
	int fd;
	u_char *old,*new;
	off_t oldsize,newsize;
	off_t *I,*V;
	off_t scan,pos,len;
	off_t lastscan,lastpos,lastoffset;
	off_t oldscore,scsc;
	off_t s,Sf,lenf,Sb,lenb;
	off_t overlap,Ss,lens;
	off_t i;
	off_t dblen,eblen;
	u_char *db,*eb;
	u_char buf[8];
	u_char header[48];
	FILE * pf;
	BZFILE * pfbz2;
	int bz2err;
	/* lobin add */
	u_char *old_total, *new_total;
	off_t old_total_size,new_total_size;
	int old_seg_nr, new_seg_nr, old_plus, new_plus;
	int n = 0;
	int ttt = 0;
	int pf_pos = 0;

	if(argc!=5) errx(1,"usage: %s oldfile newfile patchfile segBlockSize\n",argv[0]);

	seg_block_size = atoi(argv[4]);
	printf("seg_block_size = %d\n", seg_block_size);
	
	/* Allocate oldsize+1 bytes instead of oldsize bytes to ensure
		that we never try to malloc(0) and get a NULL pointer */
	if(((fd=open(argv[1],O_RDONLY,0))<0) ||
		((old_total_size=lseek(fd,0,SEEK_END))==-1) ||
		((old_total=malloc(old_total_size+1))==NULL) ||
		(lseek(fd,0,SEEK_SET)!=0) ||
		(read(fd,old_total,old_total_size)!=old_total_size) ||
		(close(fd)==-1)) err(1,"%s",argv[1]);

		/* Allocate newsize+1 bytes instead of newsize bytes to ensure
		that we never try to malloc(0) and get a NULL pointer */
	if(((fd=open(argv[2],O_RDONLY,0))<0) ||
		((new_total_size=lseek(fd,0,SEEK_END))==-1) ||
		((new_total=malloc(new_total_size+1))==NULL) ||
		(lseek(fd,0,SEEK_SET)!=0) ||
		(read(fd,new_total,new_total_size)!=new_total_size) ||
		(close(fd)==-1)) err(1,"%s",argv[2]);

	get_seg_info(old_total_size, &old_seg_nr, &old_plus);
	printf("old_total_size = %d, old_seg_nr = %d, old_plus = %d\n",
			(int)old_total_size, old_seg_nr, old_plus);
	get_seg_info(new_total_size, &new_seg_nr, &new_plus);
	printf("new_total_size = %d, new_seg_nr = %d, new_plus = %d\n",
			(int)new_total_size, new_seg_nr, new_plus);

	if(old_seg_nr != new_seg_nr){
		printf("blocksize is not suitable, please change it! \n");
		return 1;
	}

	/* Create the patch file */
	if ((pf = fopen(argv[3], "w")) == NULL)
		err(1, "%s", argv[3]);

	while(1){
		if(ttt == 0xff){
			if (fclose(pf))
				err(1, "fclose");
			break;
		}
		
		if(n < old_seg_nr){
			oldsize = seg_block_size*1024;
			old = malloc(oldsize + 1);
			memcpy(old, old_total+seg_block_size*1024*n, oldsize);
			newsize = seg_block_size*1024;
			new = malloc(newsize + 1);
			memcpy(new, new_total+seg_block_size*1024*n, newsize);
		}
		else{
			oldsize = old_plus;
			old = malloc(oldsize + 1);
			memcpy(old, old_total+seg_block_size*1024*n, oldsize);
			newsize = new_plus;
			new = malloc(newsize + 1);
			memcpy(new, new_total+seg_block_size*1024*n, newsize);
			ttt = 0xff;
		}
		n++;
		

		if(((I=malloc((oldsize+1)*sizeof(off_t)))==NULL) ||
			((V=malloc((oldsize+1)*sizeof(off_t)))==NULL)) err(1,NULL);
		qsufsort(I,V,old,oldsize);
		free(V);


		if(((db=malloc(newsize+1))==NULL) ||
			((eb=malloc(newsize+1))==NULL)) err(1,NULL);
		dblen=0;
		eblen=0;

		/* Header is
			0	8	 "FIBOFOTA"
			8	8	length of bzip2ed ctrl block
			16	8	length of bzip2ed diff block
			24	8	length of bzip2ed extra block
			32   8   	length of old file 
			40   8	length of new file */
		/* File is  patch*n
			0	48	Header
			48	??	Bzip2ed ctrl block
			??	??	Bzip2ed diff block
			??	??	Bzip2ed extra block 
			
			0	48	Header
			48	??	Bzip2ed ctrl block
			??	??	Bzip2ed diff block
			??	??	Bzip2ed extra block 

			......					*/
		memcpy(header,"FIBOFOTA",8);
		offtout(0, header + 8);
		offtout(0, header + 16);
		offtout(0, header + 24);
		offtout(oldsize, header + 32);
		offtout(newsize, header + 40);
		
		if (fwrite(header, 48, 1, pf) != 1)
			err(1, "fwrite(%s)", argv[3]);

		/* Compute the differences, writing ctrl as we go */
		//if ((pfbz2 = BZ2_bzWriteOpen(&bz2err, pf, 9, 0, 0)) == NULL)
		/* lobin add */
		if ((pfbz2 = BZ2_bzWriteOpen(&bz2err, pf, 1, 0, 0)) == NULL)
			errx(1, "BZ2_bzWriteOpen, bz2err = %d", bz2err);
		scan=0;len=0;
		lastscan=0;lastpos=0;lastoffset=0;
		while(scan<newsize) {
			oldscore=0;

			for(scsc=scan+=len;scan<newsize;scan++) {
				len=search(I,old,oldsize,new+scan,newsize-scan,
						0,oldsize,&pos);

				for(;scsc<scan+len;scsc++)
				if((scsc+lastoffset<oldsize) &&
					(old[scsc+lastoffset] == new[scsc]))
					oldscore++;

				if(((len==oldscore) && (len!=0)) || 
					(len>oldscore+8)) break;

				if((scan+lastoffset<oldsize) &&
					(old[scan+lastoffset] == new[scan]))
					oldscore--;
			};

			if((len!=oldscore) || (scan==newsize)) {
				s=0;Sf=0;lenf=0;
				for(i=0;(lastscan+i<scan)&&(lastpos+i<oldsize);) {
					if(old[lastpos+i]==new[lastscan+i]) s++;
					i++;
					if(s*2-i>Sf*2-lenf) { Sf=s; lenf=i; };
				};

				lenb=0;
				if(scan<newsize) {
					s=0;Sb=0;
					for(i=1;(scan>=lastscan+i)&&(pos>=i);i++) {
						if(old[pos-i]==new[scan-i]) s++;
						if(s*2-i>Sb*2-lenb) { Sb=s; lenb=i; };
					};
				};

				if(lastscan+lenf>scan-lenb) {
					overlap=(lastscan+lenf)-(scan-lenb);
					s=0;Ss=0;lens=0;
					for(i=0;i<overlap;i++) {
						if(new[lastscan+lenf-overlap+i]==
						   old[lastpos+lenf-overlap+i]) s++;
						if(new[scan-lenb+i]==
						   old[pos-lenb+i]) s--;
						if(s>Ss) { Ss=s; lens=i+1; };
					};

					lenf+=lens-overlap;
					lenb-=lens;
				};

				for(i=0;i<lenf;i++)
					db[dblen+i]=new[lastscan+i]-old[lastpos+i];
				for(i=0;i<(scan-lenb)-(lastscan+lenf);i++)
					eb[eblen+i]=new[lastscan+lenf+i];

				dblen+=lenf;
				eblen+=(scan-lenb)-(lastscan+lenf);

				offtout(lenf,buf);
				BZ2_bzWrite(&bz2err, pfbz2, buf, 8);
				if (bz2err != BZ_OK)
					errx(1, "BZ2_bzWrite, bz2err = %d", bz2err);

				offtout((scan-lenb)-(lastscan+lenf),buf);
				BZ2_bzWrite(&bz2err, pfbz2, buf, 8);
				if (bz2err != BZ_OK)
					errx(1, "BZ2_bzWrite, bz2err = %d", bz2err);

				offtout((pos-lenb)-(lastpos+lenf),buf);
				BZ2_bzWrite(&bz2err, pfbz2, buf, 8);
				if (bz2err != BZ_OK)
					errx(1, "BZ2_bzWrite, bz2err = %d", bz2err);

				lastscan=scan-lenb;
				lastpos=pos-lenb;
				lastoffset=pos-scan;
			};
		};
		BZ2_bzWriteClose(&bz2err, pfbz2, 0, NULL, NULL);
		if (bz2err != BZ_OK)
			errx(1, "BZ2_bzWriteClose, bz2err = %d", bz2err);

		/* Compute size of compressed ctrl data */
		if ((len = ftello(pf)) == -1)
			err(1, "ftello");
		offtout(len-48-pf_pos, header + 8);

		/* Write compressed diff data */
		//if ((pfbz2 = BZ2_bzWriteOpen(&bz2err, pf, 9, 0, 0)) == NULL)
		/* lobin add */
		if ((pfbz2 = BZ2_bzWriteOpen(&bz2err, pf, 1, 0, 0)) == NULL)
			errx(1, "BZ2_bzWriteOpen, bz2err = %d", bz2err);
		BZ2_bzWrite(&bz2err, pfbz2, db, dblen);
		if (bz2err != BZ_OK)
			errx(1, "BZ2_bzWrite, bz2err = %d", bz2err);
		BZ2_bzWriteClose(&bz2err, pfbz2, 0, NULL, NULL);
		if (bz2err != BZ_OK)
			errx(1, "BZ2_bzWriteClose, bz2err = %d", bz2err);

		/* Compute size of compressed diff data */
		if ((newsize = ftello(pf)) == -1)
			err(1, "ftello");
		offtout(newsize - len, header + 16);

		/* Write compressed extra data */
		//if ((pfbz2 = BZ2_bzWriteOpen(&bz2err, pf, 9, 0, 0)) == NULL)
		/* lobin add */
		if ((pfbz2 = BZ2_bzWriteOpen(&bz2err, pf, 1, 0, 0)) == NULL)
			errx(1, "BZ2_bzWriteOpen, bz2err = %d", bz2err);
		BZ2_bzWrite(&bz2err, pfbz2, eb, eblen);
		if (bz2err != BZ_OK)
			errx(1, "BZ2_bzWrite, bz2err = %d", bz2err);
		BZ2_bzWriteClose(&bz2err, pfbz2, 0, NULL, NULL);
		if (bz2err != BZ_OK)
			errx(1, "BZ2_bzWriteClose, bz2err = %d", bz2err);

		/* Compute size of compressed diff data */
		if ((oldsize = ftello(pf)) == -1)
			err(1, "ftello");
		offtout(oldsize - newsize, header + 24);

		/* Seek to the beginning, write the header, and close the file */
		if (fseeko(pf, pf_pos, SEEK_SET))
			err(1, "fseeko");
		if (fwrite(header, 48, 1, pf) != 1)
			err(1, "fwrite(%s)", argv[3]);
		if (fseeko(pf, 0, SEEK_END))
			err(1, "fseeko");

		pf_pos = oldsize;
		/*
		if (fclose(pf))
			err(1, "fclose");*/

		/* Free the memory we used */
		free(db);
		free(eb);
		free(I);
		free(old);
		free(new);
	}
	//fclose(pf);
	
	add_ver_and_crc(argv[3], argv[1], seg_block_size);
	return 0;
}
