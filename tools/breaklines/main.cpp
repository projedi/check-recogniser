#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

void displayImg(Mat img) {
   Mat res;
   resize(img, res, Size(300,500));
   imshow("Display image", res);
   waitKey(0);
}

// Grayscale image only
void cropCheck(Mat& src) {
   Mat thresh;
   // TODO: Last 2 params
   adaptiveThreshold(src, thresh, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY_INV, 9, 3);
   Mat element = getStructuringElement(MORPH_RECT, Size(5,3));
   erode(thresh, thresh, element);
   erode(thresh, thresh, element);
   dilate(thresh, thresh, element);
   dilate(thresh, thresh, element);
   Mat_<uchar>::iterator it = thresh.begin<uchar>();
   Mat_<uchar>::iterator end = thresh.end<uchar>();
   vector<Point> points;
   for(; it != end; ++it) {
      if(*it) points.push_back(it.pos());
   }
   //TODO: Make it slightly bigger
   RotatedRect box = minAreaRect(Mat(points));
   Mat rotated;
   Mat m = getRotationMatrix2D(box.center, box.angle, 1.0);
   warpAffine(src, rotated, m, src.size(), INTER_CUBIC);
   getRectSubPix(rotated, box.size, box.center, src);
}

void solve(string filename) {
   Mat src = imread(filename, 0);
   displayImg(src);
   cropCheck(src);
   displayImg(src);
}

int main(int argc, char** argv) {
   solve(argv[1]);
   return 0;
}
