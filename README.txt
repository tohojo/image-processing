IMAGE PROCESSING APPLICATION
By Ben Meadows and Toke Høiland-Jørgensen

This application is a project assignment for the COMPSCI773 course,
semester 1 2012, University of Auckland.

The application lives in the `code/` subdirectory and is written in
C++ using the Qt and OpenCV libraries. The `report/` subdirectory
contains the LaTeX source for the accompanying report.

So far the application does the following:

- Image segmentation - simple thresholding and split and merge
  segmentation (the latter is not working very well, see the report).

- Feature point extraction - an implementation of the SURF algorithm,
  partly based on the OpenSURF implementation. The application also
  provides an interface to run the OpenSURF and OpenCV SURF
  implementations.

- Camera calibration - obtaining rotation and translation matrices
  from calibration images using Tsai's method, and testing the results
  via backprojection.

- Distortion removal - removing distortion so straight lines stay
  straight. The parameters of the lens distortion model are calculated
  and a corrected form of the input image is output.


WORKINGS OF THE APPLICATION

The program is written in C++ using the Qt and OpenCV libraries. The
application consists of a Qt-based GUI that allows the user to load
images, select between various image processors, select parameters and
peruse the results of the processing by zooming and panning on the
output image. Furthermore, it is possible to select points of
interest (POIs) by double clicking on the image, which can be used to
select points for the calibration algorithm. The GUI also has a log
output window for textual output from the algorithms.

Each processor is implemented as a class that specifies which
parameters are available for this processor (the parameters can be set
by the user with the help of the QPropertyEditor library), and does
the actual processing. The processing is done in a separate thread, to
keep the interface responsive, and make it possible for the user to
cancel a long-running processor.

INSTALLATION

See the INSTALL file in the `code/` subdirectory.


USAGE

See the 'HOW_TO_USE' file in this directory.
