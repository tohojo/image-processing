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
*    This is the lens_distortion.cpp program (ANSI C language), associated to the
*    lens_distortion_estimation.cpp program; and to the publication
*    An Algebraic Approach to Lens Distortion by Line Rectification,
*    L. Alvarez, L. Gomez, R. Sendra
*    Published in JMIV, July 2009
*/

/**
*    Coded by Luis Alvarez and Luis Gomez, AMI Research Group, University of
*    Las Palmas de Gran Canaria, Canary Islands, SPAIN
*    February 2010
*/
/**
*       ALGEBRAIC PART
*/

#include "lens_distortion.h"

/**
GLOBAL VARIABLES
*/
int n_iterations=0;        /**  TOTAL NUMBER OF ITERATIONS */
int f_evaluations=0;       /**  TOTAL FUNCTION EVALUATIONS */
double tol_lambda= 1e-4;   /**  TOLERANCE FOR THE LAMBDA SEARCHING */


/**
* \fn int ami_calculo_recta2d( double recta[3],double **Puntos2D,int N)
* \brief function to compute a line equation by minimizing the distance to a
point collection
* \author Luis Alvarez and Luis Gomez
* \return return 0 if function finishes properly and !0 otherwise
*/
int ami_calculo_recta2d(
        double recta[3]    /**  rect coefficients (ax+by+c=0) */,
        double **Puntos2D  /**  set of 2D points */,
        int N              /**  number of points */
        )
{
  int i,j,k;
  double suu,suv,svv,um,vm,h,r[4][3],min,paso,norma;
  double cero=0.00000000000001;

  if(N<2){
    printf("Number of point to calculate the rect must be >2\n");
    return(-1);
  }

  suu=0; suv=0; svv=0; um=0; vm=0;

  for(i=0;i<N;i++){
    um+=Puntos2D[i][0];
    vm+=Puntos2D[i][1];
  }

  um/=N; vm/=N;

  for(i=0;i<N;i++){
    suu+=(Puntos2D[i][0]-um)*(Puntos2D[i][0]-um);
    svv+=(Puntos2D[i][1]-vm)*(Puntos2D[i][1]-vm);
    suv+=(Puntos2D[i][0]-um)*(Puntos2D[i][1]-vm);
  }

  suu/=N; svv/=N; suv/=N;

  if(fabs(suv)<= cero){
    if(suu<svv && svv>cero){
      recta[0]=1; recta[1]=0; recta[2]=-um;
      return(0);
    }
    if(svv<suu && suu>cero){
      recta[0]=0; recta[1]=1; recta[2]=-vm;
      return(0);
    }
    printf(" It is not possible to calculate the 2D rect\n");
    return(-1);
  }

  r[2][1]=r[3][1]=r[0][0]=r[1][0]=1.;
  h=0.5*(suu-svv)/suv;

  if(h>0){
    r[0][1]=-h-sqrt(1.+h*h);
    r[0][2]=-(um+r[0][1]*vm);
    r[1][1]=-1./r[0][1];
    r[1][2]=-(um+r[1][1]*vm);
    r[2][0]=h+sqrt(1.+h*h);
    r[2][2]=-(r[2][0]*um+vm);
    r[3][0]=-1./r[2][0];
    r[3][2]=-(r[3][0]*um+vm);
  }
  else{
    r[0][1]=-h+sqrt(1+h*h);
    r[0][2]=-(um+r[0][1]*vm);
    r[1][1]=-1./r[0][1];
    r[1][2]=-(um+r[1][1]*vm);
    r[2][0]=h-sqrt(1+h*h);
    r[2][2]=-(r[2][0]*um+vm);
    r[3][0]=-1./r[2][0];
    r[3][2]=-(r[3][0]*um+vm);
  }

  for(j=0;j<4;j++){
    norma=sqrt(r[j][0]*r[j][0]+r[j][1]*r[j][1]);
    for(i=0;i<3;i++)
      r[j][i]/=norma;
  }

  min=0.; k=0;

  for(i=0;i<N;i++){
   paso=r[0][0]*Puntos2D[i][0]+r[0][1]*Puntos2D[i][1]+r[0][2];
   min+=fabs(paso);
  }

  for(j=1;j<4;j++){
    h=0;
    for(i=0;i<N;i++){
      paso=r[j][0]*Puntos2D[i][0]+r[j][1]*Puntos2D[i][1]+r[j][2];
      h+=fabs(paso);
    }
    if(h<min){
      k=j;
      min=h;
    }
  }

  recta[0]=r[k][0]; recta[1]=r[k][1]; recta[2]=r[k][2];
  return(0);
}

/**
* \fn void ami_dibujar_segmento_unsigned_char(unsigned char *data, int width,
int height,int x0, int y0, int x1, int y1,unsigned char color)
* \brief function to draw a segment in an image
* \author Luis Alvarez
* \return void
*/
void ami_dibujar_segmento_unsigned_char(
    unsigned char *data    /**  Image to draw the segment in */,
    int width, int height  /**  Size of the image */,
    int x0, int y0         /**  Coordinates of the first segment point */,
    int x1, int y1         /**  Coordinates of the first segment point */,
    unsigned char color)   /**  Color to paint the segment with */
{
 int incx = ami_abs(x1 - x0);
 int incy = ami_abs(y1 - y0);
 int p, dx, dy;
 int x, y;
 int temp;
 int nx=width;
 int ny=height;

 if (incx>incy) {
   if (x0>x1){
     temp=x0; x0=x1; x1=temp;
     temp=y0; y0=y1; y1=temp;
   }
   x=x0; y=y0;
   p = 2*incy - incx;
   dy = (y1>y0)? 1 : -1;
   if(0<=y && y<ny && 0<=x && x<nx)
      data[y*nx+x] = color;
   do {
     if (p<0) {
       p += 2*incy;
     } else {
       p += 2*incy - 2*incx;
       y += dy;
     }
     x++;
     if(0<=y && y<ny && 0<=x && x<nx)
        data[y*nx+x] = color;
   } while (x<x1);
 } else {
     if (y0>y1) {
       temp=x0; x0=x1; x1=temp;
       temp=y0; y0=y1; y1=temp;
     }
     x=x0; y=y0;
     p = 2*incx - incy;
     dx = (x1>x0)? 1 : -1;
     if(0<=y && y<ny && 0<=x && x<nx)
        data[y*nx+x] = color;
     do {
       if (p<0) {
         p += 2*incx;
       } else {
         p += 2*incx - 2*incy;
	 x += dx;
       }
       y++;
       if(0<=y && y<ny && 0<=x && x<nx)
          data[y*nx+x] = color;
    } while (y<y1);
  }
}

/**
* \fn int ami_lens_distortion_polynomial_update_distance_2v(double *x,
double *y,int Np,double *a,int Na,double x0,double y0,int k1,int k2,
double **pol, double alfa)
* \brief function to add the information of a line point sequence to the 4
degree polynomial to compute the lens distortion model
* \author Luis Alvarez
* \return return 0 if function finishes properly
*/

int ami_lens_distortion_polynomial_update_distance_2v(
   double *x, double *y/** DISTORTED LINE COORDINATES (INPUT) */,
   int Np              /** NUMBER OF POINTS (INPUT)*/,
   double *a           /** POLYNOMIAL DEFINING THE LENS DISTORTION MODEL (INPUT) */,
   int Na              /** DEGREE OF POLYNOMIAL MODEL FOR LENS DISTORTION */,
   double x0,double y0 /** COORDINATES OF THE IMAGE CENTER */,
   int k1              /** COEFICIENT 1 OF THE POLYNOMIAL TO BE UPDATED*/,
   int k2              /** COEFICIENT 2 OF THE POLYNOMIAL TO BE UPDATED*/,
   double **pol        /** 4 DEGREE 2 VARIABLE POLYNOM TO MINIMIZE (INPUT-OUTPUT) */,
   double alfa)        /** WEIGHT OF THE DISTANCE IN THE POLYNOM ENERGY */
{
  int i,j,k;
  double *A,*x2,*y2,*d1,*d2;
  double paso,**pol1,**pol2,**p_xx,**p_xy,**p_yy;

  A=x2=y2=d1=d2=NULL;
  pol1=pol2=p_xx=p_xy=p_yy=NULL;

  /* WE CHECK alfa VALUE */
  if(alfa==0) return(0);

  /* WE ALLOCATE MEMORY */
  A=(double*)malloc( sizeof(double)*Np );
  x2=(double*)malloc( sizeof(double)*Np );
  y2=(double*)malloc( sizeof(double)*Np );
  d1=(double*)malloc( sizeof(double)*Np );
  d2=(double*)malloc( sizeof(double)*Np );
  ami_calloc2d(pol1,double,2,2);
  ami_calloc2d(pol2,double,2,2);
  ami_calloc2d(p_xx,double,3,3);
  ami_calloc2d(p_xy,double,3,3);
  ami_calloc2d(p_yy,double,3,3);

  /* WE COMPUTE THE DISTANCE TO THE IMAGE CENTER */
  for(i=0;i<Np;i++)
    d1[i]=sqrt( (x[i]-x0)*(x[i]-x0)+(y[i]-y0)*(y[i]-y0) );

  /* WE COMPUTE THE POINT TRANSFORMATION WITH THE CURRENT LENS DISTORTION MODEL */
  for(i=0;i<Np;i++){
    A[i]=ami_polynomial_evaluation(a,Na,d1[i]);
    x2[i]=x0+(x[i]-x0)*A[i];
    y2[i]=y0+(y[i]-y0)*A[i];
  }
  /* WE COMPUTE THE POLYNOMS CORRESPONDING TO THE DISTANCE ERROR */
  for(i=0;i<=2;i++) for(j=0;j<=2;j++) p_xx[i][j]=0.;

  for(i=0;i<Np;i++){
    paso=0;
    for(k=1;k<=Na;k++) if(k!=k1 && k!=k2) paso+=a[k]*pow(d1[i],(double) k);
    pol1[0][0]=paso*d1[i];
    pol1[1][0]=pow(d1[i],(double) k1+1);
    pol1[0][1]=pow(d1[i],(double) k2+1);
    ami_2v_polynom_multiplication(pol1,1,pol1,1,p_xx);
  }

  for(i=0;i<=2;i++) for(j=0;j<=2;j++) p_xx[i][j]=alfa*p_xx[i][j]/Np;

  /* WE UPDATE THE ERROR POLYNOM */
  ami_2v_polynom_multiplication(p_xx,2,p_xx,2,pol);

  /* WE FREE THE MEMORY */
  if(A!=NULL) free(A); if(x2!=NULL)free(x2); if(y2!=NULL) free(y2);
  if(d1!=NULL) free(d1); if(d2!=NULL) free(d2);
  if(p_xx!=NULL) ami_free2d(p_xx); if(p_xy!=NULL) ami_free2d(p_xy); if(p_yy!=NULL)ami_free2d(p_yy);
  if(pol1!=NULL)ami_free2d(pol1); if(pol2!=NULL) ami_free2d(pol2);

  return(0);
}


/**
* \fn double ami_lens_distortion_estimation_2v(double **x,double **y,int Nl,
int *Np,double x0,double y0,double *a,int Na,int k1,int k2, double alfa )
* \brief function to update the lens distortion polynomial model for 2 variables
. If alfa>0, we adapt a[0] to minimize the square distence between distorted and
undistorted points and we add a term to the polynomial also minimizing such
distance with weight alfa
* \author Luis Alvarez
* \return return the Error/Nl
*/
double ami_lens_distortion_estimation_2v(
    double **x,double **y  /** DISTORTED LINE COORDINATES (INPUT)*/,
    int Nl                 /** NUMBER OF LINES (RECTS)*/,
    int *Np                /** NUMBER OF POINTS (INPUT)*/,
    double x0,double y0    /** COORDINATES OF THE IMAGE CENTER */,
    double *a              /** POLYNOMIAL DEFINING THE LENS DISTORTION MODEL (INPUT) */,
    int Na                 /** DEGREE OF POLYNOMIAL MODEL FOR LENS DISTORTION */,
    int k1                 /** COEFICIENT 1 OF THE POLYNOMIAL TO BE UPDATED*/,
    int k2                 /** COEFICIENT 2 OF THE POLYNOMIAL TO BE UPDATED*/,
    double alfa            /** WEIGHT OF THE DISTANCE IN THE POLYNOM ENERGY */
    )
{
  int i,k,m;
  double **pol_v2,d,suma_Ad,suma_dd,A,Error=0;

  /* WE ALLOCATE MEMORY */
  ami_calloc2d(pol_v2,double,5,5);

  /* WE UPDATE a[0] BY MINIMIZING THE DISTANCE OF THE DISTORTED POINTS TO
  THE UNDISTORTED POINTS */
  if(alfa>0){
    suma_dd=suma_Ad=0;
    for(m=0;m<Nl;m++){
      for(i=0;i<Np[m];i++){
        d=sqrt( (x[m][i]-x0)*(x[m][i]-x0)+(y[m][i]-y0)*(y[m][i]-y0) );
        A=0;
        for(k=1;k<=Na;k++) A+=a[k]*pow(d,(double) k+1);
        suma_dd+=d*d;
        suma_Ad+=A*d;
      }
    }
    a[0]=1-suma_Ad/suma_dd;
  }
  for(m=0;m<Nl;m++){
     /* WE UPDATE DE POLYNOM TO MINIMIZE */
    ami_lens_distortion_polynomial_update_2v(x[m],y[m],Np[m],a,Na,x0,y0,k1,k2,pol_v2);
    ami_lens_distortion_polynomial_update_distance_2v(x[m],y[m],Np[m],a,Na,x0,y0,k1,k2,pol_v2,alfa);
  }
  /* WE UPDATE THE POLYNOMIAL LENS DISTORTION MODEL */
  ami_lens_distortion_model_update_2v(a,Na,k1,k2,pol_v2);
  ami_free2d(pol_v2);
  for(m=0;m<Nl;m++)
    Error+=ami_LensDistortionEnergyError(x[m],y[m],Np[m],x0,y0,a,Na);
  return(Error/Nl);
}


