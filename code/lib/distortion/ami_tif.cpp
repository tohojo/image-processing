/**
*    Please note: Except for lines CLEARLY MARKED with the phrase "Added by B Meadows, 2012",
*	 ALL code herein is authored by Luis Alvarez and Luis Gomez, AMI Research Group,
*    University of Las Palmas de Gran Canaria, Canary Islands, SPAIN (February 2010)
*	~ comment by Ben Meadows
*/

/*
 * Copyright 2009, 2010 IPOL Image Processing On Line http://www.ipol.im/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */




/**
 * \file ami_tif.h
 * \brief basic function to read/write some TIF image format
 * \author Carmelo Quintana (Luis Alvarez Master Thesis Student)
*/

#include <stdlib.h>
#include <stdio.h>

#include "ami_tif.h"

/*
********************************************************************************
           IMPLEMENTATION
********************************************************************************
*/

/*
 * Tag Image File Format (TIFF)
 *
 * Based on Rev 6.0
 */


#define	TIFF_VERSION	        42
#define TIFF_BIGTIFF_VERSION    43

#define	TIFF_BIGENDIAN		0x4d
#define	TIFF_LITTLEENDIAN	0x49


/*
 * Tag data type information.
 *
 * Note: RATIONALs are the ratio of two 32-bit integer values.
 */
#define TIFF_BYTE           1	/* 8-bit unsigned integer */
#define TIFF_ASCII          2	/* 8-bit bytes w/ last byte null */
#define TIFF_SHORT          3	/* 16-bit unsigned integer */
#define TIFF_LONG           4	/* 32-bit unsigned integer */
#define TIFF_RATIONAL       5	/* 64-bit unsigned fraction */
#define TIFF_SBYTE          6	/* !8-bit signed integer */
#define TIFF_UNDEFINED      7	/* !8-bit untyped data */
#define TIFF_SSHORT         8	/* !16-bit signed integer */
#define TIFF_SLONG          9	/* !32-bit signed integer */
#define TIFF_SRATIONAL      10	/* !64-bit signed fraction */
#define TIFF_FLOAT          11	/* !32-bit IEEE floating point */
#define TIFF_DOUBLE         12	/* !64-bit IEEE floating point */


/*
 * TIFF Tag Definitions.
 */
