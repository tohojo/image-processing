#include "camera_calibrator.h"
#include <fstream>

using namespace cv;

// The following are just used to make sorting easier; it's more convenient than overriding the Point2d class
Point2d lineP1 = Point2d(0,0);
Point2d lineP2 = Point2d(0,0);
bool compareX(Point2d a, Point2d b){ return a.x < b.x; }
bool compareY(Point2d a, Point2d b){ return a.y < b.y; }
bool lineComp(Point2d a, Point2d b){
	return (CamCalibrator::pointLineDistance(a, lineP1, lineP2) < CamCalibrator::pointLineDistance(b, lineP1, lineP2));
}

CamCalibrator::CamCalibrator(std::list<Point> points2d, std::list<Point3d> points3d, int width, int height, std::vector<point_correspondence> corr) :cout(QtDebugMsg)
{
  obj = new VirtualCalibrationObject();
  mat_R = Mat(3, 3, CV_64F, Scalar::all(0));
  mat_T = Mat(3, 1, CV_64F, Scalar::all(0));
  imageLengthX = width;
  imageLengthY = height;
  mapping = corr;

  if(!corr.empty()) {
    calPtsInImg = 0;
    calPtsInWorld = 0;
    return;
  }
  assert(points2d.size() == 63 && points3d.size() == 63);

  calPtsInImg = new Point2d[points2d.size()];
  calPtsInWorld = new Point3d[points3d.size()];

  int j = 0;
  for(std::list<Point>::iterator i = points2d.begin(); i != points2d.end(); ++i) {
    calPtsInImg[j++] = *i;
  }

  j =0;
  for(std::list<Point3d>::iterator i = points3d.begin(); i != points3d.end(); ++i) {
    calPtsInWorld[j++] = *i;
  }
}

// Basic constructor
CamCalibrator::CamCalibrator(int argc, char *argv[]) : cout(QtDebugMsg)
{
	calPtsInImg = new Point2d[63];
	calPtsInWorld = new Point3d[63];
	obj = new VirtualCalibrationObject();
	mat_R = Mat(3, 3, CV_64F, Scalar::all(0));
	mat_T = Mat(3, 1, CV_64F, Scalar::all(0));

	// READ IN IMAGE POINTS
	char *in_arg;
        const char *def_arg = "test.txt";
	if (argc > 1) {
		in_arg = argv[1];
	} else {
          in_arg = (char*) def_arg;
	}
        std::ifstream inFile(in_arg, std::ios::in);
	inFile >> imageLengthX;
	inFile >> imageLengthY;

	double in_x, in_y, in_z;
	for (int i = 0; i < 63; i++){
		inFile >> in_x;
		inFile >> in_y;
		calPtsInImg[i] = Point2d(in_x, in_y);
		//cout << i << ". x " << in_x << "y " << in_y << "\n";
	}
	inFile.close();

	// READ IN WORLD POINTS
        std::ifstream inFile2("r11-r14-r71-r74-L11-L15-L71-L75.txt", std::ios::in);
	for (int i = 0; i < 63; i++){
		inFile2 >> in_x;
		inFile2 >> in_y;
		inFile2 >> in_z;
		calPtsInWorld[i] = Point3d(in_x, in_y, in_z);
		//cout << i << ". x " << in_x << "y " << in_y << "\n";
	}
	inFile2.close();
}

CamCalibrator::~CamCalibrator()
{
  if(calPtsInImg) delete calPtsInImg;
  if(calPtsInWorld) delete calPtsInWorld;
  delete obj;

}


