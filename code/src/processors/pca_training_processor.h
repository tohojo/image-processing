#ifndef OUR_PCA_TRAINING_PROCESSOR_H
#define OUR_PCA_TRAINING_PROCESSOR_H

#include "processor.h"
#include <highgui.h>
#include <QtCore/QFileInfo>

using namespace cv;
using namespace std;


struct class_of_training_images {
	std::string identifier; // Any string used in the input txt file to identify them
	std::vector<Mat> images; // In original 2D form, not vector form
};


class PcaTrainingProcessor : public Processor
{
	Q_OBJECT

		Q_PROPERTY(QFileInfo FileList READ fileList WRITE setFileList USER true)
		Q_CLASSINFO("FileList", "filetype=text;")
		Q_PROPERTY(int NumComponentsToKeep READ numComponentsToKeep WRITE setNumComponentsToKeep USER true)
		Q_PROPERTY(bool UseHSV READ useHSV WRITE setUseHSV USER true)

public:
	PcaTrainingProcessor(QObject *parent = 0);
	~PcaTrainingProcessor();

	QString name() {return "PCA training";}

	QFileInfo fileList() {QMutexLocker l(&mutex); return file_list;}
	void setFileList(QFileInfo path);

	bool PCATrain();

	void run();
	void set_input(const Mat img);

	//	void testProgram(double smoothWeight, int mult, const char * lOut, const char * rOut, const char * lIn, const char * rIn);
	//	double testStereoResults(const char * testImageName, const char * idealImageName);

private:

	bool pcaTrainingDone;

	Mat compressed_classes;
	Mat reconstructed_classes;
	PCA pca;

	Mat trainingSetImages;
	int numImages;
	int pcaImageWidth;
	int pcaImageHeight;
	int dataPointsPerPixel;
	int totalDataPointsPerImage; // pcaImageWidth * pcaImageHeight * dataPointsPerPixel
	
	std::vector<class_of_training_images> classesOfTrainingImages; // Number of elements = number of classes
	// Each element stores the name for that class and all training images in that class

	bool loadImages();
	Mat convertImageToVector(Mat img);
	Mat convertImageToVector(Mat img, Mat depthImg);
	Mat convertVectorToImage(Mat vec);
	Mat pcaClassifyInputImage();

	int numCompsToKeep;
	float numComponentsToKeep() {QMutexLocker l(&mutex); return numCompsToKeep;}
	void setNumComponentsToKeep(int num);

	bool use_HSV;
	float useHSV() {QMutexLocker l(&mutex); return use_HSV;}
	void setUseHSV(bool yesno);

	QFileInfo file_list;

};

#endif // OUR_PCA_TRAINING_PROCESSOR_H
