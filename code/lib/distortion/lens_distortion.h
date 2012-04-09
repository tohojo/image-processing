/**
*    Please note: Except for lines CLEARLY MARKED with the phrase "Added by B Meadows, 2012",
*	 ALL code herein is authored by Luis Alvarez and Luis Gomez, AMI Research Group,
*    University of Las Palmas de Gran Canaria, Canary Islands, SPAIN (February 2010)
*	~ comment by Ben Meadows
*/

/* lens_distortion.c */

#ifndef _LENS_DISTORTION_H_
#define _LENS_DISTORTION_H_

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>
#include <string.h>
#include <time.h>
#include "ami_pol.h"

#define PI 3.1415927
#define ABS(x) (((x)>0)?(x):-(x))
#define Normalizar(x) ((x)<0?0:((x)>255?255:x))
#define Max(a,b) ((a>b)?a:b)
#define ami_abs(x) ((x)>0?(x):(-(x)))
#define ami_calloc2d(direccion,tipo,height,width) {int ml,mk; \
          direccion=(tipo **) malloc(sizeof(tipo *)*(height)); \
          direccion[0]=(tipo *)malloc(sizeof(tipo)*(width)*(height)); \
          for(ml=0;ml<(height);ml++) direccion[ml]=&(direccion[0][ml*(width)]); \
          for(ml=0;ml<height;ml++) for(mk=0;mk<width;mk++) direccion[ml][mk]=0; \
        }
#define ami_malloc2d(direccion,tipo,height,width) {int ml; \
          direccion=(tipo **) malloc(sizeof(tipo *)*(height)); \
          direccion[0]=(tipo *)malloc(sizeof(tipo)*(width)*(height)); \
          for(ml=0;ml<(height);ml++) direccion[ml]=&(direccion[0][ml*(width)]);\
        }
#define ami_malloc2d_punteros(direccion,puntero_simple,tipo,height,width) {int ml; \
          direccion=(tipo **) malloc(sizeof(tipo *)*(height)); \
          direccion[0]=(tipo *) puntero_simple; \
          for(ml=0;ml<(height);ml++) direccion[ml]=&(direccion[0][ml*(width)]);\
        }
#define ami_free2d(direccion) { free(direccion[0]); free(direccion); }
#define ami_free2d_punteros(direccion) { free(direccion); }
#define ami_malloc1d(direccion,tipo,size) {direccion=(tipo *) malloc(sizeof(tipo)*(size));}
#define ami_calloc1d(direccion,tipo,size) {int ml; direccion=(tipo *) malloc(sizeof(tipo)*(size)); \
          for(ml=0;ml<size;ml++) direccion[ml]=0;\
        }

/**
*            BEGIN: ALGEBRAIC CONTROL PARAMETERS
*/

#define ami_max_iter 1000
#define ami_tol 0.0000001
/**
*           END: ALGEBRAIC CONTROL PARAMETERS
*/



#define longitud_linea 80 	/* LENGTH OF LINE (TO READ A LINE FOR A FILE) */


/**
*           BEGIN: GRADIENT CONTROL PARAMETERS
*/
#define max_itera 50            /* MAXIMUM NUMBER OF GRADIENT ITERATIONS */
#define delta	1e-6		    /* DERIVATIVE STEP (FINITE DIFFERENCES) */
#define max_itera_lambda 10      /* MAXIMUM NUMBER OF ITERATIONS IN UNIDIMENSIONAL SEARCH */
#define tol_f 1.0e-4	        /* TOLERANCE TO STOP THE GRADIENT ITERATIONS */
#define tol_norma 1.0e-16   	/* NORM OF GRADIENT TO STOP THE GRADIENT ALGORITHM */




