
/*-------------------------------------------------------------*/
/*--- Public header file for the library.                   ---*/
/*---                                               bzlib.h ---*/
/*-------------------------------------------------------------*/

/*--
  This file is a part of bzip2 and/or libbzip2, a program and
  library for lossless, block-sorting data compression.

  Copyright (C) 1996-2005 Julian R Seward.  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.

  2. The origin of this software must not be misrepresented; you must
     not claim that you wrote the original software.  If you use this
     software in a product, an acknowledgment in the product
     documentation would be appreciated but is not required.

  3. Altered source versions must be plainly marked as such, and must
     not be misrepresented as being the original software.

  4. The name of the author may not be used to endorse or promote
     products derived from this software without specific prior written
     permission.

  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
  OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
  GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  Julian Seward, Cambridge, UK.
  jseward@bzip.org
  bzip2/libbzip2 version 1.0 of 21 March 2000

  This program is based on (at least) the work of:
     Mike Burrows
     David Wheeler
     Peter Fenwick
     Alistair Moffat
     Radford Neal
     Ian H. Witten
     Robert Sedgewick
     Jon L. Bentley

  For more information on these sources, see the manual.
--*/


#ifndef _BZLIB_H
#define _BZLIB_H



#define BZ_RUN               0
#define BZ_FLUSH             1
#define BZ_FINISH            2

#define BZ_OK                0
#define BZ_RUN_OK            1
#define BZ_FLUSH_OK          2
#define BZ_FINISH_OK         3
#define BZ_STREAM_END        4
#define BZ_SEQUENCE_ERROR    (-1)
#define BZ_PARAM_ERROR       (-2)
#define BZ_MEM_ERROR         (-3)
#define BZ_DATA_ERROR        (-4)
#define BZ_DATA_ERROR_MAGIC  (-5)
#define BZ_IO_ERROR          (-6)
#define BZ_UNEXPECTED_EOF    (-7)
#define BZ_OUTBUFF_FULL      (-8)
#define BZ_CONFIG_ERROR      (-9)

/* for flseek */
#define FL_SEEK_SET				1
#define FL_SEEK_CUR				2
#define FL_SEEK_END				3

typedef struct {
      char *next_in;
      unsigned int avail_in;
      unsigned int total_in_lo32;
      unsigned int total_in_hi32;

      char *next_out;
      unsigned int avail_out;
      unsigned int total_out_lo32;
      unsigned int total_out_hi32;

      void *state;

      void *(*bzalloc)(void *,int,int);
      void (*bzfree)(void *,void *);
      void *opaque;
} bz_stream;

/* lobin add */
typedef struct{
   unsigned int mem_addr;
   unsigned int total_len;
   int pos;
}flFILE;

typedef struct{
	unsigned int bin_addr;
	unsigned int bak_addr;
	unsigned int a_flag_addr;
	unsigned int b_flag_addr;

	unsigned int old_addr;
	unsigned int old_pos;

	unsigned int new_addr;
	unsigned int new_pos;
    int read_bak_enable;
	int reserved;
}bin_info;

/*-- Core (low-level) library functions --*/
extern int BZ2_bzDecompressInit (
      bz_stream *strm,
      int       verbosity,
      int       small
   );

extern int BZ2_bzDecompress (
      bz_stream* strm
   );

extern int BZ2_bzDecompressEnd(
      bz_stream *strm
   );

/*-- High(er) level library functions --*/
#define BZ_MAX_UNUSED 5000

typedef void BZFILE;

extern BZFILE* BZ2_bzReadOpen(
      int*  bzerror,
      flFILE* f,
      int   verbosity,
      int   small,
      void* unused,
      int   nUnused
   );

extern void BZ2_bzReadClose (
      int*    bzerror,
      BZFILE* b
   );

extern void BZ2_bzReadGetUnused (
      int*    bzerror,
      BZFILE* b,
      void**  unused,
      int*    nUnused
   );

extern int BZ2_bzRead (
      int*    bzerror,
      BZFILE* b,
      void*   buf,
      int     len
   );
/*-- Utility functions --*/
extern const char * BZ2_bzlibVersion (
      void
   );

extern int flinit(char *data, unsigned int len);
extern void fluninit(void);
extern flFILE *flopen(unsigned int mem_addr, unsigned int total_len);
extern int flseek(flFILE *fp, long offset, int origin);
extern long fltell(flFILE *fp);
extern int flread( void *buffer, int size, flFILE* fp);
extern int flwrite( const void *buffer, int size, flFILE *fp);
extern int flclose(flFILE *fp);
extern int fleof(flFILE *fp);
extern int flerror(flFILE *fp);
extern int af_flash_init(void);
extern int af_flash_erase(unsigned int addr, unsigned int len);
extern int af_flash_read(unsigned int addr, unsigned char *data, unsigned int len);
extern int af_flash_write(unsigned int addr, unsigned char *data, unsigned int len);
extern void *af_malloc(int size);
extern void af_free(void *ptr);
#endif
