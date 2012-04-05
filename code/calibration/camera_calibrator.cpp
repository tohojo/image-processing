#include "stdafx.h"
#include "camera_calibrator.h"
#include <fstream>

using namespace cv;
using namespace std;

// The following are just used to make sorting easier; it's more convenient than overriding the Point2d class
Point2d lineP1 = Point2d(0,0);
Point2d lineP2 = Point2d(0,0);
bool compareX(Point2d a, Point2d b){ return a.x < b.x; }
bool compareY(Point2d a, Point2d b){ return a.y < b.y; }
bool lineComp(Point2d a, Point2d b){
	return (CamCalibrator::pointLineDistance(a, lineP1, lineP2) < CamCalibrator::pointLineDistance(b, lineP1, lineP2));
}

// Basic constructor
CamCalibrator::CamCalibrator(int argc, char *argv[])
{
	calPtsInImg = new Point2d[63];
	obj = new CalibrationObject();
	mat_R = Mat(3, 3, CV_64F, Scalar::all(0));
	mat_T = Mat(3, 3, CV_64F, Scalar::all(0));

	// Currently just hard-coding image calibration points from output of ImageJ on img160027
	/*
	calPtsInImg[0] = Point2d(598.38, 302.94);
	calPtsInImg[1] = Point2d(721.03, 316.74);
	calPtsInImg[2] = Point2d(856.95, 332.55);
	calPtsInImg[3] = Point2d(1008.08, 350.29);
	calPtsInImg[4] = Point2d(1176.5, 370.1);
	calPtsInImg[5] = Point2d(1514.44, 398.21);
	calPtsInImg[6] = Point2d(1640.16, 403.68);
	calPtsInImg[7] = Point2d(1749.48, 408.01);
	calPtsInImg[8] = Point2d(1846.16, 411.91);
	calPtsInImg[9] = Point2d(595.23, 501.08);
	calPtsInImg[10] = Point2d(716.43, 524.99);
	calPtsInImg[11] = Point2d(849.63, 551.53);
	calPtsInImg[12] = Point2d(996.8, 580.97);
	calPtsInImg[13] = Point2d(1159.88, 613.29);
	calPtsInImg[14] = Point2d(1817.84, 608.63);
	calPtsInImg[15] = Point2d(1721.3, 617.02);
	calPtsInImg[16] = Point2d(1612.63, 626.62);
	calPtsInImg[17] = Point2d(1488.37, 637.31);
	calPtsInImg[18] = Point2d(593.9, 691.36);
	calPtsInImg[19] = Point2d(712.77, 723.72);
	calPtsInImg[20] = Point2d(843.14, 759.55);
	calPtsInImg[21] = Point2d(986.33, 798.69);
	calPtsInImg[22] = Point2d(1790.3, 796.23);
	calPtsInImg[23] = Point2d(1694.28, 815.56);
	calPtsInImg[24] = Point2d(1144.44, 841.77);
	calPtsInImg[25] = Point2d(1586.63, 837.22);
	calPtsInImg[26] = Point2d(1464.16, 861.76);
	calPtsInImg[27] = Point2d(592.35, 873.47);
	calPtsInImg[28] = Point2d(708.82, 913.29);
	calPtsInImg[29] = Point2d(836.48, 956.93);
	calPtsInImg[30] = Point2d(1764.55, 975.01);
	calPtsInImg[31] = Point2d(976.06, 1005.05);
	calPtsInImg[32] = Point2d(1669.08, 1004.13);
	calPtsInImg[33] = Point2d(1562.4, 1037.3);
	calPtsInImg[34] = Point2d(591.26, 1048.41);
	calPtsInImg[35] = Point2d(1129.77, 1057.98);
	calPtsInImg[36] = Point2d(1441.33, 1074.49);
	calPtsInImg[37] = Point2d(704.23, 1095.62);
	calPtsInImg[38] = Point2d(828.71, 1146.67);
	calPtsInImg[39] = Point2d(1740.55, 1146.7);
	calPtsInImg[40] = Point2d(1645.55, 1185.3);
	calPtsInImg[41] = Point2d(965.71, 1202.44);
	calPtsInImg[42] = Point2d(589.94, 1217.74);
	calPtsInImg[43] = Point2d(1539.6, 1228.45);
	calPtsInImg[44] = Point2d(1115.71, 1264.42);
	calPtsInImg[45] = Point2d(701.24, 1270.79);
	calPtsInImg[46] = Point2d(1420.13, 1277.54);
	calPtsInImg[47] = Point2d(1717.62, 1313.37);
	calPtsInImg[48] = Point2d(822.8, 1328.91);
	calPtsInImg[49] = Point2d(1623.25, 1360.05);
	calPtsInImg[50] = Point2d(588.16, 1381.19);
	calPtsInImg[51] = Point2d(955.88, 1392.83);
	calPtsInImg[52] = Point2d(1518.24, 1413.02);
	calPtsInImg[53] = Point2d(697.12, 1440.67);
	calPtsInImg[54] = Point2d(1101.72, 1463.46);
	calPtsInImg[55] = Point2d(1400.01, 1473.38);
	calPtsInImg[56] = Point2d(1695.24, 1474.26);
	calPtsInImg[57] = Point2d(816.15, 1505.63);
	calPtsInImg[58] = Point2d(1601.81, 1529.89);
	calPtsInImg[59] = Point2d(946.13, 1576.9);
	calPtsInImg[60] = Point2d(1497.71, 1591.72);
	calPtsInImg[61] = Point2d(1088.25, 1655.7);
	calPtsInImg[62] = Point2d(1380.72, 1662.24);

	// Currently just hard-coding image size from img160027
	imageLengthX = 2592;
	imageLengthY = 1944;

*/

	char *in_arg;
	if (argc > 1) {
		in_arg = argv[1];
	} else {
		in_arg = "test.txt";
	}
	ifstream inFile(in_arg, ios::in);
	inFile >> imageLengthX;
	inFile >> imageLengthY;

	double in_x, in_y;
	for (int i = 0; i < 63; i++){
		inFile >> in_x;
		inFile >> in_y;
		calPtsInImg[i] = Point2d(in_x, in_y);
		cout << i << ". x " << in_x << "y " << in_y << "\n";
	}
	inFile.close();
}

CamCalibrator::~CamCalibrator()
{
}

// Get image coordinates of calibration points
// Currently assumes the image has been properly segmented into 63 calibration objects
void CamCalibrator::getPtsFromSegmentedImage()
{
	cout << "Getting points from segmented image...\n";


}

