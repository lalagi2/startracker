#include <opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

const int noiseTreshold = 16;
const int starTreshold = 195;
const int roiSize = 6;

Mat_<uchar> preFiltered;

Point centers[30];
int centerdb = 0;

Mat preBinarizing ( Mat_<uchar> image ) {
	Mat_<uchar> filter;
	Mat_<uchar> out;

	//ami 18nal kisebb eldobjuk
	compare(image, noiseTreshold, filter, CV_CMP_GT);
	bitwise_and( image, filter, out );

	return out;
}

void centerOfMassCalculation(Point2d& center, int rowvalue, int columnvalue) {
	int dn = 0;

	center.x = 0;
	center.y = 0;

	for (int row = rowvalue - roiSize; row < rowvalue + roiSize + 1; row++) {
		for (int col = columnvalue - roiSize; col < columnvalue + roiSize + 1; col++) {
			dn += (int)preFiltered(row, col);
			cout << (int)preFiltered(row, col) << " ";
		}
		cout << endl;
	}

	cout << dn << endl;

	for (int row = rowvalue - roiSize; row < rowvalue + roiSize + 1; row++) {
		for (int col = columnvalue - roiSize; col < columnvalue + roiSize + 1; col++) {
			center.x += col * (int)preFiltered(row, col);
		}
	}

	for (int row = rowvalue - roiSize; row < rowvalue + roiSize + 1; row++) {
		for (int col = columnvalue - roiSize; col < columnvalue + roiSize + 1; col++) {
			center.y += row * (int)preFiltered(row, col);
		}
	}

	center.x /= dn;
	center.y /= dn;

//	center.x += 0.5;
//	center.y += 0.5;

//	cout << center.x << " " << center.y << endl;
}

void centroiding( Mat_<uchar> image ) {
	bool isChecked[image.rows][image.cols];
	Point2d center(0, 0);
	int db = 0;

	//egyik pixelt sem neztuk meg, hogy nagyobb-e, mint a starTreshold kuszob
	for (int row = 0; row < image.rows; row++) {
		for (int col = 0; col < image.cols; col++) {
			isChecked[row][col] = false;
		}
	}

	//csillag kutatasa pixelenkent
	for (int col = 0; col < image.cols; col++) {
		for (int row = 0; row < image.rows; row++) {
			//ha csillagot talaltunk && meg nem ellenoriztuk / az adott pixel meg nem volt benne egy ROIban
			if ( (image(row, col) > starTreshold) && (isChecked[row][col] == false) ) {
				//TALALAT, ha nem log ki a csillag ROI-ja a kepbol
				if (row - roiSize > 0 && col - roiSize > 0 && row + roiSize < 600 && col + roiSize < 600) {
					centerOfMassCalculation(center, row, col);
					centers[centerdb] = center;
					centerdb++;

					db++;
					//ROI pixeleit az ischeckedben truera allitjuk -> tobbet nem nezzuk meg ezeket a pixeleket
					for (int k = 0; k < roiSize; k++) {
						for (int l = 0; l < roiSize; l++) {
							isChecked[row - k][col + l] = true;
							isChecked[row + k][col - l] = true;
							isChecked[row - k][col - l] = true;
							isChecked[row + k][col + l] = true;
						}
					}
				}
			}
			isChecked[row][col] = true;
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

	centroiding(preFiltered);

	for (int i = 0; i < centerdb; i++) {
		circle(preFiltered, centers[i], 7, Scalar(255));
	}

	namedWindow( "out", CV_WINDOW_AUTOSIZE );
	imshow( "out", preFiltered );

	waitKey(0);

	return 0;
}