// Match image calibration points to measured points of the calibration object
void CamCalibrator::mapPtsToCalibrationPts()
{
	cout << "Matching image points to measured points... \n";

	// "calPtsInWorld" array is in the form:
	// R11...R14	0-3
	// R21...R24	4-7
	// R31...R34	8-11
	// R41...R44	12-15
	// R51...R54	16-19
	// R61...R64	20-23
	// R71...R74	24-27
	// L11...L15	28-32
	// L21...L25	33-37
	// L31...L35	38-42
	// L41...L45	43-47
	// L51...L55	48-52
	// L61...L65	53-57
	// L71...L75	58-62
	for (int i = 0; i < 35; i++){
		obj->lMeasurements[i] = calPtsInWorld[62-i]; // Top row left
	}
	//
	obj->rMeasurements[0] = calPtsInWorld[24]; // Top row left
	obj->rMeasurements[1] = calPtsInWorld[25];
	obj->rMeasurements[2] = calPtsInWorld[26];
	obj->rMeasurements[3] = calPtsInWorld[27]; // Top row right
	obj->rMeasurements[4] = calPtsInWorld[20];
	obj->rMeasurements[5] = calPtsInWorld[21];
	obj->rMeasurements[6] = calPtsInWorld[22];
	obj->rMeasurements[7] = calPtsInWorld[23];
	obj->rMeasurements[8] = calPtsInWorld[16];
	obj->rMeasurements[9] = calPtsInWorld[17];
	obj->rMeasurements[10] = calPtsInWorld[18];
	obj->rMeasurements[11] = calPtsInWorld[19];
	obj->rMeasurements[12] = calPtsInWorld[12];
	obj->rMeasurements[13] = calPtsInWorld[13];
	obj->rMeasurements[14] = calPtsInWorld[14];
	obj->rMeasurements[15] = calPtsInWorld[15];
	obj->rMeasurements[16] = calPtsInWorld[8];
	obj->rMeasurements[17] = calPtsInWorld[9];
	obj->rMeasurements[18] = calPtsInWorld[10];
	obj->rMeasurements[19] = calPtsInWorld[11];
	obj->rMeasurements[20] = calPtsInWorld[4];
	obj->rMeasurements[21] = calPtsInWorld[5];
	obj->rMeasurements[22] = calPtsInWorld[6];
	obj->rMeasurements[23] = calPtsInWorld[7];
	obj->rMeasurements[24] = calPtsInWorld[0]; // Bottom row left
	obj->rMeasurements[25] = calPtsInWorld[1];
	obj->rMeasurements[26] = calPtsInWorld[2];
	obj->rMeasurements[27] = calPtsInWorld[3]; // Bottom row right

	// 1. Sort calibration image points by x value
	std::sort(calPtsInImg, calPtsInImg+63, compareX);

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

	// 4. Get two inner corner points on each side: take 7 rightmost Left face points and 7 leftmost Right points;
	//		find mean x for the vertical line separating the two faces; find points (x,top) and (x,bottom) on image;
	//		find closest points on each side to these two points by triangle distance.
	double meanX = 0.0;
	for (int i = 28; i < 35; i++){ meanX += Left[i].x; }
	for (int i = 0; i < 7; i++){ meanX += Right[i].x; }
	meanX /= 14.0;
//	cout << "(Debug): Mean x-value of line separating two faces in image: " << meanX << "\n";

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

	// 5. We now have 8 corner points identified, four for each face. From these corner points,
	//		we know the horizontal lines representing the top and bottom of each face.
	//		Sort Left or Right each time according to new criteria: distance to a particular line of our choice.

	//for (int i = 0; i < 35; i++){
                //double dist = pointLineDistance(Left[i], Left75, Left71);
		//cout << "(Debug) distance to top Left line: " << dist << "\n";
	//}


	// 6.a) For all points in Left face, find distance from point to top Left line. Take five closest.
	lineSort(Left75, Left71, Left, 35);	// Sorts Left by distance to top line
	// Create new array LeftLine copied from Left[0-4], and sort by x value (compareX).
	Point2d LeftLine[5];
	for (int i = 0; i < 5; i++){ LeftLine[i] = Left[i]; }
	std::sort(LeftLine, LeftLine+5, compareX);
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
	// We have now identified the top right line of points; add them to the calibration object.
	obj->setRightAssocImagePt_RAW(7, 1, RightLine[0]);
	obj->setRightAssocImagePt_RAW(7, 2, RightLine[1]);
	obj->setRightAssocImagePt_RAW(7, 3, RightLine[2]);
	obj->setRightAssocImagePt_RAW(7, 4, RightLine[3]);

	// 7.b) For all points in Right face, find distance from point to bottom Right line. Take five closest.
	lineSort(Right11, Right14, Right, 28);	// Sorts Right by distance to bottom line
	for (int i = 0; i < 4; i++){ RightLine[i] = Right[i]; }
	std::sort(RightLine, RightLine+4, compareX);
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

	/*
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
	*/

	// Final, most important step:
	for (int i = 0; i <35; i++){
		point_correspondence pt_cor;
		pt_cor.imagePt = obj->lAssocImagePts_RAW[i];
		pt_cor.worldPt = obj->lMeasurements[i];
		mapping.push_back(pt_cor);
	}
	for (int i = 0; i <28; i++){
		point_correspondence pt_cor;
		pt_cor.imagePt = obj->rAssocImagePts_RAW[i];
		pt_cor.worldPt = obj->rMeasurements[i];
		mapping.push_back(pt_cor);
	}

	for (vector<point_correspondence>::iterator i = mapping.begin(); i != mapping.end(); i++){
		//		cout << " POINT found: <" << (i->worldPt.x) << "," << (i->worldPt.y) << "," << (i->worldPt.z)
		//			<< "     [" << (i->imagePt.x) << "," << (i->imagePt.y) << "] \n";
	}
        cout << "Mapping size:" << mapping.size();

}