// Match image calibration points to measured points of the calibration object
void CamCalibrator::matchPtsToCalibrationPts()
{
	cout << "Matching image points to measured points... \n";
	
	//for (int i = 0; i < 63; i++){ cout << "Debug: x " << calPtsInImg[i].x << ", y " << calPtsInImg[i].y << "\n"; }

	// 1. Sort calibration image points by x value
	std::sort(calPtsInImg, calPtsInImg+63, compareX);

	//for (int i = 0; i < 63; i++){ cout << "Sort debug: x " << calPtsInImg[i].x << ", y " << calPtsInImg[i].y << "\n"; }

	// 2. Divide Left (35) and Right (28) image points based on x value
	Point2d Left[35];
	Point2d Right[28];
	for (int i = 0; i < 35; i++){ Left[i] = calPtsInImg[i]; }
	for (int i = 0; i < 28; i++){ Right[i] = calPtsInImg[i+35]; }
	//for (int i = 0; i < 35; i++){ cout << "(Debug) L[" << i << "]: x = " << Left[i].x << ", y = " << Left[i].y << "\n"; }
	//for (int i = 0; i < 28; i++){ cout << "(Debug) R[" << i << "]: x = " << Right[i].x << ", y = " << Right[i].y << "\n"; }

	// 3. Get two outer corner points on each side by distance to image corners

	// (a) Left face, top
	int index = 0; double minimise = 1000000;
	for (int i = 0; i < 35; i++){
		double xdiff = Left[i].x - 0;	double ydiff = Left[i].y - 0;
		double squareDistance = sqrt( (xdiff * xdiff) + (ydiff * ydiff) );
		if (squareDistance < minimise){ index = i; minimise = squareDistance; }
	}
	Point2d Left75 = Left[index];

	// (b) Left face, bottom
	index = 0; minimise = 1000000;
	for (int i = 0; i < 35; i++){
		double xdiff = Left[i].x - 0;	double ydiff = Left[i].y - imageLengthY;
		double squareDistance = sqrt( (xdiff * xdiff) + (ydiff * ydiff) );
		if (squareDistance < minimise){ index = i; minimise = squareDistance; }
	}
	Point2d Left15 = Left[index];

	// (b) Right face, top
	index = 0; minimise = 1000000;
	for (int i = 0; i < 28; i++){
		double xdiff = Right[i].x - imageLengthX;	double ydiff = Right[i].y - 0;
		double squareDistance = sqrt( (xdiff * xdiff) + (ydiff * ydiff) );
		if (squareDistance < minimise){ index = i; minimise = squareDistance; }
	}
	Point2d Right74 = Right[index];

	// (b) Right face, bottom
	index = 0; minimise = 1000000;
	for (int i = 0; i < 28; i++){
		double xdiff = Right[i].x - imageLengthX;	double ydiff = Right[i].y - imageLengthY;
		double squareDistance = sqrt( (xdiff * xdiff) + (ydiff * ydiff) );
		if (squareDistance < minimise){ index = i; minimise = squareDistance; }
	}
	Point2d Right14 = Right[index];

	/*cout << "Debug corner pts: L75: x = " << Left75.x << ", y = " << Left75.y << "\n";
	cout << "Debug corner pts: L15: x = " << Left15.x << ", y = " << Left15.y << "\n";
	cout << "Debug corner pts: R74: x = " << Right74.x << ", y = " << Right74.y << "\n";
	cout << "Debug corner pts: R14: x = " << Right14.x << ", y = " << Right14.y << "\n";*/

	// 4. Get two inner corner points on each side: take 7 rightmost Left face points and 7 leftmost Right points;
	//		find mean x for the vertical line separating the two faces; find points (x,top) and (x,bottom) on image;
	//		find closest points on each side to these two points by triangle distance.
	double meanX = 0.0;
	for (int i = 28; i < 35; i++){ meanX += Left[i].x; }
	for (int i = 0; i < 7; i++){ meanX += Right[i].x; }
	meanX /= 14.0;
	cout << "(Debug) x-value of line separating two faces in image: " << meanX << "\n";

	// (a) Left face, top-centre
	index = 0; minimise = 1000000;
	for (int i = 0; i < 35; i++){
		double xdiff = Left[i].x - meanX;
		double ydiff = Left[i].y - 0;
		double squareDistance = sqrt( (xdiff * xdiff) + (ydiff * ydiff) );
		if (squareDistance < minimise){ index = i; minimise = squareDistance; }
	}
	Point2d Left71 = Left[index];

	// (b) Left face, bottom-centre
	index = 0; minimise = 1000000;
	for (int i = 0; i < 35; i++){
		double xdiff = Left[i].x - meanX;
		double ydiff = Left[i].y - imageLengthY;
		double squareDistance = sqrt( (xdiff * xdiff) + (ydiff * ydiff) );
		if (squareDistance < minimise){ index = i; minimise = squareDistance; }
	}
	Point2d Left11 = Left[index];

	// (b) Right face, top-centre
	index = 0; minimise = 1000000;
	for (int i = 0; i < 28; i++){
		double xdiff = Right[i].x - meanX;
		double ydiff = Right[i].y - 0;
		double squareDistance = sqrt( (xdiff * xdiff) + (ydiff * ydiff) );
		if (squareDistance < minimise){ index = i; minimise = squareDistance; }
	}
	Point2d Right71 = Right[index];

	// (b) Right face, bottom-centre
	index = 0; minimise = 1000000;
	for (int i = 0; i < 28; i++){
		double xdiff = Right[i].x - meanX;
		double ydiff = Right[i].y - imageLengthY;
		double squareDistance = sqrt( (xdiff * xdiff) + (ydiff * ydiff) );
		if (squareDistance < minimise){ index = i; minimise = squareDistance; }
	}
	Point2d Right11 = Right[index];

	/*cout << "Debug corner pts: L71: x = " << Left71.x << ", y = " << Left71.y << "\n";
	cout << "Debug corner pts: L11: x = " << Left11.x << ", y = " << Left15.y << "\n";
	cout << "Debug corner pts: R71: x = " << Right71.x << ", y = " << Right71.y << "\n";
	cout << "Debug corner pts: R11: x = " << Right11.x << ", y = " << Right11.y << "\n";*/

	// 5. We now have 8 corner points identified, four for each face. From these corner points,
	//		we know the horizontal lines representing the top and bottom of each face.
	//		Sort Left or Right each time according to new criteria: distance to a particular line of our choice.

	for (int i = 0; i < 35; i++){
		double dist = pointLineDistance(Left[i], Left75, Left71);
		//cout << "(Debug) distance to top Left line: " << dist << "\n";
	}

	// 6.a) For all points in Left face, find distance from point to top Left line. Take five closest.
	lineSort(Left75, Left71, Left, 35);	// Sorts Left by distance to top line
	// Create new array LeftLine copied from Left[0-4], and sort by x value (compareX).
	Point2d LeftLine[5];
	for (int i = 0; i < 5; i++){ LeftLine[i] = Left[i]; }
	std::sort(LeftLine, LeftLine+5, compareX);
	cout << "Debug top L-face line:\n";
	for (int i = 0; i < 5; i++){
		cout << "(" << LeftLine[i].x << "," << LeftLine[i].y << ")";
	}
	cout << "\n";
	// We have now identified the top left line of points; add them to the calibration object.
	obj->setLeftAssocImagePt_RAW(7, 5, LeftLine[0]);
	obj->setLeftAssocImagePt_RAW(7, 4, LeftLine[1]);
	obj->setLeftAssocImagePt_RAW(7, 3, LeftLine[2]);
	obj->setLeftAssocImagePt_RAW(7, 2, LeftLine[3]);
	obj->setLeftAssocImagePt_RAW(7, 1, LeftLine[4]);

	// 6.b) For all points in Left face, find distance from point to bottom Left line. Take five closest.
	lineSort(Left15, Left11, Left, 35);	// Sorts Left by distance to bottom line
	for (int i = 0; i < 5; i++){ LeftLine[i] = Left[i]; }
	std::sort(LeftLine, LeftLine+5, compareX);
	cout << "Debug bottom L-face line:\n";
	for (int i = 0; i < 5; i++){
		cout << "(" << LeftLine[i].x << "," << LeftLine[i].y << ")";
	}
	cout << "\n";
	// We have now identified the bottom left line of points; add them to the calibration object.
	obj->setLeftAssocImagePt_RAW(1, 5, LeftLine[0]);
	obj->setLeftAssocImagePt_RAW(1, 4, LeftLine[1]);
	obj->setLeftAssocImagePt_RAW(1, 3, LeftLine[2]);
	obj->setLeftAssocImagePt_RAW(1, 2, LeftLine[3]);
	obj->setLeftAssocImagePt_RAW(1, 1, LeftLine[4]);

	// 7.a) For all points in Right face, find distance from point to top Right line. Take five closest.
	lineSort(Right71, Right74, Right, 28);	// Sorts Right by distance to top line
	// Create new array RightLine copied from Right[0-3], and sort by x value (compareX).
	Point2d RightLine[4];
	for (int i = 0; i < 4; i++){ RightLine[i] = Right[i]; }
	std::sort(RightLine, RightLine+4, compareX);
	cout << "Debug top R-face line:\n";
	for (int i = 0; i < 4; i++){
		cout << "(" << RightLine[i].x << "," << RightLine[i].y << ")";
	}
	cout << "\n";
	// We have now identified the top right line of points; add them to the calibration object.
	obj->setRightAssocImagePt_RAW(7, 1, RightLine[0]);
	obj->setRightAssocImagePt_RAW(7, 2, RightLine[1]);
	obj->setRightAssocImagePt_RAW(7, 3, RightLine[2]);
	obj->setRightAssocImagePt_RAW(7, 4, RightLine[3]);

	// 7.b) For all points in Right face, find distance from point to bottom Right line. Take five closest.
	lineSort(Right11, Right14, Right, 28);	// Sorts Right by distance to bottom line
	for (int i = 0; i < 4; i++){ RightLine[i] = Right[i]; }
	std::sort(RightLine, RightLine+4, compareX);
	cout << "Debug bottom R-face line:\n";
	for (int i = 0; i < 4; i++){
		cout << "(" << RightLine[i].x << "," << RightLine[i].y << ")";
	}
	cout << "\n";
	// We have now identified the bottom right line of points; add them to the calibration object.
	obj->setRightAssocImagePt_RAW(1, 1, RightLine[0]);
	obj->setRightAssocImagePt_RAW(1, 2, RightLine[1]);
	obj->setRightAssocImagePt_RAW(1, 3, RightLine[2]);
	obj->setRightAssocImagePt_RAW(1, 4, RightLine[3]);

	// 8. For all points in each face, find distance from that point to each of that face's vertical lines.
	//     Calculate which point on the object they correspond to accordingly.
	Point2d verticalLine[7];

	// 8.a) For all points in Left face, find distance to each of the FIVE vertical lines. Calculate corresponding point.
	for (int i = 0; i < 5; i++){
		// Sort Left by distance to vertical line 5-i. (5, 4, 3, 2, 1)
		lineSort(obj->getLeftAssocImagePt_RAW(7, 5-i), obj->getLeftAssocImagePt_RAW(1, 5-i), Left, 35);
		// Get the seven closest calibration image points to vertical line 5-i.
		for (int j = 0; j < 7; j++){ verticalLine[j] = Left[j]; }
		// Sort these seven points by y-value.
		std::sort(verticalLine, verticalLine+7, compareY);
		// We have now identified the intermediate 5 in a vertical line of points; add them to the calibration object.
		//obj->setLeftAssocImagePt(7, 5-i, verticalLine[0]);
		obj->setLeftAssocImagePt_RAW(6, 5-i, verticalLine[1]);
		obj->setLeftAssocImagePt_RAW(5, 5-i, verticalLine[2]);
		obj->setLeftAssocImagePt_RAW(4, 5-i, verticalLine[3]);
		obj->setLeftAssocImagePt_RAW(3, 5-i, verticalLine[4]);
		obj->setLeftAssocImagePt_RAW(2, 5-i, verticalLine[5]);
		//obj->setLeftAssocImagePt(1, 5-i, verticalLine[6]);
	}

	// 8.b) For all points in Right face, find distance to each of the FOUR vertical lines. Calculate corresponding point.
	for (int i = 0; i < 4; i++){
		// Sort Right by distance to vertical line 4-i. (4, 3, 2, 1)
		lineSort(obj->getRightAssocImagePt_RAW(1, 4-i), obj->getRightAssocImagePt_RAW(7, 4-i), Right, 28);
		// Get the seven closest calibration image points to vertical line 4-i.
		for (int j = 0; j < 7; j++){ verticalLine[j] = Right[j]; }
		// Sort these seven points by y-value.
		std::sort(verticalLine, verticalLine+7, compareY);
		// We have now identified the intermediate 5 in a vertical line of points; add them to the calibration object.
		//obj->setRightAssocImagePt(7, 4-i, verticalLine[0]);
		obj->setRightAssocImagePt_RAW(6, 4-i, verticalLine[1]);
		obj->setRightAssocImagePt_RAW(5, 4-i, verticalLine[2]);
		obj->setRightAssocImagePt_RAW(4, 4-i, verticalLine[3]);
		obj->setRightAssocImagePt_RAW(3, 4-i, verticalLine[4]);
		obj->setRightAssocImagePt_RAW(2, 4-i, verticalLine[5]);
		//obj->setRightAssocImagePt(1, 4-i, verticalLine[6]);
	}

	cout << "\n** Associated RAW image coordinates for Left face calibration points (x/y): **\n";
	for (int i = 0; i < 7; i++){
		for (int j = 0; j < 5; j++){
			cout << obj->getLeftAssocImagePt_RAW(7-i, 5-j).x << "/" << obj->getLeftAssocImagePt_RAW(7-i, 5-j).y << " ";
		}
		cout << "\n";
	}
	cout << "** Associated RAW image coordinates for Right face calibration points (x/y): **\n";
	for (int i = 0; i < 7; i++){
		for (int j = 0; j < 4; j++){
			cout << obj->getRightAssocImagePt_RAW(7-i, j+1).x << "/" << obj->getRightAssocImagePt_RAW(7-i, j+1).y << " ";
		}
		cout << "\n";
	}
}

