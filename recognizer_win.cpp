#include "recognizer_win.h"

#include <QImage>
#include <QDebug>
#include <QColor>
#include <QFile>
#include <QTextStream>

#include "leptonica/allheaders.h"

#include <itkRGBPixel.h>
#include <itkRGBToLuminanceImageFilter.h>
#include <itkImportImageFilter.h>
#include <itkImageDuplicator.h>
#include <itkImageFileWriter.h>
#include <itkCastImageFilter.h>
#include <itkGradientMagnitudeImageFilter.h>
#include <itkThresholdImageFilter.h>
#include <itkConnectedThresholdImageFilter.h>
#include <itkMaskNegatedImageFilter.h>
#include <itkMaskImageFilter.h>
#include <itkResampleImageFilter.h>
#include <itkAffineTransform.h>
#include <itkLinearInterpolateImageFunction.h>
#include <itkImage.h>

typedef itk::Image<float, 2> InternalImageType;
typedef itk::RGBPixel<unsigned char> RGBPixelType;
typedef itk::Image<RGBPixelType, 2> InputImageType;

InternalImageType::Pointer setSourceImage(QImage *scImage);
InternalImageType::Pointer createImage(int w, int h);
InternalImageType::Pointer removeBkg(InternalImageType::Pointer itkCImage);
void writeImage(InternalImageType::Pointer image, const QString &filename);
QList<int> getYProjection(InternalImageType::Pointer image, QRect *rc = 0);

//----------------------------------------------------------------

ChequeRecognizer::ChequeRecognizer(QObject *parent) : QObject(parent) {
     tessApi.Init("./tessdata", "rus");
}

Cheque ChequeRecognizer::recognizeFile(const QString &fileName, bool *ok) {
    Cheque ch;

    int segCount = prepareData(fileName);

    QFile out("out.txt");
    out.open(QFile::WriteOnly);
    QTextStream ts(&out);

    QStringList cdata;
    for(int i=0; i<segCount; i++) {
        QString str = recognizeString(QString("seg%1.png").arg(i));
        ts << str;
        cdata.append(str);
    }
    ts.flush();
    out.close();

    QList<int> bs;
    QRegExp rxd("\\d{1,2}[\\./]\\d{1,2}[\\./]\\d{1,2}");
    QRegExp rx("([\\doOÓŒ¯Â]+)\\s*[,Ç:\\.]\\s*([\\doOÓŒ¯Â‘Ÿ]+)");
    QRegExp rxs("([\\dÁ]+\\.|:)");
    bool found = false, dateFound = false, exit = false;
    for(int i=0; i<cdata.size(); i++) {
        if(exit) break;
        if(rxd.indexIn(cdata[i]) != -1) {
            QString dt = rxd.cap(0);
            ch.date = QDateTime(QDate::fromString(dt, "dd.MM.yy")).addYears(100);
            dateFound = true;
            continue;
        }
        int pos = 0;
        QList<double> nums;
        while(true) {
            pos = rx.indexIn(cdata[i], pos);
            if(pos != -1) {
                QString str = rx.cap(0);
                QString n1 = rx.cap(1).replace(QRegExp("[oOÓŒ¯Â‘Ÿ]"), "0");
                QString n2 = rx.cap(2).replace(QRegExp("[oOÓŒ¯Â‘Ÿ]"), "0");
                nums.append(QString("%1.%2").arg(n1.toInt()).arg(n2.toInt()).toDouble());
                pos += str.size();
                found = true;
            } else if(found) {
                QString resStr = "";
                for(int j = i-1; j > (bs.isEmpty() ? 0 : bs.last()); j--) {
                    int spos = rxs.indexIn(cdata[j]);
                    if(spos != -1) {
                        resStr.prepend(cdata[j].mid(spos+rxs.cap(0).size()).trimmed());
                        break;
                    } else {
                        resStr.prepend(cdata[j].trimmed());
                    }
                }
                resStr.replace(QRegExp("[\n|\\._:\\d]"), "");
                resStr = resStr.trimmed().toLower();
                resStr[0] = resStr[0].toUpper();
                if(resStr.contains("ÔÓÍÛÔÓÍ", Qt::CaseInsensitive) || resStr.contains("ÓÔÎ‡Ú‡", Qt::CaseInsensitive)) {
                    exit = true;
                    break;
                }
                bs.append(i);
                int npos = 0;
                bool adeq = true;
                while(true) {
                    int spi = resStr.indexOf(QRegExp("[ ,Ç\\(\\)]"), npos);
                    if( (spi == -1 ? resStr.size()-npos : spi-npos) >= 20) {
                        adeq = false;
                        break;
                    }
                    if(spi == -1) break;
                    npos = spi+1;
                }
                if(adeq && !resStr.contains('\\')) {
                    int sind = resStr.indexOf(QRegExp("[\\(\\)]"));
                    if(sind != -1) resStr = resStr.left(sind);
                    resStr.replace("‰", "‡");
                    ch.goods.append(Good(resStr, nums.last()));
                }
                found = false;
                break;
            } else break;
        }
    }

    cleanUp(segCount);

    double sum = 0.0;
    for(int i=0; i<ch.goods.size(); i++) sum += ch.goods[i].cost*ch.goods[i].count;
    ch.total = sum;
    if(!dateFound) ch.date = QDateTime::currentDateTime();

    return ch;
}