/**
* \fn int ami_lens_distortion_model_update_2v( double *a,int Na,int k1,int k2,
double **pol)
* \brief function to update the lens distortion model by minimizing a 4 degree
2 variable polynom
* \author Luis Alvarez
* \return return 0 if the function finishes properly
*/
int ami_lens_distortion_model_update_2v(
   double *a   /** POLYNOMIAL DEFINING THE LENS DISTORTION MODEL (INPUT-OUTPUT) */,
   int Na      /** DEGREE OF POLYNOMIAL MODEL FOR LENS DISTORTION (INPUT)*/,
   int k1      /** COEFICIENT 1 OF THE POLYNOMIAL TO BE UPDATED (INPUT)*/,
   int k2      /** COEFICIENT 2 OF THE POLYNOMIAL TO BE UPDATED (INPUT)*/,
   double **pol)/** 4 DEGREE POLYNOM TO MINIMIZE (INPUT) */
{
  int j,i,M,Nr=0,m=Na;
  double *x,**pol_x,**pol_y,*pol_r,xr,yr,Emin,*rx,*ry,*b2,*p3;
  double sx,sy,Energy; /* NORMALIZATION FACTORS */
  double p_r3[6][6][19];

  x=pol_r=rx=ry=b2=p3=NULL; 	/*Added by Luis Gomez*/
  pol_x=pol_y=NULL; 		/*Added by Luis Gomez*/

  for(i=0;i<6;i++) for(j=0;j<6;j++) for(m=0;m<19;m++) p_r3[i][j][m]=0.;

  /* WE ALLOCATE MEMORY */
  x=(double*)malloc( sizeof(double)*3 );
  ami_calloc2d(pol_x,double,5,5);
  ami_calloc2d(pol_y,double,5,5);
  p3=(double*) malloc(sizeof(double)*5);

  /* WE NORMALIZE POLYNOM COEFICIENT */
  sx=pow(pol[4][0],(double) 0.25);
  sy=pow(pol[0][4],(double) 0.25);
  for(i=0;i<=4;i++){
    for(j=0;j<=4;j++){
      if(i>0)  pol[i][j]/=pow(sx,(double) i);
      if(j>0)  pol[i][j]/=pow(sy,(double) j);
    }
  }

  /* WE COMPUTE THE DERIVATIVES OF THE POLYNOM */
  ami_2v_polynom_derivatives(pol,4,pol_x, pol_y);

  /* WE FILL THE MATRIX TO COMPUTE THE DETERMINANT */
  for(i=0;i<=3;i++){
    for(m=0;m<=4;m++){
      p_r3[2][i+2][m]=p_r3[1][i+1][m]=p_r3[0][i][m]=pol_x[3-i][m];
      p_r3[5][i+2][m]=p_r3[4][i+1][m]=p_r3[3][i][m]=pol_y[3-i][m];
    }
  }
  /* WE COMPUTE THE RESOLVENT POLYNOM */
  pol_r=(double*) malloc(sizeof(double)*19);
  ami_polynom_determinant(p_r3,18,6,pol_r);

  /* WE COMPUTE THE RESOLVENT POLYNOM DEGREE */
  for(i=0;i<=18;i++){
    if(pol_r[i]!=0) Nr=i;
  }
  /* WE COMPUTE THE ROOT OF THE RESOLVENT POLYNOM */
  rx=(double*) malloc(sizeof(double)*Nr);
  ry=(double*) malloc(sizeof(double)*Nr);
  b2=(double*) malloc(sizeof(double)*(Nr+1));
  for(i=0;i<=Nr;i++) b2[i]=pol_r[Nr-i];
  for(i=0;i<Nr;i++) {rx[i]=0.0;ry[i]=0.0;} /*Added by Luis Gomez*/
  Nr=ami_polynomial_root(b2,Nr,rx,ry);
  /* WE COMPUTE THE X COMPONENT BY REPLACING THE ROOTS IN THE DERIVATIVES
  OF THE POLYNOM */
  xr=0; yr=0; Emin=10e90;
  for(i=0;i<Nr;i++){
    if(fabs(ry[i])> 0.000000000000001) continue;
    ami_2v_polynom_to_1v_polynom(pol_x,4,p3,rx[i],1);
    M=ami_RootCubicPolynomial(p3,3,x);
    for(m=0;m<M;m++){
     Energy=ami_2v_polynom_evaluation(pol,4,x[m],rx[i]);
     if(Energy<Emin){ Emin=Energy; xr=rx[i]; yr=x[m];}
    }
    ami_2v_polynom_to_1v_polynom(pol_y,4,p3,rx[i],1);
    M=ami_RootCubicPolynomial(p3,3,x);
    for(m=0;m<M;m++){
      Energy=ami_2v_polynom_evaluation(pol,4,x[m],rx[i]);
      if(Energy<Emin){ Emin=Energy; xr=rx[i]; yr=x[m];}
    }
  }
  /* WE UPDATE THE DISTORSION POLYNOMIAL MODEL */
  a[k1]+=(yr/sx);
  a[k2]+=(xr/sy);

  /* WE FREE THE MEMORY */
  if(x!=NULL) free(x); if(pol_x!=NULL) ami_free2d(pol_x); if(pol_y!=NULL) ami_free2d(pol_y);
  if(pol_r!=NULL) free(pol_r);
  if(p3!=NULL) free(p3); if(rx!=NULL) free(rx); if(ry!=NULL) free(ry);  if(b2!=NULL) free(b2);
  return(0);
}

/**
* \fn int ami_lens_distortion_polynomial_update_2v(double *x, double *y,int Np,
double *a,int Na,double x0,double y0,int k1, double **pol)
* \brief function To add the information of a line point sequence to the 4
degree polynomial to compute the lens distortion model
* \author Luis Alvarez
* \return return 0 if the function finishes properly
*/
int ami_lens_distortion_polynomial_update_2v(
   double *x, double *y /** DISTORTED LINE COORDINATES (INPUT)*/,
   int Np               /** NUMBER OF POINTS (INPUT)*/,
   double *a            /** POLYNOMIAL DEFINING THE LENS DISTORTION MODEL (INPUT) */,
   int Na               /** DEGREE OF POLYNOMIAL MODEL FOR LENS DISTORTION */,
   double x0,double y0  /** COORDINATES OF THE IMAGE CENTER */,
   int k1               /** COEFICIENT 1 OF THE POLYNOMIAL TO BE UPDATED*/,
   int k2               /** COEFICIENT 2 OF THE POLYNOMIAL TO BE UPDATED*/,
   double **pol         /** 4 DEGREE 2 VARIABLE POLYNOM TO MINIMIZE (INPUT-OUTPUT) */
   )
{
  int i,j;
  double *A,*x2,*y2,*d1,*d2,x2_m,y2_m,s_xx,s_yy,xA_m;
  double xd1_m,yA_m,yd1_m,xd2_m,yd2_m;
  double paso,**pol1,**pol2,**p_xx,**p_xy,**p_yy;

  A=x2=y2=d1=d2=NULL;
  pol1=pol2=p_xx=p_xy=p_yy=NULL;

  /* WE ALLOCATE MEMORY */
  A=(double*)malloc( sizeof(double)*Np );
  x2=(double*)malloc( sizeof(double)*Np );
  y2=(double*)malloc( sizeof(double)*Np );
  d1=(double*)malloc( sizeof(double)*Np );
  d2=(double*)malloc( sizeof(double)*Np );
  ami_calloc2d(pol1,double,2,2);
  ami_calloc2d(pol2,double,2,2);
  ami_calloc2d(p_xx,double,3,3);
  ami_calloc2d(p_xy,double,3,3);
  ami_calloc2d(p_yy,double,3,3);

  /* WE COMPUTE THE DISTANCE TO THE IMAGE CENTER */
  for(i=0;i<Np;i++)
    d1[i]=sqrt( (x[i]-x0)*(x[i]-x0)+(y[i]-y0)*(y[i]-y0) );

  /* WE COMPUTE THE POINT TRANSFORMATION WITH THE CURRENT LENS DISTORTION MODEL */
  for(i=0;i<Np;i++){
    A[i]=ami_polynomial_evaluation(a,Na,d1[i]);
    x2[i]=x0+(x[i]-x0)*A[i];
    y2[i]=y0+(y[i]-y0)*A[i];
  }

  /* WE COMPUTE THE DISTANCE POWER k1 AND k2 (THE COEFICIENT OF THE LENS
  DISTORTION MODEL TO BE UPDATED */
  for(i=0;i<Np;i++){
    paso=d1[i];
    d1[i]=pow(paso,(double) k1);
    d2[i]=pow(paso,(double) k2);
 }

  /* WE COMPUTE THE VARIANCE OF THE TRANSFORMED POINTS */
  x2_m=0; for(i=0;i<Np;i++) x2_m+=x2[i];  x2_m/=Np;
  s_xx=0; for(i=0;i<Np;i++) s_xx+=(x2[i]-x2_m)*(x2[i]-x2_m); s_xx/=Np;
  y2_m=0; for(i=0;i<Np;i++) y2_m+=y2[i];  y2_m/=Np;
  s_yy=0; for(i=0;i<Np;i++) s_yy+=(y2[i]-y2_m)*(y2[i]-y2_m); s_yy/=Np;

  /* WE COMPUTE SOME AVERAGES WE NEED */
  xA_m=0; for(i=0;i<Np;i++) xA_m+=(x[i]-x0)*A[i]; xA_m/=Np;
  xd1_m=0; for(i=0;i<Np;i++) xd1_m+=(x[i]-x0)*d1[i]; xd1_m/=Np;
  xd2_m=0; for(i=0;i<Np;i++) xd2_m+=(x[i]-x0)*d2[i]; xd2_m/=Np;
  yA_m=0; for(i=0;i<Np;i++) yA_m+=(y[i]-y0)*A[i]; yA_m/=Np;
  yd1_m=0; for(i=0;i<Np;i++) yd1_m+=(y[i]-y0)*d1[i]; yd1_m/=Np;
  yd2_m=0; for(i=0;i<Np;i++) yd2_m+=(y[i]-y0)*d2[i]; yd2_m/=Np;

 /* WE COMPUTE THE POLYNOMS OF THE SECOND ORDER MOMENT OF THE POINT
     p_xx p_xy AND p_yy DISTRIBUTION */
  for(i=0;i<Np;i++){
    pol1[0][0]=(x[i]-x0)*A[i]-xA_m;
    pol1[1][0]=(x[i]-x0)*d1[i]-xd1_m;
    pol1[0][1]=(x[i]-x0)*d2[i]-xd2_m;
    pol2[0][0]=(y[i]-y0)*A[i]-yA_m;
    pol2[1][0]=(y[i]-y0)*d1[i]-yd1_m;
    pol2[0][1]=(y[i]-y0)*d2[i]-yd2_m;
    ami_2v_polynom_multiplication(pol1,1,pol1,1,p_xx);
    ami_2v_polynom_multiplication(pol1,1,pol2,1,p_xy);
    ami_2v_polynom_multiplication(pol2,1,pol2,1,p_yy);
  }
  for(i=0;i<=2;i++) for(j=0;j<=2;j++) p_xx[i][j]/=1. /*s_max*/ ;
  ami_2v_polynom_multiplication(p_xx,2,p_yy,2,pol);
  for(i=0;i<=2;i++) for(j=0;j<=2;j++) p_xx[i][j]=-p_xy[i][j]/1. /*s_max*/;
  ami_2v_polynom_multiplication(p_xy,2,p_xx,2,pol);

  /* WE FREE THE MEMORY */
  if(A!=NULL) free(A); if(x2!=NULL) free(x2); if(y2!=NULL) free(y2); if(d1!=NULL) free(d1); if(d2!=NULL) free(d2);
  ami_free2d(p_xx); ami_free2d(p_xy);  ami_free2d(p_yy);
  ami_free2d(pol1); ami_free2d(pol2);

  return(0);
}

/**
* \fn void ami_2v_polynom_derivatives( double **p,int N,double **p_x,
double **p_y)
* \brief function to compute the partial derivatives of a 2 variable polynom.
The degree of the derivative polynoms is assumed to be the same that the
original one
* \author Luis Alvarez
* \return return void
*/
void ami_2v_polynom_derivatives(
    double **p     /** ORIGINAL POLYNOM (INPUT)*/,
    int N          /** DEGREE OF THE ORIGINAL POLYNOM (INPUT) */,
    double **p_x   /** DERIVATIVE OF THE POLYNOM WITH RESPECT TO THE FIRST VARIABLE (OUTPUT) */,
    double **p_y   /** DERIVATIVE OF THE POLYNOM WITH RESPECT TO THE SECOND VARIABLE(OUTPUT) */
    )
{
  int i,j;

  for(i=0;i<=N;i++)
    for(j=0;j<=N;j++)
      p_x[i][j]=p_y[i][j]=0;

  for(i=1;i<=N;i++)
    for(j=0;j<=N;j++)
      p_x[i-1][j]=i*p[i][j];

  for(i=0;i<=N;i++)
    for(j=1;j<=N;j++)
      p_y[i][j-1]=j*p[i][j];

}

/**
* \fn double ami_determinante(double **A,int N)
* \brief function to evaluate the determinant of a matrix
* \author Luis Alvarez
* \return return the determinant of the matrix
*/
double ami_determinante(
    double **A  /** INPUT MATRIX */,
    int N       /** SIZE OF THE MATRIX*/
    )
{
  int i,k,l,cont;
  double **B,paso;

  B=NULL;

  if(N==1) return(A[0][0]);
  ami_calloc2d(B,double,N-1,N-1);
  paso=0;
  cont=-1;
  for(i=0;i<N;i++){
    cont*=-1;
    for(k=0;k<N-1;k++){
      for(l=0;l<N-1;l++){
        B[k][l]=A[k+1][l>=i?l+1:l];
      }
    }
    paso+=cont*A[0][i]*ami_determinante(B,N-1);
  }
  ami_free2d(B);
  return(paso);
}

/**
* \fn void ami_polynom_determinant(double p[6][6][19],int Np,int Nd,double *q)
* \brief function to compute the determinant of a polynom matrix
* \author Luis Alvarez
* \return return void
*/
void ami_polynom_determinant(
    double p[6][6][19],
    int Np,
    int Nd,
    double *q
    )
{
  int i,j,k,l,m,cont;
  double *q2;
  double p2[6][6][19];

  q2=NULL;

  if(Nd==1){ for(i=0;i<=18;i++) q[i]=p[0][0][i]; return;}

  for(i=0;i<6;i++) for(j=0;j<6;j++) for(m=0;m<19;m++) p2[i][j][m]=0.;
  q2=(double*)malloc(sizeof(double)* (Np+1));

  for(i=0;i<=Np;i++) q[i]=0;
  cont=-1;
  for(i=0;i<Nd;i++){
    for(k=0;k<=Np;k++) q2[k]=0;
    cont*=-1;
    for(k=0;k<(Nd-1);k++){
      for(l=0;l<(Nd-1);l++){
        for(m=0;m<=Np;m++){ p2[k][l][m]= p[k+1][l>=i?l+1:l][m];}
      }
    }
    ami_polynom_determinant(p2,Np,Nd-1,q2);
    if(cont<0) for(m=0;m<=Np;m++) q2[m]=-q2[m];
    q=ami_1v_polynom_multiplication(p[0][i],Np,q2,Np,q);
  }
  if(q2!=NULL) free(q2);
}

