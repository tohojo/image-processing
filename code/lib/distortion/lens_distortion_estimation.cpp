/**
*    Please note: Except for lines CLEARLY MARKED with the phrase "Changed/Added by B Meadows, 2012",
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
*    This is the main program (ANSI C language), associated to the publication
*    An Algebraic Approach to Lens Distortion by Line Rectification,
*    L. Alvarez, L. Gomez, R. Sendra
*    Published in JMIV, July 2009
*/

/**  This program has the following components.
*         - lens_distortion_estimation.c (with the main() function)
*         - amy_prototypes.h    (having the prototypes of the functions
*                                being in lens_distortion.c)
*         - lens_distortion.c   (Set of user defined brief functions for the
*                                algebraic and gradient methods)
*         - ami_tif.h            (having the prototypes of functions for the
*                                image processing part)
*         - ami_tif.c            (Set of user defiend brief functions for the
*                                 image processing part)
*/

/**
*    Coded by Luis Alvarez and Luis Gomez, AMI Research Group, University of
*    Las Palmas de Gran Canaria, Canary Islands, SPAIN
*    February 2010
*/

/**
*     SUMMARY:
*    - this program calculates the radial distortion parameters for the
*    radial lens distortion model for a given image (input).
*    - It is necessary to provide the primitives for a reference line
*    which is known to be in the image (or some other straigth motif
*   reference). This is not automatically done, it must be provided by the user.
*    - It applys a novedous algebraic method (the one explained in the
*    publication).
*    - A standard numerical algorithm (gradient-like) is also implemented
*    to mininize the distance function and to compare results.
*/


/**
*   Requeriments:
*  - to be compiled using GCC (on Windows or Linux/UNIX systems).
*
*    INPUT/OUTPUT:
*    - INPUT:
*            an image in format TIF which a radial distortion to be
*            corrected.
*
*    - OUTPUT:
*            - the corrected distortion free image,
*            - numerical results (optimized energy values, distortion
*            parameters),
*            - CPU time (just for the running not for the pre-and post-
*            processing phases).
*/

/**
*   IMPORTANT:      CPU time strongly depends on the size image, not being
*                   linearly scaled for the gradient method.
*/

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include "lens_distortion.h"       /* Functions code associated to amy_prototypes.h */
#include "ami_tif.h"               /* Functions code associated to ami_tif.h*/
#include "lens_distortion_estimation.h" // Changed/Added by B Meadows, 2012

LensDistortionEstimation::~LensDistortionEstimation() // Changed/Added by B Meadows, 2012
{ // Changed/Added by B Meadows, 2012
} // Changed/Added by B Meadows, 2012

