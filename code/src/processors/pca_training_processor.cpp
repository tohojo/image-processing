#include "pca_training_processor.h"
#include "util.h"

#include <QDebug>

PcaTrainingProcessor::PcaTrainingProcessor(QObject *parent)
: Processor(parent)
{
	//
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

	cout << "numImgs: " << numImgs << "\n";
	cout << "pixPerImg: " << pixPerImg << "\n";
	trainingSetImages = Mat(pixPerImg, numImgs, CV_32F); // Use floats so we can subtract averages etc
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
		for (int i = 0; i < img.rows; i++){
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
	Mat covarianceMat;

	// 3. Find eigenvectors and eigenvalues of covariance matrix.
	// UNIT eigenvectors = principal directions.
	// Projections of image vector x to principal directions = principal components of x.

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
