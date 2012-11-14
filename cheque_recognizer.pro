#-------------------------------------------------
#
# Project created by QtCreator 2012-11-12T11:28:14
#
#-------------------------------------------------

QT       += core gui sql

TARGET = ChequeRecognizer
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    previewer.cpp \
    lcdialog.cpp \
    statrender.cpp \
    statwindow.cpp
win: SOURCES += recogniser_win.cpp
unix: SOURCES += recognizer.cpp

HEADERS  += mainwindow.h \
    lcdialog.h \
    previewer.h \
    statrender.h \
    statwindow.h
win: HEADERS += recogniser_win.h
unix: HEADERS += recognizer.h

win32 {
   INCLUDEPATH += C:/libs/qwt-6.0.1/include \
                D:/libs/InsightToolkit-3.20.1/Code/Algorithms \
                D:/libs/InsightToolkit-3.20.1/Code/BasicFilters \
                D:/libs/InsightToolkit-3.20.1/Code/Common \
                D:/libs/InsightToolkit-3.20.1/Code/Numerics \
                D:/libs/InsightToolkit-3.20.1/Code/IO \
                D:/libs/InsightToolkit-3.20.1/Code/Numerics/FEM \
                D:/libs/InsightToolkit-3.20.1/Code/Numerics/NeuralNetworks \
                D:/libs/InsightToolkit-3.20.1/Code/SpatialObject \
                D:/libs/InsightToolkit-3.20.1/Code/Numerics/Statistics \
                D:/libs/InsightToolkit-3.20.1/Utilities/MetaIO \
                D:/libs/InsightToolkit-3.20.1/Utilities/NrrdIO \
                D:/libs/InsightToolkit-3.20.1/Utilities/DICOMParser \
                D:/libs/InsightToolkit-3.20.1/Utilities/nifti/niftilib \
                D:/libs/InsightToolkit-3.20.1/Utilities/nifti/znzlib \
                D:/libs/InsightToolkit-3.20.1/Utilities/itkExtHdrs \
                D:/libs/InsightToolkit-3.20.1/Utilities \
                D:/libs/InsightToolkit-3.20.1/Utilities/vxl/v3p/netlib \
                D:/libs/InsightToolkit-3.20.1/Utilities/vxl/vcl \
                D:/libs/InsightToolkit-3.20.1/Utilities/vxl/core \
                D:/libs/InsightToolkit-3.20.1/build/Utilities/NrrdIO \
                D:/libs/InsightToolkit-3.20.1/build/Utilities/DICOMParser \
                D:/libs/InsightToolkit-3.20.1/build/Utilities/expat \
                D:/libs/InsightToolkit-3.20.1/build/Utilities \
                D:/libs/InsightToolkit-3.20.1/build/Utilities/vxl/vcl \
                D:/libs/InsightToolkit-3.20.1/build/Utilities/vxl/core \
                D:/libs/InsightToolkit-3.20.1/build/Utilities/gdcm
}
unix {
   INCLUDEPATH += /usr/include/qwt
}

win32: LIBS += -lqwt -llept -ltesseract -lITKCommon -litkvnl -litkvnl_algo -litkvcl -litksys -lITKIO
unix: LIBS += -lqwt -llept -ltesseract -lopencv_core -lopencv_imgproc -lopencv_highgui
TRANSLATIONS += trans.ts

RESOURCES += \
    resources.qrc

CONFIG += release
