
#ifndef CALIBRATION_EXPERIMENTS_H
#define CALIBRATION_EXPERIMENTS_H

#include <cv.h>
#include "calibration_object.h"

using namespace cv;

// Class containing the algorithm that actually does the calibration work
class CamCalibrator
{

public:
	CamCalibrator(int argc, char *argv[]);
	~CamCalibrator();
	void lineSort(Point2d line1, Point2d line2, Point2d * leftOrRightArray, int arrayLength);
	static double pointLineDistance(Point2d p, Point2d lineEnd1, Point2d lineEnd2);
	void getPtsFromSegmentedImage();
	void matchPtsToCalibrationPts();
	void calibrate();
	void checkResults();
	Mat computeLeastSquaresForKappa(double kappa);
	double imageLengthX;
	double imageLengthY;

private:
	// X/Y/Z values of actual calibration points in world coordinate system
	CalibrationObject* obj;
	Mat mat_M;
	Mat mat_X;
	Mat mat_L;
	Mat mat_R;
	Mat mat_T;
	Mat mat_M2;
	Mat mat_U;
	Point2d* calPtsInImg; //x,y for each calibration point (63 of them)
	double sx;
	double focalLength;
	double kappa1;
	double kappa2;

};

#endif // CALIBRATION_EXPERIMENTS_H
