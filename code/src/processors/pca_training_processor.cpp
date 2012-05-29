#include "pca_training_processor.h"
#include "util.h"

#include <QDebug>

PcaTrainingProcessor::PcaTrainingProcessor(QObject *parent)
: Processor(parent)
{
	numImages = 0;
	pixelsPerImage = 0;
	numCompsToKeep = 10;
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
			qDebug() << "Finished PCA processing.\n";
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

	classesOfTrainingImages = std::vector<class_of_training_images>();

	cout << "numImages: " << numImages << "\n";
	cout << "pixelsPerImage: " << pixelsPerImage << "\n";
	trainingSetImages = Mat(pixelsPerImage, numImages, CV_64FC1); // Use double so we can subtract averages etc
	// Remainder of file is: image name on one line, class to which the image belongs on next line
	// **MUST** HAVE A TRAILING NEWLINE!
	int counter = 0;
	while (file.canReadLine() && counter < numImages){
		QString qstr = file.readLine();
		string str = qstr.toStdString();
		str = str.erase(str.length()-1);
		cout << "Reading: " << str << "\n";
		Mat img = imread(str, 1); // now in colour
		pcaImageWidth = img.cols;
		pcaImageHeight = img.rows;
		Mat img2 = convertImageToVector(img);
		for (int i = 0; i < pixelsPerImage; i++){
			trainingSetImages.at<double>(i,counter) = (0.0 + img2.at<unsigned char>(i,0));
		}
		QString qstr_num = file.readLine();
		string imageClass = qstr_num.toStdString();
		bool foundMatchingClass = false;
		std::vector<class_of_training_images>::iterator c2 = classesOfTrainingImages.begin();
		while (c2 != classesOfTrainingImages.end() && ! foundMatchingClass){
			if (c2->identifier == imageClass){
				foundMatchingClass = true;
				c2->images.push_back(img);
			}
			c2++;
		}
		if (! foundMatchingClass){
			class_of_training_images coti;
			coti.identifier = imageClass;
			coti.images = std::vector<Mat>();
			coti.images.push_back(img);
			classesOfTrainingImages.push_back(coti);
		}
		counter++;
	}

	cout << "Printing set of classes for training images.\n";
	std::vector<class_of_training_images>::iterator it = classesOfTrainingImages.begin();
	for(; it != classesOfTrainingImages.end(); it++){
		cout << "CLASS: " << it->identifier << " ; NUMBER OF IMAGES: " << it->images.size() << "\n";
	}
	cout << "\n";

	cout << "Training set rows: " << trainingSetImages.rows << "\n";
	cout << "Training set cols: " << trainingSetImages.cols << "\n";

	if (trainingSetImages.rows < 20 && trainingSetImages.cols < 20){
		for (int i = 0; i < trainingSetImages.rows; i++){
			for (int j = 0; j < trainingSetImages.cols; j++){
				cout << " " << trainingSetImages.at<double>(i,j);
			}
			cout << "\n";
		}
	}

	return true;
}


