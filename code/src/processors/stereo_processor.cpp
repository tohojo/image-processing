#include "stereo_processor.h"
#include <stdio.h>

StereoProcessor::StereoProcessor(QObject *parent)
: TwoImageProcessor(parent)
{
}

StereoProcessor::~StereoProcessor()
{
}

void StereoProcessor::run()
{
	forever {
		if(abort) return;
		//
		if( (! input_image.empty()) && (! right_image.empty()) ){
			dynamicProgramming();
			mutex.lock();
			std::cout << "OUTPUT = LEFT DEPTH MAP\n";
			output_image = correctedLeftDepthMap;
		} else {
			mutex.lock();
			output_image = right_image;
		}
		emit progress(100);
		emit updated();
		if(once) return;

		if(!restart)
			condition.wait(&mutex);
		restart = false;
		mutex.unlock();
	}
}

/*
For every pixel in the right image, we extract the 7-by-7-pixel block around it
and search along the same row in the left image for the block that best matches it.
Here we search in a range of +/- 15 pixels (the disparity range) around the
pixel's location in the first image, and we use the sum of absolute differences (SAD)
to compare the image regions.
*/

// Need to normalise output image to [0...255]
// For display purposes, we saturate the depth map to have only positive values.

void StereoProcessor::dynamicProgramming(){
	Mat left_image = input_image;
	// numRowsLeft = number of rows in left image
	int numRowsLeft = left_image.rows;
	// numColsLeft = number of cols in left image
	int numColsLeft = left_image.cols;

	initial_rightDepthMap = Mat(numRowsLeft, numColsLeft, CV_32S, Scalar(0)); // 32-bit signed integers
	initial_leftDepthMap = Mat(numRowsLeft, numColsLeft, CV_32S, Scalar(0)); // 32-bit signed integers
	correctedLeftDepthMap = Mat(numRowsLeft, numColsLeft, left_image.type(), Scalar(0)); // 8-bit unsigned chars
	correctedRightDepthMap = Mat(numRowsLeft, numColsLeft, left_image.type(), Scalar(0)); // 8-bit unsigned chars

	// We progress one row of the rectified images at a time, starting with the topmost.
	for (int y_scanline = 0; y_scanline < numRowsLeft; y_scanline++){

		A = Mat(numColsLeft, numColsLeft, CV_32S, Scalar(0)); // A uses 32-bit signed integers

		std::cout << "NEXT LINE\n";

		for (int i = 0; i < numColsLeft; i++){ // i counts cols
			for (int j = 0; j < numColsLeft; j++){ // j counts cols also
				// Images appear to be of type '0', i.e. CV_8U
				int up_left = 1000000;
				int up = 1000000;
				int left = 1000000;
				if (i > 0) {
					up = A.at<int>(i-1, j);
				}
				if (j > 0) {
					left = A.at<int>(i, j-1);
				}
				if ((i > 0) && (j > 0)) {
					up_left = A.at<int>(i-1, j-1);
				}
				int minimum = min(min(up, left), up_left);
				// THE FOLLOWING TWO HAVE BEEN CORRECTED FROM (i,y) AND (j,y)
				unsigned char valueLeft = left_image.at<unsigned char>(y_scanline, i);
				// !!! CHANGED THE ABOVE FROM "right_image" ???
				unsigned char valueRight = right_image.at<unsigned char>(y_scanline, j);
				if ((i == 0) && (j == 0)){
					A.at<int>(i, j) = 0;
				} else if (valueLeft >= valueRight) {
					A.at<int>(i, j) = minimum + (valueLeft-valueRight);
				}  else if (valueRight >= valueLeft) {
					A.at<int>(i,j) = minimum + (valueRight-valueLeft);
				}
			}
		}

		/*
		for (int i = 0; i < numColsLeft; i++){
		for (int j = 0; j < numColsLeft; j++){
		std::cout << " " << (unsigned int) A.at<unsigned char>(i, j);
		}
		std::cout << "\n";
		}
		*/

		// beginning from A[n-1,n-1], and ending in A[0,0]...
		int ii = numColsLeft - 1;
		int jj = numColsLeft - 1;
		while ((ii > 0) || (jj > 0)){
			//std::cout << "i " << ii << " j " << jj << "\n";
			//if (jj - ii != 0) std::cout << "!";
			initial_leftDepthMap.at<int>(y_scanline, ii) = (jj - ii); // CORRECTED
			std::cout << "ii=" << ii << ", jj=" << jj << "\n";
			initial_rightDepthMap.at<int>(y_scanline, jj) = (ii - jj); // CORRECTED
			int up = 1000000;
			int left = 1000000;
			int up_left = 1000000;
			if (ii > 0) up = A.at<int>(ii - 1, jj);
			if (jj > 0) left = A.at<int>(ii, jj - 1);
			if ((ii > 0) && (jj > 0)) up_left = A.at<int>(ii - 1, jj - 1);
			int minimum = min(min(up, left), up_left);
			if (minimum == up){
				ii--;
				//std::cout << "up\n";
			} else if (minimum == left){
				jj--;
				//std::cout << "left\n";
			} else {
				//std::cout << "upleft\n";
				ii--;
				jj--;
			}
			if (jj-ii != 0){
				std::cout << "dif= " << jj-ii << "\n";
			}
		}
		std::cout << "SCANLINE Y = " << y_scanline << "\n";
	}
	std::cout << "STEREO MATCHING COMPLETE.\n";

	for (int i = 0; i < numRowsLeft; i++){
		for (int j = 0; j < numColsLeft; j++){
			initial_leftDepthMap.at<int>(i, j) = abs(initial_leftDepthMap.at<int>(i, j));
		}
	}

	int highestDisparity = 0;
	for (int i = 0; i < numRowsLeft; i++){
		for (int j = 0; j < numColsLeft; j++){
			if (initial_leftDepthMap.at<int>(i, j) > highestDisparity){
				highestDisparity = initial_leftDepthMap.at<int>(i, j);
			}
		}
	}
	std::cout << "\n=======================\n";
	std::cout << "HIGHEST DISPARITY = " << highestDisparity << "\n";
	std::cout << "=======================\n";

	int multiplier = 255/highestDisparity;
	for (int i = 0; i < numRowsLeft; i++){
		for (int j = 0; j < numColsLeft; j++){
			correctedLeftDepthMap.at<unsigned char>(i, j) =
				(unsigned char)(initial_leftDepthMap.at<int>(i, j) * multiplier);
		}
	}


	//		At the total beginning, A[0,0] has to be initialized with 0. 
	//		Afterwards, all others elements are evaluated in the
	//		order from the upper left to the lower right corner.
	//		It is absolutely necessary just to include already initialized
	//		matrix elements for performing the Min function inside the pseudocode above.
	//		Once the matrix has been filled, a path of minimal cost can be calculated
	//		by tracing back through the DP matrix...


	/*
	Step 1: CALCULATE MATRIX
	a1 = Right[i,y]
	a2 = Left[j,y]
	a3 = diff(a1,a2)
	a4 = a3*scale + weight // denoise
	a5 = f(a4, smooth[i,j])
	b1 = A[i-1,j]
	b2 = A[i,j-1]
	b3 = A[i-1,j-1]
	b4 = minimum(b1, b2, b3)
	b5 = b4 - path[i,j] // Reusing paths
	path[i,j] = path[i,j] * 0.875 // Reusing paths
	A[i,j] = a5 + b5 // Write DP matrix

	Step 2: FIND PATH
	c1 = A[i-1,j]
	c2 = A[i,j-1]
	c3 = A[i-1,j-1]
	c4 = minimum(c1, c2, c3)
	c1 = c1 + weight1
	c2 = c2 + weight2
	c3 = c3 + weight3
	// Choose new position
	if (c4 <= c1) { j = j-1; i = i-1; } else
	if (c4 <= c2) { j = j-1 } else
	if (c4 <= c3) { i = i-1 }
	// Remember paths
	path[i,j] = constant
	// Write disparity map
	disparity[i,y] = j
	*/
}

