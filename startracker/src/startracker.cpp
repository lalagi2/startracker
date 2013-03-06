#include <opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int gnoiseTreshold = 0;
int gstarTreshold = 60;
double unit = 1;
const int groiSize = 11;
const double atlagFaktor = 3.8; // megfelelo zajkuszob kiszamitasahoz hasznalt suly
const double csillagFaktor = 3; // megfelelo csillagkuszob kiszamitasahoz hasznalt suly

Point centers[300];
int centerdb = 0;

double root(double x){
    double a,b;
    b     = x;
    a = x = 0x3f;
    x     = b/x;
    a = x = (x+a) / 2;
    x     = b/x;
    a = x = (x+a) / 2;
    x     = b/x;
    x     = (x+a) / 2;

    return(x);
}

//zajkuszob megallapitasa
int calculateNoiseTreshold(Mat_<uchar> image) {
	int atlag = (int)mean(image).val[0];

	cout << "Noise treshold: " << atlag << " (atlagFaktorral sulyozott eredmeny)" << endl;
	return (int)atlag * atlagFaktor;
}

//csillagkuszob meghatarozasa
int calculateStarTreshold(Mat_<uchar> image) {
	double max;

	minMaxIdx(image, NULL, &max); // maximalis pixelintenzitas meghatarozasa

	cout << "Star treshold: " << (int)max / csillagFaktor << " (csillagFaktorral sulyozott eredmeny)" << endl;
	return (int)max / csillagFaktor;
}

Mat preBinarizing (Mat_<uchar> image, const int noiseTreshold) {
	Mat_<uchar> filter;
	Mat_<uchar> out;

	//ami noiseTresholdnal kisebb eldobjuk
	compare(image, noiseTreshold, filter, CV_CMP_GT);
	bitwise_and( image, filter, out );

	return out;
}

double calculateFocalDistance(int row, int col, double fov) {
	double angle = 180 - 90 - fov; // a 3szog 3. szoge
	double a; // a 3szog egy oldala
	double focalDistance;

//	egyik oldala
	a = sqrt(row / 2 * row / 2 + col / 2 * col / 2);

//	sin tetel
	focalDistance = sin(angle) * a / sin(fov / 2);
	cout << " focal distance: " << focalDistance;
	return focalDistance;
}

void centerOfMassCalculation(Mat_<uchar> image, Point2d& center, int rowValue, int columnValue) {
	Mat_<uchar> imageROI;
	Scalar dn;

	center.x = 0;
	center.y = 0;

	//calculate ROI brightness
	imageROI = image(Range(rowValue - groiSize, rowValue + groiSize), Range(columnValue - groiSize, columnValue + groiSize));
	dn = sum(imageROI);

	Moments mom = moments(imageROI, false);

	center.x = (double)(mom.m10 / mom.m00) + columnValue - groiSize;
	center.y = (double)(mom.m01 / mom.m00) + rowValue - groiSize;
}

void centroiding( Mat_<uchar> image ) {
	Mat isChecked = Mat::zeros(image.rows, image.cols, CV_8UC1);
	Mat mask = Mat::ones(groiSize * 2, groiSize * 2, CV_8UC1);
	Point2d center(0, 0);
	int db = 0;

	//csillag kutatasa pixelenkent
	for (int col = 0; col < image.cols; col++) {
		for (int row = 0; row < image.rows; row++) {
			//ha csillagot talaltunk && meg nem ellenoriztuk / az adott pixel meg nem volt benne egy ROIban
			if ( (image(row, col) > gstarTreshold) && (isChecked.at<uchar>(row, col) == (uchar)0) ) {
				//TALALAT, ha nem log ki a csillag ROI-ja a kepbol
				if (row - groiSize > 0 && col - groiSize > 0 && row + groiSize < image.rows && col + groiSize < image.cols) {
					centerOfMassCalculation(image, center, row, col);
					centers[centerdb] = center;
					centerdb++;

					//ROI pixeleit az ischeckedben atallitjuk -> tobbet nem nezzuk meg ezeket a pixeleket
					Mat imageROI = isChecked(Range(row - groiSize, row + groiSize), Range(col - groiSize, col + groiSize));
					mask.copyTo(imageROI);

					db++;
				}
			}
		}
	}
	cout << "Ennyi csillagot detektaltunk a kepen: " << db << endl;;
}

//2 pont kozti tavolsag
double length(Point2d p1, Point2d p2) {
	double a = p2.x - p1.x;
	double b = p2.y - p1.y;
	//sqrt hanyagolando lesz beagyazottnal
	return sqrt(a * a + b * b);
}

void searchingPattern(double area, double polarmom) {
	if (fabs(12506 - area) < 0.0001) cout << "jee";
}

void calculateArea () {
	double area;
	double polarmom;
	double s;
	double a;
	double b;
	double c;

	for (int csp1 = 0; csp1 < centerdb - 2; csp1++) {
		for (int csp2 = csp1 + 1; csp2 < centerdb - 1; csp2++) {
			for (int csp3 = csp1 + 2; csp3 < centerdb; csp3++) {
				a = length(centers[csp2], centers[csp1]);
				b = length(centers[csp3], centers[csp2]);
				c = length(centers[csp1], centers[csp3]);

				s = 0.5 * (a + b + c);
				//sqrt hanyagolando lesz beagyazottnal
				area = sqrt(s * (s - a) * (s - b) * (s - c));
				polarmom = area * (a * a + b * b + c * c) / 36;

				searchingPattern(area, polarmom);
//				cout << a << " " << b << " " << c << " " << area << " " << polarmom << endl;
//				cout << centers[csp2] - centers[csp1] << endl;
			}
		}
	}
}

int main( int argc, char** argv ) {
	//kep betoltese
	Mat input = imread( argv[1] );
	Mat_<uchar> preFiltered;

	namedWindow("in");
	imshow("in", input);

	//rgb to gray
	cvtColor( input, preFiltered, CV_RGB2GRAY );

	//kuszobok kiszamolasa
	gnoiseTreshold = calculateNoiseTreshold(preFiltered);
	gstarTreshold = calculateStarTreshold(preFiltered);

	//eloszures
	preFiltered = preBinarizing( preFiltered, gnoiseTreshold );
	centroiding(preFiltered);

	namedWindow( "out", CV_WINDOW_AUTOSIZE );
	//terulet kiszamolasa
	calculateArea();

	for (int i = 0; i < centerdb; i++) {
		stringstream ss;
		ss << i;
		string str = ss.str();
		putText(preFiltered, str, centers[i], FONT_HERSHEY_SIMPLEX, 0.5, cvScalar(200,200,250), 0.5, CV_AA);
		circle(preFiltered, centers[i], 7, Scalar(255));
	}

	calculateFocalDistance(preFiltered.rows, preFiltered.cols, 36);

	imshow( "out", preFiltered );
	waitKey(0);

	return 0;
}
