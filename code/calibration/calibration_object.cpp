#include "stdafx.h"
#include "calibration_object.h"
#include <cv.h>

using namespace cv;

CalibrationObject::CalibrationObject(){
	// X/Y/Z values of calibration points in world coordinate system (as measured from calibration object)
	// These are ok to stay hard-coded
	lMeasurements = new Point3d[35];
	lMeasurements[0] = Point3d(0,7.5,10.5); // Top row left
	lMeasurements[1] = Point3d(0,6,10.5);
	lMeasurements[2] = Point3d(0,4.5,10.5);
	lMeasurements[3] = Point3d(0,3,10.5);
	lMeasurements[4] = Point3d(0,1.5,10.5); // Top row right
	lMeasurements[5] = Point3d(0,7.5,9);
	lMeasurements[6] = Point3d(0,6,9);
	lMeasurements[7] = Point3d(0,4.5,9);
	lMeasurements[8] = Point3d(0,3,9);
	lMeasurements[9] = Point3d(0,1.5,9);
	lMeasurements[10] = Point3d(0,7.5,7.5);
	lMeasurements[11] = Point3d(0,6,7.5);
	lMeasurements[12] = Point3d(0,4.5,7.5);
	lMeasurements[13] = Point3d(0,3,7.5);
	lMeasurements[14] = Point3d(0,1.5,7.5);
	lMeasurements[15] = Point3d(0,7.5,6);
	lMeasurements[16] = Point3d(0,6,6);
	lMeasurements[17] = Point3d(0,4.5,6);
	lMeasurements[18] = Point3d(0,3,6);
	lMeasurements[19] = Point3d(0,1.5,6);
	lMeasurements[20] = Point3d(0,7.5,4.5);
	lMeasurements[21] = Point3d(0,6,4.5);
	lMeasurements[22] = Point3d(0,4.5,4.5);
	lMeasurements[23] = Point3d(0,3,4.5);
	lMeasurements[24] = Point3d(0,1.5,4.5);
	lMeasurements[25] = Point3d(0,7.5,3);
	lMeasurements[26] = Point3d(0,6,3);
	lMeasurements[27] = Point3d(0,4.5,3);
	lMeasurements[28] = Point3d(0,3,3);
	lMeasurements[29] = Point3d(0,1.5,3);
	lMeasurements[30] = Point3d(0,7.5,1.5); // Bottom row left
	lMeasurements[31] = Point3d(0,6,1.5);
	lMeasurements[32] = Point3d(0,4.5,1.5);
	lMeasurements[33] = Point3d(0,3,1.5);
	lMeasurements[34] = Point3d(0,1.5,1.5); // Bottom row right

	rMeasurements = new Point3d[28];
	rMeasurements[0] = Point3d(1.58,0,10.5); // Top row left
	rMeasurements[1] = Point3d(3.08,0,10.52);
	rMeasurements[2] = Point3d(4.58,0,10.53);
	rMeasurements[3] = Point3d(6.08,0,10.55); // Top row right
	rMeasurements[4] = Point3d(1.56,0,9);
	rMeasurements[5] = Point3d(3.06,0,9.02);
	rMeasurements[6] = Point3d(4.56,0,9.03);
	rMeasurements[7] = Point3d(6.06,0,9.05);
	rMeasurements[8] = Point3d(1.55,0,7.5);
	rMeasurements[9] = Point3d(3.05,0,7.52);
	rMeasurements[10] = Point3d(4.55,0,7.53);
	rMeasurements[11] = Point3d(6.05,0,7.55);
	rMeasurements[12] = Point3d(1.54,0,6);
	rMeasurements[13] = Point3d(3.04,0,6.02);
	rMeasurements[14] = Point3d(4.54,0,6.03);
	rMeasurements[15] = Point3d(6.04,0,6.05);
	rMeasurements[16] = Point3d(1.53,0,4.5);
	rMeasurements[17] = Point3d(3.03,0,4.52);
	rMeasurements[18] = Point3d(4.53,0,4.53);
	rMeasurements[19] = Point3d(6.03,0,4.55);
	rMeasurements[20] = Point3d(1.51,0,3);
	rMeasurements[21] = Point3d(3.01,0,3.02);
	rMeasurements[22] = Point3d(4.51,0,3.03);
	rMeasurements[23] = Point3d(6.01,0,3.05);
	rMeasurements[24] = Point3d(1.5,0,1.5); // Bottom row left
	rMeasurements[25] = Point3d(3,0,1.52);
	rMeasurements[26] = Point3d(4.5,0,1.53);
	rMeasurements[27] = Point3d(6,0,1.55); // Bottom row right

	lAssocImagePts_RAW = new Point2d[35];
	rAssocImagePts_RAW = new Point2d[28];
	for (int i = 0; i < 35; i++) { lAssocImagePts_RAW[i] = Point2d(0,0); }
	for (int i = 0; i < 28; i++) { rAssocImagePts_RAW[i] = Point2d(0,0); }

	lAssocImagePts_ADJ = new Point2d[35];
	rAssocImagePts_ADJ = new Point2d[28];
	for (int i = 0; i < 35; i++) { lAssocImagePts_ADJ[i] = Point2d(0,0); }
	for (int i = 0; i < 28; i++) { rAssocImagePts_ADJ[i] = Point2d(0,0); }
}

CalibrationObject::~CalibrationObject(){
}

Point3d CalibrationObject::getLeftPt(int row, int column){
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
Point3d CalibrationObject::getRightPt(int row, int column){
	return rMeasurements[(7-row)*4 + (column-1)];
}

Point2d CalibrationObject::getLeftAssocImagePt_RAW(int row, int column){
	return lAssocImagePts_RAW[(7-row)*5 + (5-column)];
}

Point2d CalibrationObject::getRightAssocImagePt_RAW(int row, int column){
	return rAssocImagePts_RAW[(7-row)*4 + (column-1)];
}

void CalibrationObject::setLeftAssocImagePt_RAW(int row, int column, Point2d pt){
	lAssocImagePts_RAW[(7-row)*5 + (5-column)].x = pt.x;
	lAssocImagePts_RAW[(7-row)*5 + (5-column)].y = pt.y;
}

void CalibrationObject::setRightAssocImagePt_RAW(int row, int column, Point2d pt){
	rAssocImagePts_RAW[(7-row)*4 + (column-1)].x = pt.x;
	rAssocImagePts_RAW[(7-row)*4 + (column-1)].y = pt.y;
}


Point2d CalibrationObject::getLeftAssocImagePt_ADJ(int row, int column){
	return lAssocImagePts_ADJ[(7-row)*5 + (5-column)];
}

Point2d CalibrationObject::getRightAssocImagePt_ADJ(int row, int column){
	return rAssocImagePts_ADJ[(7-row)*4 + (column-1)];
}

void CalibrationObject::setLeftAssocImagePt_ADJ(int row, int column, Point2d pt){
	lAssocImagePts_ADJ[(7-row)*5 + (5-column)].x = pt.x;
	lAssocImagePts_ADJ[(7-row)*5 + (5-column)].y = pt.y;
}

void CalibrationObject::setRightAssocImagePt_ADJ(int row, int column, Point2d pt){
	rAssocImagePts_ADJ[(7-row)*4 + (column-1)].x = pt.x;
	rAssocImagePts_ADJ[(7-row)*4 + (column-1)].y = pt.y;
}
