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
		if( dynamicProgramming() ) { // Returns true if successful
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


bool StereoProcessor::dynamicProgramming(){

	Mat left_image = input_image;

	if( (left_image.empty()) || (right_image.empty()) ){
		return false;
	}

	if ((left_image.rows != right_image.rows) || (left_image.cols != right_image.cols)){
		return false;
	}

	int numRowsLeft = left_image.rows;// numRowsLeft = number of rows in left image
	int numColsLeft = left_image.cols; // numColsLeft = number of cols in left image
	initial_rightDepthMap = Mat(numRowsLeft, numColsLeft, CV_32S, Scalar(0)); // 32-bit signed integers
	initial_leftDepthMap = Mat(numRowsLeft, numColsLeft, CV_32S, Scalar(0)); // 32-bit signed integers
	correctedLeftDepthMap = Mat(numRowsLeft, numColsLeft, left_image.type(), Scalar(0)); // 8-bit unsigned chars
	correctedRightDepthMap = Mat(numRowsLeft, numColsLeft, left_image.type(), Scalar(0)); // 8-bit unsigned chars

	// We progress one row of the rectified images at a time, starting with the topmost.
	for (int y_scanline = 0; y_scanline < numRowsLeft; y_scanline++){

		// Dynamic programming matrix.
		A = Mat(numColsLeft, numColsLeft, CV_32S, Scalar(0)); // A uses 32-bit signed integers
		std::cout << "" << y_scanline << "\n";

		// A[0,0] is initialised to 0. All other elements are evaluated from upper left to lower right corner.
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

		// Once the matrix has been filled, a path of minimal cost can be calculated by
		// tracing back from the lower right corner A[n-1,n-1] to upper left corner A[0,0].
		int ii = numColsLeft - 1;
		int jj = numColsLeft - 1;
		while ((ii > 0) || (jj > 0)){
			//std::cout << "i " << ii << " j " << jj << "\n";
			//if (jj - ii != 0) std::cout << "!";
			initial_leftDepthMap.at<int>(y_scanline, ii) = (jj - ii); // CORRECTED
			//std::cout << "ii=" << ii << ", jj=" << jj << "\n";
			initial_rightDepthMap.at<int>(y_scanline, jj) = (ii - jj); // CORRECTED
			int up = 1000000;
			int left = 1000000;
			int up_left = 1000000;
			if (ii > 0) up = A.at<int>(ii - 1, jj);
			if (jj > 0) left = A.at<int>(ii, jj - 1);
			if ((ii > 0) && (jj > 0)) up_left = A.at<int>(ii - 1, jj - 1);
			int minimum = min(min(up, left), up_left);
			if (minimum == up){ //std::cout << "up\n";
				ii--;
			} else if (minimum == left){ //std::cout << "left\n";
				jj--;
			} else { //std::cout << "upleft\n";
				ii--;
				jj--;
			}
		}
		emit progress(y_scanline / numRowsLeft);
	}
	std::cout << "STEREO MATCHING COMPLETE.\n";

	for (int i = 0; i < numRowsLeft; i++){
		for (int j = 0; j < numColsLeft; j++){
			initial_leftDepthMap.at<int>(i, j) = abs(initial_leftDepthMap.at<int>(i, j));
		}
	}

	int highestDisparity1 = 0;
	int highestDisparity2 = 0;
	for (int i = 0; i < numRowsLeft; i++){
		for (int j = 0; j < numColsLeft; j++){
			if (initial_leftDepthMap.at<int>(i, j) > highestDisparity1){
				highestDisparity1 = initial_leftDepthMap.at<int>(i, j);
			}
			if (initial_rightDepthMap.at<int>(i, j) > highestDisparity2){
				highestDisparity2 = initial_rightDepthMap.at<int>(i, j);
			}
		}
	}
	std::cout << "\n=======================\n";
	std::cout << "HIGHEST DISPARITIES = " << highestDisparity1 << ", " << highestDisparity2 << "\n";
	std::cout << "=======================\n";

	// Need to normalise output image to [0...255]
	// For display purposes, we saturate the depth map to have only positive values.
	int multiplier = 255/max(highestDisparity1, highestDisparity2);
	for (int i = 0; i < numRowsLeft; i++){
		for (int j = 0; j < numColsLeft; j++){
			correctedLeftDepthMap.at<unsigned char>(i, j) =
				(unsigned char)(initial_leftDepthMap.at<int>(i, j) * multiplier);
			correctedRightDepthMap.at<unsigned char>(i, j) =
				(unsigned char)(initial_rightDepthMap.at<int>(i, j) * multiplier);
		}
	}

	// Outputs disparity maps to files
	cv::imwrite("Left-Disparity-Map.png", correctedLeftDepthMap);
	cv::imwrite("Right-Disparity-Map.png", correctedRightDepthMap);

	return true;

	// PSEUDOCODE:
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