/********************************************************************************/
// Platform specific
/********************************************************************************/

QString ChequeRecognizer::recognizeString(const QString &fileName) {
    PIX *pix = pixRead(fileName.toLocal8Bit().data());
    if(pix == 0) return QString();

    tessApi.SetImage(pix);
    char *text = tessApi.GetUTF8Text();
    QString res = QString::fromUtf8(text);

    pixDestroy(&pix);
    return res;
}

void ChequeRecognizer::cleanUp(int fcount) {
    for(int i=0; i<fcount; i++) QFile::remove(QString("seg%1.png").arg(i));
}

int ChequeRecognizer::prepareData(const QString &fileName) {
    QImage img(fileName);
    InternalImageType::Pointer itkCImage = setSourceImage(&img);
    itkCImage = removeBkg(itkCImage);
    writeImage(itkCImage, "tmp.png");
    QList<int> yProj = getYProjection(itkCImage);
    int mean = 0;
    for(int i=0; i<yProj.size(); i++) mean += yProj[i];
    mean /= yProj.size()*3.0;

    QList<int> ranges;
    bool wasMax = true, skipFirst = false;
    int rStart = 0;
    for(int i=0; i<yProj.size(); i++) {
        if(yProj[i] < mean && wasMax) {
            rStart = i; wasMax = false;
        } else if(yProj[i] > mean && !wasMax) {
            ranges.append( (i-1+rStart) / 2);
            if(rStart == 0) skipFirst = true;
            wasMax = true;
        }
    }
    if(!wasMax) ranges.append( (yProj.size()-1+rStart) / 2);

    int sourceWidth = itkCImage->GetLargestPossibleRegion().GetSize()[0];
    int yPos = skipFirst ? ranges[0] : 0;
    int counter = 0;
    for(int i = skipFirst ? 1 : 0; i<ranges.size(); i++) {
        InternalImageType::IndexType iIndex, rIndex;
        InternalImageType::Pointer sImage = createImage(sourceWidth, ranges[i]-yPos);
        for(int y = yPos; y<ranges[i]; y++) {
            for(int x = 0; x<sourceWidth; x++) {
                iIndex[0] = x; iIndex[1] = y;
                rIndex[0] = x; rIndex[1] = y-yPos;
                sImage->SetPixel(rIndex, itkCImage->GetPixel(iIndex));
            }
        }
        writeImage(sImage, QString("seg%1.png").arg(counter));
        yPos = ranges[i];
        counter++;
    }

    return counter;
}

/********************************************************************************/
// ITK Functions
/********************************************************************************/

InternalImageType::Pointer setSourceImage(QImage *scImage) {
    InternalImageType::Pointer itkSImage;

    int sourceWidth = scImage->width();
    int sourceHeight = scImage->height();

    RGBPixelType *imgBuffer = new RGBPixelType[sourceWidth*sourceHeight];
    int index = 0;
    QColor tempColor;
    for(int i=0; i<sourceHeight; i++) {
        for(int j=0; j<sourceWidth; j++) {
            tempColor.setRgb(scImage->pixel(j, i));
            imgBuffer[index].Set(tempColor.red(), tempColor.green(), tempColor.blue());
            index++;
        }
    }

    typedef itk::ImportImageFilter<RGBPixelType, 2> ImportFilterType;

    ImportFilterType::Pointer importFilter = ImportFilterType::New();
    ImportFilterType::SizeType imgSize;
    imgSize[0] = sourceWidth;
    imgSize[1] = sourceHeight;

    ImportFilterType::IndexType imgStart;
    imgStart.Fill(0);

    ImportFilterType::RegionType imgRegion;
    imgRegion.SetIndex(imgStart);
    imgRegion.SetSize(imgSize);

    importFilter->SetRegion(imgRegion);

    double imgOrigin[2]; imgOrigin[0] = 0.0; imgOrigin[1] = 0.0;
    double imgSpacing[2]; imgSpacing[0] = 1.0; imgSpacing[1] = 1.0;

    importFilter->SetOrigin(imgOrigin);
    importFilter->SetSpacing(imgSpacing);
    importFilter->SetImportPointer(imgBuffer, sourceWidth*sourceHeight, false);

    typedef itk::RGBToLuminanceImageFilter<InputImageType, InternalImageType> GrayFilter;
    GrayFilter::Pointer gFilter = GrayFilter::New();
    gFilter->SetInput(importFilter->GetOutput());

    itkSImage = gFilter->GetOutput();
    itkSImage->Update();
    return itkSImage;
}

