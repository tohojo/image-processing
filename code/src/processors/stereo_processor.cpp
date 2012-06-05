#include "stereo_processor.h"
#include "util.h"
#include <fstream>

#include <QDebug>

StereoProcessor::StereoProcessor(QObject *parent)
: TwoImageProcessor(parent)
{
	denoise_matrix_length = 3;
	max_expected_disparity_bounds = 20;
	hard_multiplier = -1;
	smoothness_weight = 0.4; // Sometimes works.
	denoise_matrix_length = 3; // Must be odd number. 0 = no de-noise-ing
	weight_porcupine = 1.0;
	autoOutput = false;
}


StereoProcessor::~StereoProcessor()
{
}


void StereoProcessor::run()
{

	// Please ignore these stupid hacks :3
	/*for (int i = 0; i < 44; i+=2){
		std::string left = "PCA_NEW_128_DEPTH/databaseimage";
		std::string right = "PCA_NEW_128_DEPTH/databaseimage";
		std::stringstream ss;
		ss << i;
		left.append(ss.str());
		std::stringstream ss2;
		ss2 << (i+1);
		right.append(ss2.str());
		std::stringstream leftout;
		leftout << left;
		leftout << "D.png";
		std::stringstream rightout;
		rightout << right;
		rightout << "D.png";
		left.append(".png");
		right.append(".png");
		testProgram(false, 0.5, 6, leftout.str().c_str(), rightout.str().c_str(), left.c_str(), right.c_str());
	}
	setMatrixLength(3);
	max_expected_disparity_bounds = 30;
	weight_porcupine = 1.01;
	for (int i = 4071; i <= 4114; i++){
		std::string left = "norm_0.25_1.2/DSCF";
		std::string right = "norm_0.25_1.2/DSCF";
		std::stringstream ss;
		ss << i;
		left.append(ss.str());
		right.append(ss.str());
		std::stringstream leftout;
		leftout << left;
		leftout << "rec_l.normalD_smth.png";
		std::stringstream rightout;
		rightout << right;
		rightout << "rec_r.normalD_smth.png";
		left.append("rec_l.normal.png");
		right.append("rec_r.normal.png");
		testProgram(false, 0.7, 7, leftout.str().c_str(), rightout.str().c_str(), left.c_str(), right.c_str());
	}
	setMatrixLength(5);
	max_expected_disparity_bounds = 30;
	weight_porcupine = 1.015;
	for (int i = 4071; i <= 4114; i++){
		std::string left = "norm_0.25_1.2/DSCF";
		std::string right = "norm_0.25_1.2/DSCF";
		std::stringstream ss;
		ss << i;
		left.append(ss.str());
		right.append(ss.str());
		std::stringstream leftout;
		leftout << left;
		leftout << "rec_l.normalD_xtr_smth.png";
		std::stringstream rightout;
		rightout << right;
		rightout << "rec_r.normalD_xtr_smth.png";
		left.append("rec_l.normal.png");
		right.append("rec_r.normal.png");
		testProgram(false, 0.8, 7, leftout.str().c_str(), rightout.str().c_str(), left.c_str(), right.c_str());
	}*/

	forever {
		if(abort) return;
		emit progress(0);

		if( dynamicProgramming("Left-Disparity-Map.png", "Right-Disparity-Map.png", input_image, right_image) ) { // Returns true if successful
			mutex.lock();
			qDebug() << "OUTPUT = LEFT DEPTH MAP\n";
			output_image = Util::combine(correctedLeftDepthMap,correctedRightDepthMap);
		} else {
			mutex.lock();
			output_image = right_image;
		}

		emit progress(100);
		emit updated();
		if(once) return;

		if(!restart)
			condition.wait(&mutex);
		restart = false;
		mutex.unlock();
	}


	// INITIAL TEST: HARDMULT / CALCULATED MULT
	// Proceed thereafter under assumption of HARDMULT
	// FURTHER TESTS: LEFT/RIGHT, SMOOTHED/NOT, MEDIAN MATRIX LENGTH
	/*
	qDebug() << "STARTING TESTING.";
	setMatrixLength(0);
	testProgram(0.4, -1, "tests/con_imL_mat0_dyn_smooth.png", "tests/con_imR_mat0_dyn_smooth.png", "tests/con_imL.png", "tests/con_imR.png");
	testProgram(0.4, -1, "tests/ted_imL_mat0_dyn_smooth.png", "tests/ted_imR_mat0_dyn_smooth.png", "tests/ted_imL.png", "tests/ted_imR.png");
	testProgram(0.4, -1, "tests/tsu_imL_mat0_dyn_smooth.png", "tests/tsu_imR_mat0_dyn_smooth.png", "tests/tsu_imL.png", "tests/tsu_imR.png");
	testProgram(0.4, -1, "tests/ven_imL_mat0_dyn_smooth.png", "tests/ven_imR_mat0_dyn_smooth.png", "tests/ven_imL.png", "tests/ven_imR.png");
	testProgram(0.0, -1, "tests/con_imL_mat0_dyn.png", "tests/con_imR_mat0_dyn.png", "tests/con_imL.png", "tests/con_imR.png");
	testProgram(0.0, -1, "tests/ted_imL_mat0_dyn.png", "tests/ted_imR_mat0_dyn.png", "tests/ted_imL.png", "tests/ted_imR.png");
	testProgram(0.0, -1, "tests/tsu_imL_mat0_dyn.png", "tests/tsu_imR_mat0_dyn.png", "tests/tsu_imL.png", "tests/tsu_imR.png");
	testProgram(0.0, -1, "tests/ven_imL_mat0_dyn.png", "tests/ven_imR_mat0_dyn.png", "tests/ven_imL.png", "tests/ven_imR.png");
	qDebug() << "Set 2.";
	//
	setMatrixLength(0);
	testProgram(0.4, 4, "tests/con_imL_mat0_hardmult_smooth.png", "tests/con_imR_mat0_hardmult_smooth.png", "tests/con_imL.png", "tests/con_imR.png");
	testProgram(0.4, 4, "tests/ted_imL_mat0_hardmult_smooth.png", "tests/ted_imR_mat0_hardmult_smooth.png", "tests/ted_imL.png", "tests/ted_imR.png");
	testProgram(0.4, 16, "tests/tsu_imL_mat0_hardmult_smooth.png", "tests/tsu_imR_mat0_hardmult_smooth.png", "tests/tsu_imL.png", "tests/tsu_imR.png");
	testProgram(0.4, 8, "tests/ven_imL_mat0_hardmult_smooth.png", "tests/ven_imR_mat0_hardmult_smooth.png", "tests/ven_imL.png", "tests/ven_imR.png");
	testProgram(0.0, 4, "tests/con_imL_mat0_hardmult.png", "tests/con_imR_mat0_hardmult.png", "tests/con_imL.png", "tests/con_imR.png");
	testProgram(0.0, 4, "tests/ted_imL_mat0_hardmult.png", "tests/ted_imR_mat0_hardmult.png", "tests/ted_imL.png", "tests/ted_imR.png");
	testProgram(0.0, 16, "tests/tsu_imL_mat0_hardmult.png", "tests/tsu_imR_mat0_hardmult.png", "tests/tsu_imL.png", "tests/tsu_imR.png");
	testProgram(0.0, 8, "tests/ven_imL_mat0_hardmult.png", "tests/ven_imR_mat0_hardmult.png", "tests/ven_imL.png", "tests/ven_imR.png");
	qDebug() << "Set 3.";
	//
	setMatrixLength(3);
	testProgram(0.4, 4, "tests/con_imL_mat3_hardmult_smooth.png", "tests/con_imR_mat3_hardmult_smooth.png", "tests/con_imL.png", "tests/con_imR.png");
	testProgram(0.4, 4, "tests/ted_imL_mat3_hardmult_smooth.png", "tests/ted_imR_mat3_hardmult_smooth.png", "tests/ted_imL.png", "tests/ted_imR.png");
	testProgram(0.4, 16, "tests/tsu_imL_mat3_hardmult_smooth.png", "tests/tsu_imR_mat3_hardmult_smooth.png", "tests/tsu_imL.png", "tests/tsu_imR.png");
	testProgram(0.4, 8, "tests/ven_imL_mat3_hardmult_smooth.png", "tests/ven_imR_mat3_hardmult_smooth.png", "tests/ven_imL.png", "tests/ven_imR.png");
	testProgram(0.0, 4, "tests/con_imL_mat3_hardmult.png", "tests/con_imR_mat3_hardmult.png", "tests/con_imL.png", "tests/con_imR.png");
	testProgram(0.0, 4, "tests/ted_imL_mat3_hardmult.png", "tests/ted_imR_mat3_hardmult.png", "tests/ted_imL.png", "tests/ted_imR.png");
	testProgram(0.0, 16, "tests/tsu_imL_mat3_hardmult.png", "tests/tsu_imR_mat3_hardmult.png", "tests/tsu_imL.png", "tests/tsu_imR.png");
	testProgram(0.0, 8, "tests/ven_imL_mat3_hardmult.png", "tests/ven_imR_mat3_hardmult.png", "tests/ven_imL.png", "tests/ven_imR.png");
	qDebug() << "Set 4.";
	//
	setMatrixLength(5);
	testProgram(0.4, 4, "tests/con_imL_mat5_hardmult_smooth.png", "tests/con_imR_mat5_hardmult_smooth.png", "tests/con_imL.png", "tests/con_imR.png");
	testProgram(0.4, 4, "tests/ted_imL_mat5_hardmult_smooth.png", "tests/ted_imR_mat5_hardmult_smooth.png", "tests/ted_imL.png", "tests/ted_imR.png");
	testProgram(0.4, 16, "tests/tsu_imL_mat5_hardmult_smooth.png", "tests/tsu_imR_mat5_hardmult_smooth.png", "tests/tsu_imL.png", "tests/tsu_imR.png");
	testProgram(0.4, 8, "tests/ven_imL_mat5_hardmult_smooth.png", "tests/ven_imR_mat5_hardmult_smooth.png", "tests/ven_imL.png", "tests/ven_imR.png");
	testProgram(0.0, 4, "tests/con_imL_mat5_hardmult.png", "tests/con_imR_mat5_hardmult.png", "tests/con_imL.png", "tests/con_imR.png");
	testProgram(0.0, 4, "tests/ted_imL_mat5_hardmult.png", "tests/ted_imR_mat5_hardmult.png", "tests/ted_imL.png", "tests/ted_imR.png");
	testProgram(0.0, 16, "tests/tsu_imL_mat5_hardmult.png", "tests/tsu_imR_mat5_hardmult.png", "tests/tsu_imL.png", "tests/tsu_imR.png");
	testProgram(0.0, 8, "tests/ven_imL_mat5_hardmult.png", "tests/ven_imR_mat5_hardmult.png", "tests/ven_imL.png", "tests/ven_imR.png");

	setMatrixLength(0);
	testProgram(0.6, 4, "tests/con_imL_mat0_hardmult_smooth0.6.png", "tests/con_imR_mat0_hardmult_smooth0.6.png", "tests/con_imL.png", "tests/con_imR.png");
	testProgram(0.6, 4, "tests/ted_imL_mat0_hardmult_smooth0.6.png", "tests/ted_imR_mat0_hardmult_smooth0.6.png", "tests/ted_imL.png", "tests/ted_imR.png");
	testProgram(0.6, 16, "tests/tsu_imL_mat0_hardmult_smooth0.6.png", "tests/tsu_imR_mat0_hardmult_smooth0.6.png", "tests/tsu_imL.png", "tests/tsu_imR.png");
	testProgram(0.6, 8, "tests/ven_imL_mat0_hardmult_smooth0.6.png", "tests/ven_imR_mat0_hardmult_smooth0.6.png", "tests/ven_imL.png", "tests/ven_imR.png");
	setMatrixLength(0);
	testProgram(0.8, 4, "tests/con_imL_mat0_hardmult_smooth0.8.png", "tests/con_imR_mat0_hardmult_smooth0.8.png", "tests/con_imL.png", "tests/con_imR.png");
	testProgram(0.8, 4, "tests/ted_imL_mat0_hardmult_smooth0.8.png", "tests/ted_imR_mat0_hardmult_smooth0.8.png", "tests/ted_imL.png", "tests/ted_imR.png");
	testProgram(0.8, 16, "tests/tsu_imL_mat0_hardmult_smooth0.8.png", "tests/tsu_imR_mat0_hardmult_smooth0.8.png", "tests/tsu_imL.png", "tests/tsu_imR.png");
	testProgram(0.8, 8, "tests/ven_imL_mat0_hardmult_smooth0.8.png", "tests/ven_imR_mat0_hardmult_smooth0.8.png", "tests/ven_imL.png", "tests/ven_imR.png");

	setMatrixLength(0);
	testProgram(0.95, 4, "tests/con_imL_mat0_hardmult_smooth0.95.png", "tests/con_imR_mat0_hardmult_smooth0.95.png", "tests/con_imL.png", "tests/con_imR.png");
	testProgram(0.95, 4, "tests/ted_imL_mat0_hardmult_smooth0.95.png", "tests/ted_imR_mat0_hardmult_smooth0.95.png", "tests/ted_imL.png", "tests/ted_imR.png");
	testProgram(0.95, 16, "tests/tsu_imL_mat0_hardmult_smooth0.95.png", "tests/tsu_imR_mat0_hardmult_smooth0.95.png", "tests/tsu_imL.png", "tests/tsu_imR.png");
	testProgram(0.95, 8, "tests/ven_imL_mat0_hardmult_smooth0.95.png", "tests/ven_imR_mat0_hardmult_smooth0.95.png", "tests/ven_imL.png", "tests/ven_imR.png");

	setMatrixLength(0);
	testProgram(0.9, 4, "tests/con_imL_mat0_hardmult_smooth0.9.png", "tests/con_imR_mat0_hardmult_smooth0.9.png", "tests/con_imL.png", "tests/con_imR.png");
	testProgram(0.9, 4, "tests/ted_imL_mat0_hardmult_smooth0.9.png", "tests/ted_imR_mat0_hardmult_smooth0.9.png", "tests/ted_imL.png", "tests/ted_imR.png");
	testProgram(0.9, 16, "tests/tsu_imL_mat0_hardmult_smooth0.9.png", "tests/tsu_imR_mat0_hardmult_smooth0.9.png", "tests/tsu_imL.png", "tests/tsu_imR.png");
	testProgram(0.9, 8, "tests/ven_imL_mat0_hardmult_smooth0.9.png", "tests/ven_imR_mat0_hardmult_smooth0.9.png", "tests/ven_imL.png", "tests/ven_imR.png");
	qDebug() << "TESTING COMPLETE.";
	*/

	/*
	std::ofstream o_File("RESULTS_STEREO_TESTS.txt", std::ios::out);

	o_File << "tests/con_imL_mat0_dyn_smooth.png" << "\n";
	o_File << testStereoResults("tests/con_imL_mat0_dyn_smooth.png", "tests/con_ideal_L.png") << "\n";
	o_File << "tests/con_imR_mat0_dyn_smooth.png" << "\n";
	o_File << testStereoResults("tests/con_imR_mat0_dyn_smooth.png", "tests/con_ideal_R.png") << "\n";
	o_File << "tests/ted_imL_mat0_dyn_smooth.png" << "\n";
	o_File << testStereoResults("tests/ted_imL_mat0_dyn_smooth.png", "tests/ted_ideal_L.png") << "\n";
	o_File << "tests/ted_imR_mat0_dyn_smooth.png" << "\n";
	o_File << testStereoResults("tests/ted_imR_mat0_dyn_smooth.png", "tests/ted_ideal_R.png") << "\n";
	o_File << "tests/tsu_imL_mat0_dyn_smooth.png" << "\n";
	o_File << testStereoResults("tests/tsu_imL_mat0_dyn_smooth.png", "tests/tsu_ideal.png") << "\n";
	o_File << "tests/tsu_imR_mat0_dyn_smooth.png" << "\n";
	o_File << testStereoResults("tests/tsu_imR_mat0_dyn_smooth.png", "tests/tsu_ideal.png") << "\n";
	o_File << "tests/ven_imL_mat0_dyn_smooth.png" << "\n";
	o_File << testStereoResults("tests/ven_imL_mat0_dyn_smooth.png", "tests/ven_ideal_L.png") << "\n";
	o_File << "tests/ven_imR_mat0_dyn_smooth.png" << "\n";
	o_File << testStereoResults("tests/ven_imR_mat0_dyn_smooth.png", "tests/ven_ideal_R.png") << "\n";
	//
	o_File << "tests/con_imL_mat0_dyn.png" << "\n";
	o_File << testStereoResults("tests/con_imL_mat0_dyn.png", "tests/con_ideal_L.png") << "\n";
	o_File << "tests/con_imR_mat0_dyn.png" << "\n";
	o_File << testStereoResults("tests/con_imR_mat0_dyn.png", "tests/con_ideal_R.png") << "\n";
	o_File << "tests/ted_imL_mat0_dyn.png" << "\n";
	o_File << testStereoResults("tests/ted_imL_mat0_dyn.png", "tests/ted_ideal_L.png") << "\n";
	o_File << "tests/ted_imR_mat0_dyn.png" << "\n";
	o_File << testStereoResults("tests/ted_imR_mat0_dyn.png", "tests/ted_ideal_R.png") << "\n";
	o_File << "tests/tsu_imL_mat0_dyn.png" << "\n";
	o_File << testStereoResults("tests/tsu_imL_mat0_dyn.png", "tests/tsu_ideal.png") << "\n";
	o_File << "tests/tsu_imR_mat0_dyn.png" << "\n";
	o_File << testStereoResults("tests/tsu_imR_mat0_dyn.png", "tests/tsu_ideal.png") << "\n";
	o_File << "tests/ven_imL_mat0_dyn.png" << "\n";
	o_File << testStereoResults("tests/ven_imL_mat0_dyn.png", "tests/ven_ideal_L.png") << "\n";
	o_File << "tests/ven_imR_mat0_dyn.png" << "\n";
	o_File << testStereoResults("tests/ven_imR_mat0_dyn.png", "tests/ven_ideal_R.png") << "\n";
	//
	o_File << "tests/con_imL_mat0_hardmult_smooth.png" << "\n";
	o_File << testStereoResults("tests/con_imL_mat0_hardmult_smooth.png", "tests/con_ideal_L.png") << "\n";
	o_File << "tests/con_imR_mat0_hardmult_smooth.png" << "\n";
	o_File << testStereoResults("tests/con_imR_mat0_hardmult_smooth.png", "tests/con_ideal_R.png") << "\n";
	o_File << "tests/ted_imL_mat0_hardmult_smooth.png" << "\n";
	o_File << testStereoResults("tests/ted_imL_mat0_hardmult_smooth.png", "tests/ted_ideal_L.png") << "\n";
	o_File << "tests/ted_imR_mat0_hardmult_smooth.png" << "\n";
	o_File << testStereoResults("tests/ted_imR_mat0_hardmult_smooth.png", "tests/ted_ideal_R.png") << "\n";
	o_File << "tests/tsu_imL_mat0_hardmult_smooth.png" << "\n";
	o_File << testStereoResults("tests/tsu_imL_mat0_hardmult_smooth.png", "tests/tsu_ideal.png") << "\n";
	o_File << "tests/tsu_imR_mat0_hardmult_smooth.png" << "\n";
	o_File << testStereoResults("tests/tsu_imR_mat0_hardmult_smooth.png", "tests/tsu_ideal.png") << "\n";
	o_File << "tests/ven_imL_mat0_hardmult_smooth.png" << "\n";
	o_File << testStereoResults("tests/ven_imL_mat0_hardmult_smooth.png", "tests/ven_ideal_L.png") << "\n";
	o_File << "tests/ven_imR_mat0_hardmult_smooth.png" << "\n";
	o_File << testStereoResults("tests/ven_imR_mat0_hardmult_smooth.png", "tests/ven_ideal_R.png") << "\n";
	//
	o_File << "tests/con_imL_mat0_hardmult.png" << "\n";
	o_File << testStereoResults("tests/con_imL_mat0_hardmult.png", "tests/con_ideal_L.png") << "\n";
	o_File << "tests/con_imR_mat0_hardmult.png" << "\n";
	o_File << testStereoResults("tests/con_imR_mat0_hardmult.png", "tests/con_ideal_R.png") << "\n";
	o_File << "tests/ted_imL_mat0_hardmult.png" << "\n";
	o_File << testStereoResults("tests/ted_imL_mat0_hardmult.png", "tests/ted_ideal_L.png") << "\n";
	o_File << "tests/ted_imR_mat0_hardmult.png" << "\n";
	o_File << testStereoResults("tests/ted_imR_mat0_hardmult.png", "tests/ted_ideal_R.png") << "\n";
	o_File << "tests/tsu_imL_mat0_hardmult.png" << "\n";
	o_File << testStereoResults("tests/tsu_imL_mat0_hardmult.png", "tests/tsu_ideal.png") << "\n";
	o_File << "tests/tsu_imR_mat0_hardmult.png" << "\n";
	o_File << testStereoResults("tests/tsu_imR_mat0_hardmult.png", "tests/tsu_ideal.png") << "\n";
	o_File << "tests/ven_imL_mat0_hardmult.png" << "\n";
	o_File << testStereoResults("tests/ven_imL_mat0_hardmult.png", "tests/ven_ideal_L.png") << "\n";
	o_File << "tests/ven_imR_mat0_hardmult.png" << "\n";
	o_File << testStereoResults("tests/ven_imR_mat0_hardmult.png", "tests/ven_ideal_R.png") << "\n";
	//
	o_File << "tests/con_imL_mat3_hardmult_smooth.png" << "\n";
	o_File << testStereoResults("tests/con_imL_mat3_hardmult_smooth.png", "tests/con_ideal_L.png") << "\n";
	o_File << "tests/con_imR_mat3_hardmult_smooth.png" << "\n";
	o_File << testStereoResults("tests/con_imR_mat3_hardmult_smooth.png", "tests/con_ideal_R.png") << "\n";
	o_File << "tests/ted_imL_mat3_hardmult_smooth.png" << "\n";
	o_File << testStereoResults("tests/ted_imL_mat3_hardmult_smooth.png", "tests/ted_ideal_L.png") << "\n";
	o_File << "tests/ted_imR_mat3_hardmult_smooth.png" << "\n";
	o_File << testStereoResults("tests/ted_imR_mat3_hardmult_smooth.png", "tests/ted_ideal_R.png") << "\n";
	o_File << "tests/tsu_imL_mat3_hardmult_smooth.png" << "\n";
	o_File << testStereoResults("tests/tsu_imL_mat3_hardmult_smooth.png", "tests/tsu_ideal.png") << "\n";
	o_File << "tests/tsu_imR_mat3_hardmult_smooth.png" << "\n";
	o_File << testStereoResults("tests/tsu_imR_mat3_hardmult_smooth.png", "tests/tsu_ideal.png") << "\n";
	o_File << "tests/ven_imL_mat3_hardmult_smooth.png" << "\n";
	o_File << testStereoResults("tests/ven_imL_mat3_hardmult_smooth.png", "tests/ven_ideal_L.png") << "\n";
	o_File << "tests/ven_imR_mat3_hardmult_smooth.png" << "\n";
	o_File << testStereoResults("tests/ven_imR_mat3_hardmult_smooth.png", "tests/ven_ideal_R.png") << "\n";
	//
	o_File << "tests/con_imL_mat3_hardmult.png" << "\n";
	o_File << testStereoResults("tests/con_imL_mat3_hardmult.png", "tests/con_ideal_L.png") << "\n";
	o_File << "tests/con_imR_mat3_hardmult.png" << "\n";
	o_File << testStereoResults("tests/con_imR_mat3_hardmult.png", "tests/con_ideal_R.png") << "\n";
	o_File << "tests/ted_imL_mat3_hardmult.png" << "\n";
	o_File << testStereoResults("tests/ted_imL_mat3_hardmult.png", "tests/ted_ideal_L.png") << "\n";
	o_File << "tests/ted_imR_mat3_hardmult.png" << "\n";
	o_File << testStereoResults("tests/ted_imR_mat3_hardmult.png", "tests/ted_ideal_R.png") << "\n";
	o_File << "tests/tsu_imL_mat3_hardmult.png" << "\n";
	o_File << testStereoResults("tests/tsu_imL_mat3_hardmult.png", "tests/tsu_ideal.png") << "\n";
	o_File << "tests/tsu_imR_mat3_hardmult.png" << "\n";
	o_File << testStereoResults("tests/tsu_imR_mat3_hardmult.png", "tests/tsu_ideal.png") << "\n";
	o_File << "tests/ven_imL_mat3_hardmult.png" << "\n";
	o_File << testStereoResults("tests/ven_imL_mat3_hardmult.png", "tests/ven_ideal_L.png") << "\n";
	o_File << "tests/ven_imR_mat3_hardmult.png" << "\n";
	o_File << testStereoResults("tests/ven_imR_mat3_hardmult.png", "tests/ven_ideal_R.png") << "\n";
	//
	o_File << "tests/con_imL_mat5_hardmult_smooth.png" << "\n";
	o_File << testStereoResults("tests/con_imL_mat5_hardmult_smooth.png", "tests/con_ideal_L.png") << "\n";
	o_File << "tests/con_imR_mat5_hardmult_smooth.png" << "\n";
	o_File << testStereoResults("tests/con_imR_mat5_hardmult_smooth.png", "tests/con_ideal_R.png") << "\n";
	o_File << "tests/ted_imL_mat5_hardmult_smooth.png" << "\n";
	o_File << testStereoResults("tests/ted_imL_mat5_hardmult_smooth.png", "tests/ted_ideal_L.png") << "\n";
	o_File << "tests/ted_imR_mat5_hardmult_smooth.png" << "\n";
	o_File << testStereoResults("tests/ted_imR_mat5_hardmult_smooth.png", "tests/ted_ideal_R.png") << "\n";
	o_File << "tests/tsu_imL_mat5_hardmult_smooth.png" << "\n";
	o_File << testStereoResults("tests/tsu_imL_mat5_hardmult_smooth.png", "tests/tsu_ideal.png") << "\n";
	o_File << "tests/tsu_imR_mat5_hardmult_smooth.png" << "\n";
	o_File << testStereoResults("tests/tsu_imR_mat5_hardmult_smooth.png", "tests/tsu_ideal.png") << "\n";
	o_File << "tests/ven_imL_mat5_hardmult_smooth.png" << "\n";
	o_File << testStereoResults("tests/ven_imL_mat5_hardmult_smooth.png", "tests/ven_ideal_L.png") << "\n";
	o_File << "tests/ven_imR_mat5_hardmult_smooth.png" << "\n";
	o_File << testStereoResults("tests/ven_imR_mat5_hardmult_smooth.png", "tests/ven_ideal_R.png") << "\n";
	//
	o_File << "tests/con_imL_mat5_hardmult.png" << "\n";
	o_File << testStereoResults("tests/con_imL_mat5_hardmult.png", "tests/con_ideal_L.png") << "\n";
	o_File << "tests/con_imR_mat5_hardmult.png" << "\n";
	o_File << testStereoResults("tests/con_imR_mat5_hardmult.png", "tests/con_ideal_R.png") << "\n";
	o_File << "tests/ted_imL_mat5_hardmult.png" << "\n";
	o_File << testStereoResults("tests/ted_imL_mat5_hardmult.png", "tests/ted_ideal_L.png") << "\n";
	o_File << "tests/ted_imR_mat5_hardmult.png" << "\n";
	o_File << testStereoResults("tests/ted_imR_mat5_hardmult.png", "tests/ted_ideal_R.png") << "\n";
	o_File << "tests/tsu_imL_mat5_hardmult.png" << "\n";
	o_File << testStereoResults("tests/tsu_imL_mat5_hardmult.png", "tests/tsu_ideal.png") << "\n";
	o_File << "tests/tsu_imR_mat5_hardmult.png" << "\n";
	o_File << testStereoResults("tests/tsu_imR_mat5_hardmult.png", "tests/tsu_ideal.png") << "\n";
	o_File << "tests/ven_imL_mat5_hardmult.png" << "\n";
	o_File << testStereoResults("tests/ven_imL_mat5_hardmult.png", "tests/ven_ideal_L.png") << "\n";
	o_File << "tests/ven_imR_mat5_hardmult.png" << "\n";
	o_File << testStereoResults("tests/ven_imR_mat5_hardmult.png", "tests/ven_ideal_R.png") << "\n";
	//
	o_File << "tests/con_imL_mat0_hardmult_smooth0.6.png" << "\n";
	o_File << testStereoResults("tests/con_imL_mat0_hardmult_smooth0.6.png", "tests/con_ideal_L.png") << "\n";
	o_File << "tests/con_imR_mat0_hardmult_smooth0.6.png" << "\n";
	o_File << testStereoResults("tests/con_imR_mat0_hardmult_smooth0.6.png", "tests/con_ideal_R.png") << "\n";
	o_File << "tests/ted_imL_mat0_hardmult_smooth0.6.png" << "\n";
	o_File << testStereoResults("tests/ted_imL_mat0_hardmult_smooth0.6.png", "tests/ted_ideal_L.png") << "\n";
	o_File << "tests/ted_imR_mat0_hardmult_smooth0.6.png" << "\n";
	o_File << testStereoResults("tests/ted_imR_mat0_hardmult_smooth0.6.png", "tests/ted_ideal_R.png") << "\n";
	o_File << "tests/tsu_imL_mat0_hardmult_smooth0.6.png" << "\n";
	o_File << testStereoResults("tests/tsu_imL_mat0_hardmult_smooth0.6.png", "tests/tsu_ideal.png") << "\n";
	o_File << "tests/tsu_imR_mat0_hardmult_smooth0.6.png" << "\n";
	o_File << testStereoResults("tests/tsu_imR_mat0_hardmult_smooth0.6.png", "tests/tsu_ideal.png") << "\n";
	o_File << "tests/ven_imL_mat0_hardmult_smooth0.6.png" << "\n";
	o_File << testStereoResults("tests/ven_imL_mat0_hardmult_smooth0.6.png", "tests/ven_ideal_L.png") << "\n";
	o_File << "tests/ven_imR_mat0_hardmult_smooth0.6.png" << "\n";
	o_File << testStereoResults("tests/ven_imR_mat0_hardmult_smooth0.6.png", "tests/ven_ideal_R.png") << "\n";
	//
	o_File << "tests/con_imL_mat0_hardmult_smooth0.8.png" << "\n";
	o_File << testStereoResults("tests/con_imL_mat0_hardmult_smooth0.8.png", "tests/con_ideal_L.png") << "\n";
	o_File << "tests/con_imR_mat0_hardmult_smooth0.8.png" << "\n";
	o_File << testStereoResults("tests/con_imR_mat0_hardmult_smooth0.8.png", "tests/con_ideal_R.png") << "\n";
	o_File << "tests/ted_imL_mat0_hardmult_smooth0.8.png" << "\n";
	o_File << testStereoResults("tests/ted_imL_mat0_hardmult_smooth0.8.png", "tests/ted_ideal_L.png") << "\n";
	o_File << "tests/ted_imR_mat0_hardmult_smooth0.8.png" << "\n";
	o_File << testStereoResults("tests/ted_imR_mat0_hardmult_smooth0.8.png", "tests/ted_ideal_R.png") << "\n";
	o_File << "tests/tsu_imL_mat0_hardmult_smooth0.8.png" << "\n";
	o_File << testStereoResults("tests/tsu_imL_mat0_hardmult_smooth0.8.png", "tests/tsu_ideal.png") << "\n";
	o_File << "tests/tsu_imR_mat0_hardmult_smooth0.8.png" << "\n";
	o_File << testStereoResults("tests/tsu_imR_mat0_hardmult_smooth0.8.png", "tests/tsu_ideal.png") << "\n";
	o_File << "tests/ven_imL_mat0_hardmult_smooth0.8.png" << "\n";
	o_File << testStereoResults("tests/ven_imL_mat0_hardmult_smooth0.8.png", "tests/ven_ideal_L.png") << "\n";
	o_File << "tests/ven_imR_mat0_hardmult_smooth0.8.png" << "\n";
	o_File << testStereoResults("tests/ven_imR_mat0_hardmult_smooth0.8.png", "tests/ven_ideal_R.png") << "\n";
	//
	o_File << "tests/con_imL_mat0_hardmult_smooth0.9.png" << "\n";
	o_File << testStereoResults("tests/con_imL_mat0_hardmult_smooth0.9.png", "tests/con_ideal_L.png") << "\n";
	o_File << "tests/con_imR_mat0_hardmult_smooth0.9.png" << "\n";
	o_File << testStereoResults("tests/con_imR_mat0_hardmult_smooth0.9.png", "tests/con_ideal_R.png") << "\n";
	o_File << "tests/ted_imL_mat0_hardmult_smooth0.9.png" << "\n";
	o_File << testStereoResults("tests/ted_imL_mat0_hardmult_smooth0.9.png", "tests/ted_ideal_L.png") << "\n";
	o_File << "tests/ted_imR_mat0_hardmult_smooth0.9.png" << "\n";
	o_File << testStereoResults("tests/ted_imR_mat0_hardmult_smooth0.9.png", "tests/ted_ideal_R.png") << "\n";
	o_File << "tests/tsu_imL_mat0_hardmult_smooth0.9.png" << "\n";
	o_File << testStereoResults("tests/tsu_imL_mat0_hardmult_smooth0.9.png", "tests/tsu_ideal.png") << "\n";
	o_File << "tests/tsu_imR_mat0_hardmult_smooth0.9.png" << "\n";
	o_File << testStereoResults("tests/tsu_imR_mat0_hardmult_smooth0.9.png", "tests/tsu_ideal.png") << "\n";
	o_File << "tests/ven_imL_mat0_hardmult_smooth0.9.png" << "\n";
	o_File << testStereoResults("tests/ven_imL_mat0_hardmult_smooth0.9.png", "tests/ven_ideal_L.png") << "\n";
	o_File << "tests/ven_imR_mat0_hardmult_smooth0.9.png" << "\n";
	o_File << testStereoResults("tests/ven_imR_mat0_hardmult_smooth0.9.png", "tests/ven_ideal_R.png") << "\n";
	//
	o_File << "tests/con_imL_mat0_hardmult_smooth0.95.png" << "\n";
	o_File << testStereoResults("tests/con_imL_mat0_hardmult_smooth0.95.png", "tests/con_ideal_L.png") << "\n";
	o_File << "tests/con_imR_mat0_hardmult_smooth0.95.png" << "\n";
	o_File << testStereoResults("tests/con_imR_mat0_hardmult_smooth0.95.png", "tests/con_ideal_R.png") << "\n";
	o_File << "tests/ted_imL_mat0_hardmult_smooth0.95.png" << "\n";
	o_File << testStereoResults("tests/ted_imL_mat0_hardmult_smooth0.95.png", "tests/ted_ideal_L.png") << "\n";
	o_File << "tests/ted_imR_mat0_hardmult_smooth0.95.png" << "\n";
	o_File << testStereoResults("tests/ted_imR_mat0_hardmult_smooth0.95.png", "tests/ted_ideal_R.png") << "\n";
	o_File << "tests/tsu_imL_mat0_hardmult_smooth0.95.png" << "\n";
	o_File << testStereoResults("tests/tsu_imL_mat0_hardmult_smooth0.95.png", "tests/tsu_ideal.png") << "\n";
	o_File << "tests/tsu_imR_mat0_hardmult_smooth0.95.png" << "\n";
	o_File << testStereoResults("tests/tsu_imR_mat0_hardmult_smooth0.95.png", "tests/tsu_ideal.png") << "\n";
	o_File << "tests/ven_imL_mat0_hardmult_smooth0.95.png" << "\n";
	o_File << testStereoResults("tests/ven_imL_mat0_hardmult_smooth0.95.png", "tests/ven_ideal_L.png") << "\n";
	o_File << "tests/ven_imR_mat0_hardmult_smooth0.95.png" << "\n";
	o_File << testStereoResults("tests/ven_imR_mat0_hardmult_smooth0.95.png", "tests/ven_ideal_R.png") << "\n";
	//
	Mat static1 = Mat(375, 450, CV_8U);
	for (int i = 0; i < static1.rows; i++){
		for (int j = 0; j < static1.cols; j++){
			static1.at<unsigned char>(i,j) = (unsigned char) (((float)rand()/(float)RAND_MAX)*255);
		}
	}
	imwrite("tests/static1.png", static1);
	o_File << "tests/con_static_L.png" << "\n";
	o_File << testStereoResults("tests/static1.png", "tests/con_ideal_L.png") << "\n";
	o_File << "tests/con_static_R.png" << "\n";
	o_File << testStereoResults("tests/static1.png", "tests/con_ideal_R.png") << "\n";
	//
	o_File << "tests/ted_static_L.png" << "\n";
	o_File << testStereoResults("tests/static1.png", "tests/ted_ideal_L.png") << "\n";
	o_File << "tests/ted_static_R.png" << "\n";
	o_File << testStereoResults("tests/static1.png", "tests/ted_ideal_R.png") << "\n";
	//
	Mat static2 = Mat(288, 384, CV_8U);
	for (int i = 0; i < static2.rows; i++){
		for (int j = 0; j < static2.cols; j++){
			static2.at<unsigned char>(i,j) = (unsigned char) (((float)rand()/(float)RAND_MAX)*255);
		}
	}
	imwrite("tests/static2.png", static2);
	o_File << "tests/tsu_static.png" << "\n";
	o_File << testStereoResults("tests/static2.png", "tests/tsu_ideal.png") << "\n";
	//
	Mat static3 = Mat(383, 434, CV_8U);
	for (int i = 0; i < static3.rows; i++){
		for (int j = 0; j < static3.cols; j++){
			static3.at<unsigned char>(i,j) = (unsigned char) (((float)rand()/(float)RAND_MAX)*255);
		}
	}
	imwrite("tests/static3.png", static3);
	o_File << "tests/ven_static_L.png" << "\n";
	o_File << testStereoResults("tests/static3.png", "tests/ven_ideal_L.png") << "\n";
	o_File << "tests/ven_static_R.png" << "\n";
	o_File << testStereoResults("tests/static3.png", "tests/ven_ideal_R.png") << "\n";
	//
	o_File << "" << "\n";
	o_File << testStereoResults("", "") << "\n";


//	testProgram(0.0, -1, "tests/con_imL_mat0_dyn.png", "tests/con_imR_mat0_dyn.png", "tests/con_imL.png", "tests/con_imR.png");
//	testProgram(0.0, -1, "tests/ted_imL_mat0_dyn.png", "tests/ted_imR_mat0_dyn.png", "tests/ted_imL.png", "tests/ted_imR.png");
//	testProgram(0.0, -1, "tests/tsu_imL_mat0_dyn.png", "tests/tsu_imR_mat0_dyn.png", "tests/tsu_imL.png", "tests/tsu_imR.png");
//	testProgram(0.0, -1, "tests/ven_imL_mat0_dyn.png", "tests/ven_imR_mat0_dyn.png", "tests/ven_imL.png", "tests/ven_imR.png");


	//
	o_File.close();
	//
	qDebug() << "CHECKING COMPLETE.";
	*/

}