#define	TIFFTAG_SUBFILETYPE		    254	/* subfile data descriptor */
#define	    FILETYPE_REDUCEDIMAGE	        0x1	/* reduced resolution version */
#define	    FILETYPE_PAGE		        0x2	/* one page of many */
#define	    FILETYPE_MASK		        0x4	/* transparency mask */
#define	TIFFTAG_OSUBFILETYPE	            255	/* +kind of data in subfile */
#define	    OFILETYPE_IMAGE		        1	/* full resolution image data */
#define	    OFILETYPE_REDUCEDIMAGE	        2	/* reduced size image data */
#define	    OFILETYPE_PAGE		        3	/* one page of many */
#define	TIFFTAG_IMAGEWIDTH		    256	/* image width in pixels */
#define	TIFFTAG_IMAGELENGTH		    257	/* image length in pixels */
#define	TIFFTAG_BITSPERSAMPLE	            258	/* bits per channel (sample) */
#define	TIFFTAG_COMPRESSION		    259	/* data compression technique */
#define	    COMPRESSION_NONE		        1	/* dump mode */
#define	    COMPRESSION_CCITTRLE	        2	/* CCITT modified Huffman RLE */
#define	    COMPRESSION_CCITTFAX3	        3	/* CCITT Group 3 fax encoding */
#define	    COMPRESSION_CCITTFAX4	        4	/* CCITT Group 4 fax encoding */
#define	    COMPRESSION_LZW		        5       /* Lempel-Ziv  & Welch */
#define	    COMPRESSION_OJPEG		        6	/* !6.0 JPEG */
#define	    COMPRESSION_PACKBITS	        32773	/* Macintosh RLE */
#define	TIFFTAG_PHOTOMETRIC		    262	/* photometric interpretation */
#define	    PHOTOMETRIC_MINISWHITE	        0	/* min value is white */
#define	    PHOTOMETRIC_MINISBLACK	        1	/* min value is black */
#define	    PHOTOMETRIC_RGB		        2	/* RGB color model */
#define	    PHOTOMETRIC_PALETTE		        3	/* color map indexed */
#define	    PHOTOMETRIC_MASK		        4	/* $holdout mask */
#define	    PHOTOMETRIC_SEPARATED	        5	/* !color separations */
#define	    PHOTOMETRIC_YCBCR		        6	/* !CCIR 601 */
#define	    PHOTOMETRIC_CIELAB		        8	/* !1976 CIE L*a*b* */
#define	TIFFTAG_THRESHHOLDING	            263	/* +thresholding used on data */
#define	    THRESHHOLD_BILEVEL		        1	/* b&w art scan */
#define	    THRESHHOLD_HALFTONE		        2	/* or dithered scan */
#define	    THRESHHOLD_ERRORDIFFUSE	        3	/* usually floyd-steinberg */
#define	TIFFTAG_CELLWIDTH		    264	/* +dithering matrix width */
#define	TIFFTAG_CELLLENGTH		    265	/* +dithering matrix length */
#define	TIFFTAG_FILLORDER		    266	/* data order within a byte */
#define	    FILLORDER_MSB2LSB		        1	/* most significant -> least */
#define	    FILLORDER_LSB2MSB		        2	/* least significant -> most */
#define	TIFFTAG_DOCUMENTNAME	            269	/* name of doc. image is from */
#define	TIFFTAG_IMAGEDESCRIPTION            270	/* info about image */
#define	TIFFTAG_MAKE			    271	/* scanner manufacturer name */
#define	TIFFTAG_MODEL			    272	/* scanner model name/number */
#define	TIFFTAG_STRIPOFFSETS	            273	/* offsets to data strips */
#define	TIFFTAG_ORIENTATION		    274	/* +image orientation */
#define	    ORIENTATION_TOPLEFT		        1	/* row 0 top, col 0 lhs */
#define	    ORIENTATION_TOPRIGHT	        2	/* row 0 top, col 0 rhs */
#define	    ORIENTATION_BOTRIGHT	        3	/* row 0 bottom, col 0 rhs */
#define	    ORIENTATION_BOTLEFT		        4	/* row 0 bottom, col 0 lhs */
#define	    ORIENTATION_LEFTTOP		        5	/* row 0 lhs, col 0 top */
#define	    ORIENTATION_RIGHTTOP	        6	/* row 0 rhs, col 0 top */
#define	    ORIENTATION_RIGHTBOT	        7	/* row 0 rhs, col 0 bottom */
#define	    ORIENTATION_LEFTBOT		        8	/* row 0 lhs, col 0 bottom */
#define	TIFFTAG_SAMPLESPERPIXEL             277	/* samples per pixel */
#define	TIFFTAG_ROWSPERSTRIP	            278	/* rows per strip of data */
#define	TIFFTAG_STRIPBYTECOUNTS	            279	/* bytes counts for strips */
#define	TIFFTAG_MINSAMPLEVALUE	            280	/* +minimum sample value */
#define	TIFFTAG_MAXSAMPLEVALUE	            281	/* +maximum sample value */
#define	TIFFTAG_XRESOLUTION		    282	/* pixels/resolution in x */
#define	TIFFTAG_YRESOLUTION		    283	/* pixels/resolution in y */
#define	TIFFTAG_PLANARCONFIG	            284	/* storage organization */
#define	    PLANARCONFIG_CONTIG		        1	/* single image plane */
#define	    PLANARCONFIG_SEPARATE	        2	/* separate planes of data */
#define	TIFFTAG_PAGENAME		    285	/* page name image is from */
#define	TIFFTAG_XPOSITION		    286	/* x page offset of image lhs */
#define	TIFFTAG_YPOSITION		    287	/* y page offset of image lhs */
#define	TIFFTAG_FREEOFFSETS		    288	/* +byte offset to free block */
#define	TIFFTAG_FREEBYTECOUNTS	            289	/* +sizes of free blocks */
#define	TIFFTAG_GRAYRESPONSEUNIT            290	/* $gray scale curve accuracy */
#define	    GRAYRESPONSEUNIT_10S	        1	/* tenths of a unit */
#define	    GRAYRESPONSEUNIT_100S	        2	/* hundredths of a unit */
#define	    GRAYRESPONSEUNIT_1000S	        3	/* thousandths of a unit */
#define	    GRAYRESPONSEUNIT_10000S	        4	/* ten-thousandths of a unit */
#define	    GRAYRESPONSEUNIT_100000S	        5	/* hundred-thousandths */
#define	TIFFTAG_GRAYRESPONSECURVE	    291	/* $gray scale response curve */
#define	TIFFTAG_GROUP3OPTIONS		    292	/* 32 flag bits */
#define	TIFFTAG_T4OPTIONS		    292	/* TIFF 6.0 proper name alias */
#define	    GROUP3OPT_2DENCODING	        0x1	/* 2-dimensional coding */
#define	    GROUP3OPT_UNCOMPRESSED	        0x2	/* data not compressed */
#define	    GROUP3OPT_FILLBITS		        0x4	/* fill to byte boundary */
#define TIFFTAG_T6OPTIONS                   293 /* TIFF 6.0 proper name */
#define	    GROUP4OPT_UNCOMPRESSED	        0x2	/* data not compressed */
#define	TIFFTAG_RESOLUTIONUNIT		    296	/* units of resolutions */
#define	    RESUNIT_NONE		        1	/* no meaningful units */
#define	    RESUNIT_INCH		        2	/* english */
#define	    RESUNIT_CENTIMETER		        3	/* metric */
#define	TIFFTAG_PAGENUMBER		    297	/* page numbers of multi-page */
#define	TIFFTAG_COLORRESPONSEUNIT	    300	/* $color curve accuracy */
#define	    COLORRESPONSEUNIT_10S	        1	/* tenths of a unit */
#define	    COLORRESPONSEUNIT_100S	        2	/* hundredths of a unit */
#define	    COLORRESPONSEUNIT_1000S	        3	/* thousandths of a unit */
#define	    COLORRESPONSEUNIT_10000S	        4	/* ten-thousandths of a unit */
#define	    COLORRESPONSEUNIT_100000S	        5	/* hundred-thousandths */
#define	TIFFTAG_TRANSFERFUNCTION	    301	/* !colorimetry info */
#define	TIFFTAG_SOFTWARE		    305	/* name & release */
#define	TIFFTAG_DATETIME		    306	/* creation date and time */
#define	TIFFTAG_ARTIST			    315	/* creator of image */
#define	TIFFTAG_HOSTCOMPUTER		    316	/* machine where created */
#define	TIFFTAG_PREDICTOR		    317	/* prediction scheme w/ LZW */
#define	TIFFTAG_WHITEPOINT		    318	/* image white point */
#define	TIFFTAG_PRIMARYCHROMATICITIES	    319	/* !primary chromaticities */
#define	TIFFTAG_COLORMAP		    320	/* RGB map for pallette image */
#define	TIFFTAG_HALFTONEHINTS		    321	/* !highlight+shadow info */
#define	TIFFTAG_TILEWIDTH		    322	/* !rows/data tile */
#define	TIFFTAG_TILELENGTH		    323	/* !cols/data tile */
#define TIFFTAG_TILEOFFSETS		    324	/* !offsets to data tiles */
#define TIFFTAG_TILEBYTECOUNTS		    325	/* !byte counts for tiles */
#define	TIFFTAG_INKSET			    332	/* !inks in separated image */
#define	    INKSET_CMYK			        1	/* !cyan-magenta-yellow-black color */
#define	    INKSET_MULTIINK		        2	/* !multi-ink or hi-fi color */
#define	TIFFTAG_INKNAMES		    333	/* !ascii names of inks */
#define	TIFFTAG_NUMBEROFINKS		    334	/* !number of inks */
#define	TIFFTAG_DOTRANGE		    336	/* !0% and 100% dot codes */
#define	TIFFTAG_TARGETPRINTER		    337	/* !separation target */
#define	TIFFTAG_EXTRASAMPLES		    338	/* !info about extra samples */
#define	    EXTRASAMPLE_UNSPECIFIED	        0	/* !unspecified data */
#define	    EXTRASAMPLE_ASSOCALPHA	        1	/* !associated alpha data */
#define	    EXTRASAMPLE_UNASSALPHA	        2	/* !unassociated alpha data */
#define	TIFFTAG_SAMPLEFORMAT		    339	/* !data sample format */
#define	    SAMPLEFORMAT_UINT		        1	/* !unsigned integer data */
#define	    SAMPLEFORMAT_INT		        2	/* !signed integer data */
#define	    SAMPLEFORMAT_IEEEFP		        3	/* !IEEE floating point data */
#define	    SAMPLEFORMAT_VOID		        4	/* !untyped data */
#define	    SAMPLEFORMAT_COMPLEXINT	        5	/* !complex signed int */
#define	    SAMPLEFORMAT_COMPLEXIEEEFP	        6	/* !complex ieee floating */
#define	TIFFTAG_SMINSAMPLEVALUE		    340	/* !variable MinSampleValue */
#define	TIFFTAG_SMAXSAMPLEVALUE		    341	/* !variable MaxSampleValue */


/*
 * TIFF number of Directory entries.
 */
#define TIFF_NENTRIES_WITH_TRANS            15
#define TIFF_NENTRIES_WITHOUT_TRANS         14


