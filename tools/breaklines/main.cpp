#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;


void solve(string filename) {
   Mat res;
   Mat src = imread(filename, 0);
   vector< vector<Point> > contours;
   //TODO: from 3rd onwards
   Mat grayImg;
   //medianBlur(src, grayImg, 3);
   adaptiveThreshold(src, grayImg, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY_INV, 9, 3);
   resize(src, res, Size(300,500));
   imshow("Found contours", res);
   waitKey(0);
   Mat element = getStructuringElement(MORPH_RECT, Size(5,3));
   erode(grayImg, grayImg, element);
   erode(grayImg, grayImg, element);
   dilate(grayImg, grayImg, element);
   dilate(grayImg, grayImg, element);
   /*
   resize(grayImg, res, Size(300,500));
   imshow("Found contours", res);
   waitKey(0);
   */
   Mat_<uchar>::iterator it = grayImg.begin<uchar>();
   Mat_<uchar>::iterator end = grayImg.end<uchar>();
   vector<Point> points;
   for(; it != end; ++it) {
      if(*it) points.push_back(it.pos());
   }
   //TODO: Make it slightly bigger
   RotatedRect box = minAreaRect(Mat(points));
   Mat rotated, cropped;
   Mat m = getRotationMatrix2D(box.center, box.angle, 1.0);
   warpAffine(src, rotated, m, src.size(), INTER_CUBIC);
   getRectSubPix(rotated, box.size, box.center, cropped);
   src = cropped;

   /*
   cvtColor(src, src, CV_GRAY2BGR);
   Point2f vertices[4];
   box.points(vertices);
   for(int i = 0; i != 4; ++i)
      line(src, vertices[i], vertices[(i+1)%4], Scalar(0,0,255), 2);
   */
   /*
   vector<Vec4i> lines;
   HoughLinesP(grayImg, lines, 1, CV_PI/180, 100, src.cols / 10.f, 0);
   cout << lines.size() << endl;
   cvtColor(src, src, CV_GRAY2BGR);
   for(int i = 0; i != lines.size(); i++) {
      line(src, Point(lines[i][0], lines[i][1]), Point(lines[i][2],lines[i][3]), Scalar(255,0,0));
   }
   */
   /*
   // TODO: Last 2 parameters
   findContours(grayImg, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
   cout << contours.size() << endl;
   cvtColor(src, src, CV_GRAY2BGR);
   drawContours(src, contours, -1, Scalar(0,255,0));
   */
   resize(src, res, Size(300,500));
   imshow("Found contours", res);
   waitKey(0);
}

int main(int argc, char** argv) {
   solve(argv[1]);
   return 0;
}
