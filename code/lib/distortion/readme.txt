Algebraic Lens Distortion Model Estimation

Authors : Luis Alvarez, Luis Gomez, Rafael Sendra


INFORMATION ABOUT PROGRAM COMPILATION AND EXECUTION 

Step 1 : Download from IPOL paper page the file "lens distortion estimation.zip". The content of this zip archive is :

	a) lens_distortion.h, lens_distortion.c  : lens distortion model estimation library.
	b) ami_pol.h, ami_pol.c : polynomial real root estimation library.
	c) ami_tif.h, ami_tif.c : basic library to read/write usigned char uncompressed TIF image format.
	d) lens_distortion_estimation.c : main program with lens distortion estimation function calls.
	e) readme.txt : this file.
	f) calibration_pattern.tif : sample input image to test the program.
	g) calibration_pattern_line_primitives.dat : sample line primitives to test the program. 

Step 2 : To compile, once the zip archive files are saved in a directory we execute the "make" command : 

>make 

Step 3 : To execute the sample test included we do :

>./lens_distortion_estimation calibration_pattern.tif output.tif calibration_pattern_line_primitives.dat output_results.dat


Function parameter explanation : 

(1) calibration_pattern.tif is the input image that have to be in uncompressed TIF unsigned char (RGB or single channel) format.

(2) output.tif is the output lens distortion corrected image that is provided in TIF format

(3) calibration_pattern_line_primitives.dat is the input collection of points which belong to distorted lines presented in the image. 
This file is an ASCII file organized in the following way : First, in the file, we write the number of lines, next, for each line we 
write the number of points and the point coordinates. 

(4) output_results.dat is the output file where the coefficients of the estimated distortion models are stored. 



----------------------------------------------------------------------------------------------------------------

Understanding the output_results.dat file:

This output file contains the complete solution for all the cases considered in the publication "An Algebraic Approach to Lens Distortion by Line Rectification", L. Alvarez, L. Gomez, R. Sendra, published in JMIV, July 2009.  From all the solutions (all valid), the one selected in the IPOL online demo as "solution" (best solution) is the solution obtained by first applying the algebraic method from the trivial solution and then improving it by means of the simple gradient method. Only even distortion coefficients are used (k2, k4).

The whole set of solutions is presented as follows (see aclaration below):

----------------------------------------------------------------------------------------------------------------
/* FIRST WE COMPUTE THE TRIVIAL SOLUTION FOR THE POLYNOMIAL GRADE Na=4 (note that this case includes the Na=2) */
 
Na=4; x0=(1,0,0,0,0)

----------------------------------------------------------------------------------------------------------------
SOLUTION 1: /* ALGEBRAIC METHOD FROM TRIVIAL SOLUTION. NO ZOOM APPLIED 


Na (degree of  lens distortion model polynom) = 	2
1 parameter, 1 iteration, variables updated: 		k[2]


Na (degree of  lens distortion model polynom)= 		4
1 parameter, 2 iterations, variables updated: 		k[2], k[4]


Na (degree of  lens distortion model polynom) = 	4
2 parameters, 1 iteration, variables updated: 		k[2], k[4]


---------------------------------------------------------------------------------------------------------------- 
SOLUTION 2: /* ALGEBRAIC METHOD FROM TRIVIAL SOLUTION. ZOOM APPLIED 


Na (degree of  lens distortion model polynom) = 	2
1 parameter, 1 iteration, variables updated: 		k[2]


Na (degree of  lens distortion model polynom)= 		4
1 parameter, 2 iterations, variables updated: 		k[2], k[4]


Na (degree of  lens distortion model polynom) = 	4
2 parameters, 1 iteration, variables updated: 		k[2], k[4]

---------------------------------------------------------------------------------------------------------------- 
SOLUTION 3: GRADIENT METHOD FROM TRIVIAL SOLUTION. NO ZOOM APPLIED 
	
Na (degree of  lens distortion model polynom) = 	2
MODIFIED VARIABLES THROUGH OPTIMIZATION:		k[2]


Na (degree of  lens distortion model polynom) = 	4
MODIFIED VARIABLES THROUGH OPTIMIZATION:		k[2], k[4]

---------------------------------------------------------------------------------------------------------------- 
SOLUTION 4: GRADIENT METHOD FROM TRIVIAL SOLUTION. ZOOM APPLIED 

Na (degree of  lens distortion model polynom) = 	2
MODIFIED VARIABLES THROUGH OPTIMIZATION:		k[2]


Na (degree of  lens distortion model polynom) = 	4
MODIFIED VARIABLES THROUGH OPTIMIZATION:		k[2], k[4]

---------------------------------------------------------------------------------------------------------------- 

SOLUTION 5: ALGEBRAIC METHOD (FROM TRIVIAL SOLUTION) + GRADIENT METHOD. NO ZOOM APPLIED 

Na (degree of  lens distortion model polynom) = 	2
1 parameter, 1 iteration, variables updated: 		k[2]
MODIFIED VARIABLES THROUGH GRADIENT OPTIMIZATION: 	k[2]


Na (degree of  lens distortion model polynom) = 	4
1 parameter, 2 iterations, variables updated: 		k[2], k[4]
MODIFIED VARIABLES THROUGH GRADIENT OPTIMIZATION: 	k[2], k[4]	


Na (degree of  lens distortion model polynom) = 	4
2 parameters, 1 iteration, variables updated: 		k[2], k[4]
MODIFIED VARIABLES THROUGH GRADIENT OPTIMIZATION: 	k[2], k[4]

---------------------------------------------------------------------------------------------------------------- 

SOLUTION 6: ALGEBRAIC METHOD (FROM TRIVIAL SOLUTION) + GRADIENT METHOD. ZOOM APPLIED 

Na (degree of  lens distortion model polynom) = 	2
1 parameter, 1 iteration, variables updated: 		k[2]
MODIFIED VARIABLES THROUGH GRADIENT OPTIMIZATION: 	k[2]


Na (degree of  lens distortion model polynom) = 	4
1 parameter, 2 iterations, variables updated:		k[2], k[4]
MODIFIED VARIABLES THROUGH GRADIENT OPTIMIZATION: 	k[2], k[4]	


Na (degree of  lens distortion model polynom) = 	4
2 parameters, 1 iteration, variables updated: 		k[2], k[4]
MODIFIED VARIABLES THROUGH GRADIENT OPTIMIZATION: 	k[2], k[4]


---------------------------------------------------------------------------------------------------------------- 

REMARK 1º:

Na (degree of  lens distortion model polynom) = 	2
1 parameter, 1 iteration, variables updated: 		k[2]

	This means that 1 iteration of the algebraic method is applied and only one variable is optimized (k[2])





Na (degree of  lens distortion model polynom)= 		4
1 parameter, 2 iterations, variables updated: 		k[2], k[4]


	This means that, the algebraic method is applied as:
		first: 	one iteration (updating the k[2] variable only)
                second: one iteration updating k[2],k[4] (being the k[2] initialized at the previous solution)




Na (degree of  lens distortion model polynom) = 	4
2 parameters, 1 iteration, variables updated: 		k[2], k[4]


	This means that, 1 iteration od the algebraic method is applied for the two variables (k[2], k[4]) set free to evolve simultaneously.



REMARK 2º:
For the solution 5 and 6, the algebraic solution is exactly the same obtained for the cases solution 1 and 2, but, they were again calculated in order to  make the code clearer (avoiding saving previous solutions).
---------------------------------------------------------------------------------------------------------------- 