/**
* \fn double ami_2v_polynom_evaluation(double **p1,int N1,double x,double y)
* \brief function to evaluate a 2 variable polynom in one point
* \author Luis Alvarez
* \return return the evaluation
*/
double ami_2v_polynom_evaluation(
   double **p1         /** 2 VARIABLE POLYNOM (INPUT)*/,
   int N1              /** DEGREE OF POLYNOM 1 (INPUT)*/,
   double x,double y   /** POINT COORDINATE WHERE THE POLYNOM WILL BE EVALUATED (INPUT) */
   )
{
  int i,j;
  double *p,*q,paso;

  p=q=NULL;

  p=(double*)malloc(sizeof(double)*(N1+1));
  q=(double*)malloc(sizeof(double)*(N1+1));

  for(i=0;i<=N1;i++){
    for(j=0;j<=N1;j++) p[j]=p1[i][j];
    q[i]=ami_polynomial_evaluation(p,N1,y);
  }
  paso=ami_polynomial_evaluation(q,N1,x);
  if(p!=NULL) free(p); if(q!=NULL) free(q);
  return(paso);
}

/**
* \fn void ami_2v_polynom_to_1v_polynom( double **p1,int N1,double *p3,double z,
int flat)
* \brief function to evaluate a 2 variable polynom in one of the variable value.
The output is a 1 degree polynom
* \author Luis Alvarez
* \return return void
*/
void ami_2v_polynom_to_1v_polynom(
   double **p1     /** 2 VARIABLE POLYNOM (INPUT)*/,
   int N1          /** DEGREE OF POLYNOM 1 (INPUT)*/,
   double *p3      /** OUTPUT 1 VARIABLE POLYNOM (OUTPUT)*/,
   double z        /** POINT WHERE THE 2 VARIABLE POLYNOM IS GOING TO BE EVALUATED */,
   int flat)       /** VARIABLE WHERE THE POLYNOM IS GOING TO BE EVALUATED */
{
  int i,j;
  double *p;

  p=NULL;

  p=(double*)malloc(sizeof(double)*(N1+1));
  if(flat==1){
    for(i=0;i<=N1;i++){
      for(j=0;j<=N1;j++) p[j]=p1[i][j];
      p3[i]=ami_polynomial_evaluation(p,N1,z);
    }
  }
  else{
    for(i=0;i<=N1;i++){
      for(j=0;j<=N1;j++) p[j]=p1[j][i];
      p3[i]=ami_polynomial_evaluation(p,N1,z);
    }
  }
  if(p!=NULL) free(p);
}

/**
* \fn double* ami_1v_polynom_multiplication(double *p1,int N1,double *p2,int N2,
double *p3)
* \brief function to multiply polinoms of 1 variable. the result is added to the
output polynom coeficients
* \author Luis Alvarez
* \return return the calculation
*/
double* ami_1v_polynom_multiplication(
  double *p1   /** POLYNOM 1 (INPUT) */,
  int N1       /** DEGREE OF POLYNOM 1 (INPUT)*/,
  double *p2   /** POLYNOM 2 (INPUT) */,
  int N2       /** DEGREE OF POLYNOM 2 (INPUT) */,
  double *p3   /** OUTPUT POLYNOM (INPUT-OUTPUT)*/
  )
{
  int i,j;

  /* WE MULTIPLY THE POLYNOMS */
  for(i=0;i<=N1;i++){
    if(p1[i]!=0){
      for(j=0;j<=N2;j++)
        if(p2[j]!=0)
          p3[i+j]+=p1[i]*p2[j];
    }
  }
  return(p3);
 }
/**
* \fn void ami_2v_polynom_multiplication(double **p1,int N1,double **p2,int N2,
double **p3 )
* \brief function to multiply polynoms of 2 variables
* \author Luis Alvarez
* \return return void
*/
void ami_2v_polynom_multiplication(
  double **p1  /** POLYNOM 1 (INPUT) */,
  int N1       /** DEGREE OF POLYNOM 1 (INPUT)*/,
  double **p2  /** POLYNOM 2 (INPUT) */,
  int N2       /** DEGREE OF POLYNOM 2 (INPUT) */,
  double **p3  /** OUTPUT POLYNOM (INPUT - OUTPUT)*/
  )
{
  int i,j,k,l;
  for(i=0;i<=N1;i++){
    for(j=0;j<=N1;j++){
      if(p1[i][j]!=0){
        for(k=0;k<=N2;k++)
          for(l=0;l<=N2;l++)
            if(p2[k][l]!=0 )
              p3[i+k][j+l]+=p1[i][j]*p2[k][l];
      }
    }
  }
}

/**
* \fn int ami_RootCubicPolynomial(double *a,int N,double *x)
* \brief function to compute the real roots of a cubic polynomial. It returns
the number of roots found sorted by magnitud
* \author Luis Alvarez
* \return return 3 if the function finishes properly
*/
int ami_RootCubicPolynomial(
    double *a  /** POLINOMIAL COEFICIENTS a[0]+a[1]x+a[2]x^2 +... */,
    int N      /** DEGREE OF POLINOMIAL (IT HAS TO BE 3) */,
    double *x  /** POLINOMIAL ROOTS */
    )
{
  double a1,a2,a3,Q,R,S,T,D,A;

  if(N!=3 || a[3]==0) return(-100000);
  a1=a[2]/a[3];
  a2=a[1]/a[3];
  a3=a[0]/a[3];
  Q=(3*a2-a1*a1)/9.;
  R=(9*a1*a2-27*a3-2*a1*a1*a1)/54.;
  D=Q*Q*Q+R*R;

  if(D>0){
    S=R+sqrt(D);
    T=R-sqrt(D);
    if(S>0) S=pow(S,(double)1./3.);
    else S=-pow(-S,(double)1./3.);
    if(T>0) T=pow(T,(double)1./3.);
    else T=-pow(-T,(double)1./3.);
    x[0]=S+T-a1/3.;
    return(1);
  }
  else{
    double PI2=acos(-1.);
    if(Q!=0) A=acos(R/sqrt(-Q*Q*Q));
    else A=0;

    Q=2.*sqrt(-Q);
    x[0]=Q*cos(A/3.)-a1/3.;
    x[1]=Q*cos(A/3.+2.*PI2/3.)-a1/3.;
    x[2]=Q*cos(A/3+4.*PI2/3.)-a1/3.;

    if(fabs(x[0])>fabs(x[1])){ Q=x[1]; x[1]=x[0]; x[0]=Q; }
    if(fabs(x[0])>fabs(x[2])){ Q=x[2]; x[2]=x[0]; x[0]=Q; }
    if(fabs(x[1])>fabs(x[2])){ Q=x[2]; x[2]=x[1]; x[1]=Q; }

    return(3);
  }
}
/**
* \fn double ami_polynomial_evaluation(double *a,int Na,double x)
* \brief function to evaluate a polynom using the Horner algorithm
* \author Luis Alvarez
* \return return the evaluation
*/
double ami_polynomial_evaluation(
  double *a    /** POLYNOM COEFICIENT */,
  int Na       /** POLYNOM DEGREE */,
  double x     /** POINT WHERE THE POLYNOM IS EVALUATED */
  )
{
  double sol=a[Na];
  int i;
  for(i=Na-1;i>-1;i--) sol=sol*x+a[i];
  return(sol);
}


/**
* \fn int ami_lens_distortion_polynomial_update(double *x, double *y,int Np,
double *a,int Na,double x0,double y0,int k,double *pol)
* \brief function to add the information of a line point sequence to the 4
degree polynomial to compute the lens distortion model
* \author Luis Alvarez
* \return return 0 if the function finishes properly
*/
int ami_lens_distortion_polynomial_update(
   double *x, double *y /** DISTORTED LINE COORDINATES (INPUT)*/,
   int Np               /** NUMBER OF POINTS (INPUT)*/,
   double *a            /** POLYNOMIAL DEFINING THE LENS DISTORTION MODEL (INPUT) */,
   int Na               /** DEGREE OF POLYNOMIAL MODEL FOR LENS DISTORTION */,
   double x0,double y0  /** COORDINATES OF THE IMAGE CENTER */,
   int k                /** COEFICIENT OF THE POLYNOMIAL TO BE UPDATED*/,
   double *pol          /** 4 DEGREE POLYNOM TO MINIMIZE (INPUT-OUTPUT) */
   )
{
  int i,j;
  double *A,*x2,*y2,*d,x2_m,y2_m,s_xx,s_yy,xA_m,xd_m,yA_m,yd_m;
  double pol1[5],pol2[5],pol3[5];

  A=x2=y2=d=NULL;

  /* WE ALLOCATE MEMORY */
  A=(double*)malloc( sizeof(double)*Np );
  x2=(double*)malloc( sizeof(double)*Np );
  y2=(double*)malloc( sizeof(double)*Np );
  d=(double*)malloc( sizeof(double)*Np );

  /* WE COMPUTE THE DISTANCE TO THE IMAGE CENTER */
  for(i=0;i<Np;i++)
    d[i]=sqrt( (x[i]-x0)*(x[i]-x0)+(y[i]-y0)*(y[i]-y0) );

  /* WE COMPUTE THE POINT TRANSFORMATION WITH THE CURRENT LENS DISTORTION MODEL */
  for(i=0;i<Np;i++){
    A[i]=ami_polynomial_evaluation(a,Na,d[i]);
    x2[i]=x0+(x[i]-x0)*A[i];
    y2[i]=y0+(y[i]-y0)*A[i];
  }

  /* WE COMPUTE THE DISTANCE POWER k (THE COEFICIENT OF THE LENS DISTORTION MODEL
    TO BE UPDATED */
  for(i=0;i<Np;i++)  d[i]=pow(d[i],(double) k);

  /* WE COMPUTE THE VARIANCE OF THE TRANSFORMED POINTS */
  x2_m=0; for(i=0;i<Np;i++) x2_m+=x2[i];  x2_m/=Np;
  s_xx=0; for(i=0;i<Np;i++) s_xx+=(x2[i]-x2_m)*(x2[i]-x2_m); s_xx/=Np;
  y2_m=0; for(i=0;i<Np;i++) y2_m+=y2[i];  y2_m/=Np;
  s_yy=0; for(i=0;i<Np;i++) s_yy+=(y2[i]-y2_m)*(y2[i]-y2_m); s_yy/=Np;

   /* WE COMPUTE SOME AVERAGES WE NEED */
  xA_m=0; for(i=0;i<Np;i++) xA_m+=(x[i]-x0)*A[i]; xA_m/=Np;
  xd_m=0; for(i=0;i<Np;i++) xd_m+=(x[i]-x0)*d[i]; xd_m/=Np;
  yA_m=0; for(i=0;i<Np;i++) yA_m+=(y[i]-y0)*A[i]; yA_m/=Np;
  yd_m=0; for(i=0;i<Np;i++) yd_m+=(y[i]-y0)*d[i]; yd_m/=Np;

  /* WE COMPUTE THE POLYNOMIAL TO MINIMIZE */
  for(i=0;i<5;i++) pol1[i]=pol2[i]=pol3[i]=0;
  for(i=0;i<Np;i++){
    pol1[0]+=((x[i]-x0)*A[i]-xA_m)*((x[i]-x0)*A[i]-xA_m);
    pol1[1]+=2.*((x[i]-x0)*A[i]-xA_m)*((x[i]-x0)*d[i]-xd_m);
    pol1[2]+=((x[i]-x0)*d[i]-xd_m)*((x[i]-x0)*d[i]-xd_m);
    pol2[0]+=((y[i]-y0)*A[i]-yA_m)*((y[i]-y0)*A[i]-yA_m);
    pol2[1]+=2.*((y[i]-y0)*A[i]-yA_m)*((y[i]-y0)*d[i]-yd_m);
    pol2[2]+=((y[i]-y0)*d[i]-yd_m)*((y[i]-y0)*d[i]-yd_m);
    pol3[0]+=((y[i]-y0)*A[i]-yA_m)*((x[i]-x0)*A[i]-xA_m);
    pol3[1]+=((y[i]-y0)*A[i]-yA_m)*((x[i]-x0)*d[i]-xd_m)+
             ((y[i]-y0)*d[i]-yd_m)*((x[i]-x0)*A[i]-xA_m);
    pol3[2]+=((y[i]-y0)*d[i]-yd_m)*((x[i]-x0)*d[i]-xd_m);
  }

  for(i=0;i<3;i++){
    for(j=0;j<3;j++){
      pol[i+j]+=(pol1[i]*pol2[j]-pol3[i]*pol3[j])/1. /*s_max*/;
    }
  }
  /* WE FREE MEMORY */
  if(A!=NULL) free(A); if(x2!=NULL) free(x2); if(y2!=NULL) free(y2); if(d!=NULL) free(d);
  return(0);
}

/**
* \fn int ami_lens_distortion_model_update(double *a,int Na,int k,double *pol)
* \brief function to update the lens distortion model by minimizing a 4 degree
polynom
* \author Luis Alvarez
* \return return 0 if the function finishes properly
*/
int ami_lens_distortion_model_update(
   double *a   /** POLYNOMIAL DEFINING THE LENS DISTORTION MODEL (INPUT-OUTPUT) */,
   int Na      /** DEGREE OF POLYNOMIAL MODEL FOR LENS DISTORTION (INPUT)*/,
   int k       /** COEFICIENT OF THE POLYNOMIAL TO BE UPDATED (INPUT)*/,
   double *pol /** 4 DEGREE POLYNOM TO MINIMIZE (INPUT) */
   )
{
  int j,i,M=Na;
  double *x,*b,p[3];

  x=b=NULL;

  /* WE ALLOCATE MEMORY */
  x=(double*)malloc( sizeof(double)*3 );
  b=(double*)malloc( sizeof(double)*4 );

  b[0]=pol[1]; b[1]=2*pol[2]; b[2]=3.*pol[3]; b[3]=4.*pol[4];
  M=ami_RootCubicPolynomial(b,3,x);

  for(i=0;i<M;i++) p[i]=ami_polynomial_evaluation(pol,4,x[i]);
  j=0;
  if(M==3){
    if(p[j]>p[1] && fabs(x[1])<1. ) j=1;
    if(p[j]>p[2] && fabs(x[2])<1. ) j=2;
  }
  if(fabs(x[j])<1.) a[k]+=x[j];
  if(x!=NULL) free(x); if(b!=NULL) free(b);
  return(0);
}

