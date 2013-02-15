#include <opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

const int noiseTreshold = 16;
const int starTreshold = 200;
const int roiSize = 7;

Mat_<uchar> preFiltered;

Mat preBinarizing ( Mat_<uchar> image ) {
	Mat_<uchar> filter;
	Mat_<uchar> out;

	//ami 18nal kisebb eldobjuk
	compare(image, noiseTreshold, filter, CV_CMP_GT);
	bitwise_and( image, filter, out );

	return out;
}

void centerOfMassCalculation(Mat_<uchar> roi, double& x, double& y, int rowvalue, int columnvalue) {
	int dn = 0;
	int roij = 0;
	int roii = 0;
	x = 0;
	y = 0;

	for (int i = 0; i < roiSize; i++) {//roi meretet valtoztatni
		for (int j = 0; j < roiSize; j++) {
			dn += (int)roi[j][i];
		}
	}

//	cout << rowvalue << endl;
	for (int j = columnvalue - (roiSize - 1) / 2; j < columnvalue + (roiSize - 1) / 2; j++) {
		for (int i = 0; i < roiSize; i++) {
			x += j * (int)roi[roij][i];
		}
		roij++;
	}

	roii = 0;
	roij = 0;
	for (int i = rowvalue - (roiSize - 1) / 2; i < rowvalue + (roiSize - 1) / 2; i++) {
		for (int j = 0; j < roiSize; j++) {
			y += i * (int)roi[j][roii];
		}
		roii++;
	}

	x /= dn;
	y /= dn;

	x += 0.5;
	y += 0.5;

//	cout << x << " " << y<< endl;
}

Point centers[30];
int centerdb = 0;

void centroiding( Mat_<uchar> image ) {
	bool isChecked[image.rows][image.cols];
	uchar roi[roiSize][roiSize];
	double x;
	double y;

	int db = 0;

	//egyik pixelt sem neztuk meg, hogy nagyobb-e, mint a starTreshold kuszob
	for (int i = 0; i < image.rows; i++) {
		for (int j = 0; j < image.cols; j++) {
			isChecked[i][j] = false;
		}
	}

	//csillag kutatasa pixelenkent
	for (int j = 0; j < image.cols; j++) {
		for (int i = 0; i < image.rows; i++) {
			//ha csillagot talaltunk && meg nem ellenoriztuk / nem volt benne egy ROIban
			if ( (image(i, j) > starTreshold) && (isChecked[i][j] == false) ) {
				//ROI, TALALAT
				db++;
				Mat_<uchar> imageROI;
				if (i - roiSize > 0 && j - roiSize > 0 && i + roiSize < 600 && j + roiSize < 600) {
					imageROI = image(Range(i - roiSize, i + roiSize), Range(j - roiSize, j + roiSize));
					centerOfMassCalculation(imageROI, x, y, i, j);
					centers[centerdb] = Point2d(x, y);
					centerdb++;

					for (int k = 0; k < imageROI.cols; k++) {
						for (int l = 0; l < imageROI.rows; l++) {
							cout << (int)imageROI(k, l) << " ";

						}
						cout << endl;
					}

					for (int k = 0; k < roiSize; k++) {
						for (int l = 0; l < roiSize; l++) {
							isChecked[i + k - roiSize][j + l - roiSize] = true;
						}
					}
				}

				cout << endl;
			}
			isChecked[i][j] = true;
		}
	}
	cout << db;
}


int main( int argc, char** argv ) {
	//kep betoltese
	Mat input = imread( argv[1] );


//	namedWindow("in");
//	imshow("in", input);

	//rgb to gray
	cvtColor( input, preFiltered, CV_RGB2GRAY );

	//eloszures
	preFiltered = preBinarizing( preFiltered );

//	for (int i = 0; i < preFiltered.rows; i++) {
//		for (int j = 0; j < preFiltered.cols; j++) {
//			if (preFiltered(j, i) > starTreshold) {
//				if (preFiltered(j, i) > 20) {
//					cout << (int) preFiltered(j, i) << " " << i << " " << j << endl;
//				}
//			}
//
//		}
//	}


	centroiding(preFiltered);

	for (int i = 0; i < centerdb; i++) {
		circle(preFiltered, centers[i], 6, Scalar(255, 0, 0));
	}

	namedWindow( "out", CV_WINDOW_AUTOSIZE );
	imshow( "out", preFiltered );

	waitKey(0);

	return 0;
}
