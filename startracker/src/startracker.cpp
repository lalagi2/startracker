#include <opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

const int starTreshold = 90;
const int roiSize = 21;

Mat preBinarizing ( Mat_<uchar> image ) {
	Mat_<uchar> filter;
	Mat_<uchar> out;

	//ami 18nal kisebb eldobjuk
	compare(image, 18, filter, CV_CMP_GT);
	bitwise_and( image, filter, out );

	return out;
}

void centroid( Mat_<uchar> image ) {
	bool isChecked[image.rows][image.cols];
	uchar roi[roiSize][roiSize];

	int db = 0;

	//egyik pixelt sem neztuk meg, hogy nagyobb-e, mint a starTreshold kuszob
	for (int i = 0; i < image.rows; i++) {
		for (int j = 0; j < image.cols; j++) {
			isChecked[i][j] = false;
		}
	}

	//csillag kutatasa pixelenkent
	for (int j = 0; j < image.rows; j++) {
		for (int i = 0; i < image.cols; i++) {
			//ha csillagot talaltunk && meg nem ellenoriztuk / nem volt benne egy ROIban
			if ( (image(i, j) > starTreshold) && (isChecked[i][j] == false) ) {
				//ROI
				db++;
				for (int l = 0; l < roiSize; l++) {
					for (int k = 0; k < roiSize; k++) {
						if ( (i + k - ((roiSize - 1) / 2)) < 0) {
							roi[k][l] = (uchar)0;
						}
						else if ( (j + l - ((roiSize - 1) / 2)) < 0 ) {
							roi[k][l] = (uchar)0;
						}
						else if( (i + k - ((roiSize - 1) / 2)) > image.rows ) {
							roi[k][l] = (uchar)0;
						}
						else if ( (j + l - ((roiSize - 1) / 2)) > image.cols ){
							roi[k][l] = (uchar)0;
						}
						else {
							roi[k][l] = image(i + k - ((roiSize - 1) / 2), j + l - ((roiSize - 1) / 2));
							isChecked[i + k - ((roiSize - 1) / 2)][j + l - ((roiSize - 1) / 2)] = true;
						}

						cout << (int)roi[k][l] << " ";
					}
					cout << endl;
				}
				cout << endl << endl;
			}
			isChecked[i][j] = true;
		}
	}

	cout << db;

}


int main( int argc, char** argv ) {
	//kep betoltese
	Mat input = imread( argv[1] );
	Mat_<uchar> preFiltered;

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

//	namedWindow( "out", CV_WINDOW_AUTOSIZE );
//	imshow( "out", preFiltered );

	centroid(preFiltered);

	waitKey(0);

	return 0;
}