/**
* \fn double ami_LensDistortionEnergyError(double *x,double *y,int Np,double x0,
double y0,double *a,int Na)
* \brief function to compute the lens distortion energy error (the residual
variance of the point distribution
* \author Luis Alvarez
* \return return the evaluation
*/
double ami_LensDistortionEnergyError(
  double *x,double *y  /** ORIGINAL POINT DISTRIBUTION (INPUT)*/,
  int Np               /** NUMBER OF POINTS (INPUT)*/,
  double x0,double y0  /** CENTER OF THE IMAGE (INPUT)*/,
  double *a            /** Lens Distortion Polynomial model (INPUT)*/,
  int Na               /** Degree of Polynomial model (INPUT)*/
  )
{
    int i;
   double A,*x2,*y2,d,x2_m,y2_m,s_xx,s_yy,s_xy;

   x2=y2=NULL;

  /* WE ALLOCATE MEMORY */
  x2=(double*)malloc( sizeof(double)*Np );
  y2=(double*)malloc( sizeof(double)*Np );

  /* WE COMPUTE THE POINT TRANSFORMATION USING THE LENS DISTORTION MODEL*/
  for(i=0;i<Np;i++){
    d=sqrt( (x[i]-x0)*(x[i]-x0)+(y[i]-y0)*(y[i]-y0) );
    A=ami_polynomial_evaluation(a,Na,d);
    x2[i]=x0+(x[i]-x0)*A;
    y2[i]=y0+(y[i]-y0)*A;
  }
  /* WE COMPUTE THE VARIANCE OF THE TRANSFORMED POINTS */
  x2_m=0; for(i=0;i<Np;i++) x2_m+=x2[i];  x2_m/=Np;
  s_xx=0; for(i=0;i<Np;i++) s_xx+=(x2[i]-x2_m)*(x2[i]-x2_m); s_xx/=Np;
  y2_m=0; for(i=0;i<Np;i++) y2_m+=y2[i];  y2_m/=Np;
  s_yy=0; for(i=0;i<Np;i++) s_yy+=(y2[i]-y2_m)*(y2[i]-y2_m); s_yy/=Np;
  s_xy=0; for(i=0;i<Np;i++) s_xy+=(y2[i]-y2_m)*(x2[i]-x2_m); s_xy/=Np;

  /* WE FREE MEMORY */
  if(x2!=NULL) free(x2); if(y2!=NULL) free(y2);
  return((s_xx*s_yy-s_xy*s_xy));
}

/**
* \fn double ami_LensDistortionEnergyError_Vmin(double *x,double *y,int Np,
double x0,double y0,double *a,int Na)
* \brief function to compute the lens distortion vmin energy error of the point
distribution
* \author Luis Alvarez
* \return return the evaluation
*/
double ami_LensDistortionEnergyError_Vmin(
  double *x,double *y  /**ORIGINAL POINT DISTRIBUTION (INPUT)*/,
  int Np               /** NUMBER OF POINTS (INPUT)*/,
  double x0,double y0  /** CENTER OF THE IMAGE (INPUT)*/,
  double *a            /** Lens Distortion Polynomial model (INPUT)*/,
  int Na               /** Degree of Polynomial model (INPUT)*/
  )
{
  int i;
  double A,*x2,*y2,d,x2_m,y2_m,s_xx,s_yy,s_max,s_xy;

  x2=y2=NULL;

  /* WE ALLOCATE MEMORY */
  x2=(double*)malloc( sizeof(double)*Np );
  y2=(double*)malloc( sizeof(double)*Np );

  /* WE COMPUTE THE POINT TRANSFORMATION USING THE LENS DISTORTION MODEL*/
  for(i=0;i<Np;i++){
    d=sqrt( (x[i]-x0)*(x[i]-x0)+(y[i]-y0)*(y[i]-y0) );
    A=ami_polynomial_evaluation(a,Na,d);
    x2[i]=x0+(x[i]-x0)*A;
    y2[i]=y0+(y[i]-y0)*A;
  }
  /* WE COMPUTE THE VARIANCE OF THE TRANSFORMED POINTS */
  x2_m=0; for(i=0;i<Np;i++) x2_m+=x2[i];  x2_m/=Np;
  s_xx=0; for(i=0;i<Np;i++) s_xx+=(x2[i]-x2_m)*(x2[i]-x2_m); s_xx/=Np;
  y2_m=0; for(i=0;i<Np;i++) y2_m+=y2[i];  y2_m/=Np;
  s_yy=0; for(i=0;i<Np;i++) s_yy+=(y2[i]-y2_m)*(y2[i]-y2_m); s_yy/=Np;
  s_xy=0; for(i=0;i<Np;i++) s_xy+=(y2[i]-y2_m)*(x2[i]-x2_m); s_xy/=Np;
  s_max=s_xx>s_yy?s_xx:s_yy;

  /* WE FREE MEMORY */
  if(x2!=NULL) free(x2); if(y2!=NULL) free(y2);
  return((s_xx*s_yy-s_xy*s_xy)/s_max);
}

/**
* \fn int ami_inverse_lens_distortion(double x,double y,double x0,double y0,
double *xt,double *yt,double *a,int Na)
* \brief function to inverse the lens distortion transformation
* \author Luis Alvarez
* \return return 0 if the function finishes properly
*/
int ami_inverse_lens_distortion(
  double x,double y        /** POINT TO INVERSE (INPUT)*/,
  double x0,double y0      /** CENTER OF THE IMAGE (INPUT)*/,
  double *xt,double *yt    /** INVERVE POINT TRANSFORMED (OUTPUT) */,
  double *a                /** LENS DISTORTION MODEL POLYNOM */,
  int Na                   /** DEGREE OF THE LENS DISTORTION MODEL POLYNOM */
  )
{
  int i,Nr;
  double paso,d,*b,*b2,*rx,*ry,root;

  b=b2=rx=ry=NULL;
  Nr=0; /*Added by Luis Gomez*/

  /* WE ALLOCATE MEMORY */
  b=(double*)malloc( sizeof(double)*(Na+2) );
  b2=(double*)malloc( sizeof(double)*(Na+2) );
  rx=(double*)malloc( sizeof(double)*(Na+2) );
  ry=(double*)malloc( sizeof(double)*(Na+2) );
  for(i=0;i<Nr;i++) {rx[i]=0.0;ry[i]=0.0;} /*Added by Luis Gomez*/
  /* WE DEFINE THE POLYNOM WE NEED TO COMPUTE ROOTS */
  d=sqrt((x-x0)*(x-x0)+(y-y0)*(y-y0));
  b[0]=-1; b[1]=1.;
  paso=d;
  for(i=2;i<(Na+2);i++){
    b[i]=a[i-1]*paso;
    paso*=d;
   }
  if(Na==2){
    Nr=ami_RootCubicPolynomial(b,3,rx);
    for(i=0;i<Na;i++) ry[i]=0;
  }
  else{
    for(i=0;i<(Na+2);i++) b2[i]=b[Na+1-i];
    Nr=ami_polynomial_root(b2,Na+1,rx,ry);
  }
  /* WE SELECT THE REAL ROOT NEAR TO 1 */
  root=10e5;
  for(i=0;i<Nr;i++){
    if(fabs(ry[i])<0.00000000001 && fabs(root-1)>fabs(rx[i]-1)) root=rx[i];
  }
  /* WE TRANSFORM THE POINT COORDINATES */
  *xt=x0+(x-x0)*root;
  *yt=y0+(y-y0)*root;
  if(b!=NULL) free(b); if(rx!=NULL) free(rx); if(ry!=NULL) free(ry); if(b2!=NULL) free(b2);
  return(0);
}
/**
* \fn double ami_lens_distortion_estimation(double **x,double **y,int Nl,
int *Np,double x0,double y0,double *a,int Na,int k,double alfa)
* \brief function to compute the lens distortion model
* \author Luis Alvarez
* \return return (Error/Nl)
*/
double ami_lens_distortion_estimation(
    double **x,double **y  /** ORIGINAL COLECCION OF LINES DISTRIBUTION (INPUT)*/,
    int Nl                 /** NUMBER OF LINES */,
    int *Np                /** NUMBER OF POINTS FOR EACH LINE(INPUT)*/,
    double x0,double y0    /** CENTER OF THE IMAGE (INPUT)*/,
    double *a              /** Lens Distortion Polynomial model (INPUT-OUTPUT) */,
    int Na                 /** Degree of Polynomial model (INPUT)*/,
    int k                  /** COEFICIENT OF THE LENS DISTORTION POLYNOM MODEL TO BE UPDATED */,
    double alfa            /** WEIGHT FOR MINIMIZING THE SQUARE OF THE DISTANCE BEWTEEN DISTORTED AND UNDISTORTED POINTS */
    )
{
  double *pol,suma_dd,suma_Ad,d,A,Error=0;
  int m,i,j;
  pol=NULL;

  /* WE ALLOCATE MEMORY */
  pol=(double*)malloc( sizeof(double)*5 );

  for(i=0;i<=4;i++) pol[i]=0.;

  /* WE ADAPT a[0] TO MINIMIZE THE SQUARE OF THE DISTANCE BEWTEEN
    DISTORTED AND UNDISTORDED POINTS */
  if(alfa>0){
    suma_dd=suma_Ad=0;
    for(m=0;m<Nl;m++){
      for(i=0;i<Np[m];i++){
        d=sqrt( (x[m][i]-x0)*(x[m][i]-x0)+(y[m][i]-y0)*(y[m][i]-y0) );
        A=0;
        for(j=1;j<=Na;j++) A+=a[j]*pow(d,(double) j+1);
        suma_dd+=d*d;
        suma_Ad+=A*d;
      }
    }
    a[0]=1-suma_Ad/suma_dd;
  }

  /* WE COMPUTE THE LENS DISTORTION MODEL */
  for(i=0;i<Nl; i++){
    ami_lens_distortion_polynomial_update(x[i],y[i],Np[i],a,Na,x0,y0,k,pol);
  }
  ami_lens_distortion_model_update(a,Na,k,pol);

  /* WE FREE THE MEMORY */
  if(pol!=NULL) free(pol);
  for(i=0;i<Nl;i++) Error+=ami_LensDistortionEnergyError(x[i],y[i],Np[i],x0,y0,a,Na);
  return(Error/Nl);
}

/**
* \fn void ami_lens_distortion_zoom_normalization(double **x,double **y,int Nl,
int *Np,double x0,double y0,double *a,int Na)
* \brief function to apply the zoom strategy
* \author Luis Alvarez and Luis Gomez
* \return return void
*/
void ami_lens_distortion_zoom_normalization(
    double **x,double **y  /** ORIGINAL COLECCION OF LINES DISTRIBUTION (INPUT)*/,
    int Nl                 /** NUMBER OF LINES */,
    int *Np                /** NUMBER OF POINTS FOR EACH LINE(INPUT)*/,
    double x0,double y0    /** CENTER OF THE IMAGE (INPUT)*/,
    double *a              /** Lens Distortion Polynomial model (INPUT-OUTPUT)*/,
    int Na                 /** Degree of Polynomial model (INPUT)*/
    )
{
  int i,k,m,N=0;
  double Z,d,suma_Ad,A;

  /* WE UPDATE a BY ESTIMATING A ZOOM FACTOR Z */
   suma_Ad=0;
    for(m=0;m<Nl;m++){
      for(i=0;i<Np[m];i++){
        N++;
        d=sqrt( (x[m][i]-x0)*(x[m][i]-x0)+(y[m][i]-y0)*(y[m][i]-y0) );
        A=a[0];
        for(k=1;k<=Na;k++) A+=a[k]*pow(d,(double) k);
        suma_Ad+=A*A;
      }
    }
    Z=sqrt((double) N/suma_Ad);
    for(k=0;k<=Na;k++) a[k]*=Z;
}



/**
*        GRADIENT PART
*/


/**
* \fn int calculate_points(double *amin,double **points_2D_modified,int N,
int Na,double x0,double y0)
* \brief function to estimate the position of 2D points (pixels) for the actual
lens distortion model
* \author Luis Alvarez and Luis Gomez
* \return return 0 if the function finishes properly
*/
int calculate_points(
        double *amin                    /** Lens distortion model polynom */,
        double **points_2D_modified     /** Cloud of points to be fitted to a line */,
        int N                           /** Number of points */,
        int Na                          /** Degree of the lens distortion model */,
        double x0                       /** x center of the image */,
        double y0                        /** y center of the image */
        )
 	{
      double d1,sol;
      int i,j;

      /* Calculate the distance from each point to the center of the image */
      /* WE COMPUTE THE POINT TRANSFORMATION WITH THE CURRENT LENS DISTORTION MODEL */
  	for(i=0;i<N;i++){
          d1=sqrt(pow(points_2D_modified[i][0]-x0,2.0)+pow(points_2D_modified[i][1]-y0,2.0));
          sol=amin[Na];
          for(j=Na-1;j>-1;j--) sol=sol*d1+amin[j];
          points_2D_modified[i][0]=(points_2D_modified[i][0]-x0)*sol+x0;
 	  points_2D_modified[i][1]=(points_2D_modified[i][1]-y0)*sol+y0;
  	}
    return(0);
   }

/**
* \fn double distance_function(double *amin,double **x,double **y,int Nl,
int *Np,int Na,double x0,double y0)
* \brief function to be optimized by the gradient (objective distance function)
* \author Luis Alvarez and Luis Gomez
* \return return the RMS distance for all the points to the rect
*/
double distance_function(
        double *amin    /** Lens distortion model polynom */,
        double **x      /** Coordinates of points */,
        double **y      /** Coordinates of points */,
        int Nl          /** Number of lines */,
        int *Np         /** Number of points/line */,
        int Na          /** Degree of the lens distortion model */,
        double x0       /** x Center of the image */,
        double y0       /** y Center of the image */
        )
   {
   double **points_2D_modified,sum,f_objective,a,b,c,tmp;
   double rect[3];
   int i,j;
   points_2D_modified=NULL;

   f_evaluations++;
   f_objective=0.0;
   for (i=0;i<Nl;i++){
     points_2D_modified=(double**)malloc(sizeof(double*)*Np[i]);
     for(j=0;j<Np[i];j++){
        points_2D_modified[j]=(double*)malloc(sizeof(double)*2);
        points_2D_modified[j][0]=x[i][j];
        points_2D_modified[j][1]=y[i][j];
     }
     calculate_points(amin,points_2D_modified,Np[i],Na,x0,y0);
     ami_calculo_recta2d(rect,points_2D_modified,Np[i]);
     a=rect[1];b=rect[0];c=rect[2];
     tmp=pow(a,2.0)+pow(b,2.0);
     sum=0.0;
     for(j=0;j<Np[i];j++)	sum=sum+pow(b*points_2D_modified[j][0]+a*points_2D_modified[j][1]+c,2.0);
     sum=sum/tmp;
     sum=sum/(double)Np[i];
     f_objective=f_objective+sum;
     for(j=0;j<Np[i];j++){ free(points_2D_modified[j]);} free(points_2D_modified);
    }
    f_objective=f_objective/(double)Nl;
    return(f_objective);
   }

