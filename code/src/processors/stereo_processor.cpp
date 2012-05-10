#include "stereo_processor.h"
#include <QDebug>

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
			qDebug() << "OUTPUT = LEFT DEPTH MAP\n";
			output_image = Util::combine(correctedLeftDepthMap,correctedRightDepthMap);
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

	// ALL THESE SHOULD BE PASSED FROM THE GUI.
	int max_expected_disparity_bounds = 30; // For efficiency.
	int hard_multiplier = -1; // For middlebury tests etc where the distribution is known.
	double smoothness_weight = 0.0; // Weight of the previous scanline. Should be [0 to 0.5].
	// If set to 0, no smoothing. If set to 0.5, this scanline and the previous are weighted equally.
	// 0.875 was recommended for the former form.
	// CURRENTLY BROKEN, DON'T USE

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

	// Set up main dynamic programming matrix and previous path matrix.
	if (smoothness_weight > 0.0){
		A = Mat(numColsLeft, numColsLeft, CV_32F, Scalar(1000000)); // A uses 32-bit signed floats
		prev_path = Mat(numColsLeft, numColsLeft, CV_32F, Scalar(1000000)); // 32-bit signed floats
	} else {
		A = Mat(numColsLeft, numColsLeft, CV_32S, Scalar(1000000)); // A uses 32-bit signed integers
	}

	// We progress one row of the rectified images at a time, starting with the topmost.
	for (int y_scanline = 0; y_scanline < numRowsLeft; y_scanline++){

		std::cout << "L" << y_scanline << "\n";

		// A[0,0] is initialised to 0. All other elements are evaluated from upper left to lower right corner.
		for (int i = 0; i < numColsLeft; i++){ // i counts cols
			for (int j = 0; j < numColsLeft; j++){ // j counts cols also
				// All this assumes images are of type '0', i.e. CV_8U

				// Improvement: everything is initialised to a cost of a million;
				// only change this if within q pixels of the centre line.
				if (abs(i-j) > max_expected_disparity_bounds){
					// "max_expected_disparity_bounds" is the number of pixels from the centre to search
				} else {
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

					/*
					// 'minimum' is weighted by previous paths (scanlines)
					if (i != 0 && smoothness_weight > 0.0){
						//std::cout << "M " << minimum << "\n";
						//std::cout << "P " << prev_path.at<int>(i, j) << "\n";
						prev_path.at<int>(i, j) = (int)(prev_path.at<int>(i, j) * smoothness_weight); // Weighting
						minimum = minimum - prev_path.at<int>(i, j); // Reusing paths
						// WHY DOES THAT HAPPEN AFTERWARD? IT'S NEVER READ!
					}
					*/

					unsigned char valueLeft = left_image.at<unsigned char>(y_scanline, i);
					unsigned char valueRight = right_image.at<unsigned char>(y_scanline, j);
					if ((i == 0) && (j == 0)){
						A.at<int>(i, j) = 0;
					} else {
						unsigned char difference;
						if (valueLeft >= valueRight) {
							difference = valueLeft-valueRight;
						} else {
							difference = valueRight-valueLeft;
						}
						// Denoise goes here
						// Smoothing goes here
						A.at<int>(i,j) = minimum + difference;
						//std::cout << A.at<int>(i,j) << " ";
					}
				}
			}
		}
		// A has now been calculated.

		if (y_scanline > 0 && smoothness_weight > 0.0){
			prev_path = prev_path*smoothness_weight;
			A = A*(1.0-smoothness_weight);
			A = A+prev_path;
		}

		// Once the matrix has been filled, a path of minimal cost can be calculated by
		// tracing back from the lower right corner A[n-1,n-1] to upper left corner A[0,0].
		int ii = numColsLeft - 1;
		int jj = numColsLeft - 1;
		while ((ii > 0) || (jj > 0)){
			//qDebug() << "i " << ii << " j " << jj << "\n";
			//if (jj - ii != 0) qDebug() << "!";
			initial_leftDepthMap.at<int>(y_scanline, ii) = (jj - ii);
			//qDebug() << "ii=" << ii << ", jj=" << jj << "\n";
			initial_rightDepthMap.at<int>(y_scanline, jj) = (ii - jj);
			int up = 1000000;
			int left = 1000000;
			int up_left = 1000000;
			if (ii > 0) up = A.at<int>(ii - 1, jj);
			if (jj > 0) left = A.at<int>(ii, jj - 1);
			if ((ii > 0) && (jj > 0)) up_left = A.at<int>(ii - 1, jj - 1);
			int minimum = min(min(up, left), up_left);
			// Weight pathdirection goes here
			// 4.6
			if (minimum == up_left){ //std::cout << "upleft\n";
				ii--;
				jj--;
			} else if (minimum == left){ //std::cout << "left\n";
				jj--;
			} else { //std::cout << "up\n";
				ii--;
			}
		}
		if (smoothness_weight > 0.0)   prev_path = A.clone(); // We only use prev_path if we are smoothing.
		emit progress(y_scanline / numRowsLeft);
	}
	qDebug() << "STEREO MATCHING COMPLETE.\n";

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
	qDebug() << "HIGHEST DISPARITIES = " << highestDisparity1 << ", " << highestDisparity2;

	// Need to normalise output image to [0...255]
	// For display purposes, we saturate the depth map to have only positive values.
	float multiplier = 255.0/((float)(max(highestDisparity1, highestDisparity2)));
	if (hard_multiplier > 0){
		multiplier = 4; // Puts in a hard multiplier, e.g. for middlebury tests where it is known
	}
	for (int i = 0; i < numRowsLeft; i++){
		for (int j = 0; j < numColsLeft; j++){
			correctedLeftDepthMap.at<unsigned char>(i, j) =
				(unsigned char) min(((int)(initial_leftDepthMap.at<int>(i, j) * multiplier)),255);
			correctedRightDepthMap.at<unsigned char>(i, j) =
				(unsigned char) min(((int)(initial_rightDepthMap.at<int>(i, j) * multiplier)),255);
		}
	}

	// Outputs disparity maps to files
	cv::imwrite("Left-Disparity-Map.png", correctedLeftDepthMap);
	cv::imwrite("Right-Disparity-Map.png", correctedRightDepthMap);

	return true;


	// The path found in the coarse step becomes a stencil for the fine step

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