#define ami_qhc_seek_allocate_read(file,offset,type,nele,vector) { int tt;\
        fseek(file,offset,SEEK_SET);\
        tt = (type == 0) ? sizeof(short) : sizeof(long); tt *= nele;\
        ami_malloc1d(vector,unsigned char,tt);\
        fread(vector,tt,1,file);\
        }


/*
********************************************************************************
********************************************************************************
*/

/* SHORT to HEX in byte order II */
void hexadecimal2 (int source, unsigned char *dest)
{
  short A0 = 0, A1 = 0;

  A1 = source & 0xFF00;
  A1 >>= 8;
  A0 = source & 0x00FF;

  dest[0] = A0;
  dest[1] = A1;
}


/* LONG to HEX in byte order II */
void hexadecimal4 (int source, unsigned char *dest)
{
  long A0 = 0, A1 = 0, A2 = 0, A3 = 0;

  A3 = source & 0xFF000000;
  A3 >>= 24;
  A2 = source & 0x00FF0000;
  A2 >>= 16;
  A1 = source & 0x0000FF00;
  A1 >>= 8;
  A0 = source & 0x000000FF;

  dest[0] = A0;
  dest[1] = A1;
  dest[2] = A2;
  dest[3] = A3;
}


/* Write a directory entry */
void writeDirectoryEntry (struct TIFF_entry *entry, unsigned char *data)
{
  hexadecimal2(entry->tag,data);
  hexadecimal2(entry->type,&data[2]);
  hexadecimal4(entry->num,&data[4]);
  hexadecimal4(entry->valueOffset,&data[8]);
}


/* HEX to SHORT in byte order II or MM */
short integer2(unsigned char *source, int IIorMM)
{
  int F0;

  if (IIorMM) {
    F0 = source[0] << 8;
    return F0 | source[1];
  }
  else {
    F0 = source[1] << 8;
    return F0 | source[0];
  }
}


/* HEX to LONG in byte order II or MM without type & num */
long integer4(unsigned char *source, int IIorMM)
{
  int F0, F1, F2;

  if (IIorMM) {
    F0 = source[0] << 24;
    F1 = source[1] << 16;
    F2 = source[2] << 8;
    return F0 | F1 | F2 | source[3];
  }
  else {
    F0 = source[3] << 24;
    F1 = source[2] << 16;
    F2 = source[1] << 8;
    return F0 | F1 | F2 | source[0];
  }
}


/* HEX to LONG in byte order II or MM with type & num */
long integer4Value(unsigned char *source, int IIorMM, int type, int num)
{
  int F0, F1, F2;

  if ((IIorMM) && (type == TIFF_SHORT) && (num == 1)) {
    F0 = source[2] << 24;
    F1 = source[3] << 16;
    F2 = source[0] << 8;
    return F0 | F1 | F2 | source[1];
  }
  else
    return integer4(source,IIorMM);
}


/* Read a msg file and fill myTiff with the default information */
int read_msg_TIFF_struct (const char name[200], FILE *fptr, struct TIFF_file *myTiff)
{
  int depth, nPixels = 1024*1024;
  long inicio, final, total;

  fseek(fptr,0,SEEK_SET); inicio = ftell(fptr); fseek(fptr,0,SEEK_END); final = ftell(fptr); total = final-inicio;

  if ((total /= nPixels) == sizeof(char)) {
    myTiff->image.nBytes = sizeof(char)*nPixels; myTiff->image.charOrShort = 0; depth = 8;
  }
  else if (total == sizeof(short)) {
    myTiff->image.nBytes = sizeof(short)*nPixels; myTiff->image.charOrShort = 1; depth = 16;
  }
  else { fprintf(stderr,"The file \"%s\" is not a tiff file or msg image with 8 or 16 bits depth\n",name); return -1; }

  myTiff->width = 1024; myTiff->length = 1024; myTiff->samplesPerPixel = 1;
  myTiff->image.nPixels = nPixels; myTiff->image.IIorMM = 0; total = myTiff->image.nBytes;
  ami_malloc1d(myTiff->image.data,unsigned char,total); fseek(fptr,0,SEEK_SET); fread(myTiff->image.data,total,1,fptr);
  fprintf(stderr,"WARNING: The file \"%s\" is not a tiff file (It will treated like a 1024 x 1024 MSG IMAGE WITH %d BITS DEPTH)\n",name,depth);
  return 0;
}