bool PcaTrainingProcessor::PCATrain(){

	if (trainingSetImages.empty()) return false;
	// 1. Get training images as vectors
	// Fills "trainingSetImages"; each COLUMN is an image vector.

	PCA pca(trainingSetImages, Mat(), CV_PCA_DATA_AS_COL, numCompsToKeep);
	cout << "\nOUTPUT EIGENVALUES:\n";
	for(int i = 0; i < pca.eigenvalues.rows; i++) {
		for(int j = 0; j < pca.eigenvalues.cols; j++) {
			std::cout << pca.eigenvalues.at<double>(i,j) << " \t";
		}
		cout << "\n";
	}

	cout << "\nEIGENVECTOR ROWS: " << pca.eigenvectors.rows << "\n";
	cout << "\nEIGENVECTOR COLS: " << pca.eigenvectors.cols << "\n";
	if (pca.eigenvectors.cols < 100){ // Number of data points per image, e.g. #pixels
		cout << "\nOUTPUT EIGENVECTORS:\n";
		for(int i = 0; i < pca.eigenvectors.rows; i++) {
			for(int j = 0; j < pca.eigenvectors.cols; j++) {
				std::cout << pca.eigenvectors.at<double>(i,j) << " \t";
			}
			cout << "\n";
		}
	}

	qDebug() << "PCA training complete.";

	// Project all training images into the principal components space
	Mat compressed;
	compressed.create(numCompsToKeep, numImages, trainingSetImages.type());
	Mat reconstructed;

	/*
	qDebug() << "Calculating error for training images.";
	std::vector<class_of_training_images>::iterator it = classesOfTrainingImages.begin();
	for(; it != classesOfTrainingImages.end(); it++){
	std::vector<class_of_training_images>::iterator itMat = it->images.begin();
	for(; itMat != it->images.end(); itMat++){
	// See how much we lose
	Mat vect = trainingSetImages.col(i), coeffs = compressed.col(i);

	}
	}
	*/

	for(int i = 0; i < trainingSetImages.cols; i++){
		// See how much we lose
		Mat vect = trainingSetImages.col(i), coeffs = compressed.col(i);
		// Compress the vector. The result will be stored in column i of the output matrix.
		pca.project(vect, coeffs);
		pca.backProject(coeffs, reconstructed);
		// Measuring the error:
		Mat diff = reconstructed - vect;
		double d = 0.0;
		for (int j = 0; j < diff.rows; j++){
			d += (diff.at<double>(j,0) * diff.at<double>(j,0));
		}
		cout << "Error for training eigenface #" << i << ": " << sqrt(d) << "\n";
		// Output reconstructed image:
		std::string str = "PCA_projected";
		str.append(i, '1');
		str.append(".png");
		//cout << "MAT ELEMENTS: " << reconstructed.total() << "\n" << "NEW ROWS: " << pcaImageHeight << "\n";
		reconstructed = convertVectorToImage(reconstructed);
		imwrite(str, reconstructed);
	}





	// Ignore all this
	bool useOurWork = false;
	if(useOurWork){
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
			Mat matToAdd = trainingSetImages.col(i) * trainingSetImages.col(i).t();
			covarianceMat = covarianceMat + matToAdd;
		}

		cout << "\nCovariance matrix:\n";
		for (int i = 0; i < covarianceMat.rows; i++){
			for (int j = 0; j < covarianceMat.cols; j++){
				cout << " " << covarianceMat.at<double>(i,j);
			}
			cout << "\n";
		}

		/*
		// Easy test
		Mat covarianceMat2 = Mat(2, 2, CV_64FC1, Scalar(0));
		Mat x1 = Mat(2, 1, CV_64FC1);
		x1.at<double>(0,0) = 0;
		x1.at<double>(1,0) = -1;
		Mat x2 = Mat(2, 1, CV_64FC1);
		x2.at<double>(0,0) = 1;
		x2.at<double>(1,0) = -1;
		Mat x3 = Mat(2, 1, CV_64FC1);
		x3.at<double>(0,0) = 2;
		x3.at<double>(1,0) = -1;
		Mat x4 = Mat(2, 1, CV_64FC1);
		x4.at<double>(0,0) = -1;
		x4.at<double>(1,0) = 0;
		Mat x5 = Mat(2, 1, CV_64FC1);
		x5.at<double>(0,0) = -1;
		x5.at<double>(1,0) = 1;
		Mat x6 = Mat(2, 1, CV_64FC1);
		x6.at<double>(0,0) = -1;
		x6.at<double>(1,0) = 2;
		covarianceMat2 = covarianceMat2 + x1.col(0) * x1.col(0).t();
		covarianceMat2 = covarianceMat2 + x2.col(0) * x2.col(0).t();
		covarianceMat2 = covarianceMat2 + x3.col(0) * x3.col(0).t();
		covarianceMat2 = covarianceMat2 + x4.col(0) * x4.col(0).t();
		covarianceMat2 = covarianceMat2 + x5.col(0) * x5.col(0).t();
		covarianceMat2 = covarianceMat2 + x6.col(0) * x6.col(0).t();
		Mat V, E;
		cout << "\nSimple case... ";
		eigen(covarianceMat2, E, V);
		cout << "done.\n";
		cout << "\nTEST EIGENVALUES:\n";
		cout << "TOTAL NUMBER OF EIGENVALUES: " << E.total() << "\n";
		for(int i = 0; i < E.rows; i++) {
		for(int j = 0; j < E.cols; j++) {
		std::cout << E.at<double>(i,j) << " \t";
		}
		cout << "\n";
		}
		cout << "\nTEST EIGENVECTORS:\n";
		for(int i = 0; i < V.rows; i++) {
		for(int j = 0; j < V.cols; j++) {
		std::cout << V.at<double>(i,j) << " \t";
		}
		cout << "\n";
		}
		*/


		// 3. Find eigenvectors and eigenvalues of covariance matrix.
		// UNIT eigenvectors = principal directions.
		// Projections of image vector x to principal directions = principal components of x.
		Mat eigenvalues, eigenvectors;
		cout << "\nRunning 'eigen' function... ";
		eigen(covarianceMat, eigenvalues, eigenvectors);
		cout << "done.\n\n";
		cout << "TOTAL NUMBER OF EIGENVALUES: " << eigenvalues.total();

		// Print eigenvalues, which are sorted descending
		cout << "\nOUTPUT EIGENVALUES:\n";
		for(int i = 0; i < eigenvalues.rows; i++) {
			for(int j = 0; j < eigenvalues.cols; j++) {
				std::cout << eigenvalues.at<double>(i,j) << " \t";
			}
			cout << "\n";
		}
		cout << "\nOUTPUT EIGENVECTORS:\n";
		for(int i = 0; i < eigenvectors.rows; i++) {
			for(int j = 0; j < eigenvectors.cols; j++) {
				std::cout << eigenvectors.at<double>(i,j) << " \t";
			}
			cout << "\n";
		}
		cout << "\n";

		// 2. Calculate eigenface vectors from training set.
		// 3. Keep only the M eigenvectors corresponding to the highest eigenvalues.
		//
	}
	return true;
}


Mat PcaTrainingProcessor::convertImageToVector(Mat img){
	//cout << "MAT ELEMENTS: " << img.total() << "\n" << "NEW ROWS: " << pixelsPerImage << "\n";
	Mat vec = img.reshape(img.channels(), img.total());
	return vec;
}

Mat PcaTrainingProcessor::convertVectorToImage(Mat vec){
	//cout << "VEC ELEMENTS: " << vec.total() << "\n" << "NEW ROWS: " << pcaImageHeight << "\n";
	Mat img = vec.reshape(vec.channels(), pcaImageHeight);
	return img;
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

void PcaTrainingProcessor::setNumComponentsToKeep(int num)
{
	QMutexLocker locker(&mutex);
	numCompsToKeep = num;
	mutex.unlock();
	loadImages();
	Processor::process();
}
