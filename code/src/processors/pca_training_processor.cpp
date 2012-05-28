#include "pca_training_processor.h"
#include "util.h"

#include <QDebug>

PcaTrainingProcessor::PcaTrainingProcessor(QObject *parent)
: Processor(parent)
{
	numImages = 0;
	pixelsPerImage = 0;
}


PcaTrainingProcessor::~PcaTrainingProcessor()
{
}


void PcaTrainingProcessor::run()
{

	forever {
		if(abort) return;
		emit progress(0);

		if( PCATrain() ) { // Returns true if successful
			mutex.lock();
			qDebug() << "PCA training complete.\n";
		} else {
			mutex.lock();
			//output_image = ?;
		}

		emit progress(100);
		emit updated();
		if(once) return;

		if(!restart)
			condition.wait(&mutex);
		restart = false;
		mutex.unlock();
	}

}

bool PcaTrainingProcessor::loadImages(){
	QMutexLocker l(&mutex);
	QString filename = file_list.canonicalFilePath();
	bool valid = file_list.exists();
	if(!valid) {
		return false;
	}
	QFile file(filename);
	if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;

	bool ok1, ok2;
	QString numImgs_str = file.readLine(); // First line of file is number of images
	QString pixPerImg_str = file.readLine(); // Second line of file is pixels per image
	int numImgs = numImgs_str.toInt(&ok1); // First line of file is number of images
	int pixPerImg = pixPerImg_str.toInt(&ok2); // Second line of file is pixels per image
	if(!ok1 || !ok2) return false;
	numImages = numImgs;
	pixelsPerImage = pixPerImg;

	cout << "numImages: " << numImages << "\n";
	cout << "pixelsPerImage: " << pixelsPerImage << "\n";
	trainingSetImages = Mat(pixelsPerImage, numImages, CV_64FC1); // Use double so we can subtract averages etc
	// Remainder of file is image names
	// **MUST** HAVE A TRAILING NEWLINE!
	int counter = 0;
	while (file.canReadLine()){
		QString qstr = file.readLine();
		string str = qstr.toStdString();
		str = str.erase(str.length()-1);
		cout << "Reading: " << str << "\n";
		Mat img = imread(str, 0); // not in colour yet
		img = img.reshape(img.channels(), img.rows*img.cols);
		for (int i = 0; i < pixelsPerImage; i++){
			trainingSetImages.at<double>(i,counter) = (0.0 + img.at<unsigned char>(i,0));
		}
		counter++;
	}
	cout << "Training set rows: " << trainingSetImages.rows << "\n";
	cout << "Training set cols: " << trainingSetImages.cols << "\n";
	for (int i = 0; i < trainingSetImages.rows; i++){
		for (int j = 0; j < trainingSetImages.cols; j++){
			cout << " " << trainingSetImages.at<double>(i,j);
		}
		cout << "\n";
	}
	return true;
}


bool PcaTrainingProcessor::PCATrain(){

	if (trainingSetImages.empty()) return false;
	// 1. Get training images as vectors
	// Fills "trainingSetImages"; each COLUMN is an image vector.
	// Currently only looking at grey values

	// Centre dataset with averages
	for (int i = 0; i < trainingSetImages.rows; i++){
		double average = 0;
		for (int j = 0; j < trainingSetImages.cols; j++){
			average += trainingSetImages.at<double>(i,j);
		}
		average /= trainingSetImages.cols;
		for (int j = 0; j < trainingSetImages.cols; j++){
			trainingSetImages.at<double>(i,j) -= average;
		}
	}

	cout << "\nCentred data:\n";
	for (int i = 0; i < trainingSetImages.rows; i++){
		for (int j = 0; j < trainingSetImages.cols; j++){
			cout << " " << trainingSetImages.at<double>(i,j);
		}
		cout << "\n";
	}

	// 2. Compute covariance matrix of centred dataset.
	// C = sum[1...N] of yi * yi(transpose)
	Mat covarianceMat = Mat(pixelsPerImage, pixelsPerImage, CV_64FC1, Scalar(0));
	for (int i = 0; i < numImages; i++){
		Mat matToAdd = trainingSetImages * trainingSetImages.t();
		covarianceMat = covarianceMat + matToAdd;
	}

	cout << "\nCovariance matrix:\n";
	for (int i = 0; i < covarianceMat.rows; i++){
		for (int j = 0; j < covarianceMat.cols; j++){
			cout << " " << covarianceMat.at<double>(i,j);
		}
		cout << "\n";
	}


	// THIS IS A TEST TO SEE WHETHER THE 'eigen' FUNCTION WORKS
	double b[5][5] = {
	{ 1.96 , -6.49, -0.47, -7.20, -0.65},
	{ -6.49,  3.80, -6.39,  1.50, -6.34},
	{ -0.47, -6.39,  4.17, -1.51,  2.67},
	{ -7.20,  1.50, -1.51,  5.70,  1.80},
	{ -0.65, -6.34,  2.67,  1.80, -7.10}
	};
	//cv::Mat E = Mat(5,1,CV_64F,Scalar(0));
	//cv::Mat V = Mat(5,5,CV_64F,Scalar(0));
	cv::Mat E, V;
	cv::Mat M = Mat(5,5,CV_64FC1,b);
	//cv::eigen(M,E,V);
	cv::eigen(M,true,E,V);
	//cv::eigen(M,E,V,-100000,100000);
	//cvEigenVV(M, V, E, DBL_EPSILON, -1, -1);
	cout << "\nTEST EIGENVALUES:\n";
	for(int i = 0; i < E.cols; i++) {
		for(int j = 0; j < E.cols; j++) {
			std::cout << E.at<double>(i,j) << " \t";
		}
		cout << "\n";
	}
	cout << "\nTEST EIGENVECTORS:\n";
	for(int i = 0; i < V.cols; i++) {
		for(int j = 0; j < V.cols; j++) {
			std::cout << V.at<double>(i,j) << " \t";
		}
		cout << "\n";
	}


	// 3. Find eigenvectors and eigenvalues of covariance matrix.
	// UNIT eigenvectors = principal directions.
	// Projections of image vector x to principal directions = principal components of x.
	cout << "\n";
//	Mat eigenvalues = ;
//	Mat eigenvectors = ;
	Mat eigenvalues, eigenvectors;
	cout << "\nRunning 'eigen' function... ";
	eigen(covarianceMat, eigenvalues, eigenvectors);
	cout << "done.\n\n";

	// Print eigenvalues, which are sorted descending
	cout << "\nOUTPUT EIGENVALUES:\n";
	for(int i = 0; i < eigenvalues.cols; i++) {
		for(int j = 0; j < eigenvalues.cols; j++) {
			std::cout << eigenvalues.at<double>(i,j) << " \t";
		}
		cout << "\n";
	}
	cout << "\nOUTPUT EIGENVECTORS:\n";
	for(int i = 0; i < eigenvectors.cols; i++) {
		for(int j = 0; j < eigenvectors.cols; j++) {
			std::cout << eigenvectors.at<double>(i,j) << " \t";
		}
		cout << "\n";
	}
	cout << "\n";

	// 2. Calculate eigenface vectors from training set.
	// 3. Keep only the M eigenvectors corresponding to the highest eigenvalues.
	//
	return true;
}


void PcaTrainingProcessor::setFileList(QFileInfo path)
{
  QMutexLocker locker(&mutex);
  if(path.canonicalFilePath() == file_list.canonicalFilePath()) return;
  file_list = path;
  mutex.unlock();
  loadImages();
  Processor::process();
}
