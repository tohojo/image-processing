#include "pca_training_processor.h"
#include "util.h"

#include <QDebug>

PcaTrainingProcessor::PcaTrainingProcessor(QObject *parent)
: TwoImageProcessor(parent)
{
	numImages = 0;
	totalDataPointsPerImage = 0;
	numCompsToKeep = 5;
	pcaTrainingDone = false;
	pcaImageHeight = -1;
	pcaImageWidth = -1;
	dataPointsPerPixel = 3;
	use_HSV = true;
	usingDepth = false;
	error_threshold = 1.0;
	saveReconstructedImgs = true;
    uses_colour = true;
}


PcaTrainingProcessor::~PcaTrainingProcessor()
{
}


void PcaTrainingProcessor::run()
{

	forever {
		if(abort) return;
		emit progress(0);

		if(pcaTrainingDone || PCATrain()) { // If not trained, train first, then if there is an input image, try to classify it
			// PCATrain() returns true if PCA training is successfully done
			pcaTrainingDone = true;
			Mat result = pcaClassifyInputImage();
			if (! result.empty()){
				mutex.lock();
				output_image = result;
				qDebug() << "Finished PCA processing.\n";
			} else {
				qDebug() << "No input image to classify...\n";
			}
		} else { // PCATrain() returned false
			qDebug() << "PCA processing not performed.\n";
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
	QString useDepth_str = file.readLine(); // First line of file is whether or not to use depth maps - 'TRUE' or 'FALSE'
	QString numImgs_str = file.readLine(); // Second line of file is number of images
	QString pixPerImg_str = file.readLine(); // Third line of file is pixels per image
	int numImgs = numImgs_str.toInt(&ok1);
	int pixPerImg = pixPerImg_str.toInt(&ok2);
	std::string useDepth = useDepth_str.toStdString();
	if(!ok1 || !ok2) return false;
	numImages = numImgs;

	dataPointsPerPixel = 3;
	if (use_HSV) dataPointsPerPixel += 2;
	string sub = useDepth.substr(0,3);
	if (sub == "TRU" || sub == "Tru" || sub == "tru"){
		qDebug() << "Using depth map data for PCA.";
		usingDepth = true;
		dataPointsPerPixel  += 1; // USING THREE COLOUR CHANNELS + DEPTH MAP DATA
	} else {
		usingDepth = false;
		qDebug() << "Not using depth maps for PCA."; // USING THREE COLOUR CHANNELS
	}
	totalDataPointsPerImage = pixPerImg*dataPointsPerPixel;

	classesOfTrainingImages = std::vector<class_of_training_images>();

	cout << "numImages: " << numImages << "\n";
	cout << "Types of data considered per image pixel: " << dataPointsPerPixel << "\n";
	cout << "totalDataPointsPerImage: " << totalDataPointsPerImage << "\n";
	trainingSetImages = Mat(totalDataPointsPerImage, numImages, CV_32FC1); // Use float so we can subtract averages etc
	// Remainder of file is:
	// (a) image name on one line
	// (b) class to which the image belongs on next line
	// (c) corresponding depth map image name on next line IF depth is being used; ELSE no line
	// TXT FILE **MUST** HAVE A TRAILING NEWLINE!
	int counter = 0;
	while (file.canReadLine() && counter < numImages){
		QString qstr = file.readLine();
		string str = qstr.toStdString();
		str = str.erase(str.length()-1);
		cout << "Reading: " << str << "\n";
		Mat img = imread(str, 1); // Read in colour
		pcaImageWidth = img.cols;
		pcaImageHeight = img.rows;
		//
		Mat depth;
		if (usingDepth){
			QString depthQ = file.readLine();
			string dep = depthQ.toStdString();
			dep = dep.erase(dep.length()-1);
			cout << "Reading depth map: " << dep << "\n";
			depth = imread(dep, 0); // Read in depthmap
			if ((pcaImageWidth != depth.cols) || (pcaImageHeight != depth.rows)){
				cout << "ERROR: DEPTH MAP DOES NOT MATCH IMAGE DIMENSIONS!\n\n";
				assert(false);
			}
		}
		//
		QString qstr_num = file.readLine();
		string imageClass = qstr_num.toStdString();
		bool foundMatchingClass = false;
		std::vector<class_of_training_images>::iterator c2 = classesOfTrainingImages.begin();
		while (c2 != classesOfTrainingImages.end() && ! foundMatchingClass){
			if (c2->identifier == imageClass){
				foundMatchingClass = true;
				c2->images.push_back(img);
				c2->depthMap.push_back(depth); // If not using it, this is simply empty
			}
			c2++;
		}
		if (! foundMatchingClass){
			class_of_training_images coti;
			coti.identifier = imageClass;
			coti.images = std::vector<Mat>();
			coti.images.push_back(img);
			coti.depthMap.push_back(depth); // If not using it, this is simply empty
			classesOfTrainingImages.push_back(coti);
		}
		//
		Mat img2;
		if (usingDepth){
			img2 = convertImageToVector(img, depth); // Changes the 2D RGB (unsigned char) values to
			// 1D R,G,B,Depth (float) values, possibly +HSV if that is set
		} else {
			img2 = convertImageToVector(img); // Changes the 2D RGB (unsigned char) values to
			// 1D R,G,B, (float) values, possibly +HSV if that is set
		}
		for (int i = 0; i < totalDataPointsPerImage; i++){
			trainingSetImages.at<float>(i,counter) = img2.at<float>(i,0);
		}
		//
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
				cout << " " << trainingSetImages.at<float>(i,j);
			}
			cout << "\n";
		}
	}

	emit progress(30);

	return true;
}