// Performing Tsai calibration. SEE Tsai section G: "Calibrating a Camera Using Monoview Noncoplanar Points"
void CamCalibrator::calibrate()
{	
	cout << "\nPerforming camera calibration... ";
	double imageCX = imageLengthX/2.0;
	double imageCY = imageLengthY/2.0;

	// Initially set scale factor sx = 1
	// Assume pixel units and 1 pixel between adjacent camera sensor elements.
	sx = 1;

	// Calibration step 1: compute 3d orientation, position and scale factor

	mat_M = Mat(63, 7, CV_64F, Scalar::all(0)); // Matrix M is a Nx7 matrix with row vectors of the equation m
	mat_X = Mat(63, 1, CV_64F, Scalar::all(0)); // Matrix X is a Nx1 matrix of xd values

	// For each of the 63 points in turn (i):
	// 1. Calculate adjusted image coordinates, xd and yd, for image frame. Put in ADJ.
	// 2. Change row i of X to xd.
	// 3. Change row i of M to [yd*Xw, yd*Yw, yd*Zw, yd, -xdXw, -xdYw, -xdZw].
	for (int i = 0; i < 35; i++){
		double xd = ( obj->lAssocImagePts_RAW[i].x - imageCX ) / sx;
		double yd = obj->lAssocImagePts_RAW[i].y - imageCY;
		obj->lAssocImagePts_ADJ[i].x = xd;
		obj->lAssocImagePts_ADJ[i].y = yd;
		mat_X.at<double>(i,0) = xd;
		mat_M.at<double>(i,0) = yd*obj->lMeasurements[i].x;
		mat_M.at<double>(i,1) = yd*obj->lMeasurements[i].y;
		mat_M.at<double>(i,2) = yd*obj->lMeasurements[i].z;
		mat_M.at<double>(i,3) = yd;
		mat_M.at<double>(i,4) = -xd*obj->lMeasurements[i].x;
		mat_M.at<double>(i,5) = -xd*obj->lMeasurements[i].y;
		mat_M.at<double>(i,6) = -xd*obj->lMeasurements[i].z;
	}
	for (int i = 0; i < 28; i++){
		double xd = (obj->rAssocImagePts_RAW[i].x - imageCX) / sx;
		double yd = obj->rAssocImagePts_RAW[i].y - imageCY;
		obj->rAssocImagePts_ADJ[i].x = xd;
		obj->rAssocImagePts_ADJ[i].y = yd;
		mat_X.at<double>(i+35,0) = xd;
		mat_M.at<double>(i+35,0) = yd*obj->rMeasurements[i].x;
		mat_M.at<double>(i+35,1) = yd*obj->rMeasurements[i].y;
		mat_M.at<double>(i+35,2) = yd*obj->rMeasurements[i].z;
		mat_M.at<double>(i+35,3) = yd;
		mat_M.at<double>(i+35,4) = -xd*obj->rMeasurements[i].x;
		mat_M.at<double>(i+35,5) = -xd*obj->rMeasurements[i].y;
		mat_M.at<double>(i+35,6) = -xd*obj->rMeasurements[i].z;
	}

	/*cout << "Matrix M...\n";
	for (int i = 0; i < 63; i++){
	for (int j = 0; j < 7; j++){
	cout << " " << mat_M.at<double>(i,j);
	}
	cout << "\n";
	}
	cout << "Matrix X...\n";
	for (int i = 0; i < 63; i++){
	cout << " " << mat_X.at<double>(i,0) << "\n";
	}*/

	// Now solve using the pseudo-inverse technique to generate matrix L:
	// L = (M^transpose * M)^inverse * (M^transpose * X)
	Mat Mtrans_M_inv = (mat_M.t() * (mat_M)).inv(DECOMP_SVD);
	Mat Mtrans_X = mat_M.t() * mat_X;
	mat_L = Mtrans_M_inv * Mtrans_X;
	// DECOMP_LU is the LU decomposition (for non-singular matrices)
	// DECOMP_CHOLESKY is the Cholesky LL^T decomposition (for symmetrical positively defined matrices).
	// DECOMP_SVD is the SVD decomposition. If the matrix is singular or even non-square, the pseudo inversion is computed.
	cout << "Matrix L...\n";
	for (int i = 0; i < 7; i++){
		cout << " " << mat_L.at<double>(i,0) << "\n";
	}

	// Discover y-component of translation vector
	double a1 = mat_L.at<double>(0,0);		double a2 = mat_L.at<double>(1,0);
	double a3 = mat_L.at<double>(2,0);		double a4 = mat_L.at<double>(3,0);
	double a5 = mat_L.at<double>(4,0); double a6 = mat_L.at<double>(5,0); double a7 = mat_L.at<double>(6,0);
	double ty_sign_unknown = 1.0 / sqrt( a5*a5 + a6*a6 + a7*a7 );
	printf("T(y) [sign currently unknown]: %e \n", ty_sign_unknown);

	// Get a reference point not close to the image centre (easy enough to just check the left face)
	int pointIndex = 0;
	double distance = 0;
	for (int i = 0; i < 35; i++)
	{
		double xdiff = obj->lAssocImagePts_ADJ[i].x;
		double ydiff = obj->lAssocImagePts_ADJ[i].y;
		double squareDistance = sqrt( (xdiff * xdiff) + (ydiff * ydiff) );
		if (squareDistance > distance){
			pointIndex = i;
			distance = squareDistance;
		}
	}
	// Now start calculating parameters of R and T
	mat_R.at<double>(0,0) = a1*ty_sign_unknown; // r11
	mat_R.at<double>(0,1) = a2*ty_sign_unknown; // r12
	mat_R.at<double>(0,2) = a3*ty_sign_unknown; // r13
	mat_R.at<double>(1,0) = a5*ty_sign_unknown; // r21
	mat_R.at<double>(1,1) = a6*ty_sign_unknown; // r22
	mat_R.at<double>(1,2) = a7*ty_sign_unknown; // r23
	mat_T.at<double>(0,0) = a4*ty_sign_unknown; // tx

	// Calibration step 2: determine sign of ty

	// Use our distant reference point to check whether ty has the correct sign
	double Xw = obj->lMeasurements[pointIndex].x;
	double Yw = obj->lMeasurements[pointIndex].y;
	double xSign = mat_R.at<double>(0,0)*Xw + mat_R.at<double>(0,1)*Yw + mat_R.at<double>(0,2)*Yw + mat_T.at<double>(0,0);
	double ySign = mat_R.at<double>(1,0)*Xw + mat_R.at<double>(1,1)*Yw + mat_R.at<double>(1,2)*Yw + ty_sign_unknown;
	double XS = obj->lAssocImagePts_ADJ[pointIndex].x;
	double YS = obj->lAssocImagePts_ADJ[pointIndex].y;

	if ( (xSign > 0 && XS <= 0) || (xSign <= 0 && XS > 0) || (ySign > 0 && YS <= 0) || (ySign <= 0 && YS > 0) ) {
		cout << "Debug: signs are different, ty has wrong sign.\n";
		mat_T.at<double>(1,0) = ty_sign_unknown*-1;
	} else {
		cout << "Debug: signs are the same, ty has correct sign.\n";
		mat_T.at<double>(1,0) = ty_sign_unknown;
	}
	// We now have tx, ty.
	
	// Calibration step 3: determine true scaling factor sx

	sx = (sqrt( (mat_T.at<double>(1,0)) * (mat_T.at<double>(1,0)) )) * (sqrt(a1*a1 + a2*a2 + a3*a3));
	cout << "sx " << sx << "\n";

	// Calibration step 4: compute 3d rotation matrix R and tx

	// Recalculate r-components of R and also tx of T, using updated ty and updated sx
	mat_R.at<double>(0,0) = a1*mat_T.at<double>(1,0)/sx; // r11
	mat_R.at<double>(0,1) = a2*mat_T.at<double>(1,0)/sx; // r12
	mat_R.at<double>(0,2) = a3*mat_T.at<double>(1,0)/sx; // r13
	mat_R.at<double>(1,0) = a5*mat_T.at<double>(1,0); // r21
	mat_R.at<double>(1,1) = a6*mat_T.at<double>(1,0); // r22
	mat_R.at<double>(1,2) = a7*mat_T.at<double>(1,0); // r23
	mat_T.at<double>(0,0) = a4*mat_T.at<double>(1,0)/sx; // tx

	Mat row1R = Mat(1, 3, CV_64F, Scalar::all(0));
	Mat row2R = Mat(1, 3, CV_64F, Scalar::all(0));
	row1R.at<double>(0,0) = mat_R.at<double>(0,0);
	row1R.at<double>(0,1) = mat_R.at<double>(0,1);
	row1R.at<double>(0,2) = mat_R.at<double>(0,2);
	row2R.at<double>(0,0) = mat_R.at<double>(1,0);
	row2R.at<double>(0,1) = mat_R.at<double>(1,1);
	row2R.at<double>(0,2) = mat_R.at<double>(1,2);

	// Next: Find the third row of the rotation matrix R. The first method uses the cross product;
	// the second method uses the inner vector product plus orthonormality property as given.
	// These give different results. The second method appears to work better so is chosen.

	// Method 1: cross product
	Mat row3R = row1R.cross(row2R); // In a 3d coordinate system / rotation matrix, X cross Y = Z
	mat_R.at<double>(2,0) = row3R.at<double>(0,0);
	mat_R.at<double>(2,1) = row3R.at<double>(0,1);
	mat_R.at<double>(2,2) = row3R.at<double>(0,2);
	cout << "\n" << "R with 3rd row calculated using cross product of first two rows:\n";
	for (int i = 0; i < 3; i++){
		for (int j = 0; j < 3; j++){ cout << "\t" << mat_R.at<double>(i,j) << "\t";	}
		cout << "\n";
	}
	cout << "row1 [dot] row3: " << row1R.dot(row3R) << ";\t row2 [dot] row3: " << row2R.dot(row3R) << "\n";
	cout << "Determinant of calculated rotation matrix R:   " <<  cv::determinant(mat_R) << "\n";
	// Dot products should be ~ 0.  det(R) = 1 for a rotation matrix

	// Method 2: inner vector product plus orthonormality property
	Mat col1R = mat_R.col(0);
	Mat col2R = mat_R.col(1);
	Mat col3R = mat_R.col(2);
	Mat new_a = Mat(2, 2, CV_64F, Scalar::all(0));
	new_a.at<double>(0,0) = mat_R.at<double>(0,1);	new_a.at<double>(0,1) = mat_R.at<double>(0,2);
	new_a.at<double>(1,0) = mat_R.at<double>(1,1);	new_a.at<double>(1,1) = mat_R.at<double>(1,2);
	Mat new_b = Mat(2, 2, CV_64F, Scalar::all(0));
	new_b.at<double>(0,0) = mat_R.at<double>(0,2);	new_b.at<double>(0,1) = mat_R.at<double>(0,0);
	new_b.at<double>(1,0) = mat_R.at<double>(1,2);	new_b.at<double>(1,1) = mat_R.at<double>(1,0);
	Mat new_c = Mat(2, 2, CV_64F, Scalar::all(0));
	new_c.at<double>(0,0) = mat_R.at<double>(0,0);	new_c.at<double>(0,1) = mat_R.at<double>(0,1);
	new_c.at<double>(1,0) = mat_R.at<double>(1,0);	new_c.at<double>(1,1) = mat_R.at<double>(1,1);
	double new_a_det = determinant(new_a);
	double new_b_det = determinant(new_b);
	double new_c_det = determinant(new_c);
	double factor_squared = 1.0 / (new_a_det*new_a_det + new_b_det*new_b_det + new_c_det*new_c_det);
	double factor = sqrt(factor_squared);
	mat_R.at<double>(2,0) = factor*new_a_det;
	mat_R.at<double>(2,1) = factor*new_b_det;
	mat_R.at<double>(2,2) = factor*new_c_det;
	cout << "\n" << "R with 3rd row calculated using inner vector product + orthonormality:\n";
	for (int i = 0; i < 3; i++){
		for (int j = 0; j < 3; j++){ cout << "\t" << mat_R.at<double>(i,j) << "\t";	}
		cout << "\n";
	}
	cout << "row1 [dot] row3: " << row1R.dot(mat_R.row(2)) << ";\t row2 [dot] row3: " << row2R.dot(mat_R.row(2)) << "\n";
	cout << "Determinant of calculated rotation matrix R:   " <<  cv::determinant(mat_R) << "\n";
	// Dot products should be ~ 0.  det(R) = 1 for a rotation matrix

	cout << "\n";

	// Calibration stage 2 [or step 6]:
	// compute effective focal length, distortion coefficients kappa, and tz (z-component of translation vector T)
	// Rewrite projection relations for each reference point i as a system of linear equations

	// For each of the 63 points in turn (i):
	// 1. Calculate yi and wi.
	// 2. Change row i, first column of mat_M2 to yi.
	// 3. Change row i, second column of mat_M2 to -1.0*y.
	// 4. Change row i, only column of mat_U to wi *y.
	/*for (int i = 0; i < 35; i++){
		double yi = mat_R.at<double>(1,0)*obj->lMeasurements[i].x + mat_R.at<double>(1,1)*obj->lMeasurements[i].y + mat_R.at<double>(1,2)*obj->lMeasurements[i].z + mat_T.at<double>(1,0);
		double wi = mat_R.at<double>(2,0)*obj->lMeasurements[i].x + mat_R.at<double>(2,1)*obj->lMeasurements[i].y + mat_R.at<double>(2,2)*obj->lMeasurements[i].z + 0.0;
		mat_M2.at<double>(i,0) = yi;
		mat_M2.at<double>(i,1) = obj->lAssocImagePts_ADJ[i].y * -1.0;
		mat_U.at<double>(i,0) = wi * obj->lAssocImagePts_ADJ[i].y;
	}
	for (int i = 0; i < 28; i++){
		double yi = mat_R.at<double>(1,0)*obj->rMeasurements[i].x + mat_R.at<double>(1,1)*obj->rMeasurements[i].y + mat_R.at<double>(1,2)*obj->rMeasurements[i].z + mat_T.at<double>(1,0);
		double wi = mat_R.at<double>(2,0)*obj->rMeasurements[i].x + mat_R.at<double>(2,1)*obj->rMeasurements[i].y + mat_R.at<double>(2,2)*obj->rMeasurements[i].z + 0.0;
		mat_M2.at<double>(i+35,0) = yi;
		mat_M2.at<double>(i+35,1) = obj->rAssocImagePts_ADJ[i].y * -1.0;
		mat_U.at<double>(i+35,0) = wi*obj->rAssocImagePts_ADJ[i].y;
	}
	*/

	Mat best1, best2;
	best1 = computeLeastSquaresForKappa(0.0);
	double bestError1 = best1.at<double>(2,0);
	double bestKappa1 = 0.0;
	double bestError2, bestKappa2;
	Mat comp1 = computeLeastSquaresForKappa(-0.0001);
	Mat comp2 = computeLeastSquaresForKappa(0.0001);
	if (comp1.at<double>(2,0) < comp2.at<double>(2,0)) {
		best2 = comp1;
		bestError2 = comp1.at<double>(2,0);
		bestKappa2 = -0.0001;
	} else {
		best2 = comp2;
		bestError2 = comp2.at<double>(2,0);
		bestKappa2 = 0.0001;
	}
	for (int i = 0; i < 100; i++){
		double newKappa;
		if (bestError1 < bestError2){
			newKappa = (bestKappa1*9.0 + bestKappa2)/10.0;
		} else {
			newKappa = (bestKappa1 + bestKappa2*9.0)/10.0;
		}
		Mat F = computeLeastSquaresForKappa(newKappa);
		double newError = F.at<double>(2,0);
		if (bestError1 < bestError2){
			if (newError < bestError2){
				best2 = F;
				bestError2 = newError;
				bestKappa2 = newKappa;
			}
		} else {
			if (newError < bestError1){
				best1 = F;
				bestError1 = newError;
				bestKappa1 = newKappa;
			}
		}
	}
	if (bestError1 < bestError2){
		cout << "> ITERATIVE BEST ERROR: error " << bestError1 << ", kappa " << bestKappa1 << ",\n";
		cout << "   f " << best1.at<double>(0,0) << ", Tz " << best1.at<double>(1,0) << "\n";
		cout << "> SECOND BEST ERROR: error " << bestError2 << ", kappa " << bestKappa2 << "\n";
		cout << "   f " << best2.at<double>(0,0) << ", Tz " << best2.at<double>(1,1) << "\n";
		// Set our estimate for focal length and tz
		focalLength = best1.at<double>(0,0);
		mat_T.at<double>(2,0) = best1.at<double>(1,0);
	} else {
		cout << "> ITERATIVE BEST ERROR: error " << bestError2 << ", kappa " << bestKappa2 << ",\n";
		cout << "   f " << best2.at<double>(0,0) << ", Tz " << best2.at<double>(1,0) << "\n";
		cout << "> SECOND BEST ERROR: error " << bestError1 << ", kappa " << bestKappa1 << "\n";
		cout << "   f " << best1.at<double>(0,0) << ", Tz " << best1.at<double>(1,0) << "\n";
		focalLength = best2.at<double>(0,0);
		mat_T.at<double>(2,0) = best2.at<double>(1,0);
	}
	Mat temp_1_M = computeLeastSquaresForKappa(0.0);
	cout << "> IGNORING ERROR: error " << temp_1_M.at<double>(2,0) << ", kappa " << 0.0 << "\n";
	cout << "   f " << temp_1_M.at<double>(0,0) << ", Tz " << temp_1_M.at<double>(1,0) << "\n";

/*	for (double i = -0.00000002; i <= -0.000000005; i+=0.0000000001){
		Mat F = computeLeastSquaresForKappa(i);
		cout << "> Squared error (" << i << "): " << F.at<double>(2,0) << "\n";
	}*/

	// NEXT #1: Use steepest descent optimisation starting from already determined approximate values
	// to converge on the true values for the focal length and Tz.
	// NEXT #2: The previous assumed distortion [kappa] = 0. Rewrite to solve for [kappa] at the same time.

	cout << "\n****************\n\n";
	cout << "~~~ Camera parameters: ~~~\n\n";
	cout << " R =\t{ " << mat_R.at<double>(0,0) << "\t" << mat_R.at<double>(0,1) << "\t" << mat_R.at<double>(0,2) << "\t}\n";
	cout << "   \t{ " << mat_R.at<double>(1,0) << "\t" << mat_R.at<double>(1,1) << "\t" << mat_R.at<double>(1,2) << "\t}\n";
	cout << "   \t{ " << mat_R.at<double>(2,0) << "\t" << mat_R.at<double>(2,1) << "\t" << mat_R.at<double>(2,2) << "\t}\n";
	cout << "\n";
	cout << " T =\t{ " << mat_T.at<double>(0,0) << "\t}\n";
	cout << "   \t{ " << mat_T.at<double>(1,0) << "\t}\n";
	cout << "   \t{ " << mat_T.at<double>(2,0) << "\t}\n";
	cout << "\n";
	cout << "sx = " << sx << "\n";
	cout << "focal length = " << focalLength << "\n";
	cout << "\n****************\n\n";
}