/* Read a tiff file called "name" and fill myTiff with the information */
int read_tiff_TIFF_struct (const char name[200], struct TIFF_file *myTiff)
{
  FILE *fptr;
  int retorno, temp, i, sizeTemp;
  struct TIFF_struct info;
  struct TIFF_entry entry;
  unsigned char *data, *pO, *pBC;


  /* We initialize the pointers */
  retorno = 0; entry.tam = 2*sizeof(short)+2*sizeof(long);
  info.bitsPerSample.bitsPerSample = NULL;
  info.stripOffset.data = NULL;
  info.stripByteCounts.data = NULL;
  info.extraSamples = NULL;
  myTiff->image.data = NULL;

  /* Allocate memory maximun to a directory entry and error string */
  ami_malloc1d(data,unsigned char,entry.tam);

  /* Open file */
  if ((fptr = fopen(name,"rb")) == NULL) {
    fprintf(stderr,"Unable to open tif file \"%s\"\n",name); retorno = -1; goto fin_read;
  }


  /* 1. WE READ AND FILL THE HEAD */
  fread(data,2*sizeof(short)+sizeof(long),1,fptr);

  /* 1.1. Byte order */
  if (data[0] == data[1]) {
    if (data[0] == TIFF_LITTLEENDIAN) myTiff->image.IIorMM = info.header.IIorMM = 0; /* II */
    else if (data[0] == TIFF_BIGENDIAN) myTiff->image.IIorMM = info.header.IIorMM = 1; /* MM */
    else { retorno = read_msg_TIFF_struct(name,fptr,myTiff); goto fin_read; }
  }
  else { retorno = read_msg_TIFF_struct(name,fptr,myTiff); goto fin_read; }

  /* 1.2. Is a tiff image? */
  temp = info.header.magicNumber = integer2(&data[2],info.header.IIorMM);
  if ((temp != TIFF_VERSION) && (temp != TIFF_BIGTIFF_VERSION)) {
    fprintf(stderr,"The file \"%s\" is not a tiff file\n",name); retorno = -1; goto fin_read;
  }

  /* 1.3. Offset of the first IFD */
  temp = info.header.offsetIFD = integer4(&data[4],info.header.IIorMM);

  /* 1.4. Seek the pointer in the first IFD and get the num of directory entries */
  fseek(fptr,temp,SEEK_SET);
  fread(data,sizeof(short),1,fptr);
  info.header.nDirectoryEntries = integer2(data,info.header.IIorMM);


  /* 2. WE READ THE DIRECTORY ENTRIES */
  for (i = 0; i < info.header.nDirectoryEntries; i++) {

    /* 2.1. Read the next directory entry */
    fread(data,entry.tam,1,fptr);
    entry.tag = integer2(data,info.header.IIorMM);
    entry.type = integer2(&data[2],info.header.IIorMM);
    entry.num = integer4(&data[4],info.header.IIorMM);
    entry.valueOffset = integer4Value(&data[8],info.header.IIorMM,entry.type,entry.num);

    /* 2.2. Fill the further info of tiff file */
    switch (entry.tag) {
    case TIFFTAG_IMAGEWIDTH:
      myTiff->width = entry.valueOffset;
      break;
    case TIFFTAG_IMAGELENGTH:
      myTiff->length = entry.valueOffset;
      break;
    case TIFFTAG_BITSPERSAMPLE:
      ami_malloc1d(info.bitsPerSample.bitsPerSample,int,entry.num);
      if ((info.bitsPerSample.samplesPerPixel = entry.num) == 1) {
	info.bitsPerSample.bitsPerSample[0] = info.bitsPerSample.bitsPerPixel = temp = entry.valueOffset;
	myTiff->image.charOrShort = (temp == 8) ? 0 : 1;
	if ((temp != 8) && (temp != 16)) {
	  fprintf(stderr,"The file \"%s\" uses a data type not available\n",name); retorno = -1; goto fin_read;
	}
      }
      else if ((entry.num == 3) || (entry.num == 4)) {
	info.bitsPerSample.offset = entry.valueOffset; myTiff->image.charOrShort = 0;
      }
      else {
	fprintf(stderr,"The file \"%s\" has a number of channels not available\n",name); retorno = -1; goto fin_read;
      }
      break;
    case TIFFTAG_COMPRESSION:
      if ((info.compression = entry.valueOffset) != COMPRESSION_NONE) {
	fprintf(stderr,"The file \"%s\" uses a compresion method not available\n",name); retorno = -1;  goto fin_read;
      }
      break;
    case TIFFTAG_PHOTOMETRIC:
      if ((info.photometric = entry.valueOffset) > PHOTOMETRIC_RGB) {
	fprintf(stderr,"The file \"%s\" has a photometric not available\n",name); retorno = -1; goto fin_read;
      }
      break;
    case TIFFTAG_FILLORDER:
      if ((info.fillOrder = entry.valueOffset) != FILLORDER_MSB2LSB) {
	fprintf(stderr,"The file \"%s\" has a fillorder not valid\n",name); retorno = -1; goto fin_read;
      }
      break;
    case TIFFTAG_STRIPOFFSETS:
      info.stripOffset.valueOffset = entry.valueOffset;
      if ((info.stripOffset.nStrips = entry.num) > 1) {
	if (entry.type == TIFF_SHORT) info.stripOffset.shortOrLong = 0;
	else if (entry.type == TIFF_LONG) info.stripOffset.shortOrLong = 1;
	else {
	  fprintf(stderr,"The file \"%s\" has an error about type definition strips\n",name); retorno = -1; goto fin_read;
	}
      }
      break;
    case TIFFTAG_SAMPLESPERPIXEL:
      myTiff->samplesPerPixel = entry.valueOffset;
      if ((entry.valueOffset != 1) && (entry.valueOffset != 3) && (entry.valueOffset != 4)) {
	fprintf(stderr,"The file \"%s\" not is a bilevel, grayscale or RGB image\n",name);
	retorno = -1; goto fin_read;
      }
      break;
    case TIFFTAG_ROWSPERSTRIP:
      info.rowsPerStrip = entry.valueOffset;
      break;
    case TIFFTAG_STRIPBYTECOUNTS:
      info.stripByteCounts.valueOffset = entry.valueOffset;
      if ((info.stripByteCounts.nStrips = entry.num) > 1) {
	if (entry.type == TIFF_SHORT) info.stripByteCounts.shortOrLong = 0;
	else if (entry.type == TIFF_LONG) info.stripByteCounts.shortOrLong = 1;
	else {
	  fprintf(stderr,"The file \"%s\" has an error about type definition strips\n",name); retorno = -1; goto fin_read;
	}
      }
      break;
    case TIFFTAG_XRESOLUTION:
      info.xResolution.offset = entry.valueOffset;
      break;
    case TIFFTAG_YRESOLUTION:
      info.yResolution.offset = entry.valueOffset;
      break;
    case TIFFTAG_PLANARCONFIG:
      if ((info.planarConfiguration = entry.valueOffset) != PLANARCONFIG_CONTIG) {
	fprintf(stderr,"The file \"%s\" uses a planar configuration not available\n",name); retorno = -1; goto fin_read;
      }
      break;
    case TIFFTAG_RESOLUTIONUNIT:
      info.resolutionUnit = entry.valueOffset;
      break;
    case TIFFTAG_EXTRASAMPLES:
      if (entry.num == 1) {
	ami_malloc1d(info.extraSamples,int,1);
	if ((info.extraSamples[0] = entry.valueOffset) != EXTRASAMPLE_UNASSALPHA) {
	  fprintf(stderr,"The file \"%s\" uses a extrasample not available\n",name); retorno = -1; goto fin_read;
	}
      }
      else {
	fprintf(stderr,"The file \"%s\" has a number of extra channels not valid\n",name); retorno = -1; goto fin_read;
      }
      break;
    default:
      break;
    }
  }

  /* 2.3. Read the offset of the next IFD */
  fread(data,sizeof(long),1,fptr);
  info.header.offsetIFD = integer4(data,info.header.IIorMM);


  /* 3. TESTS */

  /* 3.1. is there other IFD? */
  if (info.header.offsetIFD) fprintf(stderr,"WARNING: The file \"%s\" has many images (We read only the first)\n",name);

  /* 3.2. Any error about the number of samples per pixel ?  */
  if ((info.bitsPerSample.samplesPerPixel != myTiff->samplesPerPixel) ||
      ((info.extraSamples != NULL) && (myTiff->samplesPerPixel != 4)) ||
      ((myTiff->samplesPerPixel == 4) && (info.extraSamples == NULL))) {
    fprintf(stderr,"The file \"%s\" has an error about the num of channels\n",name); retorno = -1; goto fin_read;
  }

  /* 3.3. Any error about the number of samples per pixel and photometric ? */
  if (((myTiff->samplesPerPixel == 1) && (info.photometric > PHOTOMETRIC_MINISBLACK)) ||
      ((info.photometric < PHOTOMETRIC_RGB) && (myTiff->samplesPerPixel != 1)) ||
      ((myTiff->samplesPerPixel > 1) && (info.photometric != PHOTOMETRIC_RGB)) ||
      ((info.photometric == PHOTOMETRIC_RGB) && (myTiff->samplesPerPixel == 1))) {
    fprintf(stderr,"The file \"%s\" has an error about the num of channels and the color space\n",name);
    retorno = -1; goto fin_read;
  }

  /* 3.4. Any error about the num of strips ? */
  if (info.stripOffset.nStrips != info.stripByteCounts.nStrips) {
    fprintf(stderr,"The file \"%s\" has an error about the num of strips\n",name); retorno = -1; goto fin_read;
  }


  /* 4. READ THE BITS PER SAMPLE (if samplesPerPixel > 1) */
  if ((temp = myTiff->samplesPerPixel) > 1) {
    fseek(fptr,info.bitsPerSample.offset,SEEK_SET);
    fread(data,sizeof(short)*temp,1,fptr);

    for (info.bitsPerSample.bitsPerPixel = i = 0, pO = data; i < temp; i++, pO += 2) {
      if ((info.bitsPerSample.bitsPerSample[i] = integer2(pO,info.header.IIorMM)) == 8)
	info.bitsPerSample.bitsPerPixel += 8;
      else {
	fprintf(stderr,"The file \"%s\" has an error about the depht of the channels\n",name);
	retorno = -1; goto fin_read;
      }
    }
  }


  /* 5. FILL MORE INFORMATION */
  temp = myTiff->image.nPixels = myTiff->width*myTiff->length;
  temp = myTiff->image.nBytes = temp*info.bitsPerSample.bitsPerPixel/8;
  ami_malloc1d(myTiff->image.data,unsigned char,temp);


  /* 6. READ THE IMAGE */
  if (info.stripOffset.nStrips == 1) { /* 6.1. Read the unique strip */
    if (temp != info.stripByteCounts.valueOffset) {
      fprintf(stderr,"The file \"%s\" has a number of bytes not valid\n",name); retorno = -1; goto fin_read;
    }
    /* 6.1.1. Seek the pointer file and read the data */
    fseek(fptr,info.stripOffset.valueOffset,SEEK_SET);
    fread(myTiff->image.data,temp,1,fptr);
  }
  else if (info.stripOffset.nStrips > 1) { /* 6.2. Read all strips */
    /* 6.2.1. Read the offsets/bytecounts of strips */
    ami_qhc_seek_allocate_read(fptr,info.stripOffset.valueOffset,info.stripOffset.shortOrLong,
			       info.stripOffset.nStrips,info.stripOffset.data);
    ami_qhc_seek_allocate_read(fptr,info.stripByteCounts.valueOffset,info.stripByteCounts.shortOrLong,
			       info.stripByteCounts.nStrips,info.stripByteCounts.data);

    /* 6.2.2. And now... read the image */
    for (sizeTemp = i = 0, pO = info.stripOffset.data, pBC = info.stripByteCounts.data;
	 i < info.stripOffset.nStrips; i++, sizeTemp += temp) {
      /* 6.2.2.1. Seek the cursor to each strip */
      if (info.stripOffset.shortOrLong == 0) { temp = integer2(pO,info.header.IIorMM); pO += 2; }
      else { temp = integer4(pO,info.header.IIorMM); pO += 4; }
      fseek(fptr,temp,SEEK_SET);

      /* 6.2.2.2. Num of byte of each strip and read it */
      if (info.stripByteCounts.shortOrLong == 0) { temp = integer2(pBC,info.header.IIorMM); pBC += 2; }
      else { temp = integer4(pBC,info.header.IIorMM); pBC += 4; }
      fread(&myTiff->image.data[sizeTemp],temp,1,fptr);
    }

    if (myTiff->image.nBytes != sizeTemp) {
      fprintf(stderr,"The file \"%s\" has a number of bytes not valid\n",name); retorno = -1; goto fin_read;
    }
  }
  else {
    fprintf(stderr,"The file \"%s\" has a number os strips not valid\n",name); retorno = -1; goto fin_read;
  }


  /* 7. READ X & Y RESOLUTION */
  temp = sizeof(long)*2;

  /* 7.1. Read x resolution */
  fseek(fptr,info.xResolution.offset,SEEK_SET); fread(data,temp,1,fptr);
  info.xResolution.num = integer4(data,info.header.IIorMM);
  info.xResolution.den = integer4(&data[4],info.header.IIorMM);

  /* 7.2. Read y resolution */
  fseek(fptr,info.yResolution.offset,SEEK_SET); fread(data,temp,1,fptr);
  info.yResolution.num = integer4(data,info.header.IIorMM);
  info.yResolution.den = integer4(&data[4],info.header.IIorMM);

 fin_read:

  /* 8. CLOSE FILE AND FREE USED MEMORY */
  fclose(fptr); free(data); pO = pBC = NULL;
  if (info.bitsPerSample.bitsPerSample != NULL) free(info.bitsPerSample.bitsPerSample);
  if (info.stripByteCounts.data != NULL) free(info.stripByteCounts.data);
  if (info.stripOffset.data != NULL) free(info.stripOffset.data);
  if (info.extraSamples != NULL) free(info.extraSamples);

  return retorno;
}


