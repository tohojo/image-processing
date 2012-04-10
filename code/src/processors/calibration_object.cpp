#include <stdio.h>
#include "calibration_object.h"
#include <cv.h>

using namespace cv;

VirtualCalibrationObject::VirtualCalibrationObject(){
	// X/Y/Z values of calibration points in world coordinate system (as measured from calibration object)
	// These are ok to stay hard-coded
	lMeasurements = new Point3d[35];
	rMeasurements = new Point3d[28];

	lAssocImagePts_RAW = new Point2d[35];
	rAssocImagePts_RAW = new Point2d[28];
	for (int i = 0; i < 35; i++) { lAssocImagePts_RAW[i] = Point2d(0,0); }
	for (int i = 0; i < 28; i++) { rAssocImagePts_RAW[i] = Point2d(0,0); }
}

VirtualCalibrationObject::~VirtualCalibrationObject(){
}

Point3d VirtualCalibrationObject::getLeftPt(int row, int column){
	return lMeasurements[(7-row)*5 + (5-column)];
}
/*
Left face:									Right face:
75=[00]	74=[01]	73=[02]	72=[03]	71=[04]		71=[00]	72=[01]	73=[02]	74=[03]
65=[05]	64=[06]	63=[07]	62=[08]	61=[09]		61	62	63	64
55	54	53	52	51							51	52	53	54
45	44	43	42	41							41	42	43	44
35	34	33	32	31							31	32	33	34
25	24	23	22	21							21	22	23	24
15	14	13	12	11=[34]						11	12	13	14=[27]
etc
*/
Point3d VirtualCalibrationObject::getRightPt(int row, int column){
	return rMeasurements[(7-row)*4 + (column-1)];
}

Point2d VirtualCalibrationObject::getLeftAssocImagePt_RAW(int row, int column){
	return lAssocImagePts_RAW[(7-row)*5 + (5-column)];
}

Point2d VirtualCalibrationObject::getRightAssocImagePt_RAW(int row, int column){
	return rAssocImagePts_RAW[(7-row)*4 + (column-1)];
}

void VirtualCalibrationObject::setLeftAssocImagePt_RAW(int row, int column, Point2d pt){
	lAssocImagePts_RAW[(7-row)*5 + (5-column)].x = pt.x;
	lAssocImagePts_RAW[(7-row)*5 + (5-column)].y = pt.y;
}

void VirtualCalibrationObject::setRightAssocImagePt_RAW(int row, int column, Point2d pt){
	rAssocImagePts_RAW[(7-row)*4 + (column-1)].x = pt.x;
	rAssocImagePts_RAW[(7-row)*4 + (column-1)].y = pt.y;
}

