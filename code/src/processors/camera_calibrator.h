
#ifndef CALIBRATION_EXPERIMENTS_H
#define CALIBRATION_EXPERIMENTS_H

#include <stdio.h>
#include <list>
#include <cv.h>
#include "calibration_object.h"
#include <QDebug>

using namespace cv;

struct point_correspondence {
	Point2f imagePt;
	Point2f imagePt_adj;
	Point3f worldPt;
};

// Class containing the algorithm that actually does the calibration work
class CamCalibrator
{

public:
	CamCalibrator(int argc, char *argv[]);
  CamCalibrator(std::list<Point> points2d, std::list<Point3d> points3d, int width, int height, std::vector<point_correspondence> corr);
	~CamCalibrator();
	void lineSort(Point2d line1, Point2d line2, Point2d * leftOrRightArray, int arrayLength);
	static double pointLineDistance(Point2d p, Point2d lineEnd1, Point2d lineEnd2);
	void mapPtsToCalibrationPts();
	void calibrate();
	void checkResults();
  std::vector<point_correspondence> getMapping() {return mapping;}
	Mat computeLeastSquaresForKappa(double kappa_one, double kappa_two);
	double findStandardDeviation(double * entries, int count);
	double imageLengthX;
	double imageLengthY;

  Mat getRotationMatrix() {return mat_R;}
  Mat getTranslationMatrix() {return mat_T;}
  double getFocalLength() {return focalLength;}

protected:
	// X/Y/Z values of actual calibration points in world coordinate system
	VirtualCalibrationObject* obj;
        std::vector<point_correspondence> mapping;
	Mat mat_M;
	Mat mat_X;
	Mat mat_L;
	Mat mat_R;
	Mat mat_T;
	Mat mat_M2;
	Mat mat_U;
	Point2d* calPtsInImg; //x,y for each calibration point (63 of them)
	Point3d* calPtsInWorld;
	double sx;
	double focalLength;
	double kappa1;
	double kappa2;
  QDebug cout;
};

#endif // CALIBRATION_EXPERIMENTS_H