/**
* \fn 	double find_lambda(double lambda1,double lambda2,double lambda3,
double f_1,double f_2,double f_3,double *amin, double **x,double **y,
int Nl, int *Np,int Na,double *grad_f,int  *change_k,double x0,double y0)
* \brief function to minimize in one dimension (searching lambda)
* \author Luis Alvarez and Luis Gomez
* \return return the value of lambda step
*/
double find_lambda(
        double lambda1     /** First TTP point */,
        double lambda2     /** Second TTP point */,
        double lambda3     /** Third TTP point */,
        double f_1         /** f_objective(lambda1) */,
        double f_2         /** f_objective(lambda2) */,
        double f_3         /** f_objective(lambda3) */,
        double *amin_copy /** Copy of amin */,
        double *amin       /** Lens distortion model polynom */,
        double **x         /** Coordinates of points */,
        double **y         /** Coordinates of points */,
        int Nl             /** Number of lines */,
        int *Np            /** Number of points/line */,
        int Na             /** Degree of the lens distortion model */,
        double *grad_f     /** Gradient vector at amin */,
        int  *change_k     /** To indicate what variable optimize (1: optimize, 0: no optimize) */,
        double x0          /** x center of the image  */,
        double y0          /** y center of the image  */
        )
   {

      double f_min,lambda,lambda_tmp1,lambda_tmp2,f_tmp1=f_3,f_tmp2=f_2,f=f_1;
      int i;
    /*minimum is in (lambda1,lambda2) */
      lambda_tmp1=(lambda2+lambda1)/2.0;
      for(i=0;i<=Na;i++){
   		 if(change_k[i]==1)	*(amin_copy+i)=*(amin+i)-lambda_tmp1*(*(grad_f+i));
      }
      f_tmp1=distance_function(amin_copy,x,y,Nl,Np,Na,x0,y0);
      if(f_tmp1<f_1) return(lambda_tmp1);

      lambda_tmp2=(lambda3+lambda2)/2.0;
      for(i=0;i<=Na;i++){
   		 if(change_k[i]==1)	*(amin_copy+i)=*(amin+i)-lambda_tmp2*(*(grad_f+i));
      }
      f_tmp2=distance_function(amin_copy,x,y,Nl,Np,Na,x0,y0);
      if(f_tmp2<f_1) return(lambda_tmp2);
      f=f_1;
      do{
   	f_min=f;
        lambda=lambda1+1e-8;
        for(i=0;i<=Na;i++){
     		 if(change_k[i]==1)	*(amin_copy+i)=*(amin+i)-lambda*(*(grad_f+i));
        }
        f=distance_function(amin_copy,x,y,Nl,Np,Na,x0,y0);
      }while(f<f_min);
      return(lambda);
   }


 /**
* \fn double minimize_cuadratic_polynom((double lambda1,double lambda2,double lambda3,
double f_1,double f_2,double f_3,double *amin, double **x,double **y,
int Nl, int *Np,int Na,double *grad_f,int  *change_k,double x0,double y0)
* \brief function to build and minimize the cuadratic TPP polynom
* \author Luis Alvarez and Luis Gomez
* \return return the minimum of the cuadratic polynom
*/
 double minimize_cuadratic_polynom(
        double lambda1     /** First TTP point */,
        double lambda2     /** Second TTP point */,
        double lambda3     /** Third TTP point */,
        double f_1         /** f_objective(lambda1) */,
        double f_2         /** f_objective(lambda2) */,
        double f_3         /** f_objective(lambda3) */,
        double *amin_copy /** Copy of amin */,
        double *amin       /** Lens distortion model polynom */,
        double **x         /** Coordinates of points */,
        double **y         /** Coordinates of points */,
        int Nl             /** Number of lines */,
        int *Np            /** Number of points/line */,
        int Na             /** Degree of the lens distortion model */,
        double *grad_f     /** Gradient vector at amin */,
        int  *change_k     /** To indicate what variable optimize (1: optimize, 0: no optimize) */,
        double x0          /** x center of the image  */,
        double y0          /** y center of the image  */
        )
	{

        double a12,a23,a31,b12,b23,b31,min_lambda;

        a12=lambda1-lambda2;
	a23=lambda2-lambda3;
	a31=lambda3-lambda1;
	b12=pow(lambda1,2.0)-pow(lambda2,2.0);
	b23=pow(lambda2,2.0)-pow(lambda3,2.0);
	b31=pow(lambda3,2.0)-pow(lambda1,2.0);

	min_lambda=0.5*(b23*f_1+b31*f_2+b12*f_3);
        min_lambda=min_lambda/(a23*f_1+a31*f_2+a12*f_3);
        /* to avoid numerical errors, we check the condition*/
        /* lambda1<minimo_lambda<lambda3 */
	    if((lambda1<min_lambda)&&(min_lambda<lambda3)) return(min_lambda);
         else{
  	      min_lambda=find_lambda(lambda1,lambda2,lambda3,f_1,f_2,f_3,amin_copy,amin,x,y,Nl,Np,Na,grad_f,change_k,x0,y0);
		  return(min_lambda);
         }
   }

 /**
* \fn double cuadratic_fitting(double *amin_copy,double *amin,double **x,
double **y,int Nl,int Na,int *Np,double lambda1,double lambda2,double lambda3,
double f_1,double f_2,double f_3,double *grad_f,int *change_k,double x0,
double y0)
* \brief function to find the minimum of the interpolating polynom
* \author Luis Alvarez and Luis Gomez
* \return return 0 if the function finishes properly
*/
double cuadratic_fitting(
        double *amin_copy  /** Copy of amin */,
        double *amin       /** Lens distortion model polynom */,
        double **x         /** Coordinates of points */,
        double **y         /** Coordinates of points */,
        int Nl             /** Number of lines */,
        int Na             /** Degree of the lens distortion model */,
        int *Np            /** Number of points/line */,
   	double lambda1     /** First TTP point */,
        double lambda2     /** Second TTP point */,
        double lambda3     /** Third TTP point */,
        double f_1         /** f_objective(lambda1) */,
        double f_2         /** f_objective(lambda2) */,
        double f_3         /** f_objective(lambda3) */,
        double *grad_f     /** Gradient vector at amin */,
        int *change_k      /** to indicate what variable optimize (1: optimize, 0: no optimize) */,
        double x0          /** x center of the image  */,
        double y0          /** y center of the image */
        )
 	{
         double minimo_lambda,f_minimo;
         double error=1e100;
         int iteraciones_lambda,i;

         iteraciones_lambda=0;
         /* We loop till getting the minimum */
         while(error>tol_lambda){
           minimo_lambda=minimize_cuadratic_polynom(lambda1,lambda2,lambda3,f_1,f_2,f_3,amin_copy,amin,x,y,Nl,Np,Na,grad_f,change_k,x0,y0);
           for(i=0;i<=Na;i++){
            if(change_k[i]==1)	*(amin_copy+i)=*(amin+i)-minimo_lambda*(*(grad_f+i));
           }
	   f_minimo=distance_function(amin_copy,x,y,Nl,Np,Na,x0,y0);
           if(minimo_lambda>lambda2){
         	if(f_minimo>f_2){
            	  lambda3=minimo_lambda;
               	  f_3=f_minimo;
            	}
            	else{
            	  lambda1=lambda2;
               	  f_1=f_2;
               	  lambda2=minimo_lambda;
               	  f_2=f_minimo;
            	 }
         	}
         	else{
         	  if(f_minimo>=f_2){
            	    lambda1=minimo_lambda;
               	    f_1=f_minimo;
            	  }
            	else{
               	  lambda3=lambda2;
               	  f_3=f_2;
               	  lambda2=minimo_lambda;
               	  f_2=f_minimo;
            	 }
           }
           error=fabs(lambda3-lambda1);
           if(f_minimo==f_2)lambda2=lambda2+tol_lambda;
           iteraciones_lambda++;
           if(iteraciones_lambda==max_itera_lambda) return(lambda2);
         }
         return(lambda2);
  }

/**
* \fn double minimize_lambda(double *amin,double *amin_copy,double **x,
double **y,int Na,int Nl,int *Np,double *grad_f,double f, int *change_k,
double x0,double y0)
* \brief function Unidimensional lambda miminization
* \author Luis Alvarez and Luis Gomez
* \return return the minimum in the 1D search
*/
double minimize_lambda(
        double *amin           /** Lens distortion model polynom */,
        double **x             /** Coordinates of points */,
        double **y             /** Coordinates of points */,
        int Na                 /** Degree of the lens distortion model */,
        int Nl                 /** Number of lines */,
        int *Np                /** Number of points/line */,
        double *grad_f         /** Gradient vector at amin */,
        double f               /** function value at amin */,
        int *change_k          /** To indicate what variable optimize (1: optimize, 0: no optimize) */,
        double x0              /** x center of the image */,
        double y0              /** y center of the image */
        )
 	{
    double lambda1,lambda2,lambda3,lambda;
    double f_1,f_2,f_3=0.,last_f3,last_f2;
    double tol_ff=1.0e-10;
    double *amin_copy;
    int i;

    amin_copy=NULL;
    amin_copy=(double*)malloc( sizeof(double)*(Na+1) );


     f_1=f;
     /* search the TTP points */
     lambda1=0.0;
     /* search lambda2 */
     lambda2=fabs(grad_f[1]);
     for(i=0;i<=Na;i++)*(amin_copy+i)=*(amin+i)-lambda2*(*(grad_f+i));
     f_2=distance_function(amin_copy,x,y,Nl,Np,Na,x0,y0);
     if(f_2>f_1){
       lambda3=lambda2;
       f_3=f_2;
       /* search lambda2 by dividing the (lambda1,lambda3) interval */
       lambda2=lambda3/2.0;
       while(1){
         last_f2=f_2;
         for(i=0;i<=Na;i++){
     		if(change_k[i]==1) *(amin_copy+i)=*(amin+i)-lambda2*(*(grad_f+i));
      	  }
       	  f_2=distance_function(amin_copy,x,y,Nl,Np,Na,x0,y0);
       	  if(f_2<f_1) break;
          if(fabs((f_2-last_f2)/last_f2)<=tol_ff){   /* Avoid the flat zone */
              if(amin_copy!=NULL) free(amin_copy);   /*free memory*/
              return(lambda2);
          }
         lambda2=lambda2/2.0;
      	}
      }
      else{
        /* search lambda3 by dividing the (lambda1,lambda2) interval */
          lambda3=lambda2*4.0;
          while(1){
          last_f3=f_3;
            for(i=0;i<=Na;i++){
      			if(change_k[i]==1)	*(amin_copy+i)=*(amin+i)-lambda3*(*(grad_f+i));
      		}
            f_3=distance_function(amin_copy,x,y,Nl,Np,Na,x0,y0);
            if(f_3>f_2) break;
           if(fabs((f_3-last_f3)/last_f3)<=tol_ff){             /* Avoid the flat zone */
             if(amin_copy!=NULL) free(amin_copy);		/* Free memory */
             return(lambda3);    /* Avoid the flat zone */
            }
            lambda3=4*lambda3;
          }
      }
      /* We have the points satisfying the TTP condition
      lambda1,f_1;lambda_2,f_2;lambda3,f_3
      minimize the cuadratic polynom */
      lambda=cuadratic_fitting(amin_copy,amin,x,y,Nl,Na,Np,lambda1,lambda2,lambda3,f_1,f_2,f_3,grad_f,change_k,x0,y0);
      if(amin_copy!=NULL) free(amin_copy);
      return(lambda);
   }

/**
* \fn double  gradient_method(double  *amin,double  **x,double  **y,int Nl,
int *Np,int Na,int *change_k, double x0,double y0,int zoom)
* \brief function to minimize the distance function by gradient
* \author Luis Alvarez and Luis Gomez
* \return return the vector solution (distortion coefficients)
*/
double  gradient_method(
        double  *amin      /** Lens distortion model polynom */,
        double  **x        /** Coordinates of points */,
        double  **y        /** Coordinates of points */,
        int     Nl         /** Number of lines */,
        int     *Np        /** Number of points/line */,
        int     Na         /** Degree of the lens distortion model */,
        int     *change_k  /** to indicate what variable optimize (1: optimize, 0: no optimize) */,
        double  x0         /** x center of the image */,
        double  y0         /** y center of the image */,
        int     zoom       /** Zoom strategy */
        )
  	{
	double	*grad_f,f_objective,last_f=1.0e100;
   	double	kk,lambda;
	int i;

	grad_f=NULL;
        grad_f=(double*)malloc( sizeof(double)*(Na+1) );

    f_objective=distance_function(amin,x,y,Nl,Np,Na,x0,y0);
    while(fabs((f_objective-last_f)/last_f)>tol_f){
        last_f=f_objective;
     	for(i=0;i<=Na;i++){
      	/* Move along each axis and incremental step to calculate the derivative */
      	   kk=amin[i];
           amin[i]=kk+delta;
           grad_f[i]=(distance_function(amin,x,y,Nl,Np,Na,x0,y0)-f_objective)/delta;
      	   amin[i]=kk;
   	}
        /* Activate to stop the gradient when the gradient_norm<tol_norma gradient_norm=0.0;
	   for(i=0;i<=Na;i++)	gradient_norm=gradient_norm+pow(grad_f[i],2.0);
           gradient_norm=sqrt(gradient_norm);		if(gradient_norm<=tol_norma) break; */
   	lambda=minimize_lambda(amin,x,y,Na,Nl,Np,grad_f,f_objective,change_k,x0,y0);
   	for(i=1;i<=Na;i++) if(change_k[i]==1) *(amin+i)=*(amin+i)-lambda*(*(grad_f+i));
        n_iterations++;
        f_objective=distance_function(amin,x,y,Nl,Np,Na,x0,y0);
         /* Activate to have the trace of the execution */
         /* printf("\nIteracion=%d\tf=%1.18f\t|grad(f)|=%1.18f",n_iteraciones,f_objective,gradient_norm); */
        if(n_iterations==max_itera) break;
     }
      /** ZOOM UPDATE amin[0] */
      if(zoom==1) ami_lens_distortion_zoom_normalization(x,y,Nl,Np,x0,y0,amin,Na);
      if(grad_f!=NULL) free(grad_f);
      return(0);
   }

