#include "recognizer.h"

#include <QSettings>
#include <QtCore/QStringList>
#include <QtCore/QString>
#include <QtCore/QTextStream>
#include <QtCore/QRegExp>

#include <QtCore/QTextCodec>
#include <QtCore/QDebug>

#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

#include <opencv2/opencv.hpp>

#define SETTING_ORGANIZATION "qt-box-editor"
#define SETTING_APPLICATION "QT Box Editor"

using namespace cv;

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

vector<Rect> retrieveBoundingBoxes(Mat& src) {
   Mat img = src.clone();
   cvtColor(src,src,CV_GRAY2BGR);
   bitwise_not(img, img);
   threshold(img, img, 120, 255, THRESH_TOZERO);
   Mat element = getStructuringElement(MORPH_RECT, Size(5,3));
   dilate(img, img, element);
   erode(img, img, element);
   //displayImg(img, 500);
   vector< vector<Point> > contours;
   Mat hierarchy;
   findContours(img, contours, hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
   //cout << contours.size() << endl;
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
   //cout << rects.size() << endl;
   sortAndMergeRects(rects);
   //groupRectangles(rects, 1, 0.2);
   //cout << rects.size() << endl;
   /*
   for(int i = 0; i != rects.size(); i++) {
      Rect r = rects[i];
      //cout << r << endl;
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
   //displayImg(src,500);
   */
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
       //PIX *pix = pixRead(fileName.toLocal8Bit());
       //tessApi.SetImage(pix);
       tessApi.SetImage((uchar*)mat.data,mat.cols,mat.rows,mat.channels(),mat.step1());
       char *text = tessApi.GetUTF8Text();
       QString txt = QString::fromUtf8(text);
       qDebug() << "Got " << txt;
       QStringList str;
       str = txt.split("\n");
       res += str;
    }
    return res;
}

QString getDataPath() {
    QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                       SETTING_ORGANIZATION, SETTING_APPLICATION);
    QString dataPath;
    if (settings.contains("Tesseract/DataPath")) {
      dataPath = settings.value("Tesseract/DataPath").toString();
    }
    return dataPath;
}

ChequeRecognizer::ChequeRecognizer(QObject *parent) : QObject(parent) {
}

Cheque ChequeRecognizer::recognizeFile(const QString &fileName) {
    Cheque ch;
     #ifdef _WIN32
     QString envQString = "TESSDATA_PREFIX=" + getDataPath() ;
     QByteArray byteArrayWin = envQString.toUtf8();
     const char * env = byteArrayWin.data();
     putenv(env);
     #else
     //QByteArray byteArray1 = getDataPath().toUtf8();
     //const char * datapath = byteArray1.data();
     //setenv("TESSDATA_PREFIX", datapath, 1);
     #endif

    QStringList str = recogniseLineByLine(fileName,"eng");
    QStringList strNames = recogniseLineByLine(fileName,"rus");
    qDebug() << "recogniseLineByLine finished: " << str.count();

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf-8")); //cyrillic .cpp
    QRegExp rgood("[0-9]+[,.][0-9][0-9].[0-9]+[,.][0-9][0-9]$");
    QRegExp rcost("[0-9]+[,.][0-9][0-9]$");
    QRegExp rname("[0-9,.]*[хХxX]");
    //QRegExp rcount("[хХxX]");
    QRegExp rcount("[xX]");
    QRegExp rtotal("ИТОГ");
    QRegExp rtotalc("[0-9]+[,.][0-9][0-9]");
    //QRegExp rdatetime("[0-9][0-9]-[0-9][0-9]-[0-9][0-9] [0-9][0-9]:[0-9][0-9]");
    //QRegExp rdatetime("[0-9][0-9][-~][0-9][0-9][-~][0-9][0-9] [0-9][0-9]:[0-9][0-9]");
    QRegExp rdatetime("^#[0-9][0-9][0-9][0-9]");


    QTextStream out(stdout);

    double total = 0;
    
    //for (QStringList::Iterator it = str.begin(); it != str.end(); ++it) {
    for (int i = 0; i != str.count(); ++i) {
       QString numStr = str[i];
       QString nameStr = strNames[i];
       qDebug() << "Working on " << numStr << "; " << nameStr;
        if (numStr.contains(rgood)) {
            int index1 = numStr.indexOf(rgood);
            int index2 = numStr.indexOf(rcost);
            int index3 = numStr.indexOf(rname);
            int index4 = numStr.indexOf(rcount);

            QString gname = nameStr.mid(0,index3);
            gname = gname.replace('"', '\'');
            //double gcost = (*it).mid(index1, index2-index1).replace(',', '.').toDouble();
            double gcost = numStr.mid(index2).replace(',', '.').toDouble();
            double gcount = numStr.mid(index3, index4-index3).replace(',', '.').toDouble();
//            out << (*it).mid(0,index1) << endl;
//            out << (*it).mid(index2, -1) << endl;
//            out << *it << endl;
//            out << (*it).mid(index3, index4-index3).replace(',', '.') << endl;
//            out << gname << endl;
//            out << gcost << endl;
//            out << gcount << endl;
            ch.goods.append(Good(gname, gcost, gcount));
            total += gcost;//*gcount;
        }
        /*
        if (nameStr.contains(rtotal)) {
            rtotalc.indexIn(numStr);
            int index1 = numStr.indexOf(rtotalc);
            //total = (*it).mid(index1, rtotalc.matchedLength()).replace(',', '.').toDouble();
//            out << (*it).mid(index1, rtotalc.matchedLength()).replace(',', '.') << endl;
        }
        */
        if (numStr.contains(rdatetime)) {
            numStr = numStr.replace(QRegExp("[^0-9]"), " ").replace("  ", " ");
            qDebug() << "Fucking date: " << numStr;
            QString datetime = (numStr).right(14);
            datetime[datetime.count()-3] = ' ';
            qDebug() << "Fucking date2: " << datetime;
            ch.date = QDateTime::fromString(datetime, "dd MM yy hh mm");
            /*
            int index1 = numStr.indexOf(rdatetime);
            QString datetime = (numStr).mid(index1, rdatetime.matchedLength());
            ch.date = QDateTime::fromString(datetime, "dd-MM-yy hh:mm");
            */
        }
    }
    ch.total = total;
    qDebug() << "At the end of file recognising";
    /*
    ch.date = QDateTime::fromString("13-11-12 19:26", "dd-MM-yy hh:mm");

    //pixDestroy(&pix);
    //delete [] text;

    ch.date = QDateTime::currentDateTime();
    ch.total = 40.0;
    ch.goods.append(Good("milk", 40.0, 1));
    */
    return ch;
}
