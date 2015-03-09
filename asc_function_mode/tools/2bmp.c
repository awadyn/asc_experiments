/******************************************************************************
* Copyright (C) 2011 by Jonathan Appavoo, Boston University
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*****************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <bmpfile.h>

int verboseflg=0;

typedef struct image_extract {
  uintptr_t s;
  uintptr_t e;
  struct image_extract *next;
} image_extract_t;

typedef struct image {
  bmpfile_t *bmp;
  uintptr_t bsize;
  intptr_t endx, endy;
  intptr_t cursor_x, cursor_y;
  rgb_pixel_t fg, bg;
  uint32_t num;
  intptr_t numpixels;
  char *fileprefix;
  image_extract_t *ehead;
  image_extract_t *ecur;
  char *estr;
  intptr_t padcount;
  int blockimg;
} image_t;

int 
img_init(image_t *img, uintptr_t blocksize, 
	 uintptr_t cols, uintptr_t rows, int blockimg,
	 char *fileprefix,
	 image_extract_t *ehead, int estrlen)
{
  int n=0;
  bzero(img, sizeof(image_t));
  img->bsize = blocksize;
  img->bmp = bmp_create(cols, rows, 1);
  if (img->bmp == 0) return -1;
  img->endx = bmp_get_width(img->bmp)-1;
  img->endy = bmp_get_height(img->bmp)-1;
  img->blockimg = blockimg;
  img->fg.red = img->fg.green = img->fg.blue = img->fg.alpha=0; 
  img->bg.red = img->bg.green = img->bg.blue = img->bg.alpha=255; 
  img->fileprefix = fileprefix;
  img->ehead = ehead;
  img->ecur = ehead;
  if (ehead) img->estr = malloc(estrlen+1);

  while (img->ecur) {
    if ((img->ecur->s > img->bsize) ||
       (img->ecur->e > img->bsize) ||
	(img->ecur->e < img->ecur->s)) {
	 fprintf(stderr, "Bad extract specification: %ld %ld bsize=%ld\n",
		 img->ecur->s, img->ecur->e, img->bsize);
	 return -1;
       }
    n += snprintf(&(img->estr[n]), estrlen+1-n, ":%d,%d", 
		(unsigned int)img->ecur->s, 
		(unsigned int)img->ecur->e);
    img->ecur=img->ecur->next;
  }
  img->ecur = img->ehead;

  return 1;
}

int 
img_extract_add(image_extract_t **ehead, uintptr_t s, uintptr_t e)
{
  unsigned int count = e - s + 1;

  if (s > e) return -1;

  image_extract_t *cur, *prev, *node = malloc(sizeof(image_extract_t));

  node->s = s; node->e = e;
  node->next = NULL;  

  if (*ehead == NULL) {
    *ehead = node;
    return count;
  }

  cur = *ehead;
  prev = cur;
  while (cur) {
    if (s < cur->s && (e<cur->s)) {
      // before
      node->next = cur;
      // one item on list so add at head
      if (prev == cur) *ehead = node; 
      else prev->next = node;
      return count;
    }  
    if (s>cur->e && e>cur->e) {
      // after
      if (cur->next == 0) {
	// cur last on list add at end
	cur->next = node;
	return count;
      } else {
	prev = cur;
	cur = cur->next;
	continue;
      }
    }
    // not before or after must overlap so return error
    return -1; 
  }
  return -1;
}

void
img_dump_extracts(image_t *img)
{
  image_extract_t *n;

  printf("extracts:");
  if (img->ehead==0) {
    printf("0,%ld\n",img->endx);
    return;
  }
  for (n = img->ehead; n; n=n->next) {
    printf("%ld,%ld ", n->s, n->e);
  }
  printf("\n");
}

inline void
img_cleanup(image_t *img)
{
  
  bmp_destroy(img->bmp);
  while (img->ehead) {
    image_extract_t *n = img->ehead;
    img->ehead = n->next;
    free(n);
  }
}

inline bool
img_empty(image_t *img)
{
  return img->numpixels == 0;
}

inline bool
img_full(image_t *img) {
  return img->cursor_y > img->endy;
}

inline void
img_draw_bit(image_t *img, unsigned char byte, unsigned char bitnum)
{
  rgb_pixel_t color = (byte & (1<<bitnum)) ? img->fg : img->bg;
 
  if (verboseflg) {
    printf("(%ld,%ld)\t:", img->cursor_x, img->cursor_y);
    if (byte & (1<<bitnum)) printf("1\n");
    else printf("0\n");
  }
  if (!bmp_set_pixel(img->bmp, img->cursor_x, img->cursor_y, color)) {
    fprintf(stderr, "ERROR: from bmp_set_pixel\n");
    exit(-1);
  }
  img->numpixels++;
  if (img->cursor_x < img->endx) img->cursor_x++;
  else {
    if (verboseflg) printf("END of ROW x=%ld y=%ld:\n", img->cursor_x, img->cursor_y);
    img->cursor_x =  0;
    img->cursor_y++;
  }
}

inline void
img_draw_byte(image_t *img, unsigned char byte)
{
  img_draw_bit(img, byte, 0);
  img_draw_bit(img, byte, 1);
  img_draw_bit(img, byte, 2);
  img_draw_bit(img, byte, 3);
  img_draw_bit(img, byte, 4);
  img_draw_bit(img, byte, 5);
  img_draw_bit(img, byte, 6);
  img_draw_bit(img, byte, 7);
}


inline int
img_flush(image_t *img)
{
  int padcount=0;
  char outf[80];

  if (img->estr==0) {
    snprintf(outf, 80, "%d_%s_bs%ld_bi%d_%ldx%ld.bmp", 
	     img->num, 
	     img->fileprefix,
	     img->bsize,
	     img->blockimg,
	     img->endx+1, img->endy+1);
  } else {
    snprintf(outf, 80, "%d_%s_bs%ld_bi%d%s_%ldx%ld.bmp", 
	     img->num, 
	     img->fileprefix,
	     img->bsize,
	     img->blockimg,
	     img->estr,
	     img->endx+1, img->endy+1);
  }
  if (img_empty(img)) return 1;

  while (!img_full(img))  {
    padcount++;
    img_draw_bit(img, 0, 0);
  }

  if (!bmp_save(img->bmp, outf)) {
    perror("ERROR: bmp_save");
    return -1;
  }
  if (verboseflg) printf("%s: padded by %d\n", outf, padcount);
  img->num++;
  img->cursor_x  = img->cursor_y = 0;
  img->numpixels = 0;
  img->padcount += padcount;

  return 1;
}

uintptr_t 
img_paint_buf(image_t *img, uint8_t *buf, uintptr_t size)
{ 
  uintptr_t  i,j,bc;
  uintptr_t  bsize = img->bsize;
  int       blockimg = img->blockimg;
  
  bc=0;
  img->ecur = img->ehead;
  for (i=0, j=0; i<size; i++) { 
      if (img->ehead == 0 || 
	  (img->ecur && j>=img->ecur->s && j<=img->ecur->e)) {
	img_draw_byte(img, buf[i]);
      }
      if (img->ecur != 0 && j==img->ecur->e) img->ecur = img->ecur->next;
      if (img_full(img)) img_flush(img);
      j++;
      if (j==bsize) { 
	j=0; bc++;
	img->ecur = img->ehead;
	if (blockimg) img_flush(img);
      }
  }

  return size;
}

#ifdef __UI_STAND_ALONE__
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#define BUF_NUMBLOCKS (1024)

#if 0
void
ui_block_color(UI *ui, const uint8_t *blk, uintptr_t bsize, 
	       uint8_t *fg, uint8_t *bg)
{
  *bg = ui->colors[UI_COLOR_WHITE];
  *fg = ui->colors[UI_COLOR_BLACK];
}
#endif 

void
usage()
{
  fprintf(stderr, "2bmp [-r <rows>] [-c <cols>] [-z <zoom>] "
	  "[[-e <s,e>] [-e <s,e>] ...] [-b] [-v] <blocksize> <file>\n"
	  "   This program visualizes a file as one or more bit map images. \n"
	  "   It treats <file> as a raw binary stream of bytes.  Each byte  \n"
	  "   painted as 8 pixels one for each bit.  Bytes are treated in   \n"
	  "   logical units of blocks defined by the <blocksize> in bytes specified. \n"
	  "   The file is assumed to be composed of one or more blocks.     \n"
	  "   With block 0 starting at the begining of the file.            \n"
          "   -b        specifies that a image should be created for every block\n"
	  "   -e <s,e>  one or more -e <s,e> can be specified to only paint  \n"
          "             extracts of each block starting in the range from s to\n"
          "             e.  Where s and e are block indices from 0 to blocksize-1\n"
          "   -r <rows> specifes the number of rows in pixels for the images\n"
	  "   -c <cols> specifies the number of cols in pixels for the images\n"
          "   -v        verbose operation prints output of progress\n\n"
	  " eg. \n"
          "    2bmp -c 800 -b 65544 extract.sv.trc\n"
          "      Will create a series of images that are 800 pixels wide by the \n"
	  "      number of rows required to contain a block's worth of data \n"
          "      (padded as necessary).  -b forces one image to be created for\n"
          "      every block.  These images can be easily visualized with the \n"
          "      imagick animate command (see doanimate script)\n\n"
          "   2bmp -r 100 65544 extract.sv.trc\n"
          "      Will create a series of images that are 100 pixels high and \n"
          "      enough pixels wide to contain a single block (65544*8=524352 pixels.\n\n"
          "   2bmp -e 0,20 -e 600,700 -r 100 65544 extract.sv.trc\n"
          "      Similar to the above but will plot only the bytes 0-20 and 600-700\n"
          "      inclusive for each block.  The image will be sized 100 rows by\n"
          "      the number of pixels required for 122 bytes (122*8=976)\n");
}

int 
main(int argc, char **argv)
{
  int            fd, n;
  int            blockimg = 0;
  uintptr_t      cols=0, rows=0, offset, blocksize;
  char          *filename; 
  uint8_t        zoom=1;
  int            bufsize;
  unsigned char *buf;
  image_t        image;
  int            opt;
  int            estrlen=0;
  uintptr_t       ecount=0;
  image_extract_t *ehead = NULL;

  while ((opt = getopt(argc, argv, "r:c:z:e:bv")) != -1) {
    switch (opt) {
    case 'e': {
      int s,e;
      intptr_t rc;
      sscanf(optarg, "%d,%d", &s, &e);
      rc = img_extract_add(&ehead, s, e);
      if (rc<0) {
	fprintf(stderr, "ERROR: %s bad extract specification\n", optarg);
	exit (-1);
      }
      estrlen+=(strlen(optarg)+1);
      ecount += rc;
    }
      break;
    case 'z': 
      zoom = atoi(optarg);
      break;
    case 'r':
      rows = atoi(optarg);
      break; 
    case 'c':
      cols = atoi(optarg);
      break;
    case 'b':
      blockimg=1;
      break;
    case 'v':
      verboseflg=1;
      break;
    default:
      fprintf(stderr, "ERROR: unknown option: %c\n", opt);
      exit(-1);
    }
  }

  if (argc-optind < 2) {
    usage();
    return -1;
  }

  blocksize = atoi(argv[optind]);
  filename  = argv[optind+1];
  ecount    = (ecount==0) ? blocksize : ecount;
  cols      = (cols==0) ? (8*ecount) : cols;
  cols      = (cols > (8*ecount)) ? (8*ecount) : cols;

  if (rows==0) {
    rows = (8 * ecount) / cols;
    if ((rows * cols) != (8*ecount)) rows++;
  } 
  bufsize   = blocksize * BUF_NUMBLOCKS;

  if (cols % 8 != 0) {
    fprintf(stderr, "ERROR: cols=%ld must be a multiple of 8\n", cols);
    return -1;
  }

  if (cols * rows < ecount*8) {
    fprintf(stderr, "ERROR cols(%ld) * rows(%ld)=%ld  < ecount*8=%ld\n", 
	    cols, rows, cols*rows, ecount*8);
    return -1;
  }

  if (img_init(&image, blocksize, cols, rows, blockimg,
	       filename, ehead, estrlen)<0) {
    perror("ERROR: bmp_create");
    return -1;
  }

  buf       = malloc(bufsize);
  if (buf==0) {
    perror("ERROR: malloc");
    return -1;
  }
  
  fd = open(filename, O_RDONLY);
  if (fd < 0 ) {
    perror("ERROR: open");
    return -1;
  }

  if (verboseflg) {
    printf("file=%s blocksize=%ld cols=%ld rows=%ld zoom=%d,"
	 " bufsize=%d blockimg=%d extracts%s\n",
	 filename, blocksize, cols, rows, zoom, bufsize,
	 blockimg,image.estr);
    img_dump_extracts(&image);
  }

  while ((n = read(fd, buf, bufsize)) > 0) {
    offset = 0;
    while ((int)offset < n) {
      if (verboseflg) { 
	printf("offset=%ld n=%d buf=%p (%p) %ld\n", offset, n, buf, 
	     &(buf[offset]), 
	     blocksize); 
      }
      offset += img_paint_buf(&image, &(buf[offset]), 
			      n-offset); 
    }
  }

  img_flush(&image);

  if (verboseflg) {
    printf("total padding done = %ld\n", image.padcount);
  }
  img_cleanup(&image);

  return 0;
}
#endif