// Calculates best fit of focalLength and Tz for a given kappa
// Outputs a 3x1 matrix containing focalLength, Tz, error squared of solution
Mat CamCalibrator::computeLeastSquaresForKappa(double kappa){
	Mat mat_M2a = Mat(63, 2, CV_64F, Scalar::all(0));
	Mat mat_Ua = Mat(63, 1, CV_64F, Scalar::all(0));
	for (int i = 0; i < 35; i++){
		double yi = mat_R.at<double>(1,0)*obj->lMeasurements[i].x + mat_R.at<double>(1,1)*obj->lMeasurements[i].y + mat_R.at<double>(1,2)*obj->lMeasurements[i].z + mat_T.at<double>(1,0);
		double wi = mat_R.at<double>(2,0)*obj->lMeasurements[i].x + mat_R.at<double>(2,1)*obj->lMeasurements[i].y + mat_R.at<double>(2,2)*obj->lMeasurements[i].z;
		double rSq = sqrt( ((1.0/sx) * obj->lAssocImagePts_ADJ[i].x)*((1.0/sx) * obj->lAssocImagePts_ADJ[i].x)
			+ ( obj->lAssocImagePts_ADJ[i].y * obj->lAssocImagePts_ADJ[i].y ) );
		double kappa_error = obj->lAssocImagePts_ADJ[i].y * kappa * (rSq * rSq);
		mat_M2a.at<double>(i,0) = yi;
		mat_M2a.at<double>(i,1) = (obj->lAssocImagePts_ADJ[i].y * -1.0) - kappa_error;
		mat_Ua.at<double>(i,0) = (wi * obj->lAssocImagePts_ADJ[i].y) + (wi*kappa_error);
	}
	for (int i = 0; i < 28; i++){
		double yi = mat_R.at<double>(1,0)*obj->rMeasurements[i].x + mat_R.at<double>(1,1)*obj->rMeasurements[i].y + mat_R.at<double>(1,2)*obj->rMeasurements[i].z + mat_T.at<double>(1,0);
		double wi = mat_R.at<double>(2,0)*obj->rMeasurements[i].x + mat_R.at<double>(2,1)*obj->rMeasurements[i].y + mat_R.at<double>(2,2)*obj->rMeasurements[i].z;
		double rSq = sqrt( ((1.0/sx) * obj->rAssocImagePts_ADJ[i].x)*((1.0/sx) * obj->rAssocImagePts_ADJ[i].x)
			+ ( obj->rAssocImagePts_ADJ[i].y * obj->rAssocImagePts_ADJ[i].y ) );
		double kappa_error = obj->rAssocImagePts_ADJ[i].y * kappa * (rSq * rSq);
		mat_M2a.at<double>(i+35,0) = yi;
		mat_M2a.at<double>(i+35,1) = (obj->rAssocImagePts_ADJ[i].y * -1.0) - kappa_error;
		mat_Ua.at<double>(i+35,0) = (wi*obj->rAssocImagePts_ADJ[i].y) + (wi*kappa_error);
	}
	// Now solve using the pseudo-inverse technique to generate matrix F:
	Mat M2trans_M2_inv = (mat_M2a.t() * (mat_M2a)).inv(DECOMP_SVD);
	Mat M2trans_U = mat_M2a.t() * mat_Ua;
	Mat F = M2trans_M2_inv * M2trans_U;

	// Now compare results to ideal results
	double output_foc = F.at<double>(0,0);
	double output_Tz = F.at<double>(1,0);
	double currentSquaredError = 0.0;
	for (int i = 0; i < 35; i++){
		double yi = mat_R.at<double>(1,0)*obj->lMeasurements[i].x + mat_R.at<double>(1,1)*obj->lMeasurements[i].y + mat_R.at<double>(1,2)*obj->lMeasurements[i].z + mat_T.at<double>(1,0);
		double wi = mat_R.at<double>(2,0)*obj->lMeasurements[i].x + mat_R.at<double>(2,1)*obj->lMeasurements[i].y + mat_R.at<double>(2,2)*obj->lMeasurements[i].z;
		double rSq = sqrt( ((1.0/sx) * obj->lAssocImagePts_ADJ[i].x)*((1.0/sx) * obj->lAssocImagePts_ADJ[i].x)
			+ ( obj->lAssocImagePts_ADJ[i].y * obj->lAssocImagePts_ADJ[i].y ) );
		double kappa_error = obj->lAssocImagePts_ADJ[i].y * kappa * (rSq * rSq);
		double output1 = (yi * output_foc) +
			(((obj->lAssocImagePts_ADJ[i].y * -1.0) - kappa_error) * output_Tz);
		double output2 = (wi * obj->lAssocImagePts_ADJ[i].y) + (wi*kappa_error);
		double sq_error = (output1-output2)*(output1-output2);
		//cout << "Err " << sq_error << "\n";
		currentSquaredError += sq_error;
	}
	for (int i = 0; i < 28; i++){
		double yi = mat_R.at<double>(1,0)*obj->rMeasurements[i].x + mat_R.at<double>(1,1)*obj->rMeasurements[i].y + mat_R.at<double>(1,2)*obj->rMeasurements[i].z + mat_T.at<double>(1,0);
		double wi = mat_R.at<double>(2,0)*obj->rMeasurements[i].x + mat_R.at<double>(2,1)*obj->rMeasurements[i].y + mat_R.at<double>(2,2)*obj->rMeasurements[i].z;
		double rSq = sqrt( ((1.0/sx) * obj->rAssocImagePts_ADJ[i].x)*((1.0/sx) * obj->rAssocImagePts_ADJ[i].x)
			+ ( obj->rAssocImagePts_ADJ[i].y * obj->rAssocImagePts_ADJ[i].y ) );
		double kappa_error = obj->rAssocImagePts_ADJ[i].y * kappa * (rSq * rSq);
		double output1 = (yi * output_foc) +
			(((obj->rAssocImagePts_ADJ[i].y * -1.0) - kappa_error) * output_Tz);
		double output2 = (wi * obj->rAssocImagePts_ADJ[i].y) + (wi*kappa_error);
		double sq_error = (output1-output2)*(output1-output2);
		//cout << "Err " << sq_error << "\n";
		currentSquaredError += sq_error;
	}
	Mat solution = Mat(3, 1, CV_64F, Scalar::all(0));
	solution.at<double>(0,0) = F.at<double>(0,0);
	solution.at<double>(1,0) = F.at<double>(1,0);
	solution.at<double>(2,0) = currentSquaredError;
	return solution;
}