// Performing Tsai calibration. SEE Tsai section G: "Calibrating a Camera Using Monoview Noncoplanar Points"
void CamCalibrator::calibrate()
{

	std::ofstream outFile("image_points.txt", std::ios::out);
	for (vector<point_correspondence>::iterator i = mapping.begin(); i != mapping.end(); i++){
		outFile << i->imagePt.x << " "  << i->imagePt.y << "\n";
	}
	outFile.close();

	cout << "\nPerforming camera calibration... \n";
	double imageCX = imageLengthX/2.0;
	double imageCY = imageLengthY/2.0;

	// Initially set scale factor sx = 1
	// Assume pixel units and 1 pixel between adjacent camera sensor elements.
	sx = 1;

	// Calibration step 1: compute 3d orientation, position and scale factor

	mat_M = Mat(mapping.size(), 7, CV_64F, Scalar::all(0)); // Matrix M is a Nx7 matrix with row vectors of the equation m
	mat_X = Mat(mapping.size(), 1, CV_64F, Scalar::all(0)); // Matrix X is a Nx1 matrix of xd values

	// For each of the ~63 points in turn (i):
	// 1. Calculate adjusted image coordinates, xd and yd, for image frame. Put in ADJ.
	// 2. Change row i of X to xd.
	// 3. Change row i of M to [yd*Xw, yd*Yw, yd*Zw, yd, -xdXw, -xdYw, -xdZw].
	int counter = 0;
	for (vector<point_correspondence>::iterator i = mapping.begin(); i != mapping.end(); i++){
		double xd = (i->imagePt.x - imageCX ) / sx;
                // This simultaneously adjusts 0 to be in the middle
                // of the image, and flips the Y coordinates (because
                // OpenCV uses top-left corner as 0,0).
		double yd = imageCY - i->imagePt.y;
		i->imagePt_adj = Point2d(xd, yd);
		mat_X.at<double>(counter,0) = xd;
		mat_M.at<double>(counter,0) = yd*(i->worldPt.x);
		mat_M.at<double>(counter,1) = yd*(i->worldPt.y);
		mat_M.at<double>(counter,2) = yd*(i->worldPt.z);
		mat_M.at<double>(counter,3) = yd;
		mat_M.at<double>(counter,4) = -xd*(i->worldPt.x);
		mat_M.at<double>(counter,5) = -xd*(i->worldPt.y);
		mat_M.at<double>(counter,6) = -xd*(i->worldPt.z);
		counter++;
	}

	// Now solve using the pseudo-inverse technique to generate matrix L:
	// L = (M^transpose * M)^inverse * (M^transpose * X)
	Mat Mtrans_M_inv = (mat_M.t() * (mat_M)).inv(DECOMP_SVD);
	Mat Mtrans_X = mat_M.t() * mat_X;
	mat_L = Mtrans_M_inv * Mtrans_X;
	// DECOMP_LU is the LU decomposition (for non-singular matrices)
	// DECOMP_CHOLESKY is the Cholesky LL^T decomposition (for symmetrical positively defined matrices).
	// DECOMP_SVD is the SVD decomposition. If the matrix is singular or even non-square, the pseudo inversion is computed.

	/*cout << "Matrix L...\n";
	for (int i = 0; i < 7; i++){
		cout << " " << mat_L.at<double>(i,0) << "\n";
	}
	*/

	// Discover y-component of translation vector
	double a1 = mat_L.at<double>(0,0);		double a2 = mat_L.at<double>(1,0);
	double a3 = mat_L.at<double>(2,0);		double a4 = mat_L.at<double>(3,0);
	double a5 = mat_L.at<double>(4,0); double a6 = mat_L.at<double>(5,0); double a7 = mat_L.at<double>(6,0);
	double ty_sign_unknown = 1.0 / sqrt( a5*a5 + a6*a6 + a7*a7 );
//	cout << "T(y) [sign currently unknown]: " << ty_sign_unknown << "\n";

	// Get a reference point not close to the image centre (easy enough to just check the left face)
	point_correspondence index_pc;
	double distance = 0;
	for (vector<point_correspondence>::iterator i = mapping.begin(); i != mapping.end(); i++){
		double xdiff = i->imagePt_adj.x;
		double ydiff = i->imagePt_adj.y;
		double squareDistance = sqrt( (xdiff * xdiff) + (ydiff * ydiff) );
		if (squareDistance > distance){
			index_pc.imagePt = i->imagePt;
			index_pc.worldPt = i->worldPt;
			index_pc.imagePt_adj = i->imagePt_adj;
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
	double Xw = index_pc.worldPt.x;
	double Yw = index_pc.worldPt.y;
	double xSign = mat_R.at<double>(0,0)*Xw + mat_R.at<double>(0,1)*Yw + mat_R.at<double>(0,2)*Yw + mat_T.at<double>(0,0);
	double ySign = mat_R.at<double>(1,0)*Xw + mat_R.at<double>(1,1)*Yw + mat_R.at<double>(1,2)*Yw + ty_sign_unknown;
	double XS = index_pc.imagePt.x;
	double YS = index_pc.imagePt.y;
	if ( (xSign > 0 && XS <= 0) || (xSign <= 0 && XS > 0) || (ySign > 0 && YS <= 0) || (ySign <= 0 && YS > 0) ) {
		mat_T.at<double>(1,0) = ty_sign_unknown*-1;
		cout << "Changing sign of Ty. Ty = " << mat_T.at<double>(1,0) << "\n";
	} else {
		mat_T.at<double>(1,0) = ty_sign_unknown;
		cout << "Sign of Ty is correct. Ty = " << mat_T.at<double>(1,0) << "\n";
	}
	// We now have tx, ty.

	// Calibration step 3: determine true scaling factor sx

	sx = (sqrt( (mat_T.at<double>(1,0)) * (mat_T.at<double>(1,0)) )) * (sqrt(a1*a1 + a2*a2 + a3*a3));
	//cout << "sx " << sx << "\n";

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
	cout << "\n" << "Third row of R calculated using cross product of first two rows:\n";
	for (int j = 0; j < 3; j++){ cout << "\t" << mat_R.at<double>(2,j) << "\t\n";	}
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
	cout << "\n" << "Third row of R calculated using inner vector product + orthonormality:\n";
	for (int j = 0; j < 3; j++){ cout << "\t" << mat_R.at<double>(2,j) << "\t\n";	}
	cout << "row1 [dot] row3: " << row1R.dot(mat_R.row(2)) << ";\t row2 [dot] row3: " << row2R.dot(mat_R.row(2)) << "\n";
	cout << "Determinant of calculated rotation matrix R:   " <<  cv::determinant(mat_R) << "\n";
	// Dot products should be ~ 0.  det(R) = 1 for a rotation matrix

	cout << "\n";

	// Calibration stage 2 [or step 6]:
	// compute effective focal length, distortion coefficients kappa, and tz (z-component of translation vector T)
	// Rewrite projection relations for each reference point i as a system of linear equations

	// For each of the ~63 points in turn (i):
	// 1. Calculate yi and wi.
	// 2. Change row i, first column of mat_M2 to yi.
	// 3. Change row i, second column of mat_M2 to -1.0*y.
	// 4. Change row i, only column of mat_U to wi *y.

	kappa1 = 0.0;
	double step1 = 0.001;
	double minstep1 = 0.00000000000001;
	kappa2 = 0.0;
	double step2 = 0.001;
	double minstep2 = 0.00000000000001;
	Mat kap;
	int steps_taken = 0;
	do {
		kap = computeLeastSquaresForKappa(kappa1, kappa2);
		double currentError = kap.at<double>(2,0);
		//
		double plus_plus = computeLeastSquaresForKappa(kappa1 + step1, kappa2 + step2).at<double>(2,0);
		double plus_minus = computeLeastSquaresForKappa(kappa1 + step1, kappa2 - step2).at<double>(2,0);
		double minus_plus = computeLeastSquaresForKappa(kappa1 - step1, kappa2 + step2).at<double>(2,0);
		double minus_minus = computeLeastSquaresForKappa(kappa1 - step1, kappa2 - step2).at<double>(2,0);
		//
		double plus_same = computeLeastSquaresForKappa(kappa1 + step1, kappa2).at<double>(2,0);
		double minus_same = computeLeastSquaresForKappa(kappa1 - step1, kappa2).at<double>(2,0);
		double same_plus = computeLeastSquaresForKappa(kappa1, kappa2 + step2).at<double>(2,0);
		double same_minus = computeLeastSquaresForKappa(kappa1, kappa2 - step2).at<double>(2,0);
		//
		if (( (currentError < plus_same) && (currentError < minus_same) ) || ( (currentError < same_plus) && (currentError < same_minus) )){
			if ( (currentError < plus_same) && (currentError < minus_same) ) {
				step1 = step1/2.0;
			}
			if ( (currentError < same_plus) && (currentError < same_minus) ){
				step2 = step2/2.0;
			}
		} else {
			// Pick the result with the lowest error
			if ( (plus_plus <= plus_minus) && (plus_plus <= minus_plus) && (plus_plus <= minus_minus) ){
				kappa1 = kappa1 + step1;
				kappa2 = kappa2 + step2;
			} else if ( (plus_minus <= plus_plus) && (plus_minus <= minus_plus) && (plus_minus <= minus_minus) ){
				kappa1 = kappa1 + step1;
				kappa2 = kappa2 - step2;
			} else if ( (minus_plus <= plus_plus) && (minus_plus <= plus_minus) && (minus_plus <= minus_minus) ){
				kappa1 = kappa1 - step1;
				kappa2 = kappa2 + step2;
			} else if ( (minus_minus <= plus_plus) && (minus_minus <= plus_minus) && (minus_minus <= minus_plus) ){
				kappa1 = kappa1 - step1;
				kappa2 = kappa2 - step2;
			}
		}
		steps_taken++;
	} while ((step1 > minstep1 || step2 > minstep2) && steps_taken < 10000);

	cout << "Kappa(1) = " << kappa1 << "; Kappa(2) = " << kappa2 << "\n";
	cout << "Kappas determined at step size " << step1 << " / " << step2 << "\n";
	cout << "Gradient descent iterations required: " << steps_taken << "\n";
	cout << "Calculated error for k1: " << kap.at<double>(2,0) << " ;  calculated f: " << kap.at<double>(0,0) << " ;  calculated Tz: " << kap.at<double>(1,0) << "\n";

	// Set our estimate for focal length and tz
	focalLength = kap.at<double>(0,0);
	mat_T.at<double>(2,0) = kap.at<double>(1,0);
	// We also now have our kappa-1 value

	Mat temp_1_M = computeLeastSquaresForKappa(0.0, 0.0);
	cout << "For comparison, error without kappas = " << temp_1_M.at<double>(2,0) << "\n";
	cout << "f without kappas = " << temp_1_M.at<double>(0,0) << "; Tz = " << temp_1_M.at<double>(1,0) << "\n";

	//for (double i = -0.00000002; i <= -0.000000005; i+=0.0000000001){
	//	Mat Fff = computeLeastSquaresForKappa(i);
	//	cout << "> Squared error (" << i << "): " << Fff.at<double>(2,0) << "\n";
	//}

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
	cout << "kappa-1 = " << kappa1 << "\n";
	cout << "kappa-2 = " << kappa2 << "\n";
	cout << "\n****************\n\n";

	checkResults();
}

// Calculates best fit of focalLength and Tz for a given kappa
// Outputs a 3x1 matrix containing focalLength, Tz, error squared of solution
Mat CamCalibrator::computeLeastSquaresForKappa(double k1, double k2){
	Mat mat_M2a = Mat(mapping.size(), 2, CV_64F, Scalar::all(0));
	Mat mat_Ua = Mat(mapping.size(), 1, CV_64F, Scalar::all(0));
	int count = 0;
	for (vector<point_correspondence>::iterator i = mapping.begin(); i != mapping.end(); i++){
		double yi = mat_R.at<double>(1,0)*i->worldPt.x + mat_R.at<double>(1,1)*i->worldPt.y + mat_R.at<double>(1,2)*i->worldPt.z + mat_T.at<double>(1,0);
		double wi = mat_R.at<double>(2,0)*i->worldPt.x + mat_R.at<double>(2,1)*i->worldPt.y + mat_R.at<double>(2,2)*i->worldPt.z;
		double rSq = sqrt( ((1.0/sx) * i->imagePt_adj.x)*((1.0/sx) * i->imagePt_adj.x)
			+ ( i->imagePt_adj.y * i->imagePt_adj.y ) );
		double kappa_error = ( k1*(rSq*rSq) + k2*(rSq*rSq*rSq*rSq) );
		double Yi = i->imagePt_adj.y + (i->imagePt_adj.y * kappa_error);
		mat_M2a.at<double>(count,0) = yi;
		mat_M2a.at<double>(count,1) = (Yi * -1.0);
		mat_Ua.at<double>(count,0) = (wi * Yi);
		count++;
	}

	// Now solve using the pseudo-inverse technique to generate matrix F:
	Mat M2trans_M2_inv = (mat_M2a.t() * (mat_M2a)).inv(DECOMP_SVD);
	Mat M2trans_U = mat_M2a.t() * mat_Ua;
	Mat F = M2trans_M2_inv * M2trans_U;

	// Now compare results to ideal results
	double output_foc = F.at<double>(0,0);
	double output_Tz = F.at<double>(1,0);
	double currentSquaredError = 0.0;

	for (vector<point_correspondence>::iterator i = mapping.begin(); i != mapping.end(); i++){
		double yi = mat_R.at<double>(1,0)*i->worldPt.x + mat_R.at<double>(1,1)*i->worldPt.y + mat_R.at<double>(1,2)*i->worldPt.z + mat_T.at<double>(1,0);
		double wi = mat_R.at<double>(2,0)*i->worldPt.x + mat_R.at<double>(2,1)*i->worldPt.y + mat_R.at<double>(2,2)*i->worldPt.z;
		double rSq = sqrt( ((1.0/sx) * i->imagePt_adj.x)*((1.0/sx) * i->imagePt_adj.x)
			+ ( i->imagePt_adj.y * i->imagePt_adj.y ) );
		double kappa_error = ( k1*(rSq*rSq) + k2*(rSq*rSq*rSq*rSq) );
		double Yi = i->imagePt_adj.y + (i->imagePt_adj.y * kappa_error);
		double output1 = (yi * output_foc) + (-Yi * output_Tz);
		double output2 = (wi * Yi);
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

	cout << "Checking results: finding error when applying calibration parameters to world points.\n";
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
	Mat perspective = Mat(3, 4, CV_64F, Scalar::all(0));
	perspective.at<double>(0,0) = 1.0;
	perspective.at<double>(1,1) = 1.0;
	perspective.at<double>(2,2) = 1.0/focalLength;
	double x_error_divd_sx = 0.0;
	double y_error_nil = 0.0;

	for (vector<point_correspondence>::iterator i = mapping.begin(); i != mapping.end(); i++){
		Point3d pt = i->worldPt;
		Point2d pt_ADJ = i->imagePt_adj;
		Mat pt1 = Mat(4, 1, CV_64F, Scalar::all(1));
		pt1.at<double>(0,0) = pt.x;
		pt1.at<double>(1,0) = pt.y;
		pt1.at<double>(2,0) = pt.z;
		pt1 = transform * pt1;
		pt1 = perspective * pt1;
		pt1.at<double>(0,0) /= pt1.at<double>(2,0);
		pt1.at<double>(1,0) /= pt1.at<double>(2,0);
		pt1.at<double>(2,0) = 1.0;
		double xx = pt1.at<double>(0,0);
		double yy = pt1.at<double>(1,0);
		double r = sqrt(xx*xx + yy*yy);
		double x_nil = xx - xx * (kappa1*(r*r) + kappa2*(r*r*r*r));
		double y_nil = yy - yy * (kappa1*(r*r) + kappa2*(r*r*r*r));
		double x_divd_sx = x_nil/sx;
		y_error_nil += sqrt( (pt_ADJ.y - y_nil)*(pt_ADJ.y - y_nil) );
		x_error_divd_sx += sqrt( (pt_ADJ.x - x_divd_sx)*(pt_ADJ.x - x_divd_sx) );
	}

	cout << " Mean x pixel error: " << x_error_divd_sx/mapping.size() << "\n";
	cout << " Mean y pixel error: " << y_error_nil/mapping.size() << "\n";

	cout << "\nFinding mean squared error by back-projecting rays to calibration points.\n";
	Mat rotate = Mat(4, 4, CV_64F, Scalar::all(0));
	for (int i = 0; i < 3; i++){
		for (int j = 0; j < 3; j++){
			rotate.at<double>(i,j) = mat_R.at<double>(i,j);
		}
	}
	rotate.at<double>(3,3) = 1.0;
	Mat translate = Mat(4, 4, CV_64F, Scalar::all(0));
	for (int i = 0; i < 4; i++){
		translate.at<double>(i,i) = 1.0;
	}
	translate.at<double>(0,3) = mat_T.at<double>(0,0);
	translate.at<double>(1,3) = mat_T.at<double>(1,0);
	translate.at<double>(2,3) = mat_T.at<double>(2,0);

	Mat rotate_inv = rotate.t();
	Mat translate_inv = Mat(4, 4, CV_64F, Scalar::all(0));
	for (int i = 0; i < 4; i++){
		translate_inv.at<double>(i,i) = 1.0;
	}
	translate_inv.at<double>(0,3) = -1.0*translate.at<double>(0,3);
	translate_inv.at<double>(1,3) = -1.0*translate.at<double>(1,3);
	translate_inv.at<double>(2,3) = -1.0*translate.at<double>(2,3);
	Mat perspective_inv = Mat(4, 4, CV_64F, Scalar::all(0));
	perspective_inv.at<double>(0,0) = 1.0/focalLength;
	perspective_inv.at<double>(1,1) = 1.0/focalLength;
	perspective_inv.at<double>(2,2) = 1.0/focalLength;
	perspective_inv.at<double>(3,2) = 1.0/focalLength;

	double totalSquaredError = 0.0;
	double squaredErrorX = 0.0;
	double squaredErrorY = 0.0;
	double squaredErrorZ = 0.0;
	double lowestSquaredError = 1000000.0;
	double highestSquaredError = -1.0;

	double * errors = new double[mapping.size()];

	int count = 0;
	for (vector<point_correspondence>::iterator i = mapping.begin(); i != mapping.end(); i++){
		Point3d ideal = i->worldPt;
		Point2d i_ADJ = i->imagePt_adj;
		//Point2d i_RAW = i->imagePt;
		Mat iPoint = Mat(4, 1, CV_64F, Scalar::all(1));
		iPoint.at<double>(0,0) = i_ADJ.x;
		iPoint.at<double>(1,0) = i_ADJ.y;
		double xx = iPoint.at<double>(0,0);
		double yy = iPoint.at<double>(1,0);
		iPoint.at<double>(0,0) = xx + xx * kappa1 * ( sqrt(xx*xx + yy*yy) ) * ( sqrt(xx*xx + yy*yy) );
		iPoint.at<double>(1,0) = yy + yy * kappa1 * ( sqrt(xx*xx + yy*yy) ) * ( sqrt(xx*xx + yy*yy) );
		iPoint.at<double>(0,0) /= sx;
		iPoint.at<double>(2,0) = focalLength;
		iPoint.at<double>(3,0) = 1;
		Mat i_factorZ = perspective_inv * iPoint;
		Mat i_real = Mat(4, 1, CV_64F, Scalar::all(0));
		i_real.at<double>(0,0) = translate_inv.at<double>(0,3);
		i_real.at<double>(1,0) = translate_inv.at<double>(1,3);
		i_real.at<double>(2,0) = translate_inv.at<double>(2,3);
		i_real = rotate_inv * i_real;
		i_factorZ = rotate_inv * i_factorZ;
		double calc_x;
		double calc_y;
		double calc_z;
		if (ideal.x == 0) { // Solve for x = 0
			calc_x = 0;
			double solvedValue = -1.0 * i_real.at<double>(0,0) / i_factorZ.at<double>(0,0);
			calc_y = i_factorZ.at<double>(1,0)*solvedValue + i_real.at<double>(1,0);
			calc_z = i_factorZ.at<double>(2,0)*solvedValue + i_real.at<double>(2,0);
		} else if (ideal.y == 0) { // Solve for y = 0
			calc_y = 0;
			double solvedValue = -1.0 * i_real.at<double>(1,0) / i_factorZ.at<double>(1,0);
			calc_x = i_factorZ.at<double>(0,0)*solvedValue + i_real.at<double>(0,0);
			calc_z = i_factorZ.at<double>(2,0)*solvedValue + i_real.at<double>(2,0);
		} else {
			double camToOrigin = sqrt( mat_T.at<double>(0,0)*mat_T.at<double>(0,0)
				+ mat_T.at<double>(1,0)*mat_T.at<double>(1,0)
				+ mat_T.at<double>(2,0)*mat_T.at<double>(2,0) );
			cout << "Using distance from camera to origin: " << camToOrigin << "\n";
			calc_x = i_factorZ.at<double>(0,0)*camToOrigin + i_real.at<double>(0,0);
			calc_y = i_factorZ.at<double>(1,0)*camToOrigin + i_real.at<double>(1,0);
			calc_z = i_factorZ.at<double>(2,0)*camToOrigin + i_real.at<double>(2,0);
		}
		//		cout << "================\n";
		//		cout << "Xw: " << ideal.x << "  Yw: " << ideal.y << "  Zw: " << ideal.z << "\n";
		//		cout << "X = " << calc_x << "  Y = " << calc_y << "  Z = " << calc_z << "\n";
		double err = sqrt ( (calc_x-ideal.x)*(calc_x-ideal.x) +
			(calc_y-ideal.y)*(calc_y-ideal.y) + (calc_z-ideal.z)*(calc_z-ideal.z) );
		//		cout << "SQ ERROR: " << err << "\n";
		totalSquaredError += err;
		squaredErrorX += sqrt ((calc_x-ideal.x)*(calc_x-ideal.x));
		squaredErrorY += sqrt ((calc_y-ideal.y)*(calc_y-ideal.y));
		squaredErrorZ += sqrt ((calc_z-ideal.z)*(calc_z-ideal.z));
		if (lowestSquaredError > err) lowestSquaredError = err;
		if (highestSquaredError < err) highestSquaredError = err;
		errors[count] = err;
		count++;
	}


	// Create a file of the errors for each point
	// Comparing these with a t-test can see whether the calibration is more accurate on e.g. higher-quality or less-distorted images.
	std::ofstream outFile("errors.txt", std::ios::out);
	for (unsigned int i = 0; i < mapping.size(); i++){
		outFile << errors[i] << "\n";
	}
	outFile.close();

	cout << "TOTAL ERROR: " << totalSquaredError << "\n";
	cout << "MEAN ERROR: " << totalSquaredError/mapping.size() << "\n";
	cout << "LOWEST POINT ERROR: " << lowestSquaredError << "\n";
	cout << "HIGHEST POINT ERROR: " << highestSquaredError << "\n";
	cout << "x ERROR: " << squaredErrorX/mapping.size() << "\n";
	cout << "y ERROR: " << squaredErrorY/mapping.size() << "\n";
	cout << "z ERROR: " << squaredErrorZ/mapping.size() << "\n";
	cout << "STANDARD DEVIATION: " << findStandardDeviation(errors, mapping.size()) << "\n";
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


double CamCalibrator::findStandardDeviation(double * entries, int count){
	if (count < 2) {
	} else {
		double mean = 0;
		for (int j = 0; j < count; j++){
			mean += entries[j]; // step 1, find mean
		}
		mean /= count;
		double sumOfSquares = 0;
		for (int j = 0; j < count; j++){
			// step 2, get deviations; step 3, square
			sumOfSquares += (entries[j] - mean) * (entries[j] - mean);
		}
		sumOfSquares /= (count-1);
		double stdev = sqrt(sumOfSquares);
		return stdev;
	}
}