int read_tiff_unsigned_char (
const char name[200] /** Input image file name to read */,
unsigned char **grey /** Output grey level image data (pointer memory is allocated inside the function call) */,
unsigned char **r /** Output RED image data (pointer memory is allocated inside the function call) */,
unsigned char **g /** Output GREEN image data (pointer memory is allocated inside the function call) */,
unsigned char **b /** Output BLUE image data (pointer memory is allocated inside the function call) */,
int *width /** Output Image width */,
int *height /** Output Image height */
)
{
  struct TIFF_file myTiff;
  int retorno, i, j, size;
  unsigned char *p;
  int charOrShort;

  /* WE FIT TO NULL IMAGE POINTERS */
  if(grey[0]!=NULL){ free(grey[0]); grey[0]=NULL; }
  if(r[0]!=NULL){ free(r[0]); r[0]=NULL; }
  if(g[0]!=NULL){ free(g[0]); g[0]=NULL; }
  if(b[0]!=NULL){ free(b[0]); b[0]=NULL; }

  retorno = read_tiff_TIFF_struct(name,&myTiff);  /* Call the read function */

  if (retorno != -1) {
    if (myTiff.samplesPerPixel == 1) { /* It's a bilever or grayscale image */
      *width = myTiff.width; *height = myTiff.length; charOrShort = myTiff.image.charOrShort;
      if(charOrShort!=0) return(-1);
      ami_malloc1d(grey[0],unsigned char,myTiff.image.nPixels);
      if (myTiff.image.charOrShort == 0)
        for (i = 0; i < myTiff.image.nBytes; i++) grey[0][i] = (unsigned char) myTiff.image.data[i];
      else
	for (i = j = 0, p = myTiff.image.data; i < myTiff.image.nBytes; j++, p += 2, i += 2)
	  grey[0][j] = (unsigned char) integer2(p,myTiff.image.IIorMM);
    }
    else if (myTiff.samplesPerPixel == 3) { /* It's a RGB full-color image */
      *width = myTiff.width; *height = myTiff.length; size = myTiff.image.nPixels;
      ami_malloc1d(r[0],unsigned char,size); ami_malloc1d(g[0],unsigned char,size); ami_malloc1d(b[0],unsigned char,size);
      for (i = j = 0, p = myTiff.image.data; i < myTiff.image.nBytes; j++, i += 3, p += 3) {
        r[0][j] = (unsigned char) p[0]; g[0][j] = (unsigned char) p[1]; b[0][j] = (unsigned char) p[2];
      }
    }
    else { retorno = -1; }
  }
  if(retorno==-1)
    fprintf(stderr,"The file \"%s\" is not a TIF unsigned char basic format\n",name);

  if (myTiff.image.data != NULL) free(myTiff.image.data);
  return retorno;
}


