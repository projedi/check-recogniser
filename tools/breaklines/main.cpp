#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

void displayImg(Mat img, int height) {
   Mat res;
   resize(img, res, Size(300,height));
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

void retrieveBoundingBoxes(Mat& src) {
   Mat img = src.clone();
   cvtColor(src,src,CV_GRAY2BGR);
   bitwise_not(img, img);
   threshold(img, img, 120, 255, THRESH_TOZERO);
   Mat element = getStructuringElement(MORPH_RECT, Size(5,3));
   dilate(img, img, element);
   erode(img, img, element);
   displayImg(img, 500);
   vector< vector<Point> > contours;
   Mat hierarchy;
   findContours(img, contours, hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
   cout << contours.size() << endl;
   //vector< vector<Point> > rects;
   for(int i = 0; i != contours.size(); i++) {
      Point2f center;
      float radius;
      minEnclosingCircle(contours[i], center, radius);
      if(radius < 5 || radius > 100) continue;
      Mat mat;
      getRectSubPix(src, Size(2*radius, 2*radius), center, mat);
      imshow("Letters", mat);
      waitKey(0);
      /*
      vector<Point> points;
      points.push_back(Point((int)(center.x - radius), (int)(center.y - radius)));
      points.push_back(Point((int)(center.x + radius), (int)(center.y - radius)));
      points.push_back(Point((int)(center.x + radius), (int)(center.y + radius)));
      points.push_back(Point((int)(center.x - radius), (int)(center.y + radius)));
      rects.push_back(points);
      */
   }
   //cout << rects.size() << endl;
   //drawContours(src, rects, -1, Scalar(0, 0, 255), 2, 8);
   displayImg(src,500);
}

void solve(string filename) {
   Mat src = imread(filename, 0);
   //displayImg(src);
   cropCheck(src);
   displayImg(src,500);
   retrieveBoundingBoxes(src);
}

int main(int argc, char** argv) {
   solve(argv[1]);
   return 0;
}