Mat PcaTrainingProcessor::pcaClassifyInputImage(){
	if (input_image.empty() || input_image.rows != pcaImageHeight || input_image.cols != pcaImageWidth){
		return Mat();
	}
	if (usingDepth && (right_image.empty() || right_image.rows != pcaImageHeight || right_image.cols != pcaImageWidth)){
		return Mat();
	}

	// Methodology:
	// 1. Convert the input image to an eigenface
	// 2. Compare the input image to all stored class eigenvectors and output the correlation for each
	// 3. Choose the best and output the input image joined to that choice
	// 4. Modification: have the debug pane print the most appropriate.
	// 5. Modification: calculate the variance IN each class, and only classify if within X (error_threshold) of that.

	Mat imgVec;
	if (usingDepth){
		imgVec = convertImageToVector(input_image, right_image);
	} else {
		imgVec = convertImageToVector(input_image);
	}

	Mat eigenVec = pca.project(imgVec);

	emit progress(10);

	std::vector<float> errors = std::vector<float>();

	for (int i = 0; i < compressed_classes.cols; i++){
		Mat diff = eigenVec - compressed_classes.col(i);
		float d = 0.0;
		for (int j = 0; j < diff.rows; j++){
			d += (diff.at<float>(j,0) * diff.at<float>(j,0));
		}
		d = sqrt(d);
		errors.push_back(d);
		string cut = classesOfTrainingImages.at(i).identifier;
		cut = cut.erase(classesOfTrainingImages.at(i).identifier.length()-1);
		cout << "Difference between input image and \"" << cut << "\" : " << d << "\n";
		cout << "Error threshold for this class: " << classesOfTrainingImages.at(i).worstError
			<< " ; when modified " << (classesOfTrainingImages.at(i).worstError * error_threshold) << "\n";
	}

	emit progress(50);

	int index1 = -1; // If these don't change, the input is unclassifiable
	int index2 = -1;
	float minError = 1000000000;
	float nextMinError = 1000000000;
	for (unsigned int i = 0; i < errors.size(); i++){
		float err = errors.at(i);
		bool admissible = (err <= (classesOfTrainingImages.at(i).worstError * error_threshold));
		// MUST calculate 'admissible' dynamically, not store it as a variable for each class,
		// because we do not recalculate the PCA when we change 'error_threshold'.
		if (admissible && (err <= minError)){
			nextMinError = minError;
			index2 = index1;
			minError = err;
			index1 = i;
		} else if (admissible && (err <= nextMinError)){
			nextMinError = err;
			index2 = i;
		}
	}
	emit progress(80);

	Mat outputimg;
	if(index1 != -1){
		string cut = classesOfTrainingImages.at(index1).identifier;
		cut = cut.erase(classesOfTrainingImages.at(index1).identifier.length()-1);
		float conf = 100*(1 - (minError/(error_threshold * classesOfTrainingImages.at(index1).worstError)));
		qDebug() << "Best guess (closest matching class) is: " << cut.data() << " (error is " << minError << " distance from class eigenvector)";
		qDebug() << "Confidence in best guess is: " << (int) conf << "%";
		if (index2 != -1){
			string cut2 = classesOfTrainingImages.at(index2).identifier;
			cut2 = cut2.erase(classesOfTrainingImages.at(index2).identifier.length()-1);
			qDebug() << "Second-best guess (next closest match) is: " << cut2.data() << " (error is " << nextMinError << " distance from class eigenvector)";
			float conf_2 = 100*(1 - (nextMinError/(error_threshold * classesOfTrainingImages.at(index2).worstError)));
			qDebug() << "Confidence in second-best guess is: " << (int) conf_2 << "%";
		} else {
			qDebug() << "Error of second-best guess is too high to make it a possibility.";
		}

		outputimg = convertVectorToImage(reconstructed_classes.col(index1));
	} else { // Not classifiable
		qDebug() << "Image not classifiable as any of our principle classes!";
		Mat inputimg = imread("UNCLASSIFIED.png", 1);
		Size bluh = Size(input_image.cols, input_image.rows); // Width, height
		cv::resize(inputimg, outputimg, bluh);
	}
	emit progress(90);

	cout << "input_image.type " << input_image.type() << "\n";
	cout << "outputimg.type " << outputimg.type() << "\n";

	if (outputimg.empty()){ // Presumably because the 'not classifiable' image is absent
		outputimg = input_image.clone();
		outputimg.setTo(Scalar(0));
	}
	Mat rec_plus = Util::combine(input_image, outputimg);

	return rec_plus;
}