/**
* \fn double  optimize(double  *amin,double **x, double **y,double **xx,
double **yy,int Nl,int *Np,int Na, int *change_k,double x0,double y0,
double factor_n,int zoom, FILE *fp1)
* \brief function to execute the gradient method and save information to a file
* \author Luis Alvarez and Luis Gomez
* \return return 1 if the function finishes properly
*/
int  optimize(
        double  *amin      /** Lens distortion model polynom */,
        double  **x        /** Coordinates of points (normalized) */,
        double  **y        /** Coordinates of points (normalized) */,
        double  **xx       /** Coordinates of points */,
        double  **yy       /** Coordinates of points */,
        int     Nl         /** Number of lines */,
        int     *Np        /** Number of points/line */,
        double  x0         /** x center of the image */,
        double  y0         /** y center of the image */,
        double  factor_n   /** Factor to normalize coordinates */,
        int     zoom       /** Zoom strategy */,
        FILE    *fp1       /** Pointer to the output file */,
        double  *trivial   /** Trivial Emin,Vmin,Dmin values */,
        int     control    /** To start from trivial solution or algebraic solution */
        )
 {

    int starttime, stoptime,i;
    double paso,Emin,Vmin,tmp,D;
    double timeused;
    int *change_k;            /* to activate the variables to be optimized by gradient method */
    int Na=2;                  /* DEGREE OF THE POLYNOM */
    change_k=NULL;

     if(zoom==0){
        fprintf(fp1,"\n*************************************************************************\n");
        fprintf(fp1,"***********************GRADIENT METHOD: WITHOUT ZOOM*********************\n");
        fprintf(fp1,"*************************************************************************\n");
    }

   else{
        fprintf(fp1,"\n*************************************************************************\n");
        fprintf(fp1,"***********************GRADIENT METHOD: WITH ZOOM************************\n");
        fprintf(fp1,"*************************************************************************\n");
    }

    change_k=NULL;
    f_evaluations=0;n_iterations=0;
    if(control==1) amin[0]=1; for(i=1;i<=Na;i++) amin[i]=0; /* trivial solution */
    ami_calloc1d(change_k,int,Na+1);
    change_k[0]=0;change_k[2]=1;  /* OPTIMIZED ONLY */
    starttime = clock();
    gradient_method(amin,x,y,Nl,Np,Na,change_k,0.0,0.0,zoom);
    stoptime = clock();
    timeused = difftime(stoptime,starttime);
    fprintf(fp1,"Na (degree of  lens distortion model polynom) = %d",Na);
   /********************************************************/
   /* Final solution is in amin (distortion parameters) */
   /* Gradient works in normalized coordinates. */
   /* We undo the normalization to have the solutionin pixels */
    paso=1.0;for(i=0;i<=Na;i++){ amin[i]=amin[i]*paso; paso/=factor_n;}
    /* We get the final solution and print into a file */
    fprintf(fp1,"\nMODIFIED VARIABLES THROUGH OPTIMIZATION:");
    for(i=0;i<=Na;i++) if(change_k[i]==1) fprintf(fp1,"\tk[%d]",i);
    Emin=0.0;for(i=0;i<Nl;i++)Emin+=ami_LensDistortionEnergyError(xx[i],yy[i],Np[i],x0,y0,amin,Na);Emin=Emin/(double)Nl;
    Vmin=0.0;for(i=0;i<Nl;i++)Vmin+=ami_LensDistortionEnergyError_Vmin(xx[i],yy[i],Np[i],x0,y0,amin,Na);Vmin=Vmin/(double)Nl;
    D=distance_function(amin,xx,yy,Nl,Np,Na,x0,y0);
     /* CALCULATE THE IMPROVEMTS RESPECTO TO TRIVIAL SOLUTION */
    tmp=fabs((Emin-trivial[0])/trivial[0])*100.0;
    fprintf(fp1,"\n\nEmin =\t%1.15e with Na=%d. Improvement = \t%f\n",Emin,Na,tmp);
    tmp=fabs((Vmin-trivial[1])/trivial[1])*100.0;
    fprintf(fp1,"Vmin =\t%1.15e with Na=%d. Improvement = \t%f\n",Vmin,Na,tmp);
    tmp=fabs((D-trivial[2])/trivial[2])*100.0;
    fprintf(fp1,"D =\t%1.15e with Na=%d. Improvement = \t%f\n",D,Na,tmp);
    fprintf(fp1,"\n\nDistortion parameters:\n");
    for(i=0;i<=Na;i++) fprintf(fp1,"a[%d] = %1.15e\n",i,amin[i]);
    fprintf(fp1,"\nNumber ot gradient iterations= %d",n_iterations);
    fprintf(fp1,"\nCPU_time = %f (seconds)",timeused/(double)CLOCKS_PER_SEC);
    fprintf(fp1,"\nf_evaluations = %d",f_evaluations);
    fprintf(fp1,"\n------------------------------\n\n");
    if(change_k!=NULL) free(change_k); /* FREE THE MEMORY */


    f_evaluations=0;n_iterations=0;
    Na=4;
    change_k=NULL;                                     	/* DEGREE OF THE POLYNOM */
    ami_calloc1d(change_k,int,Na+1);           		 /* WE ALLOCATE MEMORY */
    if(control==1) amin[0]=1; for(i=1;i<=Na;i++) amin[i]=0;   /* TRIVIAL SOLUTION */
    change_k[0]=0;change_k[2]=1;change_k[3]=0;change_k[4]=1;  /* OPTIMIZED ONLY K2 AND K4 */
    starttime = clock();
    gradient_method(amin,x,y,Nl,Np,Na,change_k,0.0,0.0,zoom);
    stoptime = clock();
    timeused = difftime(stoptime,starttime);
    fprintf(fp1,"Na (degree of  lens distortion model polynom) = %d",Na);
   /********************************************************/
   /*Final solution is in amin (distortion parameters) */
   /* Gradient works in normalized coordinates.*/
   /* We undo the normalization to have the solutionin pixels */
    paso=1.0;for(i=0;i<=Na;i++){ amin[i]=amin[i]*paso; paso/=factor_n;}
    /* We get the final solution and print into a file */
    fprintf(fp1,"\nMODIFIED VARIABLES THROUGH OPTIMIZATION:");
    for(i=0;i<=Na;i++) if(change_k[i]==1) fprintf(fp1,"\tk[%d]",i);
    Emin=0.0;for(i=0;i<Nl;i++)Emin+=ami_LensDistortionEnergyError(xx[i],yy[i],Np[i],x0,y0,amin,Na);Emin=Emin/(double)Nl;
    Vmin=0.0;for(i=0;i<Nl;i++)Vmin+=ami_LensDistortionEnergyError_Vmin(xx[i],yy[i],Np[i],x0,y0,amin,Na);Vmin=Vmin/(double)Nl;
    D=distance_function(amin,xx,yy,Nl,Np,Na,x0,y0);
    /* CALCULATE THE IMPROVEMENTS RESPECTO TO TRIVIAL SOLUTION */
    tmp=fabs((Emin-trivial[0])/trivial[0])*100.0;
    fprintf(fp1,"\n\nEmin =\t%1.15e with Na=%d. Improvement = \t%f\n",Emin,Na,tmp);
    tmp=fabs((Vmin-trivial[1])/trivial[1])*100.0;
    fprintf(fp1,"Vmin =\t%1.15e with Na=%d. Improvement = \t%f\n",Vmin,Na,tmp);
    tmp=fabs((D-trivial[2])/trivial[2])*100.0;
    fprintf(fp1,"D =\t%1.15e with Na=%d. Improvement = \t%f\n",D,Na,tmp);
    fprintf(fp1,"\n\nDistortion parameters:\n");
    for(i=0;i<=Na;i++) fprintf(fp1,"a[%d] = %1.15e\n",i,amin[i]);
    fprintf(fp1,"\nNumber ot gradient iterations= %d",n_iterations);
    fprintf(fp1,"\nCPU_time = %f (seconds)",timeused/(double)CLOCKS_PER_SEC);
    fprintf(fp1,"\nf_evaluations = %d",f_evaluations);
    fprintf(fp1,"\n*************************************************************************\n");
    if(change_k!=NULL) free(change_k);
    return(1);
 }

/**
* \fn algebraic_method(int Nl, int *Np,int Na,double *a,double x0,double y0,
double **x,double **y,double **xx,double **yy,double factor_n,int zoom)
* \brief function to execute the algebraic method and save information to a file
* \author Luis Alvarez and Luis Gomez
* \return return 1 if the function finishes properly
*/
int  algebraic_method(
    int     Nl         /** Number of lines */,
    int     *Np        /** Number of points/line */,
    double  *a         /** Lens distortion model polynom */,
    double  x0         /** x center of the image (pixels) */,
    double  y0         /** y center of the image (pixels) */,
    double  **x        /** Coordinates of points (normalized) */,
    double  **y        /** Coordinates of points (normalized) */,
    double  **xx       /** Coordinates of points */,
    double  **yy       /** Coordinates of points */,
    double   factor_n  /** Factor to normalize coordinates */,
    int      zoom      /** Zoom strategy*/,
    FILE    *fp1       /** Pointer to the output file*/,
    double  *trivial   /** Trivial Emin,Vmin,Dmin values */
    )
 {

    int starttime, stoptime,i,m;
    double paso,Emin,Vmin;
    double timeused;
    double tmp,D;     /* auxiliar variable */
    int Na=2;       /* degree of the polynom */

    if(zoom==0){
        fprintf(fp1,"\n*************************************************************************\n");
        fprintf(fp1,"***********************ALGEBRAIC METHOD: WITHOUT ZOOM********************\n");
        fprintf(fp1,"*************************************************************************\n");
    }

   else{
        fprintf(fp1,"\n*************************************************************************\n");
        fprintf(fp1,"***********************ALGEBRAIC METHOD: WITH ZOOM***********************\n");
        fprintf(fp1,"*************************************************************************\n");
    }


    /* WE USE SEVERAL PARAMETER COMBINATIONS*/
    fprintf(fp1,"Na (degree of  lens distortion model polynom) = 2");
    fprintf(fp1,"\n1 parameter, 1 iteration, variables updated: 2");
    a[0]=1; for(i=1;i<=Na;i++) a[i]=0;
    starttime = clock();
    Emin=ami_lens_distortion_estimation(x,y,Nl,Np,(double) 0.,(double) 0.,a,Na,2,(double) 0.);
    if(zoom==1) ami_lens_distortion_zoom_normalization(x,y,Nl,Np,(double) 0.,(double) 0.,a,Na);
    stoptime = clock();
    /* WE UNDO THE NORMALIZATION TRANSFORM FOR DISTORTION PARAMETERS */
    paso=1.;
    for(i=0;i<=Na;i++){
        a[i]=a[i]*paso;
        paso/=factor_n;
    }
    Emin=0; for(m=0;m<Nl;m++) Emin+=ami_LensDistortionEnergyError(xx[m],yy[m],Np[m],x0,y0,a,Na);Emin=Emin/(double)Nl;
    Vmin=0; for(m=0;m<Nl;m++) Vmin+=ami_LensDistortionEnergyError_Vmin(xx[m],yy[m],Np[m],x0,y0,a,Na);Vmin=Vmin/(double)Nl;
    D=distance_function(a,xx,yy,Nl,Np,Na,x0,y0);
    /* CALCULATE THE IMPROVEMENTS RESPEC TO TO TRIVIAL SOLUTION */
    tmp=fabs((Emin-trivial[0])/trivial[0])*100.0;
    fprintf(fp1,"\n\nEmin =\t%1.15e with Na=%d. Improvement = \t%f\n",Emin,Na,tmp);
    tmp=fabs((Vmin-trivial[1])/trivial[1])*100.0;
    fprintf(fp1,"Vmin =\t%1.15e with Na=%d. Improvement = \t%f\n",Vmin,Na,tmp);
    tmp=fabs((D-trivial[2])/trivial[2])*100.0;
    fprintf(fp1,"D =\t%1.15e with Na=%d. Improvement = \t%f\n",D,Na,tmp);
    fprintf(fp1,"\nDistortion parameters:\n");
    for(i=0;i<=Na;i++) fprintf(fp1,"a[%d] = %1.15e\n",i,a[i]);
    timeused = difftime(stoptime,starttime);
    fprintf(fp1,"\nCPU_time=%f (seconds)",timeused/(double)CLOCKS_PER_SEC);
    fprintf(fp1,"\n------------------------------");

    Na=4;          /* degree of the polynom */
    fprintf(fp1,"\n\nNa (degree of  lens distortion model polynom)= 4");
    fprintf(fp1,"\n1 parameter, 2 iterations, variables updated: 2, 4");
    a[0]=1; for(i=1;i<=Na;i++) a[i]=0;      /* trivial solution */
    starttime = clock();
    Emin=ami_lens_distortion_estimation(x,y,Nl,Np,(double) 0.,(double) 0.,a,Na,2,(double) 0.);
    Emin=ami_lens_distortion_estimation(x,y,Nl,Np,(double) 0.,(double) 0.,a,Na,4,(double) 0.);
    if(zoom==1) ami_lens_distortion_zoom_normalization(x,y,Nl,Np,(double) 0.,(double) 0.,a,Na);
    stoptime = clock();
    /* WE UNDO THE NORMALIZATION TRANSFORM FOR DISTORTION PARAMETERS */
    paso=1.;
    for(i=0;i<=Na;i++){
        a[i]=a[i]*paso;
        paso/=factor_n;
    }
    Emin=0; for(m=0;m<Nl;m++) Emin+=ami_LensDistortionEnergyError(xx[m],yy[m],Np[m],x0,y0,a,Na);Emin=Emin/(double)Nl;
    Vmin=0; for(m=0;m<Nl;m++) Vmin+=ami_LensDistortionEnergyError_Vmin(xx[m],yy[m],Np[m],x0,y0,a,Na);Vmin=Vmin/(double)Nl;
    D=distance_function(a,xx,yy,Nl,Np,Na,x0,y0);
    tmp=fabs((Emin-trivial[0])/trivial[0])*100.0;
    fprintf(fp1,"\n\nEmin =\t%1.15e with Na=%d. Improvement = \t%f\n",Emin,Na,tmp);
    tmp=fabs((Vmin-trivial[1])/trivial[1])*100.0;
    fprintf(fp1,"Vmin =\t%1.15e with Na=%d. Improvement = \t%f\n",Vmin,Na,tmp);
    tmp=fabs((D-trivial[2])/trivial[2])*100.0;
    fprintf(fp1,"D =\t%1.15e with Na=%d. Improvement = \t%f\n",D,Na,tmp);
    fprintf(fp1,"\nDistortion parameters:\n");
    for(i=0;i<=Na;i++) fprintf(fp1,"a[%d] = %1.15e\n",i,a[i]);
    stoptime = clock();
    timeused = difftime(stoptime,starttime);
    fprintf(fp1,"\nCPU_time=%f (seconds)",timeused/(double)CLOCKS_PER_SEC);
    fprintf(fp1,"\n------------------------------");

    fprintf(fp1,"\n\nNa (degree of  lens distortion model polynom) = 4");
    fprintf(fp1,"\n2 parameters, 1 iteration, variables updated: 2, 4");
    a[0]=1; for(i=1;i<=Na;i++) a[i]=0;  /* trivial solution */
    starttime = clock();
    /* WE RUN A SAFE PREVIOUS ITERATION TO AVOID CONVERGENCE PROBLEMS*/
    Emin=ami_lens_distortion_estimation(x,y,Nl,Np,(double) 0.,(double) 0.,a,Na,2,(double) 0.);
    Emin=ami_lens_distortion_estimation(x,y,Nl,Np,(double) 0.,(double) 0.,a,Na,4,(double) 0.);
    /* WE RUN THE ALGEBRAIC METHOD FOR BOTH PARAMETERS IN ONE ITERATION */
    Emin=ami_lens_distortion_estimation_2v(x,y,Nl,Np,(double) 0.,(double) 0.,a,Na,2,4,(double) 0.);
    if(zoom==1) ami_lens_distortion_zoom_normalization(x,y,Nl,Np,(double) 0.,(double) 0.,a,Na);
    stoptime = clock();
    /* WE UNDO THE NORMALIZATION TRANSFORM FOR DISTORTION PARAMETERS */
    paso=1.;
    for(i=0;i<=Na;i++){
        a[i]=a[i]*paso;
        paso/=factor_n;
    }
    Emin=0; for(m=0;m<Nl;m++) Emin+=ami_LensDistortionEnergyError(xx[m],yy[m],Np[m],x0,y0,a,Na);Emin=Emin/(double)Nl;
    Vmin=0; for(m=0;m<Nl;m++) Vmin+=ami_LensDistortionEnergyError_Vmin(xx[m],yy[m],Np[m],x0,y0,a,Na);Vmin=Vmin/(double)Nl;
    D=distance_function(a,xx,yy,Nl,Np,Na,x0,y0);
    tmp=fabs((Emin-trivial[0])/trivial[0])*100.0;
    fprintf(fp1,"\n\nEmin =\t%1.15e with Na=%d. Improvement = \t%f\n",Emin,Na,tmp);
    tmp=fabs((Vmin-trivial[1])/trivial[1])*100.0;
    fprintf(fp1,"Vmin =\t%1.15e with Na=%d. Improvement = \t%f\n",Vmin,Na,tmp);
    tmp=fabs((D-trivial[2])/trivial[2])*100.0;
    fprintf(fp1,"D =\t%1.15e with Na=%d. Improvement = \t%f\n",D,Na,tmp);
    fprintf(fp1,"\n\nDistortion parameters:\n");
    for(i=0;i<=Na;i++) fprintf(fp1,"a[%d] = %1.15e\n",i,a[i]);
    timeused = difftime(stoptime,starttime);
    fprintf(fp1,"\nCPU_time=%f (seconds)",timeused/(double)CLOCKS_PER_SEC);
    fprintf(fp1,"\n*************************************************************************\n\n\n");
    return(1);
 }


