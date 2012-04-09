/**
*    Please note: Except for lines CLEARLY MARKED with the phrase "Added by B Meadows, 2012",
*	 ALL code herein is authored by Luis Alvarez and Luis Gomez, AMI Research Group,
*    University of Las Palmas de Gran Canaria, Canary Islands, SPAIN (February 2010)
*	~ comment by Ben Meadows
*/
/* ami_pol.c */

#ifndef _AMI_POL_H_
#define _AMI_POL_H_

#include <malloc.h>
#include <math.h>

long double ami_horner(long double *pol,int degree,long double x,long double *fp);
long double ami_root_bisection(long double *pol,int degree,long double a,long double b,long double TOL);
int ami_polynomial_root(double *pol, int degree, double *root_r, double *root_i);

#endif