Mat StereoProcessor::medianFilter(Mat * mat, int filterSize){
	Mat new_mat = Mat(mat->rows, mat->cols, mat->type());
	qDebug() << "Filtering image...   ";
	int cols = mat->cols;
	int rows = mat->rows;
	int half = filterSize/2;
	for (int i = 0; i < rows; i++){
		for (int j = 0; j < cols; j++){
			int lowest_row = max(0, i-half);
			int lowest_col = max(0, j-half);
			int highest_row = min(rows-1, i+half);
			int highest_col = min(cols-1, j+half);
			std::vector<unsigned char> vec;
			for (int ii = lowest_row; ii <= highest_row; ii++){
				for (int jj = lowest_col; jj <= highest_col; jj++){
					vec.push_back(mat->at<unsigned char>(ii, jj));
				}
			}
			sort(vec.begin(), vec.end());
			unsigned char newValue = vec.at(vec.size()/2);
			new_mat.at<unsigned char>(i,j) = newValue;
		}
	}
	qDebug() << "done.";
	return new_mat;
}


bool StereoProcessor::dynamicProgramming(const char * leftName, const char * rightName, Mat left_input, Mat right_input){
	
	if( (left_input.empty()) || (right_input.empty()) ){
		return false;
	}

	if ((left_input.rows != right_input.rows) || (left_input.cols != right_input.cols)){
		return false;
	}

	if( (denoise_matrix_length > 0) && ((denoise_matrix_length % 2) != 0)) { // Odds only
		left_input = medianFilter(&left_input, denoise_matrix_length);
		emit progress(5);
		right_input = medianFilter(&right_input, denoise_matrix_length);
		emit progress(10);
		emit progress(15);
		emit updated();
	}

	int numRowsLeft = left_input.rows;// numRowsLeft = number of rows in left image
	int numColsLeft = left_input.cols; // numColsLeft = number of cols in left image
	initial_leftDepthMap = Mat(numRowsLeft, numColsLeft, CV_32S, Scalar(0)); // 32-bit signed integers
	initial_rightDepthMap = Mat(numRowsLeft, numColsLeft, CV_32S, Scalar(0)); // 32-bit signed integers
	initial_leftDepthMap_B = Mat(numRowsLeft, numColsLeft, CV_32S, Scalar(0)); // 32-bit signed integers
	initial_rightDepthMap_B = Mat(numRowsLeft, numColsLeft, CV_32S, Scalar(0)); // 32-bit signed integers
	correctedLeftDepthMap = Mat(numRowsLeft, numColsLeft, left_input.type(), Scalar(0)); // 8-bit unsigned chars
	correctedRightDepthMap = Mat(numRowsLeft, numColsLeft, left_input.type(), Scalar(0)); // 8-bit unsigned chars

	costMat = Mat(numRowsLeft, 2, CV_32S); // First column stores costs of paths in scanlines in forward direction, second for backward direction

	// Set up main dynamic programming matrix and previous path matrix.
	if (smoothness_weight > 0.0){
		A = Mat(numColsLeft, numColsLeft, CV_32F, Scalar(10000000)); // A uses 32-bit signed floats
		A_b = Mat(numColsLeft, numColsLeft, CV_32F, Scalar(10000000)); // A_b uses 32-bit signed floats
		prev_path_F = Mat(numColsLeft, numColsLeft, CV_32F, Scalar(10000000)); // 32-bit signed floats
		prev_path_B = Mat(numColsLeft, numColsLeft, CV_32F, Scalar(10000000)); // 32-bit signed floats
	} else {
		A = Mat(numColsLeft, numColsLeft, CV_32S, Scalar(10000000)); // A uses 32-bit signed integers
		A_b = Mat(numColsLeft, numColsLeft, CV_32S, Scalar(10000000)); // A_b uses 32-bit signed integers
	}

	// We progress one row of the rectified images at a time, starting with the topmost.
	for (int y_scanline = 0; y_scanline < numRowsLeft; y_scanline++){

		// A[0,0] is initialised to 0. All other elements are evaluated from upper left to lower right corner.
		for (int i = 0; i < numColsLeft; i++){ // i counts cols
			for (int j = 0; j < numColsLeft; j++){ // j counts cols also
				// All this assumes images are of type '0', i.e. CV_8U
				// Improvement: everything is initialised to a cost of a million;
				// only change this if within q pixels of the centre line.
				if (abs(i-j) > max_expected_disparity_bounds){
					// "max_expected_disparity_bounds" is the number of pixels from the centre to search
				} else {
					int up_left = 10000000;
					int up = 10000000;
					int left = 10000000;
					if (i > 0) {
						up = A.at<int>(i-1, j);
					}
					if (j > 0) {
						left = A.at<int>(i, j-1);
						if (i > 0){// && (j > 0)) {
							up_left = A.at<int>(i-1, j-1);
						}
					}
					int minimum = min(min(up, left), up_left);

					unsigned char valueLeft = left_input.at<unsigned char>(y_scanline, i);
					unsigned char valueRight = right_input.at<unsigned char>(y_scanline, j);
					if ((i == 0) && (j == 0)){
						A.at<int>(i, j) = 0;
					} else {
						unsigned char difference;
						if (valueLeft >= valueRight) {
							difference = valueLeft-valueRight;
						} else {
							difference = valueRight-valueLeft;
						}
						// Denoise goes here
						// Smoothing goes here
						A.at<int>(i,j) = minimum + difference;
						//std::cout << A.at<int>(i,j) << " ";
					}
				}
			}
		}
		// A has now been calculated.

		if (y_scanline > 0 && smoothness_weight > 0.0){
			prev_path_B = smoothness_weight * prev_path_B;
			A = (1.0 - smoothness_weight) * A;
			A = A + prev_path_B;
		}

		// Once the matrix has been filled, a path of minimal cost can be calculated by
		// tracing back from the lower right corner A[n-1,n-1] to upper left corner A[0,0].
		int ii = numColsLeft - 1;
		int jj = numColsLeft - 1;
		int cost = 0;
		while ((ii > 0) || (jj > 0)){
			//qDebug() << "i " << ii << " j " << jj << "\n";
			//if (jj - ii != 0) qDebug() << "!";
			initial_leftDepthMap.at<int>(y_scanline, ii) = (jj - ii);
			//qDebug() << "ii=" << ii << ", jj=" << jj << "\n";
			initial_rightDepthMap.at<int>(y_scanline, jj) = (ii - jj);
			int up = 10000000;
			int left = 10000000;
			int up_left = 10000000;
			if (ii > 0) {
				up = (int) (A.at<int>(ii - 1, jj) * weight_porcupine);
			}
			if (jj > 0) {
				left = (int) (A.at<int>(ii, jj - 1) * weight_porcupine);
				if (ii > 0)// && (jj > 0))
					up_left = A.at<int>(ii - 1, jj - 1);
			}
			int minimum = min(min(up, left), up_left);
			// Weight pathdirection goes here
			// 4.6
			cost += minimum;
			if (minimum == up_left){ //std::cout << "upleft\n";
				ii--;
				jj--;
			} else if (minimum == left){ //std::cout << "left\n";
				jj--;
			} else { //std::cout << "up\n";
				ii--;
			}
		}
		costMat.at<int>(y_scanline,0) = cost;


		// Now we move on to the backwards matrix, which is essentially the same


		// A_b[n-1,n-1] is initialised to 0. All other elements are evaluated from lower right to upper left corner.
		for (int i = numColsLeft-1; i >= 0; i--){ // i counts cols
			for (int j = numColsLeft-1; j >= 0; j--){ // j counts cols also
				// All this assumes images are of type '0', i.e. CV_8U

				// Improvement: everything is initialised to a cost of a million;
				// only change this if within q pixels of the centre line.
				if (abs(i-j) > max_expected_disparity_bounds){
					// "max_expected_disparity_bounds" is the number of pixels from the centre to search
				} else {
					int down_right = 10000000;
					int down = 10000000;
					int right = 10000000;
					if (i < numColsLeft-1) {
						down = A_b.at<int>(i+1, j);
					}
					if (j < numColsLeft-1) {
						right = A_b.at<int>(i, j+1);
						if (i < numColsLeft-1){// && (j < numColsLeft-1)) {
							down_right = A_b.at<int>(i+1, j+1);
						}
					}
					int minimum = min(min(down, right), down_right);

					unsigned char valueLeft = left_input.at<unsigned char>(y_scanline, i);
					unsigned char valueRight = right_input.at<unsigned char>(y_scanline, j);
					if ((i == numColsLeft-1) && (j == numColsLeft-1)){
						A_b.at<int>(i, j) = 0;
					} else {
						unsigned char difference;
						if (valueLeft >= valueRight) {
							difference = valueLeft-valueRight;
						} else {
							difference = valueRight-valueLeft;
						}
						// Denoise goes here
						// Smoothing goes here
						A_b.at<int>(i,j) = minimum + difference;
						//std::cout << A.at<int>(i,j) << " ";
					}
				}
			}
		}
		// A_b has now been calculated.

		if (y_scanline > 0 && smoothness_weight > 0.0){
			prev_path_F = smoothness_weight * prev_path_F;
			A_b = (1.0 - smoothness_weight) * A_b;
			A_b = A_b + prev_path_F;
		}

		// Once the backwards matrix has been filled, a path of minimal cost can be calculated by
		// tracing back from the upper left corner A[0,0] to the lower right corner A[n-1,n-1].
		ii = 0;
		jj = 0;
		cost = 0;
		while ((ii < numColsLeft-1) || (jj < numColsLeft-1)){
			//qDebug() << "i " << ii << " j " << jj << "\n";
			//if (jj - ii != 0) qDebug() << "!";
			initial_leftDepthMap_B.at<int>(y_scanline, ii) = (jj - ii);
			//qDebug() << "ii=" << ii << ", jj=" << jj << "\n";
			initial_rightDepthMap_B.at<int>(y_scanline, jj) = (ii - jj);
			int down = 10000000;
			int right = 10000000;
			int down_right = 10000000;
			if (ii < numColsLeft-1) down = (int) (A_b.at<int>(ii + 1, jj) * weight_porcupine);
			if (jj < numColsLeft-1){
				right = (int) (A_b.at<int>(ii, jj + 1) * weight_porcupine);
				if (ii < numColsLeft-1) //&& (jj < numColsLeft-1))
					down_right = A_b.at<int>(ii + 1, jj + 1);
			}
			int minimum = min(min(down, right), down_right);
			// Weight pathdirection goes here
			// 4.6
			cost += minimum;
			if (minimum == down_right){ //std::cout << "down_right\n";
				ii++;
				jj++;
			} else if (minimum == right){ //std::cout << "right\n";
				jj++;
			} else { //std::cout << "down\n";
				ii++;
			}
		}
		costMat.at<int>(y_scanline,1) = cost;

		// We only use prev_path if we are smoothing.
		if (smoothness_weight > 0.0){
			prev_path_F = A.clone();
			prev_path_B = A_b.clone();
		}

		int prog = 15 + (int)(85.0*((double)y_scanline / (double)numRowsLeft));
		if (prog % 5 == 0) emit progress(prog);

	}
	qDebug() << "STEREO MATCHING COMPLETE.\n";

	if (hard_multiplier > 0){ // Use absolute values, it seems to work

		for (int i = 0; i < numRowsLeft; i++){
			for (int j = 0; j < numColsLeft; j++){
				initial_leftDepthMap.at<int>(i, j) = abs(initial_leftDepthMap.at<int>(i, j));
				initial_rightDepthMap.at<int>(i, j) = abs(initial_rightDepthMap.at<int>(i, j));
				initial_leftDepthMap_B.at<int>(i, j) = abs(initial_leftDepthMap_B.at<int>(i, j));
				initial_rightDepthMap_B.at<int>(i, j) = abs(initial_rightDepthMap_B.at<int>(i, j));
				// FIXED
			}
		}

		int highestDisparityL1 = 0;
		int highestDisparityL2 = 0;
		int highestDisparityR1 = 0;
		int highestDisparityR2 = 0;
		for (int i = 0; i < numRowsLeft; i++){
			for (int j = 0; j < numColsLeft; j++){
				if (initial_leftDepthMap.at<int>(i, j) > highestDisparityL1) highestDisparityL1 = initial_leftDepthMap.at<int>(i, j);
				if (initial_leftDepthMap_B.at<int>(i, j) > highestDisparityL2) highestDisparityL2 = initial_leftDepthMap_B.at<int>(i, j);
				if (initial_rightDepthMap.at<int>(i, j) > highestDisparityR1) highestDisparityR1 = initial_rightDepthMap.at<int>(i, j);
				if (initial_rightDepthMap_B.at<int>(i, j) > highestDisparityR2) highestDisparityR2 = initial_rightDepthMap_B.at<int>(i, j);
			}
		}
		qDebug() << "HIGHEST DISPARITIES = {L " << highestDisparityL1 << ", R " << highestDisparityR1 << "}";
		qDebug() << "HIGHEST DISPARITIES = {L " << highestDisparityL2 << ", R " << highestDisparityR2 << "}";

		// Need to normalise output image to [0...255]
		// For display purposes, we saturate the depth map to have only positive values.
		float multiplier = hard_multiplier; // Puts in a hard multiplier, e.g. for middlebury tests where it is known
		int numForwards = 0;
		int numBackwards = 0;
		for (int i = 0; i < numRowsLeft; i++){
			if (costMat.at<int>(i,0) < costMat.at<int>(i,1)){ // Use forwards depth map for left image
				for (int j = 0; j < numColsLeft; j++){
					correctedLeftDepthMap.at<unsigned char>(i, j) =
						(unsigned char) min(((int)(initial_leftDepthMap.at<int>(i, j) * multiplier)),255);
					correctedRightDepthMap.at<unsigned char>(i, j) =
						(unsigned char) min(((int)(initial_rightDepthMap.at<int>(i, j) * multiplier)),255);
				}
				numForwards++;
			} else { // Use backwards depth map for left image
				for (int j = 0; j < numColsLeft; j++){
					correctedLeftDepthMap.at<unsigned char>(i, j) =
						(unsigned char) min(((int)(initial_leftDepthMap_B.at<int>(i, j) * multiplier)),255);
					correctedRightDepthMap.at<unsigned char>(i, j) =
						(unsigned char) min(((int)(initial_rightDepthMap_B.at<int>(i, j) * multiplier)),255);
				}
				numBackwards++;
			}
		}

		qDebug() << "Number of forward scanlines used: " << numForwards;
		qDebug() << "Number of backward scanlines used: " << numBackwards;

	} else {
		// Not using hard multiplier; adjust negative disparities instead of using absolute value.
		// Uses the 'ignore_outliers' variable
		int lowestDisparityL1 = 0;
		int lowestDisparityL2 = 0;
		int lowestDisparityR1 = 0;
		int lowestDisparityR2 = 0;
		for (int i = 0; i < numRowsLeft; i++){
			for (int j = 0; j < numColsLeft; j++){
				if (initial_leftDepthMap.at<int>(i, j) < lowestDisparityL1) lowestDisparityL1 = initial_leftDepthMap.at<int>(i, j);
				if (initial_leftDepthMap_B.at<int>(i, j) < lowestDisparityL2) lowestDisparityL2 = initial_leftDepthMap_B.at<int>(i, j);
				if (initial_rightDepthMap.at<int>(i, j) < lowestDisparityR1) lowestDisparityR1 = initial_rightDepthMap.at<int>(i, j);
				if (initial_rightDepthMap_B.at<int>(i, j) < lowestDisparityR2) lowestDisparityR2 = initial_rightDepthMap_B.at<int>(i, j);
			}
		}

		int lowestDisparity = min(lowestDisparityL1, min(lowestDisparityL2, min(lowestDisparityR1, lowestDisparityR2)));
		lowestDisparity *= -1; //
		lowestDisparity = (int) ( ((double) lowestDisparity) * 0.8); // We reduce the mapping because of outliers

		for (int i = 0; i < numRowsLeft; i++){
			for (int j = 0; j < numColsLeft; j++){
				initial_leftDepthMap.at<int>(i, j) = initial_leftDepthMap.at<int>(i, j) + lowestDisparity;
				initial_rightDepthMap.at<int>(i, j) = initial_rightDepthMap.at<int>(i, j) + lowestDisparity;
				initial_leftDepthMap_B.at<int>(i, j) = initial_leftDepthMap_B.at<int>(i, j) + lowestDisparity;
				initial_rightDepthMap_B.at<int>(i, j) = initial_rightDepthMap_B.at<int>(i, j) + lowestDisparity;
				// FIXED
			}
		}

		int highestDisparityL1 = 0;
		int highestDisparityL2 = 0;
		int highestDisparityR1 = 0;
		int highestDisparityR2 = 0;
		for (int i = 0; i < numRowsLeft; i++){
			for (int j = 0; j < numColsLeft; j++){
				if (initial_leftDepthMap.at<int>(i, j) > highestDisparityL1) highestDisparityL1 = initial_leftDepthMap.at<int>(i, j);
				if (initial_leftDepthMap_B.at<int>(i, j) > highestDisparityL2) highestDisparityL2 = initial_leftDepthMap_B.at<int>(i, j);
				if (initial_rightDepthMap.at<int>(i, j) > highestDisparityR1) highestDisparityR1 = initial_rightDepthMap.at<int>(i, j);
				if (initial_rightDepthMap_B.at<int>(i, j) > highestDisparityR2) highestDisparityR2 = initial_rightDepthMap_B.at<int>(i, j);
			}
		}
		qDebug() << "HIGHEST DISPARITIES(f) = {L " << highestDisparityL1 << ", R " << highestDisparityR1 << "}";
		qDebug() << "HIGHEST DISPARITIES(b) = {L " << highestDisparityL2 << ", R " << highestDisparityR2 << "}";
		qDebug() << "LOWEST DISPARITIES(f) = {L " << lowestDisparityL1 << ", R " << lowestDisparityR1 << "}";
		qDebug() << "LOWEST DISPARITIES(b) = {L " << lowestDisparityL2 << ", R " << lowestDisparityR2 << "}";

		int highestDisparity = max(max(highestDisparityL1, highestDisparityL2), max(highestDisparityR1, highestDisparityR2));
		highestDisparity = (int) ( ((double) highestDisparity) * 0.8); // We reduce the mapping because of outliers

		// Need to normalise output image to [0...255]
		// For display purposes, we saturate the depth map to have only positive values.
		// 0 = ???
		// Otherwise, [1...255] = disparity map
		float multiplier = 255.0/((float)(highestDisparity));
		if (hard_multiplier > 0){
			multiplier = 255.0/hard_multiplier; // Puts in a hard multiplier, e.g. for middlebury tests where it is known
		}
		int numForwards = 0;
		int numBackwards = 0;
		for (int i = 0; i < numRowsLeft; i++){
			if (costMat.at<int>(i,0) < costMat.at<int>(i,1)){ // Use forwards depth map for left image
				for (int j = 0; j < numColsLeft; j++){
					correctedLeftDepthMap.at<unsigned char>(i, j) =
						(unsigned char) max(0, min(((int)(initial_leftDepthMap.at<int>(i, j) * multiplier)),255));
					correctedRightDepthMap.at<unsigned char>(i, j) =
						(unsigned char) max(0, min(((int)(initial_rightDepthMap.at<int>(i, j) * multiplier)),255));
				}
				numForwards++;
			} else { // Use backwards depth map for left image
				for (int j = 0; j < numColsLeft; j++){
					correctedLeftDepthMap.at<unsigned char>(i, j) =
						(unsigned char) max(0, min(((int)(initial_leftDepthMap_B.at<int>(i, j) * multiplier)),255));
					correctedRightDepthMap.at<unsigned char>(i, j) =
						(unsigned char) max(0, min(((int)(initial_rightDepthMap_B.at<int>(i, j) * multiplier)),255));
				}
				numBackwards++;
			}
		}

		qDebug() << "Number of forward scanlines used: " << numForwards;
		qDebug() << "Number of backward scanlines used: " << numBackwards;

	}

	if(autoOutput){
		std::string inp = input_image_filename.toStdString();
		inp = inp.append("_DEPTH.png");
		std::string outp = right_image_file.fileName().toStdString();
		outp = outp.append("_DEPTH.png");
		cv::imwrite(inp, correctedLeftDepthMap);
		cv::imwrite(outp, correctedRightDepthMap);
	} else {
		// Outputs disparity maps to files anyway
		cv::imwrite(leftName, correctedLeftDepthMap);
		cv::imwrite(rightName, correctedRightDepthMap);
	}
	return true;

	// PSEUDOCODE:
	/*
	Step 1: CALCULATE MATRIX
	a1 = Right[i,y]
	a2 = Left[j,y]
	a3 = diff(a1,a2)
	a4 = a3*scale + weight // denoise (DONE)
	a5 = f(a4, smooth[i,j])
	b1 = A[i-1,j]
	b2 = A[i,j-1]
	b3 = A[i-1,j-1]
	b4 = minimum(b1, b2, b3)
	b5 = b4 - path[i,j] // Reusing paths
	path[i,j] = path[i,j] * 0.875 // Reusing paths
	A[i,j] = a5 + b5 // Write DP matrix

	Step 2: FIND PATH
	c1 = A[i-1,j]
	c2 = A[i,j-1]
	c3 = A[i-1,j-1]
	c4 = minimum(c1, c2, c3)
	c1 = c1 + weight1
	c2 = c2 + weight2
	c3 = c3 + weight3
	// Choose new position
	if (c4 <= c1) { j = j-1; i = i-1; } else
	if (c4 <= c2) { j = j-1 } else
	if (c4 <= c3) { i = i-1 }
	// Remember paths
	path[i,j] = constant
	// Write disparity map
	disparity[i,y] = j
	*/
}