/**
* \fn algebraic_method_pre_gradient(int Nl,int *Np,int Na,double *a,double x0,double y0,
double **x,double **y,double **xx,double **yy,double factor_n,int zoom,int *change_k)
* \brief function to calculate the solution for the gradient method applied from the
algebraic method solution
* \author Luis Alvarez and Luis Gomez
* \return return 1 if the function finishes properly
*/
int  algebraic_method_pre_gradient(
    int     Nl          /** Number of lines */,
    int     *Np         /** Number of points/line */,
    double  *a          /** Lens distortion model polynom */,
    double  x0          /** x center of the image (pixels) */,
    double  y0          /** y center of the image (pixels) */,
    double  **x         /** Coordinates of points (normalized) */,
    double  **y         /** Coordinates of points (normalized) */,
    double  **xx        /** Coordinates of points */,
    double  **yy        /** Coordinates of points  */,
    double  factor_n    /** Factor to normalize coordinates */,
    int     zoom        /** Zoom strategy */,
    FILE    *fp1       /** Pointer to the output file*/,
    double  *trivial   /** Trivial Emin,Vmin,Dmin values */,
    int     control    /** To start gradient at trivial solution or algebraic solution */
    )
 {

    int starttime, stoptime,i,m;
    double paso,Emin,Vmin,D,tmp;
    double timeused;

    int Na=2;                           /* DEGREE OF THE POLYNOM */

     if(zoom==0){
        fprintf(fp1,"\n*************************************************************************\n");
        fprintf(fp1,"************ALGEBRAIC METHOD + GRADIENT: WITHOUT ZOOM********************\n");
        fprintf(fp1,"*************************************************************************\n");
    }

   else{
        fprintf(fp1,"\n*************************************************************************\n");
        fprintf(fp1,"************ALGEBRAIC METHOD + GRADIENT: WITH ZOOM***********************\n");
        fprintf(fp1,"*************************************************************************\n");
    }


    /* NA = 2 CASE */
    /* WE USE SEVERAL PARAMETER COMBINATIONS*/
    fprintf(fp1,"Na (degree of  lens distortion model polynom) = 2");
    fprintf(fp1,"\n1 parameter, 1 iteration, variables updated: 2");
    a[0]=1; for(i=1;i<=Na;i++) a[i]=0;        /* TRIVIAL SOLUTION FOR THE ALGEBRAIC METHOD */
    starttime = clock();
    Emin=ami_lens_distortion_estimation(x,y,Nl,Np,(double) 0.,(double) 0.,a,Na,2,(double) 0.);
    if(zoom==1) ami_lens_distortion_zoom_normalization(x,y,Nl,Np,(double) 0.,(double) 0.,a,Na);
    stoptime = clock();
    /* WE UNDO THE NORMALIZATION TRANSFORM FOR DISTORTION PARAMETERS */
    paso=1.;
    for(i=0;i<=Na;i++){
        a[i]=a[i]*paso;
        paso/=factor_n;
    }
    Emin=0; for(m=0;m<Nl;m++) Emin+=ami_LensDistortionEnergyError(xx[m],yy[m],Np[m],x0,y0,a,Na);Emin=Emin/(double)Nl;
    Vmin=0; for(m=0;m<Nl;m++) Vmin+=ami_LensDistortionEnergyError_Vmin(xx[m],yy[m],Np[m],x0,y0,a,Na);Vmin=Vmin/(double)Nl;
    D=distance_function(a,xx,yy,Nl,Np,Na,x0,y0);
    tmp=fabs((Emin-trivial[0])/trivial[0])*100.0;
    fprintf(fp1,"\n\nEmin =\t%1.15e with Na=%d. Improvement = \t%f\n",Emin,Na,tmp);
    tmp=fabs((Vmin-trivial[1])/trivial[1])*100.0;
    fprintf(fp1,"Vmin =\t%1.15e with Na=%d. Improvement = \t%f\n",Vmin,Na,tmp);
    tmp=fabs((D-trivial[2])/trivial[2])*100.0;
    fprintf(fp1,"D =\t%1.15e with Na=%d. Improvement = \t%f\n",D,Na,tmp);
    fprintf(fp1,"\n\nDistortion parameters:\n");
    for(i=0;i<=Na;i++) fprintf(fp1,"a[%d] = %1.15e\n",i,a[i]);
    timeused = difftime(stoptime,starttime);
    fprintf(fp1,"\nCPU_time=%f (seconds)",timeused/(double)CLOCKS_PER_SEC);

    fprintf(fp1,"\n*\n");
    fprintf(fp1,"*******ALGEBRAIC METHOD FINISHED. START GRADIENT OPTIMIZATION************\n");
    fprintf(fp1,"*************************************************************************\n");
    /* Apply gradient from the solution */
    optimize(a,x,y,xx,yy,Nl,Np,x0,y0,factor_n,zoom,fp1,trivial,control);

    /* NA = 4 CASE */
    /* WE USE SEVERAL PARAMETER COMBINATIONS*/
    fprintf(fp1,"\n\nNa (degree of  lens distortion model polynom)= 4");
    fprintf(fp1,"\n1 parameter, 2 iterations, variables updated: 2, 4");
    a[0]=1; for(i=1;i<=Na;i++) a[i]=0;      /* trivial solution */
    starttime = clock();
    Emin=ami_lens_distortion_estimation(x,y,Nl,Np,(double) 0.,(double) 0.,a,Na,2,(double) 0.);
    Emin=ami_lens_distortion_estimation(x,y,Nl,Np,(double) 0.,(double) 0.,a,Na,4,(double) 0.);
    if(zoom==1) ami_lens_distortion_zoom_normalization(x,y,Nl,Np,(double) 0.,(double) 0.,a,Na);
    stoptime = clock();
    /* WE UNDO THE NORMALIZATION TRANSFORM FOR DISTORTION PARAMETERS */
    paso=1.;
    for(i=0;i<=Na;i++){
        a[i]=a[i]*paso;
        paso/=factor_n;
    }
    Emin=0; for(m=0;m<Nl;m++) Emin+=ami_LensDistortionEnergyError(xx[m],yy[m],Np[m],x0,y0,a,Na);Emin=Emin/(double)Nl;
    Vmin=0; for(m=0;m<Nl;m++) Vmin+=ami_LensDistortionEnergyError_Vmin(xx[m],yy[m],Np[m],x0,y0,a,Na);Vmin=Vmin/(double)Nl;
    D=distance_function(a,xx,yy,Nl,Np,Na,x0,y0);
    tmp=fabs((Emin-trivial[0])/trivial[0])*100.0;
    fprintf(fp1,"\n\nEmin =\t%1.15e with Na=%d. Improvement = \t%f\n",Emin,Na,tmp);
    tmp=fabs((Vmin-trivial[1])/trivial[1])*100.0;
    fprintf(fp1,"Vmin =\t%1.15e with Na=%d. Improvement = \t%f\n",Vmin,Na,tmp);
    tmp=fabs((D-trivial[2])/trivial[2])*100.0;
    fprintf(fp1,"D =\t%1.15e with Na=%d. Improvement = \t%f\n",D,Na,tmp);
    fprintf(fp1,"\n\nDistortion parameters:\n");
    for(i=0;i<=Na;i++) fprintf(fp1,"a[%d] = %1.15e\n",i,a[i]);
    timeused = difftime(stoptime,starttime);
    fprintf(fp1,"\nCPU_time=%f (seconds)",timeused/(double)CLOCKS_PER_SEC);
    fprintf(fp1,"\n*************************************************************************\n");
    fprintf(fp1,"\n*******ALGEBRAIC METHOD FINISHED. START GRADIENT OPTIMIZATION************\n");
    fprintf(fp1,"\n*************************************************************************\n");
    /* Apply gradient from the solution */
    optimize(a,x,y,xx,yy,Nl,Np,x0,y0,factor_n,zoom,fp1,trivial,control);

    fprintf(fp1,"\n------------------------------");
    /* NA = 4 CASE */
    /* WE USE SEVERAL PARAMETER COMBINATIONS*/
    fprintf(fp1,"\n\nNa (degree of  lens distortion model polynom) = 4");
    fprintf(fp1,"\n2 parameters, 1 iteration, variables updated: 2, 4");
    a[0]=1; for(i=1;i<=Na;i++) a[i]=0;  /* trivial solution */
    starttime = clock();
    /* WE RUN A SAFE PREVIOUS ITERATION TO AVOID CONVERGENCE PROBLEMS*/
    Emin=ami_lens_distortion_estimation(x,y,Nl,Np,(double) 0.,(double) 0.,a,Na,2,(double) 0.);
    Emin=ami_lens_distortion_estimation(x,y,Nl,Np,(double) 0.,(double) 0.,a,Na,4,(double) 0.);
    /* WE RUN THE ALGEBRAIC METHOD FOR BOTH PARAMETERS IN ONE ITERATION */
    Emin=ami_lens_distortion_estimation_2v(x,y,Nl,Np,(double) 0.,(double) 0.,a,Na,2,4,(double) 0.);
    if(zoom==1) ami_lens_distortion_zoom_normalization(x,y,Nl,Np,(double) 0.,(double) 0.,a,Na);
    stoptime = clock();
    /* WE UNDO THE NORMALIZATION TRANSFORM FOR DISTORTION PARAMETERS */
    paso=1.;
    for(i=0;i<=Na;i++){
        a[i]=a[i]*paso;
        paso/=factor_n;
    }
    Emin=0; for(m=0;m<Nl;m++) Emin+=ami_LensDistortionEnergyError(xx[m],yy[m],Np[m],x0,y0,a,Na);Emin=Emin/(double)Nl;
    Vmin=0; for(m=0;m<Nl;m++) Vmin+=ami_LensDistortionEnergyError_Vmin(xx[m],yy[m],Np[m],x0,y0,a,Na);Vmin=Vmin/(double)Nl;
    D=distance_function(a,xx,yy,Nl,Np,Na,x0,y0);
    tmp=fabs((Emin-trivial[0])/trivial[0])*100.0;
    fprintf(fp1,"\n\nEmin =\t%1.15e with Na=%d. Improvement = \t%f\n",Emin,Na,tmp);
    tmp=fabs((Vmin-trivial[1])/trivial[1])*100.0;
    fprintf(fp1,"Vmin =\t%1.15e with Na=%d. Improvement = \t%f\n",Vmin,Na,tmp);
    tmp=fabs((D-trivial[2])/trivial[2])*100.0;
    fprintf(fp1,"D =\t%1.15e with Na=%d. Improvement = \t%f\n",D,Na,tmp);
    fprintf(fp1,"\n\nDistortion parameters:\n");
    for(i=0;i<=Na;i++) fprintf(fp1,"a[%d] = %1.15e\n",i,a[i]);
    timeused = difftime(stoptime,starttime);
    fprintf(fp1,"\nCPU_time=%f (seconds)",timeused/(double)CLOCKS_PER_SEC);
    fprintf(fp1,"\n*************************************************************************\n");
    fprintf(fp1,"\n*******ALGEBRAIC METHOD FINISHED. START GRADIENT OPTIMIZATION************\n");
    fprintf(fp1,"\n*************************************************************************\n");
    /* Apply gradient from the solution */
    optimize(a,x,y,xx,yy,Nl,Np,x0,y0,factor_n,zoom,fp1,trivial,control);
    return(1);
 }

/**
* \fn trivial_solution(int Nl,int *Np,int Na,double *a,double x0,double y0,
double **x,double **y,double **xx,double **yy,double factor_n,FILE *fp1)
* \brief function to calculate the trivial solution
* \author Luis Alvarez and Luis Gomez
* \return return 1 if the function finishes properly
*/

int  trivial_solution(
    int     Nl         /** Number of lines */,
    int     *Np        /** Number of points/line */,
    int      Na        /** Degree of the lens distortion model */,
    double  *a         /** Lens distortion model polynom */,
    double  x0         /** x center of the image (pixels) */,
    double  y0         /** y center of the image (pixels) */,
    double  **xx       /** Coordinates of points */,
    double  **yy       /** Coordinates of points */,
    double  factor_n   /** Factor to normalize coordinates */,
    FILE    *fp1       /** Pointer to the output file */,
    double  *trivial   /** Trivial Emin,Vmin,Dmin values */
    )
 {

    int starttime, stoptime,i,m;
    double Emin,Vmin=factor_n,D;
    double timeused;

    Na=4;
    a[0]=1; for(i=1;i<=Na;i++) a[i]=0;

    fprintf(fp1,"*********************LENS DISTORTION MODEL: OUTPUT FILE*********************\n");
    fprintf(fp1,"\n****************************************************************************\n");
    fprintf(fp1,"******************CALCULATED USING NORMALIZED COORDINATES*******************");
    fprintf(fp1,"\n**************************SOLUTION IN PIXELS********************************");
    fprintf(fp1,"\n****************************************************************************\n\n\n");
    /* WE DO THE CALCULATION IN NORMALIZED COORDINATES AND TRANSFORM TO PIXELS AT THE END */
    /* SOLUTION AND DISTORTION PARAMETERS: IN PIXELS */
    /* //////////////////////////////////////////////////////////////////////////////////////////// */
    /* ******************************WE MEASURE CPU TIME FOR CALCULATIONS ONLY********************* */
    /* //////////////////////////////////////////////////////////////////////////////////////////// */
   /* WE COMPUTE THE DISTORSION MODEL */
   /* SOLUCION TRIVIAL (normalizada = pixeles) */
   /* WE ESTIMATE THE Emin,Vmin IN PIXELS */
    fprintf(fp1,"TRIVIAL SOLUTION");
    starttime = clock();
    Emin=0; for(m=0;m<Nl;m++) Emin+=ami_LensDistortionEnergyError(xx[m],yy[m],Np[m],x0,y0,a,Na);Emin=Emin/(double)Nl;
    Vmin=0; for(m=0;m<Nl;m++) Vmin+=ami_LensDistortionEnergyError_Vmin(xx[m],yy[m],Np[m],x0,y0,a,Na);Vmin=Vmin/(double)Nl;
    stoptime = clock();
    timeused = difftime(stoptime,starttime);
    fprintf(fp1,"\nEmin =\t%1.15e \twith Na=%d\n",Emin,Na);
    fprintf(fp1,"Vmin =\t%1.15e \twith Na=%d\n",Vmin,Na);
    D=distance_function(a,xx,yy,Nl,Np,Na,x0,y0);
    fprintf(fp1,"D =\t%1.15e \twith Na=%d\n",D,Na);
    fprintf(fp1,"\nDistortion parameters:\n");
    for(i=0;i<=Na;i++) fprintf(fp1,"a[%d] = %1.15e\n",i,a[i]);
    fprintf(fp1,"\nCPU_time=%f (seconds)",timeused/(double)CLOCKS_PER_SEC);
    fprintf(fp1," \n*************************************************************************\n\n\n");
    /* SAVE THE TRIVIAL VALUES TO CALCULATE THE % IMPROVEMENT */
    trivial[0]=Emin;trivial[1]=Vmin;trivial[2]=D;
    return(1);
 }


