/**
*    Please note: Except for lines CLEARLY MARKED with the phrase "Added by B Meadows, 2012",
*	 ALL code herein is authored by Luis Alvarez and Luis Gomez, AMI Research Group,
*    University of Las Palmas de Gran Canaria, Canary Islands, SPAIN (February 2010)
*	~ comment by Ben Meadows
*/

/* ami_tif.c */

#ifndef _AMI_TIF_H_
#define _AMI_TIF_H_

#define ami_malloc1d(direccion,tipo,size) \
    {direccion=(tipo *) malloc(sizeof(tipo)*(size));}

/* RATIONAL: Offset, num & den */
struct TIFF_rational {
  long num;
  long den;
  long offset;
};


/* STRIPDATA: Num of strips, offset or real value (if num =1) & data (if num > 1) */
struct TIFF_stripData {
  int shortOrLong;    /* 0 short, 1 long */
  int nStrips;        /* num of strips */
  long valueOffset;
  unsigned char *data;
};


/* BITSPERSAMPLE: Offset and/or bits per sample */
struct TIFF_bitsPerSample {
  int samplesPerPixel;    /* 1, 3 or 4 */
  int bitsPerPixel;       /* Sum of bitsPerSample */
  long offset;
  int *bitsPerSample;     /* Num of bits per channels */
};


/* HEADER: byte order, magic number, offset & num of directory entries of the first IFD*/
struct TIFF_header {
  int IIorMM;      /* byte order: 0 II, 1 MM */
  int magicNumber;
  long offsetIFD;
  int nDirectoryEntries; /* num of directory entries of the first IFD */
};


/* TIFF STRUCT: Further information about the tiff file */
struct TIFF_struct {
  int compression;                          /* used compresion : only valid "no compresion" */
  int photometric;                         /* basseline tiff: only valid bilevel, greyscale or RGB images */
  int fillOrder;                           /* logical order of bits within a byte. Only valid MSB2LSB */
  int rowsPerStrip;                        /* num of rows per strip */
  int planarConfiguration;                 /* how the components of each pixel are stored: only valid 1 chumky format */
  int resolutionUnit;                      /* unit of measurement for XResolution and YResolution. */
  int *extraSamples;                       /* value of the unique extra sample: only valid unassociated alpha */
  struct TIFF_header header;               /* header of the tiff file */
  struct TIFF_bitsPerSample bitsPerSample; /* bits per channel: 8 or 16 */
  struct TIFF_rational xResolution;        /* num of pixels per ResolutionUnit in the ImageWidth direction. */
  struct TIFF_rational yResolution;        /* num of pixels per ResolutionUnit in the ImageLength direction. */
  struct TIFF_stripData stripOffset;       /* offset of each strips */
  struct TIFF_stripData stripByteCounts;   /* num of bytes of each strips */
};


/* TIFF ENTRY: tag, type, num of values, value or offset */
struct TIFF_entry {
  int tag;
  int type;
  int num;
  int valueOffset;
  int tam;        /* 2*sizeof(short)+2*sizeof(long) */
};


/* INFO IMAGE (to write/read data): num of values, type & data */
struct TIFF_infoImage {
  int IIorMM;         /* order byte: 0 II, 1 MM */
  int nPixels;        /* width*length */
  int nBytes;         /* nPixels*bitsPerPixel/8 */
  int charOrShort;    /* 0 char, 1 short */
  unsigned char *data;
};


/* TIFF FILE: width, length, samplesPerPixel & data */
struct TIFF_file {
  int width;
  int length;
  int samplesPerPixel;          /* num of channels per pixel: 1, 3 or 4 */
  struct TIFF_infoImage image;  /* image to write/read data */
};

void hexadecimal2(int source, unsigned char *dest);
void hexadecimal4(int source, unsigned char *dest);
void writeDirectoryEntry(struct TIFF_entry *entry, unsigned char *data);
short integer2(unsigned char *source, int IIorMM);
long integer4(unsigned char *source, int IIorMM);
long integer4Value(unsigned char *source, int IIorMM, int type, int num);
int read_tiff_TIFF_struct(const char name[200], struct TIFF_file *myTiff);
int read_tiff_unsigned_char(const char name[200], unsigned char **grey, unsigned char **r, unsigned char **g, unsigned char **b, int *width, int *height);
int read_tiff_1c(char name[200], float **data, int *width, int *length, int *charOrShort);
int read_tiff_3c(char name[200], unsigned char **r, unsigned char **g, unsigned char **b, int *width, int *length);
int read_tiff_4c(char name[200], unsigned char **r, unsigned char **g, unsigned char **b, unsigned char **trans, int *width, int *length);
int write_tiff_TIFF_struct(char name[200], struct TIFF_file *myTiff);
int write_tiff_unsigned_char(char name[200], unsigned char *grey, unsigned char *r, unsigned char *g, unsigned char *b, int width, int height);
int write_tiff_1c(char name[200], float *data, int width, int length, int charOrShort);
int write_tiff_3c(char name[200], unsigned char *r, unsigned char *g, unsigned char *b, int width, int length);
int write_tiff_4c(char name[200], unsigned char *r, unsigned char *g, unsigned char *b, unsigned char *trans, int width, int length);

#endif