LensDistortionEstimation::LensDistortionEstimation(int argc, const char *argv[])
{
  unsigned char *r, *g, *b; /* RGB input image channels */
  unsigned char *ru,*gu,*bu;/* RGB output undistorted image channels */
  unsigned char *grey;      /* input grey level image channel */
  unsigned char *greyu;     /* output grey level undistorted image channel */
  int width;                /* image width */
  int height;               /* image height */
  int Na;                   /* Degree of the lens distortion model polynomial */
  int *Np;                  /* Number of points/line */
  int Nl;                   /* Number of lines */
  double *a;                /* Lens distortion model polynom */
  double **x,**y;           /* Coordinates of points (normalized) */
  double **xx,**yy;         /* Coordinates of points (pixels) */
  double x0,y0;             /* x center of the image (pixels) */
  double *trivial;          /* Vector to save the trivial initial solution, Emin, Vmin, D */

  /* AUXILIARY VARIABLES */
  double factor_n;          /* Auxiliar variable to normalize */
  int i,cont,m,zoom,control;
  char filename[300];
  FILE *fp2;

  /* WE INIT TO NULL ALL POINTERS */
  r=g=b=ru=gu=bu=grey=greyu=NULL;
  a=NULL;
  x=y=xx=yy=NULL;
  Np=NULL;
  trivial=NULL;


  /* WE CHECK COMMAND LINE PARAMETES */
  if(argc!=5){
    printf("lens_distortion_estimation : number of parameter inadequate\n");
    printf("Usage\n");
    printf("lens_distortion_estimation input_image.tif output_undistorted_image.tif input_line_primitives.dat output_lens_distortion_models.dat");
//    return(-1); // Changed/Added by B Meadows, 2012
	}

  /* WE READ INPUT IMAGE FROM DISK */
  if (read_tiff_unsigned_char(argv[1],&grey,&r,&g,&b,&width,&height)!=0){
    printf("TIF image can not be loaded\n");
//    return(-1); // Changed/Added by B Meadows, 2012
  }

  /* WE READ POINT LINE PRIMITES FROM DISK */
  if (read_line_primitives(argv[3],&Nl,&Np,&x,&y)!=0){
    printf("point line primitives can not be loaded\n");
    /* we free the image memory */
    if(grey!=NULL) free(grey); if(r!=NULL) free(r); if(g!=NULL) free(g); if(b!=NULL) free(b);
 //   return(-1); // Changed/Added by B Meadows, 2012
  }

  /* WE ALLOCATE MEMORY FOR AUXILARY POINTS AND WE NORMALIZE ORIGINAL POINTS */
  xx=(double**)malloc( sizeof(double*)*Nl);  /*  x pixels normalized coordinates */
  yy=(double**)malloc( sizeof(double*)*Nl);  /*  y pixels normalized coordinates */
  for(i=0;i<Nl;i++){
    xx[i]=(double*)malloc( sizeof(double)*Np[i] );
    yy[i]=(double*)malloc( sizeof(double)*Np[i] );
  }

  /* WE ALLOCATE MEMORY FOR AUXILARY POINTS AND WE NORMALIZE ORIGINAL POINTS */
  x0=width*0.5; y0=height*0.5; /* image center */
  factor_n=0.0; cont=0;
  for(m=0;m<Nl;m++){
     for(i=0;i<Np[m];i++){
      xx[m][i]=x[m][i];
      yy[m][i]=y[m][i];
      x[m][i]-=x0;
      y[m][i]-=y0;
      cont++;
      factor_n+=x[m][i]*x[m][i]+y[m][i]*y[m][i];
    }
  }
  factor_n=sqrt(factor_n/cont);
  for(m=0;m<Nl;m++) for(i=0;i<Np[m];i++){ x[m][i]/=factor_n; y[m][i]/=factor_n; }

  /* WE ALLOCATE MEMORY FOR MAXIMUM LENS DISTORTION POLYNOMIAL SIZE AND WE INITIALIZE TO TRIVIAL CASE */
  ami_calloc1d(a,double,5);
  a[0]=1;
  Na=4;


  /* WE COMPUTE ALL THE CASES, FOR BOTH, THE ALGEBRAIC METHOD AND FOR THE GRADIENT METHOD
  AND SAVE THE WHOLE SOLUTION TO THE INDICATED "output_lens_distortion_models.dat"
  WE PREPARE THE OUTPUT FILE */
  if(!(fp2=fopen(argv[4],"w"))){
    printf("Error while opening the %s file",filename);
    exit(0);
  }

   /* FIRST WE COMPUTE THE TRIVIAL SOLUTION AND SAVE IT TO THE OUTPUT FILE */
   /* WE ALLOCATE MEMORY TRIVIAL SOLUTION FOR Emin, Vmin, D */
   ami_calloc1d(trivial,double,5);
   trivial_solution(Nl,Np,Na,a,x0,y0,xx,yy,factor_n,fp2,trivial);

  /* ALGEBRAIC METHOD FROM TRIVIAL SOLUTION: 1 AND 2 PARAMETERS; WITHOUT ZOOM */
  zoom=0;   /* NO ZOOM APPLIED */
  algebraic_method(Nl,Np,a,x0,y0,x,y,xx,yy,factor_n,zoom,fp2,trivial);

  /* ALGEBRAIC METHOD FROM TRIVIAL SOLUTION: 1 AND 2 PARAMETERS; WITH ZOOM */
  zoom=1;   /* ZOOM APPLIED */
  algebraic_method(Nl,Np,a,x0,y0,x,y,xx,yy,factor_n,zoom,fp2,trivial);

  /* GRADIENT METHOD FROM TRIVIAL SOLUTION: POLYNOM DEGREE = 2,4 VARIABLES; TO BE OPTIMIZED: K2, K4; WITHOUT ZOOM */
  zoom=0;            /* NO ZOOM APPLIED */
  control=1;    /* START FROM TRIVIAL SOLUTION */
  optimize(a,x,y,xx,yy,Nl,Np,x0,y0,factor_n,zoom,fp2,trivial,control);

  /* GRADIENT METHOD FROM TRIVIAL SOLUTION: POLYNOM DEGREE = 2, 4 VARIABLES; TO BE OPTIMIZED: K2, K4; WITH ZOOM */
  zoom=1;   /*  ZOOM APPLIED */
  control=1;/* START FROM TRIVIAL SOLUTION */
  optimize(a,x,y,xx,yy,Nl,Np,x0,y0,factor_n,zoom,fp2,trivial,control);

  /* ALGEBRAIC METHOD FROM TRIVIAL SOLUTION + GRADIENT TO IMPROVE THE ALGEBRAIC SOLUTION; WITHOUT ZOOM */
  zoom=0;   /* NO ZOOM APPLIED */
  control=0;
  algebraic_method_pre_gradient(Nl,Np,a,x0,y0,x,y,xx,yy,factor_n,zoom,fp2,trivial,control);

  /* ALGEBRAIC METHOD FROM TRIVIAL SOLUTION + GRADIENT TO IMPROVE THE ALGEBRAIC SOLUTION; WITH ZOOM */
  zoom=1;       /* ZOOM APPLIED */
  control=0;    /* START FROM ALGEBRAIC SOLUTION */
  algebraic_method_pre_gradient(Nl,Np,a,x0,y0,x,y,xx,yy,factor_n,zoom,fp2,trivial,control);

  fclose(fp2);    /* close the output file */

  for(i=0;i<=Na;i++) printf("a[%d]=%e\n",i,a[i]);

  /* WE COMPUTE THE UNDISTORTED IMAGE */
  if(grey!=NULL){ /* grey level image */
    /* WE ALLOCATE MEMORY FOR THE NEW UNDISTORTED IMAGE */
    ami_malloc1d(greyu,unsigned char,width*height);
    undistort_image_1c(Na,a,grey,greyu,width,height);
    strcpy(filename,argv[2]);
    if (write_tiff_unsigned_char(filename,greyu,r,g,b,width,height)!=0){
      printf("TIF image can not be saved\n");
    }
  }
  if(r!=NULL && g!=NULL && b!=NULL){ /* RGB image */
    /* WE ALLOCATE MEMORY FOR THE NEW UNDISTORTED IMAGE */
    ami_malloc1d(ru,unsigned char,width*height);
    ami_malloc1d(gu,unsigned char,width*height);
    ami_malloc1d(bu,unsigned char,width*height);
    undistort_image_3c(Na,a,r,g,b,ru,gu,bu,width,height);
    strcpy(filename,argv[2]);
    if (write_tiff_unsigned_char(filename,grey,ru,gu,bu,width,height)!=0){
      printf("TIF image can not be saved\n");
    }
  }

  /* WE FREE THE MEMORY */
  if(r!=NULL) free(r); if(g!=NULL) free(g); if(b!=NULL) free(b);
  if(ru!=NULL) free(ru); if(gu!=NULL) free(gu); if(bu!=NULL) free(bu);
  if(grey!=NULL) free(grey); if(greyu!=NULL) free(greyu);
  if(a!=NULL) free(a);
  if(x!=NULL){for(i=0;i<Nl;i++){ if(x[i]!=NULL) free(x[i]);} free(x);}
  if(y!=NULL){for(i=0;i<Nl;i++){ if(y[i]!=NULL) free(y[i]);} free(y);}
  if(xx!=NULL){for(i=0;i<Nl;i++){ if(xx[i]!=NULL) free(xx[i]);} free(xx);}
  if(yy!=NULL){for(i=0;i<Nl;i++){ if(yy[i]!=NULL) free(yy[i]);} free(yy);}
  if(trivial!=NULL) free(trivial);
  if(Np!=NULL) free(Np);
//  return(0); // Changed/Added by B Meadows, 2012

}