/**
* \fn void read_primitives_from_file(FILE *fp_1,int Nl,int *Np,double *x0,
double *y0,double **x,double **y,double **xx,double **yy)
* \brief function to read the image primitives input file
* \author Luis Alvarez and Luis Gomez
* \return return void
*/
void read_primitives_from_file(
    FILE *fp_1     /** Pointer to file */,
    int Nl         /** Number of lines */,
    int *Np        /** Number of points/line */,
    double *x0     /** x center of the image (pixels) */,
    double *y0     /** y center of the image (pixels) */,
    double **x     /** Coordinates of points (read pixels..later are nomalized) */,
    double **y     /** Coordinates of points (read pixels.. later are normalized) */,
    double **xx    /** Coordinates of points (pixels) */,
    double **yy    /** Coordinates of points (pixels) */
    )
 {
    float tmp1,tmp2;
    int i,j;
    char s[100];
    FILE *fp;
    if(xx==NULL || yy==NULL){;}
    rewind(fp_1);                       /* rewind the file */
    fscanf(fp_1,"%d\n",&Nl);            /* get number of lines */
    fgets(s,100,fp_1);                  /* jump comments (we do not use this data) */
    /* We read the data */
     for(i=0;i<Nl;i++){
        fgets(s,100,fp_1);              /* jump rect (we do not use this data) */
        fscanf(fp_1,"%d\n",&Np[i]);     /* get number of points for the "i" line */
        for(j=0;j<Np[i];j++){
             fscanf(fp_1,"%f %f\n",&tmp1,&tmp2);
             x[i][j]=(double)tmp1;y[i][j]=(double)tmp2;
        }
    }
    fgets(s,80,fp_1);                   /* jump comments */
    fscanf(fp_1,"%f %f",&tmp1,&tmp2);   /* get center of image */
    *x0=(double)tmp1;*y0=(double)tmp2;
    /* we do not use the distortion model to initialize (code is wanted) */
    fclose(fp_1);                       /* close the file */

    /* WE WRITE DATA TO A FILE TO CHECK THAT THEY HAVE BEEN READ PROPERLY */
    if(!(fp=fopen("data_test.txt","w"))){
            puts("Error while opening the data_test.txt file");
            exit(0);
    }
    fprintf(fp,"%d\n",Nl);                  /* write the number of total rects */
    for(i=0;i<Nl;i++){
        fprintf(fp,"rects\n");              /* write comments (we do not use this data) */
        fprintf(fp,"%d\n",Np[i]);           /* write number of points for the "i" line */
        for(j=0;j<Np[i];j++) fprintf(fp,"%f %f\n",x[i][j],y[i][j]);
    }
    fprintf(fp,"DISTORTION MODEL\n");       /* write comments (we do not use this data) */
    fprintf(fp,"%f %f",*x0,*y0);            /* print center of image */
    fclose(fp);                             /* close the file */
 }

/**
* \fn void undistort_image_3c(int Na,double  *a,unsigned char *r,unsigned char *g,unsigned char *b,
unsigned char *r_undistort,unsigned char *g_undistort,unsigned char *b_undistort,int width,int height)
* \brief function to correct a RGB image lens distortion
* \author Luis Alvarez
*/
void undistort_image_3c(
int Na /** Degree of the lens distortion model */,
double *a /** Lens distortion model polynom */,
unsigned char *r /** INPUT RED CHANNEL */,
unsigned char *g /** INPUT GREEN CHANNEL */,
unsigned char *b /** INPUT BLUE CHANNEL */,
unsigned char *r_undistort /** OUTPUT UNDISTORTED RED CHANNEL */,
unsigned char *g_undistort /** OUTPUT UNDISTORTED GREEN CHANNEL */,
unsigned char *b_undistort /** OUTPUT UNDISTORTED BLUE CHANNEL */,
int width /** INPUT IMAGE WIDTH */,
int height /** INPUT IMAGE HEIGHT */
)
{
  int i,j,i2,j2;
  long m,m2,size=width*height;
  double di,dj;
  double *r_v,*g_v,*b_v,*weight;
  double x0=width/2.;
  double y0=height/2.;
  double norm,norm2;
  double i2d,j2d,w;

  r_v=g_v=b_v=weight=NULL;

  /*  WE ALLOCATE MEMORY FOR TEMPORAL VECTORS */
  ami_calloc1d(r_v,double,size);
  ami_calloc1d(g_v,double,size);
  ami_calloc1d(b_v,double,size);
  ami_calloc1d(weight,double,size);

  /*  WE COMPUTE THE UNDISTORTED POINT AND THE CONTRIBUTION OF THE POINT TO THE OUTPUT IMAGE */
  for(i=0;i<height;i++){
    for(j=0;j<width;j++){
      m=i*width+j;
      /* WE COMPUTE THE DISTANCE TO THE IMAGE CENTER */
      norm=sqrt( (j-x0)*(j-x0)+(i-y0)*(i-y0) );
      norm2=ami_polynomial_evaluation(a,Na,norm);

      j2d=x0+(j-x0)*norm2;
      i2d=y0+(i-y0)*norm2;
      i2=(int) i2d;
      j2=(int) j2d;

      if( i2<0 || i2>=height || j2<0 || j2>=width ) continue;
      di=i2d-i2; dj=j2d-j2;
      m2=i2*width+j2;
      w=((1.-di)*(1.-dj));
      r_v[m2]+=(double)w*r[m];
      g_v[m2]+=(double)w*g[m];
      b_v[m2]+=(double)w*b[m];
      weight[m2]+=w;
      if( (di*(1.-dj))>0. && (i2+1)<height){
        m2=(i2+1)*width+j2;
        w=(di*(1.-dj));
        r_v[m2]+=(double)w*r[m];
        g_v[m2]+=(double)w*g[m];
        b_v[m2]+=(double)w*b[m];
        weight[m2]+=w;
      }
      if( ((1-di)*(dj))>0. && (j2+1)<width){
        m2=(i2)*width+j2+1;
        w=(1.-di)*(dj);
        r_v[m2]+=(double)w*r[m];
        g_v[m2]+=(double)w*g[m];
        b_v[m2]+=(double)w*b[m];
        weight[m2]+=w;
      }
      if( ((di)*(dj))>0. && (j2+1)<width && (i2+1)<height){
        m2=(i2+1)*width+j2+1;
        w=(di)*(dj);
        r_v[m2]+=(double)w*r[m];
        g_v[m2]+=(double)w*g[m];
        b_v[m2]+=(double)w*b[m];
        weight[m2]+=w;
      }
    }
  }

  /* printf("Filling the new image\n"); */
  /*  WE FILL NEW IMAGE VALUES */
  for(m=0;m<size;m++){
    if(weight[m]>0.){
      r_undistort[m]=(unsigned char) (r_v[m]/weight[m]);
      g_undistort[m]=(unsigned char) (g_v[m]/weight[m]);
      b_undistort[m]=(unsigned char) (b_v[m]/weight[m]);
    }
    else{
      r_undistort[m]=0;
      g_undistort[m]=0;
      b_undistort[m]=0;
    }
  }

  /* WE FILL HOLES */
  for(i=0;i<height;i++){
    for(j=0;j<width;j++){
      m=i*width+j;
      m2=-1;
      if(weight[m]==0.){
          if(j<(width-1) && weight[m+1]!=0) m2=m+1;
          else if(j>0 && weight[m-1]!=0) m2=m-1;
          else if( i<(height-1) &&  weight[m+width]!=0) m2=m+width;
          else if( i>0 && weight[m-width]!=0) m2=m-width;
          if(m2!=-1){
            r_undistort[m]=r_undistort[m2];
            g_undistort[m]=g_undistort[m2];
            b_undistort[m]=b_undistort[m2];
        }
      }
    }
  }


  /* WE FREE THE MEMORY */
  if(r_v!=NULL) free(r_v);
  if(g_v!=NULL) free(g_v);
  if(b_v!=NULL) free(b_v);
  if(weight!=NULL) free(weight);
}

/**
* \fn void undistort_image_1c(int Na,double  *a,unsigned char *grey,
  ,unsigned char *grey_undistort,int width,int height)
* \brief function to correct an 1 channel image lens distortion
* \author Luis Alvarez
*/
void undistort_image_1c(
int Na /** Degree of the lens distortion model */,
double *a /** Lens distortion model polynom */,
unsigned char *g /** INPUT GREY LEVEL CHANNEL */,
unsigned char *g_undistort /** OUTPUT UNDISTORTED GREY CHANNEL */,
int width /** INPUT IMAGE WIDTH */,
int height /** INPUT IMAGE HEIGHT */
)
{
  int i,j,i2,j2;
  long m,m2,size=width*height;
  double di,dj;
  double *g_v,*weight;
  double x0=width/2.;
  double y0=height/2.;
  double norm,norm2;
  double i2d,j2d,w;

  g_v=weight=NULL;

  /*  WE ALLOCATE MEMORY FOR TEMPORAL VECTORS */
  ami_calloc1d(g_v,double,size);
  ami_calloc1d(weight,double,size);

  /*  WE COMPUTE THE UNDISTORTED POINT AND THE CONTRIBUTION OF THE POINT TO THE OUTPUT IMAGE */
  for(i=0;i<height;i++){
    for(j=0;j<width;j++){
      m=i*width+j;
      /* WE COMPUTE THE DISTANCE TO THE IMAGE CENTER */
      norm=sqrt( (j-x0)*(j-x0)+(i-y0)*(i-y0) );
      norm2=ami_polynomial_evaluation(a,Na,norm);

      j2d=x0+(j-x0)*norm2;
      i2d=y0+(i-y0)*norm2;
      i2=(int) i2d;
      j2=(int) j2d;

      if( i2<0 || i2>=height || j2<0 || j2>=width ) continue;
      di=i2d-i2; dj=j2d-j2;
      m2=i2*width+j2;
      w=((1.-di)*(1.-dj));
      g_v[m2]+=(double)w*g[m];
      weight[m2]+=w;
      if( (di*(1.-dj))>0. && (i2+1)<height){
        m2=(i2+1)*width+j2;
        w=(di*(1.-dj));
        g_v[m2]+=(double)w*g[m];
        weight[m2]+=w;
      }
      if( ((1-di)*(dj))>0. && (j2+1)<width){
        m2=(i2)*width+j2+1;
        w=(1.-di)*(dj);
        g_v[m2]+=(double)w*g[m];
        weight[m2]+=w;
      }
      if( ((di)*(dj))>0. && (j2+1)<width && (i2+1)<height){
        m2=(i2+1)*width+j2+1;
        w=(di)*(dj);
        g_v[m2]+=(double)w*g[m];
        weight[m2]+=w;
      }
    }
  }

  /* printf("Filling the new image\n"); */
  /*  WE FILL NEW IMAGE VALUES */
  for(m=0;m<size;m++){
    if(weight[m]>0.){
      g_undistort[m]=(unsigned char) (g_v[m]/weight[m]);
    }
    else{
      g_undistort[m]=0;
    }
  }

   /* WE FILL HOLES */
  for(i=0;i<height;i++){
    for(j=0;j<width;j++){
      m=i*width+j;
      m2=-1;
      if(weight[m]==0.){
          if(j<(width-1) && weight[m+1]!=0) m2=m+1;
          else if(j>0 && weight[m-1]!=0) m2=m-1;
          else if( i<(height-1) &&  weight[m+width]!=0) m2=m+width;
          else if( i>0 && weight[m-width]!=0) m2=m-width;
          if(m2!=-1){
            g_undistort[m]=g_undistort[m2];
        }
      }
    }
  }


  /* WE FREE THE MEMORY */
  if(g_v!=NULL) free(g_v);
  if(weight!=NULL) free(weight);
}

/**
 * \fn int read_line_primitives(char filename[300],int *Nl,int **Np,double ***x,double ***y)
 * \brief function to read point line primitives
 * \author Luis Alvarez
*/
int read_line_primitives(
const char filename[300] /** INPUT FILE NAME */,
int *Nl /** OUTPUT NUMBER OF LINES */,
int **Np /** OUTPUT NUMBER OF POINTS FOR EACH LINE */,
double ***x /** OUTPUT POINT X COORDINATES  */,
double ***y /** OUTPUT POINT X COORDINATES  */ )
{
  FILE *fp_1;
  int i,j,k;
  float tmp1,tmp2;
  if(!(fp_1=fopen(filename,"r"))){
    printf("Error while opening the file %s\n",filename);
    return(1);
  }

  fscanf(fp_1,"%d\n",Nl); /*get number of lines*/
  if((*Nl)<=0){
    printf("Error: The number of lines is 0 %s\n",filename);
    return(2);
  }
  *Np=(int*)malloc( sizeof(int)*(*Nl));
  *x=(double**)malloc( sizeof(double*)*(*Nl) );   /* x pixels coordinates */
  *y=(double**)malloc( sizeof(double*)*(*Nl) );   /* y pixels coordinates */

  printf("Nl=%d\n",*Nl);
  for(i=0;i<(*Nl);i++){
    fscanf(fp_1,"%d",&k);               /*get number of points for the "i" line  */
    (*Np)[i]=k;
    printf("Np[%d]=%d\n",i,(*Np)[i]);
    (*x)[i]=(double*)malloc( sizeof(double)*k );
    (*y)[i]=(double*)malloc( sizeof(double)*k);
    for(j=0;j<k;j++){
       fscanf(fp_1,"%f %f\n",&tmp1,&tmp2);
       (*x)[i][j]=tmp1;
       (*y)[i][j]=tmp2;
       printf("x[%d][%d]=%f x[%d][%d]=%f\n",i,j,(*x)[i][j],i,j,(*y)[i][j]);
    }
  }
  fclose(fp_1);
  return(0);
}