void StereoProcessor::setMatrixLength(const int mat_length){
	QMutexLocker locker(&mutex);
	if(denoise_matrix_length == mat_length) return;
	denoise_matrix_length = mat_length;
	mutex.unlock();
	process();
}
void StereoProcessor::setDisparityBounds(const int a){
	QMutexLocker locker(&mutex);
	if(max_expected_disparity_bounds == a) return;
	max_expected_disparity_bounds = a;
	mutex.unlock();
	process();
}
void StereoProcessor::setHardMultiplier(const int a){
	QMutexLocker locker(&mutex);
	if(hard_multiplier == a) return;
	hard_multiplier = a;
	mutex.unlock();
	process();
}
void StereoProcessor::setSmoothnessWeight(const double a){
	QMutexLocker locker(&mutex);
	if(smoothness_weight == a) return;
	smoothness_weight = a;
	mutex.unlock();
	process();
}
void StereoProcessor::setWeightPorcupine(const double a){
	QMutexLocker locker(&mutex);
	if(weight_porcupine == a) return;
	weight_porcupine = a;
	mutex.unlock();
	process();
}

void StereoProcessor::setAutoOutput(const bool yesno){
	QMutexLocker locker(&mutex);
	if(autoOutput == true){
		autoOutput = yesno;
		return;
	} else {
		autoOutput = yesno;
	}
	mutex.unlock();
	process();
}

void StereoProcessor::testProgram(bool autoOut, double smoothWeight, int hardMult, const char * lOut, const char * rOut, const char * lIn, const char * rIn){
	autoOutput = autoOut;
	setSmoothnessWeight(smoothWeight);
	setHardMultiplier(hardMult);
	dynamicProgramming(lOut, rOut, imread(lIn, 0), imread(rIn, 0));
}

double StereoProcessor::testStereoResults(const char * testImageName, const char * idealImageName){
	Mat testImage = imread(testImageName, 0);
	Mat idealImage = imread(idealImageName, 0);
	//
	if( (testImage.empty()) || (idealImage.empty()) || (testImage.rows != idealImage.rows) || (testImage.cols != idealImage.cols)){
		return -1;
	}
	//
	double sumAbsErrors = 0;
	for (int i = 0; i < testImage.rows; i++){
		for (int j = 0; j < testImage.cols; j++){
			sumAbsErrors += abs(testImage.at<unsigned char>(i,j) - idealImage.at<unsigned char>(i,j));
		}
	}
	return sumAbsErrors;
}