int ami_calculo_recta2d(double recta[3], double **Puntos2D, int N);
void ami_dibujar_segmento_unsigned_char(unsigned char *data, int width, int height, int x0, int y0, int x1, int y1, unsigned char color);
int ami_lens_distortion_polynomial_update_distance_2v(double *x, double *y, int Np, double *a, int Na, double x0, double y0, int k1, int k2, double **pol, double alfa);
double ami_lens_distortion_estimation_2v(double **x, double **y, int Nl, int *Np, double x0, double y0, double *a, int Na, int k1, int k2, double alfa);
int ami_lens_distortion_model_update_2v(double *a, int Na, int k1, int k2, double **pol);
int ami_lens_distortion_polynomial_update_2v(double *x, double *y, int Np, double *a, int Na, double x0, double y0, int k1, int k2, double **pol);
void ami_2v_polynom_derivatives(double **p, int N, double **p_x, double **p_y);
double ami_determinante(double **A, int N);
void ami_polynom_determinant(double p[6][6][19], int Np, int Nd, double *q);
double ami_2v_polynom_evaluation(double **p1, int N1, double x, double y);
void ami_2v_polynom_to_1v_polynom(double **p1, int N1, double *p3, double z, int flat);
double *ami_1v_polynom_multiplication(double *p1, int N1, double *p2, int N2, double *p3);
void ami_2v_polynom_multiplication(double **p1, int N1, double **p2, int N2, double **p3);
int ami_RootCubicPolynomial(double *a, int N, double *x);
double ami_polynomial_evaluation(double *a, int Na, double x);
int ami_lens_distortion_polynomial_update(double *x, double *y, int Np, double *a, int Na, double x0, double y0, int k, double *pol);
int ami_lens_distortion_model_update(double *a, int Na, int k, double *pol);
double ami_LensDistortionEnergyError(double *x, double *y, int Np, double x0, double y0, double *a, int Na);
double ami_LensDistortionEnergyError_Vmin(double *x, double *y, int Np, double x0, double y0, double *a, int Na);
int ami_inverse_lens_distortion(double x, double y, double x0, double y0, double *xt, double *yt, double *a, int Na);
double ami_lens_distortion_estimation(double **x, double **y, int Nl, int *Np, double x0, double y0, double *a, int Na, int k, double alfa);
void ami_lens_distortion_zoom_normalization(double **x, double **y, int Nl, int *Np, double x0, double y0, double *a, int Na);
int calculate_points(double *amin, double **points_2D_modified, int N, int Na, double x0, double y0);
double distance_function(double *amin, double **x, double **y, int Nl, int *Np, int Na, double x0, double y0);
double find_lambda(double lambda1, double lambda2, double lambda3, double f_1, double f_2, double f_3, double *amin_copy, double *amin, double **x, double **y, int Nl, int *Np, int Na, double *grad_f, int *change_k, double x0, double y0);
double minimize_cuadratic_polynom(double lambda1, double lambda2, double lambda3, double f_1, double f_2, double f_3, double *amin_copy, double *amin, double **x, double **y, int Nl, int *Np, int Na, double *grad_f, int *change_k, double x0, double y0);
double cuadratic_fitting(double *amin_copy, double *amin, double **x, double **y, int Nl, int Na, int *Np, double lambda1, double lambda2, double lambda3, double f_1, double f_2, double f_3, double *grad_f, int *change_k, double x0, double y0);
double minimize_lambda(double *amin, double **x, double **y, int Na, int Nl, int *Np, double *grad_f, double f, int *change_k, double x0, double y0);
double gradient_method(double *amin, double **x, double **y, int Nl, int *Np, int Na, int *change_k, double x0, double y0, int zoom);
int optimize(double *amin, double **x, double **y, double **xx, double **yy, int Nl, int *Np, double x0, double y0, double factor_n, int zoom, FILE *fp1, double *trivial, int control);
int algebraic_method(int Nl, int *Np, double *a, double x0, double y0, double **x, double **y, double **xx, double **yy, double factor_n, int zoom, FILE *fp1, double *trivial);
int algebraic_method_pre_gradient(int Nl, int *Np, double *a, double x0, double y0, double **x, double **y, double **xx, double **yy, double factor_n, int zoom, FILE *fp1, double *trivial, int control);
int trivial_solution(int Nl, int *Np, int Na, double *a, double x0, double y0, double **xx, double **yy, double factor_n, FILE *fp1, double *trivial);
void read_primitives_from_file(FILE *fp_1, int Nl, int *Np, double *x0, double *y0, double **x, double **y, double **xx, double **yy);
void undistort_image_3c(int Na, double *a, unsigned char *r, unsigned char *g, unsigned char *b, unsigned char *r_undistort, unsigned char *g_undistort, unsigned char *b_undistort, int width, int height);
void undistort_image_1c(int Na, double *a, unsigned char *g, unsigned char *g_undistort, int width, int height);
int read_line_primitives(const char filename[300], int *Nl, int **Np, double ***x, double ***y);

#endif