//----------------------------------------------------------------

void writeImage(InternalImageType::Pointer image, const QString &filename) {
    typedef itk::Image<unsigned char, 2> WriterImageType;
    typedef itk::CastImageFilter<InternalImageType, WriterImageType> CasterType;
    CasterType::Pointer caster = CasterType::New();
    caster->SetInput(image);

    char *buf = new char[filename.size()+1];
    for(int i=0; i<filename.size(); i++) buf[i] = filename[i].toAscii();
    buf[filename.size()] = 0;

    typedef itk::ImageFileWriter<WriterImageType> WriterType;
    WriterType::Pointer writer = WriterType::New();
    writer->SetFileName(buf);
    writer->SetInput(caster->GetOutput());
    writer->Update();
}

InternalImageType::Pointer createImage(int w, int h) {
    InternalImageType::Pointer sImage = InternalImageType::New();
    InternalImageType::RegionType region;
    InternalImageType::IndexType iIndex;
    iIndex[0] = 0; iIndex[1] = 0;
    InternalImageType::SizeType iSize;
    iSize[0] = w; iSize[1] = h;
    region.SetIndex(iIndex);
    region.SetSize(iSize);
    sImage->SetRegions(region);
    sImage->Allocate();
    return sImage;
}

//----------------------------------------------------------------

InternalImageType::Pointer removeBkg(InternalImageType::Pointer itkCImage) {
    typedef itk::GradientMagnitudeImageFilter<InternalImageType, InternalImageType> GFType;
    GFType::Pointer gradientFilter = GFType::New();
    gradientFilter->SetInput(itkCImage);
    gradientFilter->Update();

//    typedef itk::ThresholdImageFilter<InternalImageType> TFType;
//    TFType::Pointer threshFilter = TFType::New();
//    threshFilter->SetInput(gradientFilter->GetOutput());
//    threshFilter->SetOutsideValue(0);
//    threshFilter->ThresholdOutside(0, 255);

    InternalImageType::Pointer tempImage = gradientFilter->GetOutput();
    tempImage->Update();
    InternalImageType::IndexType seedIndex;
    seedIndex[0] = 793; seedIndex[1] = 998;
    InternalImageType::PixelType seedPixel = tempImage->GetPixel(seedIndex);

    typedef itk::ConnectedThresholdImageFilter<InternalImageType, InternalImageType> CTFType;
    CTFType::Pointer ctFilter = CTFType::New();
    ctFilter->SetLower(seedPixel - 11);
    ctFilter->SetUpper(seedPixel + 11);
    ctFilter->SetReplaceValue(255);
    ctFilter->SetInput(tempImage);
    ctFilter->SetSeed(seedIndex);

    typedef itk::MaskNegatedImageFilter<InternalImageType, InternalImageType, InternalImageType> MNFType;
    MNFType::Pointer maskFilter = MNFType::New();
    maskFilter->SetInput1(itkCImage);
    maskFilter->SetInput2(ctFilter->GetOutput());
    maskFilter->SetOutsideValue(255.0);

    itkCImage = maskFilter->GetOutput();
    itkCImage->Update();
    return itkCImage;
}

//----------------------------------------------------------------

QList<int> getYProjection(InternalImageType::Pointer image, QRect *rc) {
    int pCounter, w, h, sx, sy;
    if(!rc) {
        InternalImageType::SizeType iSize = image->GetLargestPossibleRegion().GetSize();
        w = iSize[0]; h = iSize[1];
        sx = 0; sy = 0;
    } else {
        w = rc->right();
        h = rc->bottom();
        sx = rc->left();
        sy = rc->top();
    }
    InternalImageType::PixelType iPixel;
    InternalImageType::IndexType iIndex;
    QList<int> res;
    for(int y=sy; y<h; y++) {
        pCounter = 0;
        for(int x=sx; x<w; x++) {
            iIndex[0] = x; iIndex[1] = y;
            iPixel = image->GetPixel(iIndex);
            if(iPixel<200.0) pCounter++;
        }
        res.append(pCounter);
    }
    return res;
}
