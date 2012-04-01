
#include "stdafx.h"
#include <iostream>
#include "camera_calibrator.h"

int main(int argc, char *argv[])
{
	CamCalibrator cc;
	cc.getPtsFromSegmentedImage();	// Stage 1: get image coordinates of calibration points
	cc.matchPtsToCalibrationPts();	// Stage 2: match image calibration points to measured points
	cc.calibrate();					// Stage 3: do the camera calibration
	return 0;
}
