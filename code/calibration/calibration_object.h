#ifndef CALIBRATION_OBJECT_H
#define CALIBRATION_OBJECT_H

#include <cv.h>

using namespace cv;

// Class storing a representation of the calibration object, with 35 Left face points and 28 Right face points
class CalibrationObject
{

public:
	CalibrationObject();
	~CalibrationObject();
	Point3d getLeftPt(int row, int column);
	Point3d getRightPt(int row, int column);
	Point2d getLeftAssocImagePt(int row, int column);
	Point2d getRightAssocImagePt(int row, int column);
	void setLeftAssocImagePt(int row, int column, Point2d pt);
	void setRightAssocImagePt(int row, int column, Point2d pt);

	// X/Y/Z values of actual calibration points in world coordinate system
	// It wasn't actually very useful storing these separately; change if I have time
	Point3d *lMeasurements;
	Point3d *rMeasurements;

	// x/y values of calibration points in image
	// It wasn't actually very useful storing these separately; change if I have time
	Point2d *lAssocImagePts;
	Point2d *rAssocImagePts;

};

#endif // CALIBRATION_OBJECT_H