// Back-project rays using the calculated matrices R, T, and values s, focalLength, deltaX, deltaY
void CamCalibrator::checkResults(){
	// Apply matrices in inverse order to image point, to transform into real world measurement
	// Then check the accuracy against the appropriate point on the calibration object



	// First do the forward direction, to check
	Point3d pt1 = obj->getLeftPt(1,1);
	Point3d pt2 = obj->getRightPt(2,2);
	Point2d point1i = obj->getLeftAssocImagePt_RAW(1,1);
	Point2d point2i = obj->getRightAssocImagePt_RAW(2,2);
	Point2d point1iadj = obj->getLeftAssocImagePt_ADJ(1,1);
	Point2d point2iadj = obj->getRightAssocImagePt_ADJ(2,2);
	Mat point1 = Mat(4, 1, CV_64F, Scalar::all(1));
	point1.at<double>(0,0) = pt1.x;
	point1.at<double>(1,0) = pt1.y;
	point1.at<double>(2,0) = pt1.z;
	Mat point2 = Mat(4, 1, CV_64F, Scalar::all(1));
	point2.at<double>(0,0) = pt2.x;
	point2.at<double>(1,0) = pt2.y;
	point2.at<double>(2,0) = pt2.z;
	Mat transform = Mat(4, 4, CV_64F, Scalar::all(0));
	for (int i = 0; i < 3; i++){
		for (int j = 0; j < 3; j++){
			transform.at<double>(i,j) = mat_R.at<double>(i,j);
		}
	}
	transform.at<double>(0,3) = mat_T.at<double>(0,0);
	transform.at<double>(1,3) = mat_T.at<double>(1,0);
	transform.at<double>(2,3) = mat_T.at<double>(2,0);
	transform.at<double>(3,3) = 1.0;
	Mat newpoint1 = transform * point1;
	Mat newpoint2 = transform * point2;
	//Mat focal = Mat(3, 4, CV_64F, Scalar::all(0));
	//focal.at<double>(0,0) = 1.0;
	//focal.at<double>(1,1) = 1.0;
	//focal.at<double>(2,2) = 1.0/focalLength;
	//Mat newpoint1a = focal * newpoint1;
	//Mat newpoint2a = focal * newpoint2;
	newpoint1.at<double>(0,0) = focalLength * (newpoint1.at<double>(0,0)/newpoint1.at<double>(2,0)); // x = f(X/Z) in camera coordinates XYZ
	newpoint1.at<double>(1,0) = focalLength * (newpoint1.at<double>(1,0)/newpoint1.at<double>(2,0)); // y = f(Y/Z) in camera coordinates XYZ
	newpoint2.at<double>(0,0) = focalLength * (newpoint2.at<double>(0,0)/newpoint2.at<double>(2,0)); // x = f(X/Z) in camera coordinates XYZ
	newpoint2.at<double>(1,0) = focalLength * (newpoint2.at<double>(1,0)/newpoint2.at<double>(2,0)); // y = f(Y/Z) in camera coordinates XYZ

		cout << "A2\n";
/*
	Mat xcyc = Mat(3, 3, CV_64F, Scalar::all(0));
	xcyc.at<double>(2,2) = 1.0;
	xcyc.at<double>(0,0) = -1.0;
	xcyc.at<double>(1,1) = -1.0;
	xcyc.at<double>(0,2) = imageLengthX/2; //?
	xcyc.at<double>(1,2) = imageLengthY/2; //?
	//Mat newpoint1b = xcyc * newpoint1a;
	//Mat newpoint2b = xcyc * newpoint2a;
	Mat newpoint1b = xcyc * newpoint1;
	Mat newpoint2b = xcyc * newpoint2;
	*/
	
	cout << "A1\n";

	//
	cout << "> Point1: (" << point1.at<double>(0,0) << ", " << point1.at<double>(1,0) << ", " << point1.at<double>(2,0) << ")\n";
	cout << " Point1 calculated point: (" << newpoint1.at<double>(0,0) << ", " << newpoint1.at<double>(1,0) << ", " << newpoint1.at<double>(2,0) << ")\n";
	//cout << " Point1a calculated point: (" << newpoint1a.at<double>(0,0) << ", " << newpoint1a.at<double>(1,0) << ", " << newpoint1a.at<double>(2,0) << ")\n";
//	cout << " Point1b calculated point: (" << newpoint1b.at<double>(0,0) << ", " << newpoint1b.at<double>(1,0) << ", " << newpoint1b.at<double>(2,0) << ")\n";
	cout << " Point1 RAW image point: (" << point1i.x << ", " << point1i.y << ")\n";
	cout << " Point1 ADJ image point: (" << point1iadj.x << ", " << point1iadj.y << ")\n";
	//
	cout << "> Point2: (" << point2.at<double>(0,0) << ", " << point2.at<double>(1,0) << ", " << point2.at<double>(2,0) << ")\n";
	cout << " Point2 calculated point: (" << newpoint2.at<double>(0,0) << ", " << newpoint2.at<double>(1,0) << ", " << newpoint2.at<double>(2,0) << ")\n";
	//cout << " Point2a calculated point: (" << newpoint2a.at<double>(0,0) << ", " << newpoint2a.at<double>(1,0) << ", " << newpoint2a.at<double>(2,0) << ")\n";
//	cout << " Point2b calculated point: (" << newpoint2b.at<double>(0,0) << ", " << newpoint2b.at<double>(1,0) << ", " << newpoint2b.at<double>(2,0) << ")\n";
	cout << " Point2 RAW image point: (" << point2i.x << ", " << point2i.y << ")\n";
	cout << " Point2 ADJ image point: (" << point2iadj.x << ", " << point2iadj.y << ")\n";

//	(u v 1) = (-1 -1 1 on diagonal, xc yc in the third column) * (1 1 1/f diagonal) * rotation & trans
	// * X Y Z real coordinates

	// what about scale factor? maybe divide the u and v resulting by s; or maybe it factors into f

// xc yc map into the distance to image centre, maybe? -1 if opposite to normal??

}

