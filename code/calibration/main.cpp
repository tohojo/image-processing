
#include "stdafx.h"
#include <iostream>
#include "camera_calibrator.h"

// USAGE: "./CamCalibrator.exe <image_points_input_file.txt>"
// Defaults to using "test.txt", which contains points from ImageJ on img160027
int main(int argc, char *argv[])
{
	CamCalibrator cc(argc, argv);
	cc.getPtsFromSegmentedImage();	// Stage 1: get image coordinates of calibration points
	cc.matchPtsToCalibrationPts();	// Stage 2: match image calibration points to measured points
	cc.calibrate();					// Stage 3: do the camera calibration
	return 0;
}
