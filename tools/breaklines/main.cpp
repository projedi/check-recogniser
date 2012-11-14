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

bool rectSort(Rect r1, Rect r2) { return (r1.y < r2.y); }

void sortAndMergeRects(vector<Rect>& rects) {
   for(int i = 0; i != rects.size(); i++) {
      Rect& r1 = rects[i];
      if(r1.width == 0 && r1.height == 0) continue;
      for(int j = i+1; j != rects.size(); j++) {
         Rect& r2 = rects[j];
         if(r2.width == 0 && r2.height == 0) continue;
         int xdiff = (r1.x + r1.width/2) - (r2.x + r2.width/2);
         int ydiff = (r1.y + r1.height/2) - (r2.y + r2.height/2);
         //TODO: Use more intelligent way
         if(ydiff <= 30 || (r1 & r2) == r2 || (r1 & r2) == r1) {
            //if(  r1.contains(Point(r2.x+r2.width/2,r2.y+r2.height/2))
            //  || r2.contains(Point(r1.x+r1.width/2,r1.y+r1.height/2))) {
               r1 |= r2;
               rects.erase(rects.begin()+j);
               j--;
               //r2.x = 0;
               //r2.y = 0;
               //r2.width = 0;
               //r2.height = 0;
            }
      }
   }
   sort(rects.begin(), rects.end(), rectSort);
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
   vector<Rect> rects;
   vector< vector<Point> > squares;
   for(int i = 0; i != contours.size(); i++) {
      //Point2f center;
      //float radius;
      //minEnclosingCircle(contours[i], center, radius);
      Rect box = minAreaRect(Mat(contours[i])).boundingRect();
      if(box.width < 5 || box.height < 5 || box.height > 60) continue;
      int dx = (int)(box.width / 10);
      float dy = (int)(box.height / 10);
      box.width += 2*dx;
      box.height += 2*dy;
      box.x -= dx;
      box.y -= dy;
      //Mat mat;
      //getRectSubPix(src, Size(2*radius, 2*radius), center, mat);
      //imshow("Letters", mat);
      //waitKey(0);
      //Rect rect((int)(center.x-radius),(int)(center.y-radius),(int)(2*radius),(int)(2*radius));
      rects.push_back(box);
      /*
      vector<Point> points;
      points.push_back(Point((int)(center.x - radius), (int)(center.y - radius)));
      points.push_back(Point((int)(center.x + radius), (int)(center.y - radius)));
      points.push_back(Point((int)(center.x + radius), (int)(center.y + radius)));
      points.push_back(Point((int)(center.x - radius), (int)(center.y + radius)));
      squares.push_back(points);
      */
   }
   cout << rects.size() << endl;
   sortAndMergeRects(rects);
   //groupRectangles(rects, 1, 0.2);
   cout << rects.size() << endl;
   for(int i = 0; i != rects.size(); i++) {
      Rect r = rects[i];
      cout << r << endl;
      Mat mat;
      getRectSubPix(src, Size(r.width, r.height), Point(r.x+r.width/2, r.y+r.height/2), mat);
      //imshow("Letters", mat);
      //waitKey(0);
      vector<Point> points;
      points.push_back(Point(r.x, r.y));
      points.push_back(Point(r.x+r.width, r.y));
      points.push_back(Point(r.x+r.width, r.y+r.height));
      points.push_back(Point(r.x, r.y+r.height));
      squares.push_back(points);
   }
   drawContours(src, squares, -1, Scalar(0, 0, 255), 2, 8);
   displayImg(src,500);
}

void solve(string filename) {
   Mat src = imread(filename, 0);
   //displayImg(src);
   //cropCheck(src);
   displayImg(src,500);
   retrieveBoundingBoxes(src);
   //TODO: After that use tesseract to convert to text.
   //Then parse this text like on a demo. Pray.
}

int main(int argc, char** argv) {
   solve(argv[1]);
   return 0;
}
