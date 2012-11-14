#include "recognizer.h"

#include <QtCore/QStringList>
#include <QtCore/QString>
#include <QtCore/QRegExp>

#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

#include <opencv2/opencv.hpp>

using namespace cv;

bool rectSort(Rect r1, Rect r2) { return (r1.y < r2.y); }

void sortAndMergeRects(vector<Rect>& rects) {
   for(unsigned int i = 0; i != rects.size(); i++) {
      Rect& r1 = rects[i];
      if(r1.width == 0 && r1.height == 0) continue;
      for(unsigned int j = i+1; j != rects.size(); j++) {
         Rect& r2 = rects[j];
         if(r2.width == 0 && r2.height == 0) continue;
         int diff = (r1.y - r2.y) + (r1.height - r2.height)/2;
         if(diff <= 30 || (r1 & r2) == r2 || (r1 & r2) == r1) {
            r1 |= r2;
            rects.erase(rects.begin()+j);
            j--;
         }
      }
   }
   sort(rects.begin(), rects.end(), rectSort);
}

vector<Rect> retrieveBoundingBoxes(Mat& src) {
   Mat img = src.clone();
   cvtColor(src,src,CV_GRAY2BGR);
   bitwise_not(img, img);
   threshold(img, img, 120, 255, THRESH_TOZERO);
   Mat element = getStructuringElement(MORPH_RECT, Size(5,3));
   dilate(img, img, element);
   erode(img, img, element);
   vector< vector<Point> > contours;
   Mat hierarchy;
   findContours(img, contours, hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
   vector<Rect> rects;
   vector< vector<Point> > squares;
   for(unsigned int i = 0; i != contours.size(); i++) {
      Rect box = minAreaRect(Mat(contours[i])).boundingRect();
      if(box.width < 5 || box.height < 5 || box.height > 60) continue;
      int dx = (int)(box.width / 10);
      float dy = (int)(box.height / 10);
      box.width += 2*dx;
      box.height += 2*dy;
      box.x -= dx;
      box.y -= dy;
      rects.push_back(box);
   }
   sortAndMergeRects(rects);
   return rects;
}

QStringList recogniseLineByLine(QString fileName, std::string lang) {
    tesseract::TessBaseAPI tessApi;
    tessApi.Init(NULL, lang.c_str());
    Mat src = imread(fileName.toStdString(), 0);
    vector<Rect> rects = retrieveBoundingBoxes(src);
    QStringList res;
    for(vector<Rect>::iterator it = rects.begin(); it != rects.end(); it++) {
       int w = (*it).width;
       int h = (*it).height;
       int cx = (*it).x + (*it).width / 2;
       int cy = (*it).y + (*it).height / 2;
       Mat mat;
       getRectSubPix(src, Size(w,h), Point(cx,cy), mat);
       tessApi.SetImage((uchar*)mat.data,mat.cols,mat.rows,mat.channels(),mat.step1());
       char *text = tessApi.GetUTF8Text();
       QString txt = QString::fromUtf8(text);
       QStringList str;
       str = txt.split("\n");
       res += str;
    }
    return res;
}

ChequeRecognizer::ChequeRecognizer(QObject *parent) : QObject(parent) { }

Cheque ChequeRecognizer::recognizeFile(const QString &fileName) {
    Cheque ch;
    setenv("TESSDATA_PREFIX", ".", 1);

    QStringList str = recogniseLineByLine(fileName,"eng");
    QStringList strNames = recogniseLineByLine(fileName,"rus");

    QRegExp rgood("[0-9]+[,.][0-9][0-9].[0-9]+[,.][0-9][0-9]$");
    QRegExp rcost("[0-9]+[,.][0-9][0-9]$");
    QRegExp rname("[0-9,.]*[xX]");
    QRegExp rcount("[xX]");
    QRegExp rtotal("ИТОГ");
    QRegExp rtotalc("[0-9]+[,.][0-9][0-9]");
    QRegExp rdatetime("^#[0-9][0-9][0-9][0-9]");

    double total = 0;
    
    for (int i = 0; i != str.count(); ++i) {
       QString numStr = str[i];
       QString nameStr = strNames[i];
        if (numStr.contains(rgood)) {
            int index2 = numStr.indexOf(rcost);
            int index3 = numStr.indexOf(rname);
            int index4 = numStr.indexOf(rcount);

            QString gname = nameStr.mid(0,index3);
            gname = gname.replace('"', '\'');
            double gcost = numStr.mid(index2).replace(',', '.').toDouble();
            double gcount = numStr.mid(index3,index4-index3).replace(',', '.').toDouble();
            ch.goods.append(Good(gname, gcost, gcount));
            total += gcost;
        }
        if (numStr.contains(rdatetime)) {
            numStr = numStr.replace(QRegExp("[^0-9]"), " ").replace("  ", " ");
            QString datetime = (numStr).right(14);
            datetime[datetime.count()-3] = ' ';
            ch.date = QDateTime::fromString(datetime, "dd MM yy hh mm");
        }
    }
    ch.total = total;
    return ch;
}