double PcaTrainingProcessor::getBaseline(Mat imgs, int def){
	PCA pca_baseline = PCA(imgs, Mat(), def);
	double sum = 0;
	for(int i = 0; i < pca_baseline.eigenvalues.rows; i++) {
		for(int j = 0; j < pca_baseline.eigenvalues.cols; j++) {
			sum += pca_baseline.eigenvalues.at<float>(i,j);
		}
	}
	return sum;
}


bool PcaTrainingProcessor::PCATrain(){

	if (trainingSetImages.empty()) return false;
	// 1. Get training images as vectors
	// Fills "trainingSetImages"; each COLUMN is an image vector.

	emit progress(40);

	double sumOfEigenvalues = getBaseline(trainingSetImages, CV_PCA_DATA_AS_COL);
	
	pca = PCA(trainingSetImages, Mat(), CV_PCA_DATA_AS_COL, numCompsToKeep);

	double keptSum = 0;
	cout << "\nOUTPUT EIGENVALUES:\n";
	for(int i = 0; i < pca.eigenvalues.rows; i++) {
		for(int j = 0; j < pca.eigenvalues.cols; j++) {
			std::cout << pca.eigenvalues.at<float>(i,j) << " \t";
			keptSum += pca.eigenvalues.at<float>(i,j);
		}
		cout << "\n";
	}

	double informationRetained = keptSum/sumOfEigenvalues;
	qDebug() << "Quantity of image information retained: " << informationRetained;

	cout << "\nEIGENVECTOR ROWS: " << pca.eigenvectors.rows << "\n";
	cout << "\nEIGENVECTOR COLS: " << pca.eigenvectors.cols << "\n";
	
	emit progress(50);

	qDebug() << "PCA training complete.";

	qDebug() << "Projecting training images into & from PCA space to compute error.";
	// Project all training images into the principal components space
	Mat compressed;
	compressed.create(numCompsToKeep, numImages, trainingSetImages.type());
	Mat reconstructed;
	for(int i = 0; i < trainingSetImages.cols; i++){
		// See how much we lose overall (per class is calculated later)
		Mat vect = trainingSetImages.col(i);
		Mat coeffs = compressed.col(i);
		// Compress the vector. The result will be stored in column i of the output matrix.
		pca.project(vect, coeffs);
		pca.backProject(coeffs, reconstructed);
		// Measuring the error:
		Mat diff = reconstructed - vect;
		float d = 0.0;
		/*for (int j = 0; j < diff.rows; j++){
			d += (diff.at<float>(j,0) * diff.at<float>(j,0));
		}
		d = sqrt(d);
		cout << "Error for training eigenface #" << i << ": " << d << "\n";*/
		// Output reconstructed image:
		std::string str = "PCA_projected";
		stringstream ss;
		ss << i;
		str.append(ss.str());
		str.append(".png");
		reconstructed = convertVectorToImage(reconstructed);
		if (saveReconstructedImgs){
			imwrite(str, reconstructed);
		}
	}

	emit progress(70);

	qDebug() << "Calculating mean eigenimage for each class.";
	compressed_classes = Mat(numCompsToKeep, classesOfTrainingImages.size(), trainingSetImages.type(), Scalar(0));
	reconstructed_classes = Mat(totalDataPointsPerImage, classesOfTrainingImages.size(), trainingSetImages.type(), Scalar(0));
	Mat temp_reconstructed;
	Mat overall_mean(totalDataPointsPerImage, 1, trainingSetImages.type(), Scalar(0));
	int overall_mean_num_images = 0;
	int count = 0;
	std::vector<class_of_training_images>::iterator it = classesOfTrainingImages.begin();
	for(; it != classesOfTrainingImages.end(); it++){
		float worstErrorForThisClass = 0.0;
		for(unsigned int itMat = 0; itMat < it->images.size(); itMat++){
			overall_mean_num_images++;
			Mat eigenvec;
			if (usingDepth){
				eigenvec = convertImageToVector(it->images.at(itMat), it->depthMap.at(itMat));
			} else {
				eigenvec = convertImageToVector(it->images.at(itMat));
			}
			Mat proj;
			proj.create(numCompsToKeep, 1, trainingSetImages.type());
			pca.project(eigenvec, proj);
			compressed_classes.col(count) = compressed_classes.col(count) + proj;
			// Now calculate error
			pca.backProject(proj, temp_reconstructed);
			Mat diff = temp_reconstructed - eigenvec;
			if(saveReconstructedImgs){
				overall_mean = overall_mean + eigenvec;
			}
			float d = 0.0;
			for (int j = 0; j < diff.rows; j++){
				d += (diff.at<float>(j,0) * diff.at<float>(j,0));
			}
			d = sqrt(d);
			if (d > worstErrorForThisClass) worstErrorForThisClass = d;
		}
		compressed_classes.col(count) = compressed_classes.col(count) / it->images.size(); // GET MEAN EIGENVECTOR FOR CLASS
		pca.backProject(compressed_classes.col(count), reconstructed_classes.col(count));
		std::string mean_str = "PCA_class_mean_";
		mean_str.append(it->identifier);
		mean_str = mean_str.erase(mean_str.length()-1);
		mean_str.append(".png");
		Mat columnMean = reconstructed_classes.col(count);
		Mat newColumnMean = columnMean.clone();
		if (saveReconstructedImgs){
			Mat imageMean = convertVectorToImage(newColumnMean);
			cout << "Writing " << mean_str << "\n";
			imwrite(mean_str, imageMean);
		}
		// Store worst error in the class_of_training_images vector
		it->worstError = worstErrorForThisClass;
		cout << "Worst error for this class: " << worstErrorForThisClass << "\n";
		count++;
	}
	emit progress(80);
	qDebug() << "Finished calculating means.";
	if(saveReconstructedImgs){
		overall_mean = overall_mean / ((float)(overall_mean_num_images));
		Mat meanImg = convertVectorToImage(overall_mean);
		imwrite("OVERALL_PCA_MEAN.png", meanImg);
	}

	emit progress(90);



	// Ignore all this
	bool useOurWork = false;
	if(useOurWork){
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

		cout << "\nCentred data:\n";
		for (int i = 0; i < trainingSetImages.rows; i++){
			for (int j = 0; j < trainingSetImages.cols; j++){
				cout << " " << trainingSetImages.at<float>(i,j);
			}
			cout << "\n";
		}

		// 2. Compute covariance matrix of centred dataset.
		// C = sum[1...N] of yi * yi(transpose)
		Mat covarianceMat = Mat(totalDataPointsPerImage, totalDataPointsPerImage, CV_32FC1, Scalar(0));
		for (int i = 0; i < numImages; i++){
			Mat matToAdd = trainingSetImages.col(i) * trainingSetImages.col(i).t();
			covarianceMat = covarianceMat + matToAdd;
		}

		cout << "\nCovariance matrix:\n";
		for (int i = 0; i < covarianceMat.rows; i++){
			for (int j = 0; j < covarianceMat.cols; j++){
				cout << " " << covarianceMat.at<float>(i,j);
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
				std::cout << eigenvalues.at<float>(i,j) << " \t";
			}
			cout << "\n";
		}
		cout << "\nOUTPUT EIGENVECTORS:\n";
		for(int i = 0; i < eigenvectors.rows; i++) {
			for(int j = 0; j < eigenvectors.cols; j++) {
				std::cout << eigenvectors.at<float>(i,j) << " \t";
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


// USE THIS FOR PCA USING RGB (3) OR RGB+HSV (5)
Mat PcaTrainingProcessor::convertImageToVector(Mat img){
	if (dataPointsPerPixel != 3 && dataPointsPerPixel != 5){
		// Should be using '3' for one matrix of RGB or '5' for one matrix of RGB to be converted to HSV!
		qDebug() << "Problem: " << dataPointsPerPixel << " channels required for PCA does not match the single image submitted!";
		assert(false);
	}
	Mat reshaped = img.reshape(img.channels(), img.total());
	Mat vec = Mat(totalDataPointsPerImage, 1, CV_32FC1); // Single channel, 1 column containing all image data
	if (dataPointsPerPixel == 3){
		for (int i = 0; i < reshaped.rows; i++){
			Vec3b colourPixel = reshaped.at<Vec3b>(i,0);
			vec.at<float>(i*3, 0) = (float)(colourPixel[0]);
			vec.at<float>(i*3+1, 0) = (float)(colourPixel[1]);
			vec.at<float>(i*3+2, 0) = (float)(colourPixel[2]);
		}
		return vec;
	} else if (dataPointsPerPixel == 5){
		Mat convertedToHSV;
		cv::cvtColor(reshaped, convertedToHSV, CV_BGR2HSV); // OPENCV USES BGR BY DEFAULT
		for (int i = 0; i < reshaped.rows; i++){
			Vec3b colourPixel = reshaped.at<Vec3b>(i,0);
			vec.at<float>(i*5, 0) = (float)(colourPixel[0]);
			vec.at<float>(i*5+1, 0) = (float)(colourPixel[1]);
			vec.at<float>(i*5+2, 0) = (float)(colourPixel[2]);
			Vec3b hsvPixel = convertedToHSV.at<Vec3b>(i,0);
			vec.at<float>(i*5+3, 0) = (float)(hsvPixel[0]);
			vec.at<float>(i*5+4, 0) = (float)(hsvPixel[1]);
		}
		return vec;
	}
	return Mat();
}

// USE THIS FOR PCA USING RGB+DEPTH (4) OR RGB+DEPTH+HSV (6)
Mat PcaTrainingProcessor::convertImageToVector(Mat img, Mat depthImg){
	if ((dataPointsPerPixel != 4) && (dataPointsPerPixel != 6)){
		// Should be using '4' for one matrix of RGB + one of depth
		// or '5' for one matrix of RGB to be converted to HSV + one of depth!
		qDebug() << "Problem: " << dataPointsPerPixel << " channels required for PCA does not match the two images submitted!";
		assert(false);
	}
	Mat reshaped = img.reshape(img.channels(), img.total());
	Mat reshapedDepth = depthImg.reshape(depthImg.channels(), depthImg.total());
	Mat vec = Mat(totalDataPointsPerImage, 1, CV_32FC1); // Single channel, 1 column containing all image data
	if (dataPointsPerPixel == 4){ // Three colours + depth used in PCA
		for (int i = 0; i < reshaped.rows; i++){
			Vec3b colourPixel = reshaped.at<Vec3b>(i,0);
			vec.at<float>(i*4, 0) = (float)(colourPixel[0]);
			vec.at<float>(i*4+1, 0) = (float)(colourPixel[1]);
			vec.at<float>(i*4+2, 0) = (float)(colourPixel[2]);
			vec.at<float>(i*4+3, 0) = (float)(reshapedDepth.at<unsigned char>(i,0)); // DEPTH MAP DATA
		}
		return vec;
	} else if (dataPointsPerPixel == 6){ // Three colours + depth + HSV
		Mat convertedToHSV;
		cv::cvtColor(reshaped, convertedToHSV, CV_BGR2HSV);
		for (int i = 0; i < reshaped.rows; i++){
			Vec3b colourPixel = reshaped.at<Vec3b>(i,0);
			vec.at<float>(i*6, 0) = (float)(colourPixel[0]);
			vec.at<float>(i*6+1, 0) = (float)(colourPixel[1]);
			vec.at<float>(i*6+2, 0) = (float)(colourPixel[2]);
			Vec3b hsvPixel = convertedToHSV.at<Vec3b>(i,0);
			vec.at<float>(i*6+3, 0) = (float)(hsvPixel[0]);
			vec.at<float>(i*6+4, 0) = (float)(hsvPixel[1]);
			vec.at<float>(i*6+5, 0) = (float)(reshapedDepth.at<unsigned char>(i,0)); // DEPTH MAP DATA
		}
		return vec;
	}
	return Mat();
}

Mat PcaTrainingProcessor::convertVectorToImage(Mat vec){
	Mat base(pcaImageWidth*pcaImageHeight,1,CV_8UC3);
	int i = 0;
	while(i < vec.rows){
		Vec3b colourPixel( (unsigned char) (vec.at<float>(i,0)),
			(unsigned char) (vec.at<float>(i+1,0)),
			(unsigned char) (vec.at<float>(i+2,0)) );
		base.at<Vec3b>(i/dataPointsPerPixel, 0) = colourPixel;
		i += dataPointsPerPixel;
	}
	base = base.reshape(3, pcaImageHeight);
	return base;
}

void PcaTrainingProcessor::setFileList(QFileInfo path)
{
	QMutexLocker locker(&mutex);
	if(path.canonicalFilePath() == file_list.canonicalFilePath()) return;
	file_list = path;
	mutex.unlock();
	loadImages();
	pcaTrainingDone = false;
	Processor::process();
}

void PcaTrainingProcessor::setUseHSV(bool yesno){
	QMutexLocker locker(&mutex);
	use_HSV = yesno;
	mutex.unlock();
	loadImages();
	pcaTrainingDone = false;
	Processor::process();
}

void PcaTrainingProcessor::setNumComponentsToKeep(int num)
{
	QMutexLocker locker(&mutex);
	numCompsToKeep = num;
	mutex.unlock();
	loadImages();
	pcaTrainingDone = false;
	Processor::process();
}

void PcaTrainingProcessor::setErrorThreshold(float thresh)
{
	QMutexLocker locker(&mutex);
	error_threshold = thresh;
	mutex.unlock();
	Processor::process();
}

void PcaTrainingProcessor::setSaveEigenMeans(bool yesno){
	QMutexLocker locker(&mutex);
	saveReconstructedImgs = yesno;
	mutex.unlock();
	loadImages();
	pcaTrainingDone = false;
	Processor::process();
}

