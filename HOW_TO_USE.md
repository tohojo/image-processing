## IMPORTANT ##

If you have not already read the README file and installed the application in accordance with the INSTALL file in the `code/` subdirectory, do so now!

## ... ##

## HOW TO USE ##

General:
- The GUI features a list of processors, a panel displaying the input image, a panel for textual output, a panel displaying the processed image, and a properties panel. These can be removed from the display using the View menu.
- Images can be loaded and saved from the File menu.
- Points of interest (POIs) are generated and manipulated in the calibration processor. POIs can be cleared, saved or loaded from the POIs menu, at any time.
- Known issue (1): some of the values in the properties panel may not automatically update; clicking on the properties panel should force an update.
- Known issue (2): the progress bar only gives an indicator of the actual processor progress. If progress is not displayed as 100%, the processor has not finished.
- The default processor is "No processing", which simply displays the input image.

Segmentation processor:
- "Segmenting mode" parameter: choose either the thresholding algorithm with (a) a global or (b) an adaptive threshold, or (c) the split and merge algorithm.
- "Dark background" parameter (default false): specifies whether the input image has a dark or light background.
- "Delta" parameter (default 50): this is the parameter used for the uniformity predicate in the split and merge algorithm.
- "Threshold" property: this value updates with the calculated global or adapted threshold when the thresholding algorithm is being used.
- In a debug build, the textual output box should display (in adaptive thresholding) the change to the threshold, or (in split and merge) the number of regions split, filtered out, and then merged.

Feature point extraction processor:
- "Octaves" parameter: valid range is 1-5 (due to response layers and filter sizes from SURF).
- "Intervals" parameter: valid range is 3-4 (due to response layers and filter sizes from SURF).
- "InitSample" parameter: number of pixels to take one sample from. At value 1, every pixel is considered (very computationally expensive). At default value 2, every second pixel in each direction is considered. etc.
- "Threshold" parameter: threshold over which points are considered features. Due to differences in the implementations, we suggest values under 0.001 for SURF_OPENSURF, and values over 1 for SURF and SURF_OPENCV. This obviously depends on the input image.
- "Extractor" parameter: which implementation of the SURF extractor to use. SURF is our own implementation. SURF_OPENSURF is from the OpenSURF library. SURF_OPENCV is from the OpenCV library.

Calibration processor:
- Step 1 (or 2): load a text file of 3d points (separated by commas or whitespace) into the "Points3d" parameter. One such file is "real_world_measurements.txt", included in the 'test-images' directory.
- Step 2 (or 1): load an image of a suitable calibration object.
- Step 3: confirm the location of calibration points, marked as POIs on the image. It is only necessary at this point that the POI be within the correct thresholded region. Double click to add a POI or right click on a POI to remove it. Increasing the "FeatureThreshold" parameter will decrease the number of POIs automatically generated and vice versa. All POIs can be cleared from the POIs menu.
- Step 4: the standard calibration object has 63 POIs. If this number of POIs have been picked out (see POICount in the properties panel), the calibration can proceed. Change the "Stage" parameter to STAGE_2.
- Step 5: at this point, the POIs will be automatically recentred on the calibration points they are marking. If any POIs are misplaced, return to stage 1 to re-mark them.
- Also at this point, the camera calibration will automatically happen, and the results are printed to the textual output box in a debug build.
- Step 6: remove POIs as you wish and press "Re-process" to perform the calibration without considering these calibration points. Note that POIs added at this stage do not alter the algorithm; to add calibration points you must start again at stage 1.

Distortion removal processor:
- This processor works on (distorted) chessboard images IN UNCOMPRESSED .TIF FORMAT ONLY.
- The "SquaresAcross" and "SquaresDown" parameters describe the dimensionality of the chessboard.
- If not all the chessboard corners are being identified by the program, it is SOMETIMES possible to reduce the dimensionality and perform the distortion correction on a subset of the chessboard. This is not guaranteed, however.
- If the chessboard can be identified, the corner points of the squares should be displayed on the output image panel. These should be joined in an orderly fashion, row by row or column by column; if they are not, the distortion calculation will probably fail.
- The IPOLdistortion library automatically corrects the input image, storing it in file 'output_undistorted_image.tif'.
- The IPOLdistortion library also outputs the model of distortion it fits to the data. This is stored in file 'output_lens_distortion_models.dat'.
