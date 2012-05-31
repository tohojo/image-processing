#ifndef STEREO_PROCESSOR_H
#define STEREO_PROCESSOR_H

#include "two_image_processor.h"
#include <highgui.h>
#include <QtCore/QFileInfo>

using namespace cv;

class StereoProcessor : public TwoImageProcessor
{
	Q_OBJECT

	Q_PROPERTY(int MatrixSize READ matrixLength WRITE setMatrixLength USER true)
	Q_PROPERTY(int DispBounds READ disparityBounds WRITE setDisparityBounds USER true)
	Q_PROPERTY(int HardMultiplier READ hardMultiplier WRITE setHardMultiplier USER true)
	Q_PROPERTY(double VerticalSmoothness READ smoothnessWeight WRITE setSmoothnessWeight USER true)
	Q_PROPERTY(double WeightPorcupine READ weightPorcupine WRITE setWeightPorcupine USER true)
	Q_PROPERTY(bool AutoWriteOutput READ getAutoOutput WRITE setAutoOutput USER true)

public:
	StereoProcessor(QObject *parent = 0);
	~StereoProcessor();

	QString name() {return "Stereo matching";}

	Mat medianFilter(Mat * mat, int filtersize);

	int matrixLength() {QMutexLocker locker(&mutex); return denoise_matrix_length;}
	void setMatrixLength(const int mat_length);

	int disparityBounds() {QMutexLocker locker(&mutex); return max_expected_disparity_bounds;}
	void setDisparityBounds(const int a);

	int hardMultiplier() {QMutexLocker locker(&mutex); return hard_multiplier;}
	void setHardMultiplier(const int a);

	double smoothnessWeight() {QMutexLocker locker(&mutex); return smoothness_weight;}
	void setSmoothnessWeight(const double a);

	double weightPorcupine() {QMutexLocker locker(&mutex); return weight_porcupine;}
	void setWeightPorcupine(const double a);

	bool autoOutput;
	bool getAutoOutput() {QMutexLocker locker(&mutex); return autoOutput;}
	void setAutoOutput(const bool yesno);

	bool dynamicProgramming(const char * lName, const char * rName, Mat left_input, Mat right_input);
	void run();

	void testProgram(bool autoOut, double smoothWeight, int mult, const char * lOut, const char * rOut, const char * lIn, const char * rIn);
	double testStereoResults(const char * testImageName, const char * idealImageName);

private:
	Mat initial_leftDepthMap; // Takes integer values
	Mat initial_rightDepthMap; // Takes integer values
	Mat initial_leftDepthMap_B; // Takes integer values
	Mat initial_rightDepthMap_B; // Takes integer values
	Mat correctedLeftDepthMap; // Mapped to 0...255 for output
	Mat correctedRightDepthMap; // Mapped to 0...255 for output
	Mat A; // dynamic programming matrix
	Mat A_b; // backwards dynamic programming matrix
	Mat prev_path_F; // optimising matrix
	Mat prev_path_B; // optimising matrix
	Mat costMat;

	// PARAMETERS
	int denoise_matrix_length; // Must be odd number. 0 = no de-noise-ing
	int max_expected_disparity_bounds; // For efficiency.
	int hard_multiplier; // For middlebury tests etc where the distribution is known.
	double smoothness_weight; // Weight of the previous scanline. Should be [0 to 0.5].
	// If set to 0, no smoothing. If set to 0.5, this scanline and the previous are weighted equally.
	// 0.875 was recommended for the former form.	// CURRENTLY BROKEN, DON'T USE
	double weight_porcupine;

};

#endif