int read_tiff_1c (
   char name[200],
   float **data,
   int *width, int *length,
   int *charOrShort
   )
{
  struct TIFF_file myTiff;
  int retorno, i, j;
  unsigned char *p;

  retorno = read_tiff_TIFF_struct(name,&myTiff);  /* Call the read function */

  if (retorno != -1) {
    if (myTiff.samplesPerPixel == 1) { /* It's a bilever or grayscale image */
      *width = myTiff.width; *length = myTiff.length; *charOrShort = myTiff.image.charOrShort;
      ami_malloc1d(data[0],float,myTiff.image.nPixels);
      if (myTiff.image.charOrShort == 0)
	for (i = 0; i < myTiff.image.nBytes; i++) data[0][i] = (float) myTiff.image.data[i];
      else
	for (i = j = 0, p = myTiff.image.data; i < myTiff.image.nBytes; j++, p += 2, i += 2)
	  data[0][j] = (float) integer2(p,myTiff.image.IIorMM);
    }
    else { fprintf(stderr,"The file \"%s\" is not a bilevel or grayscale image\n",name); retorno = -1; }
  }

  if (myTiff.image.data != NULL) free(myTiff.image.data);
  return retorno;
}


int read_tiff_3c (
   char name[200],
   unsigned char **r, unsigned char **g, unsigned char **b,
   int *width, int *length
   )
{
  struct TIFF_file myTiff;
  int retorno, size, i, j;
  unsigned char *p;

  retorno = read_tiff_TIFF_struct(name,&myTiff);  /* Call the read function */

  if (retorno != -1) {
    if (myTiff.samplesPerPixel == 3) { /* It's a RGB full-color image */
      *width = myTiff.width; *length = myTiff.length; size = myTiff.image.nPixels;
      ami_malloc1d(r[0],unsigned char,size); ami_malloc1d(g[0],unsigned char,size); ami_malloc1d(b[0],unsigned char,size);
      for (i = j = 0, p = myTiff.image.data; i < myTiff.image.nBytes; j++, i += 3, p += 3) {
        r[0][j] = (unsigned char) p[0]; g[0][j] = (unsigned char) p[1]; b[0][j] = (unsigned char) p[2];
      }
    }
    else { fprintf(stderr,"The file \"%s\" is not a RGB Full-Color image\n",name); retorno = -1; }
  }

  if (myTiff.image.data != NULL) free(myTiff.image.data);
  return retorno;
}


int read_tiff_4c (
   char name[200],
   unsigned char **r, unsigned char **g, unsigned char **b, unsigned char **trans,
   int *width, int *length
   )
{
  struct TIFF_file myTiff;
  int retorno, size, i, j;
  unsigned char *p;

  retorno = read_tiff_TIFF_struct(name,&myTiff);   /* Call the read function */

  if (retorno != -1) {
    if (myTiff.samplesPerPixel == 4) { /* It's a RGB full-color with trans channel image */
      *width = myTiff.width; *length = myTiff.length; size = myTiff.image.nPixels;
      ami_malloc1d(r[0],unsigned char,size); ami_malloc1d(g[0],unsigned char,size);
      ami_malloc1d(b[0],unsigned char,size); ami_malloc1d(trans[0],unsigned char,size);
      for (i = j = 0, p = myTiff.image.data; i < myTiff.image.nBytes; j++, i += 4, p += 4) {
	r[0][j] = (unsigned char) p[0]; g[0][j] = (unsigned char) p[1]; b[0][j] = (unsigned char) p[2];
	trans[0][j] = (unsigned char) p[3];
      }
    }
    else { fprintf(stderr,"The file \"%s\" is not a RGB Full-Color with transparency channel image\n",name); retorno--; }
  }

  if (myTiff.image.data != NULL) free(myTiff.image.data);
  return retorno;
}


