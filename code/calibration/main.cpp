
#include "stdafx.h"
#include <iostream>
#include "camera_calibrator.h"

// USAGE: "./CamCalibrator.exe <image_points_input_file.txt>"
// Defaults to using "test.txt", which contains points from ImageJ on img160027
// The input file must begin with the length (x) and height (y) of the image in pixels.
// After this, the input file should contain x y pairs separated by whitespace.
int main(int argc, char *argv[])
{
	CamCalibrator cc(argc, argv);
	cc.getPtsFromSegmentedImage();	// Stage 1: get image coordinates of calibration points
	cc.matchPtsToCalibrationPts();	// Stage 2: match image calibration points to measured points
	cc.calibrate();					// Stage 3: do the camera calibration
	cc.checkResults();				// Stage 4: back-project rays using the calculated information
	return 0;
}