// Sorts an array of points by distance of those points to a given line (first sets lineP1, lineP2; then sorts using lineComp)
void CamCalibrator::lineSort(Point2d line1, Point2d line2, Point2d * leftOrRightArray, int arrayLength){
	lineP1 = line1;
	lineP2 = line2;
	std::sort(leftOrRightArray, leftOrRightArray + arrayLength, lineComp);
}

// Function for calculating the distance from a point to a line
double CamCalibrator::pointLineDistance(Point2d p, Point2d lineEnd1, Point2d lineEnd2){
	double u = ( (p.x - lineEnd1.x)*(lineEnd2.x-lineEnd1.x) + (p.y - lineEnd1.y)*(lineEnd2.y-lineEnd1.y) );
	u /= ( (lineEnd2.x-lineEnd1.x)*(lineEnd2.x-lineEnd1.x) + (lineEnd2.y-lineEnd1.y)*(lineEnd2.y-lineEnd1.y) );
	double xOnLine = lineEnd1.x + u*(lineEnd2.x - lineEnd1.x);
	double yOnLine = lineEnd1.y + u*(lineEnd2.y - lineEnd1.y);
	return sqrt( (p.x-xOnLine)*(p.x-xOnLine) + (p.y-yOnLine)*(p.y-yOnLine) );
}