/* Create and write a file tiff called "name" given by myTiff */
int write_tiff_TIFF_struct  (char name[200], struct TIFF_file *myTiff)
{
  FILE *fptr;
  int i, j, retorno;
  long temp, offset;
  struct TIFF_struct info;
  struct TIFF_entry entry;
  unsigned char *data, *p;


  /* We initialize the pointers */
  retorno = 0; entry.tam = 2*sizeof(short)+2*sizeof(long);
  info.bitsPerSample.bitsPerSample = NULL;
  info.stripOffset.data = NULL;
  info.stripByteCounts.data = NULL;
  info.extraSamples = NULL;

  /* Allocate memory: maximun data is 4*sizeof(long) -> 2 RATIONAL */
  ami_malloc1d(data,unsigned char,4*sizeof(long));

  /* Create file */
  if ((fptr = fopen(name,"wb")) == NULL) {
    fprintf(stderr,"Unable to create tif file \"%s\"\n",name); retorno = -1; goto fin_write;
  }

  /* 1. WRITE THE HEADER */
  /* 1.1. Order byte II. We change myTiff.image.IIorMM if it's necessary */
  myTiff->image.IIorMM = info.header.IIorMM = 0; data[0] = data[1] = TIFF_LITTLEENDIAN; offset = sizeof(short);

  /* 1.2. Magic Number */
  info.header.magicNumber = TIFF_VERSION; hexadecimal2(info.header.magicNumber,&data[offset]); offset += sizeof(short);

  /* 1.3. Offset of the IFD */
  temp = offset;
  info.header.offsetIFD = offset += sizeof(long); hexadecimal4(offset,&data[temp]);
  fwrite(data,offset,1,fptr);


  /* 2. WRITE THE NUMBER OF THE IFD ENTRIES */
  fseek(fptr,offset,SEEK_SET);
  info.header.nDirectoryEntries = (myTiff->samplesPerPixel == 4) ? TIFF_NENTRIES_WITH_TRANS  : TIFF_NENTRIES_WITHOUT_TRANS;
  hexadecimal2(info.header.nDirectoryEntries,data); fwrite(data,sizeof(short),1,fptr);
  offset += sizeof(short); temp = offset;
  offset += info.header.nDirectoryEntries*entry.tam+sizeof(long); /* offset + IFD's end and the offset of the next IFD */


  /* 3. WRITE THE DIRECTORY ENTRIES */
  for (i = 0; i < info.header.nDirectoryEntries; i++, temp += entry.tam) {
    /* 3.1. Seek the pointer in the next entry */
    fseek(fptr,temp,SEEK_SET);

    /* 3.2. Create the directory entry i and fill the further information */
    switch (i) {
    case 0: /* The width */
      entry.tag = TIFFTAG_IMAGEWIDTH; entry.type = TIFF_LONG; entry.num = 1; entry.valueOffset = myTiff->width;
      break;
    case 1: /* The length */
      entry.tag = TIFFTAG_IMAGELENGTH; entry.type = TIFF_LONG; entry.num = 1; entry.valueOffset = myTiff->length;
      break;
    case 2: /* Bit per channel */
      entry.tag = TIFFTAG_BITSPERSAMPLE; entry.type = TIFF_SHORT;
      info.bitsPerSample.samplesPerPixel = entry.num = myTiff->samplesPerPixel;
      ami_malloc1d(info.bitsPerSample.bitsPerSample,int,entry.num);
      if (entry.num == 1) {
        entry.valueOffset = (myTiff->image.charOrShort == 0) ? 8 : 16;
	info.bitsPerSample.bitsPerPixel = info.bitsPerSample.bitsPerSample[0] = entry.valueOffset;
      }
      else if ((entry.num == 3) || (entry.num == 4)) {
        info.bitsPerSample.offset = entry.valueOffset = offset; offset += sizeof(short)*entry.num;
        for (j = 0; j < entry.num; j++) info.bitsPerSample.bitsPerSample[j] = 8;
        info.bitsPerSample.bitsPerPixel = 8*entry.num;
        myTiff->image.charOrShort = 0; /* Change the type image if it's necessary */
      }
      else {
        fprintf(stderr,"The file \"%s\" has a number of channels not available\n",name); retorno = -1; goto fin_write;
      }
      break;
    case 3: /* The compresion */
      entry.tag = TIFFTAG_COMPRESSION; entry.type = TIFF_SHORT; entry.num = 1;
      info.compression = entry.valueOffset = COMPRESSION_NONE;
      break;
    case 4: /* Photometric Interpretation */
      entry.tag = TIFFTAG_PHOTOMETRIC; entry.type = TIFF_SHORT; entry.num = 1;
      info.photometric = entry.valueOffset = (myTiff->samplesPerPixel == 1) ? PHOTOMETRIC_MINISBLACK : PHOTOMETRIC_RGB;
      break;
    case 5: /* The fill order */
      entry.tag = TIFFTAG_FILLORDER; entry.type = TIFF_SHORT; entry.num = 1;
      info.fillOrder = entry.valueOffset = FILLORDER_MSB2LSB;
      break;
    case 6: /* The offsets of the strips */
      entry.tag = TIFFTAG_STRIPOFFSETS;
      entry.type = TIFF_LONG; info.stripOffset.shortOrLong = 1;
      info.stripOffset.nStrips = entry.num = 1;
      info.stripOffset.valueOffset = entry.valueOffset = offset+4*sizeof(long); /* Offset + 2 rational */
      break;
    case 7: /* Channels per pixel */
      entry.tag = TIFFTAG_SAMPLESPERPIXEL; entry.type = TIFF_SHORT; entry.num = 1;
      entry.valueOffset = myTiff->samplesPerPixel;
      if ((entry.valueOffset != 1) && (entry.valueOffset != 3) && (entry.valueOffset != 4)) {
	fprintf(stderr,"The file \"%s\" not is a bilevel, grayscale or RGB full image\n",name);
	retorno = -1; goto fin_write;
      }
      break;
    case 8: /* Rows per strip */
      entry.tag = TIFFTAG_ROWSPERSTRIP; entry.type = TIFF_LONG; entry.num = 1;
      info.rowsPerStrip = entry.valueOffset = myTiff->length;
      break;
    case 9: /* Number of bytes of strips */
      entry.tag = TIFFTAG_STRIPBYTECOUNTS;
      entry.type = TIFF_LONG; info.stripByteCounts.shortOrLong = 1;
      info.stripByteCounts.nStrips = entry.num = 1;
      info.stripByteCounts.valueOffset = entry.valueOffset = myTiff->image.nBytes;
      break;
    case 10:   /* X resolution */
      entry.tag = TIFFTAG_XRESOLUTION; entry.type = TIFF_RATIONAL; entry.num = 1;
      info.xResolution.offset = entry.valueOffset =  offset; offset += 2*sizeof(long);
      info.xResolution.num = 72; info.xResolution.den = 1;
      break;
    case 11:   /* Y resolution */
      entry.tag = TIFFTAG_YRESOLUTION; entry.type = TIFF_RATIONAL; entry.num = 1;
      info.yResolution.offset = entry.valueOffset =  offset; offset += 2*sizeof(long);
      info.yResolution.num = 72; info.yResolution.den = 1;
      break;
    case 12:   /* Planar config */
      entry.tag = TIFFTAG_PLANARCONFIG; entry.type = TIFF_SHORT; entry.num = 1;
      info.planarConfiguration = entry.valueOffset = PLANARCONFIG_CONTIG;
      break;
    case 13:   /* Resolution unit */
      entry.tag = TIFFTAG_RESOLUTIONUNIT; entry.type = TIFF_SHORT; entry.num = 1;
      info.resolutionUnit = entry.valueOffset = RESUNIT_INCH;
      break;
    case 14:   /* Extra samples */
      if (myTiff->samplesPerPixel == 4) {
	entry.tag = TIFFTAG_EXTRASAMPLES; entry.type = TIFF_SHORT;
	entry.num = 1; ami_malloc1d(info.extraSamples,int,1);
	info.extraSamples[0] = entry.valueOffset = EXTRASAMPLE_UNASSALPHA;
      }
      break;
    default:
      break;
    }

    /* 3.3. Write the directory entry */
    writeDirectoryEntry(&entry,data); fwrite(data,entry.tam,1,fptr);
  }

  /* 4. WRITE THE NEXT IFD */
  fseek(fptr,temp,SEEK_SET);
  info.header.offsetIFD = 0; hexadecimal4(info.header.offsetIFD,data); fwrite(data,sizeof(long),1,fptr);

  /* 5. WRITE THE BITS PER SAMPLE (if samples per pixel > 1) */
  if (myTiff->samplesPerPixel > 1) {
    for (j = 0, p = data; j < myTiff->samplesPerPixel; j++, p +=2) hexadecimal2(info.bitsPerSample.bitsPerSample[j],p);
    fseek(fptr,info.bitsPerSample.offset,SEEK_SET); fwrite(data,myTiff->samplesPerPixel*sizeof(short),1,fptr);
  }

  /* 6. WRITE THE X AND Y RESOLUTION  */
  fseek(fptr,info.xResolution.offset,SEEK_SET);
  hexadecimal4(info.xResolution.num,data); hexadecimal4(info.xResolution.den,&data[4]);
  hexadecimal4(info.yResolution.num,&data[8]); hexadecimal4(info.yResolution.den,&data[12]);
  fwrite(data,4*sizeof(long),1,fptr);


  /* 7. WRITE THE IMAGE DATA */
  fseek(fptr,info.stripOffset.valueOffset,SEEK_SET); fwrite(myTiff->image.data,myTiff->image.nBytes,1,fptr);


 fin_write:

  /* 8. CLOSE FILE AND FREE USED MEMORY */
  fclose(fptr);
  free(data); p = NULL;
  if (info.bitsPerSample.bitsPerSample != NULL) free(info.bitsPerSample.bitsPerSample);
  if (info.extraSamples != NULL) free(info.extraSamples);

  return retorno;
}


