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
	trainingSetImages = Mat(pixelsPerImage, numImages, CV_32F); // Use floats so we can subtract averages etc
	// Remainder of file is image names
	// **MUST** HAVE A TRAILING NEWLINE!
	int counter = 0;
	while (file.canReadLine()){
		QString qstr = file.readLine();
		string str = qstr.toStdString();
		str = str.erase(str.length()-1);
		cout << "Reading: " << str << "\n";
		Mat img = imread(str, 0); //greyscale
		img = img.reshape(img.channels(), img.rows*img.cols);
		for (int i = 0; i < pixelsPerImage; i++){
			trainingSetImages.at<float>(i,counter) = (0.0 + img.at<unsigned char>(i,0));
		}
		counter++;
	}
	cout << "Training set rows: " << trainingSetImages.rows << "\n";
	cout << "Training set cols: " << trainingSetImages.cols << "\n";
	for (int i = 0; i < trainingSetImages.rows; i++){
		for (int j = 0; j < trainingSetImages.cols; j++){
			cout << " " << trainingSetImages.at<float>(i,j);
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
		float average = 0;
		for (int j = 0; j < trainingSetImages.cols; j++){
			average += trainingSetImages.at<float>(i,j);
		}
		average /= trainingSetImages.cols;
		for (int j = 0; j < trainingSetImages.cols; j++){
			trainingSetImages.at<float>(i,j) -= average;
		}
	}

	cout << "\n";
	for (int i = 0; i < trainingSetImages.rows; i++){
		for (int j = 0; j < trainingSetImages.cols; j++){
			cout << " " << trainingSetImages.at<float>(i,j);
		}
		cout << "\n";
	}

	// 2. Compute covariance matrix of centred dataset.
	// C = sum[1...N] of yi * yi(transpose)
	Mat covarianceMat = Mat(pixelsPerImage, pixelsPerImage, CV_32F);
	for (int i = 0; i < numImages; i++){
		covarianceMat = covarianceMat + (trainingSetImages.col(i) * (trainingSetImages.col(i).t()) );
	}

	cout << "\nCovariance\n";
	for (int i = 0; i < covarianceMat.rows; i++){
		for (int j = 0; j < covarianceMat.cols; j++){
			cout << " " << covarianceMat.at<float>(i,j);
		}
		cout << "\n";
	}

	// 3. Find eigenvectors and eigenvalues of covariance matrix.
	// UNIT eigenvectors = principal directions.
	// Projections of image vector x to principal directions = principal components of x.
	cout << "\nA";
	Mat eigenvalues, eigenvectors;
	cout << "\nB";
	eigen(covarianceMat, eigenvalues, eigenvectors);
	cout << "\nC";

	// Print eigenvalues, which are sorted descending
	for(int i = 0; i < eigenvalues.cols; i++) std::cout << eigenvalues.at<double>(0,i) << " \t";

	/*
	CvScalar scalar;
	CvMat* eigenvec  = cvCreateMat(pixelsPerImage, pixelsPerImage, CV_32F); //eigenvectors
	CvMat* eigenval  = cvCreateMat(1, pixelsPerImage, CV_32F);  //eigenvalues (1xN)
	cvZero(eigenvec);
	cvZero(eigenval);
	cvEigenVV(&covarianceMat, eigenvec, eigenval, 1);
	for(int j = 0; j < eigenval->cols; j++){
		// Access the obtained eigenvalues
		scalar = cvGet2D(eigenval, 0, j);
		cout << "EIG " << scalar.val[0] << "\n";
	}
	cvReleaseMat(&eigenvec); //house-cleaning :P 
	cvReleaseMat(&eigenval);
	*/



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