int write_tiff_unsigned_char (
char name[200] /* Input image file name to read */,
unsigned char *grey /* Output grey level image data (pointer memory is NULL if image has not 1 channel ) */,
unsigned char *r /* Output RED image data (pointer memory is NULL if image has not 3 channels) */,
unsigned char *g /* Output GREEN image data (pointer memory is NULL if image has not 3 channels) */,
unsigned char *b /* Output BLUE image data (pointer memory is NULL if image has not 3 channels) */,
int width /* Output Image width */,
int height /* Output Image height */
)
{

  struct TIFF_file myTiff;
  int retorno, i, j, temp;
  unsigned char *p;

  if(grey!=NULL){ /* WE DEAL WITH A GREYLEVEL IMAGE */
    myTiff.width = width; myTiff.length = height;  myTiff.samplesPerPixel = 1;
    myTiff.image.IIorMM = 0; myTiff.image.nPixels = width*height;
    myTiff.image.charOrShort = 0;
    temp =  sizeof(char) ;
    myTiff.image.nBytes = myTiff.image.nPixels*myTiff.samplesPerPixel*temp;

    ami_malloc1d(myTiff.image.data,unsigned char,myTiff.image.nBytes);
    for (i = 0; i < myTiff.image.nBytes; i++) myTiff.image.data[i] = (unsigned char) grey[i];
    retorno = write_tiff_TIFF_struct(name,&myTiff);  /* Call the write function */
    free(myTiff.image.data);
    return retorno;
  }
  if(r!=NULL && g!=NULL && b!=NULL){ /* WE DEAL WITH A GREYLEVEL IMAGE */
    myTiff.width = width; myTiff.length = height;  myTiff.samplesPerPixel = 3;
    myTiff.image.IIorMM = 0; myTiff.image.charOrShort = 0; myTiff.image.nPixels = width*height;
    myTiff.image.nBytes = myTiff.image.nPixels*myTiff.samplesPerPixel*sizeof(char);

    ami_malloc1d(myTiff.image.data,unsigned char,myTiff.image.nBytes);
    for (i = j = 0, p = myTiff.image.data; i < myTiff.image.nBytes; j++, i += 3, p += 3) {
      p[0] = r[j]; p[1] = g[j]; p[2] = b[j];
    }

    retorno = write_tiff_TIFF_struct(name,&myTiff);  /* Call the write function */

    free(myTiff.image.data);
    return retorno;

  }
  return(-1);

}

int write_tiff_1c (
   char name[200],
   float *data,
   int width, int length,
   int charOrShort
   )
{
  struct TIFF_file myTiff;
  int retorno, i, j, temp;
  unsigned char *p;

  myTiff.width = width; myTiff.length = length;  myTiff.samplesPerPixel = 1;
  myTiff.image.IIorMM = 0; myTiff.image.nPixels = width*length;
  temp = ((myTiff.image.charOrShort = charOrShort) == 0) ? sizeof(char) : sizeof(short);
  myTiff.image.nBytes = myTiff.image.nPixels*myTiff.samplesPerPixel*temp;

  ami_malloc1d(myTiff.image.data,unsigned char,myTiff.image.nBytes);
  if (charOrShort == 0) for (i = 0; i < myTiff.image.nBytes; i++) myTiff.image.data[i] = (unsigned char) data[i];
  else
    for (i = j = 0, p = myTiff.image.data; i < myTiff.image.nBytes; j++, p += 2, i += 2) hexadecimal2((int)data[j],p);

  retorno = write_tiff_TIFF_struct(name,&myTiff);  /* Call the write function */

  free(myTiff.image.data);
  return retorno;
}


int write_tiff_3c (
   char name[200],
   unsigned char *r, unsigned char *g, unsigned char *b,
   int width, int length
   )
{
  struct TIFF_file myTiff;
  int retorno, i, j;
  unsigned char *p;

  myTiff.width = width; myTiff.length = length;  myTiff.samplesPerPixel = 3;
  myTiff.image.IIorMM = 0; myTiff.image.charOrShort = 0; myTiff.image.nPixels = width*length;
  myTiff.image.nBytes = myTiff.image.nPixels*myTiff.samplesPerPixel*sizeof(char);

  ami_malloc1d(myTiff.image.data,unsigned char,myTiff.image.nBytes);
  for (i = j = 0, p = myTiff.image.data; i < myTiff.image.nBytes; j++, i += 3, p += 3) {
    p[0] = r[j]; p[1] = g[j]; p[2] = b[j];
  }

  retorno = write_tiff_TIFF_struct(name,&myTiff);  /* Call the write function */

  free(myTiff.image.data);
  return retorno;
}


int write_tiff_4c (
   char name[200],
   unsigned char *r, unsigned char *g, unsigned char *b, unsigned char *trans,
   int width, int length
   )
{
  struct TIFF_file myTiff;
  int retorno,  i, j;
  unsigned char *p;

  myTiff.width = width; myTiff.length = length;  myTiff.samplesPerPixel = 4;
  myTiff.image.IIorMM = 0; myTiff.image.charOrShort = 0; myTiff.image.nPixels = width*length;
  myTiff.image.nBytes = myTiff.image.nPixels*myTiff.samplesPerPixel*sizeof(char);

  ami_malloc1d(myTiff.image.data,unsigned char,myTiff.image.nBytes);
  for (i = j = 0, p = myTiff.image.data; i < myTiff.image.nBytes; j++, i += 4, p += 4) {
    p[0] = r[j]; p[1] = g[j]; p[2] = b[j]; p[3] = trans[j];
  }

  retorno = write_tiff_TIFF_struct(name,&myTiff);   /* Call the write function */

  free(myTiff.image.data);
  return retorno;
}






