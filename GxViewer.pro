
QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QMAKE_CXXFLAGS +=-std=c++11


TEMPLATE = app

LIBS += -lgxiapi\

TARGET = GxViewer
TEMPLATE = app

unix:!mac:QMAKE_LFLAGS += -L/usr/lib -L./ -Wl,--rpath=.:/usr/lib

INCLUDEPATH += ./include/

SOURCES += main.cpp\
    ExposureGain.cpp \
    GxViewer.cpp \
    ImageImprovement.cpp \
    AcquisitionThread.cpp \
    Fps.cpp \
    FrameRateControl.cpp \
    WhiteBalance.cpp \
    Common.cpp

HEADERS += $$files(./include/*.h)

HEADERS  += \
    ExposureGain.h \
    GxViewer.h \
    ImageImprovement.h \
    AcquisitionThread.h \
    Fps.h \
    FrameRateControl.h \
    Common.h \
    WhiteBalance.h

FORMS    += \
    ExposureGain.ui \
    GxViewer.ui \
    ImageImprovement.ui \
    FrameRateControl.ui \
    WhiteBalance.ui
INCLUDEPATH+=/usr/local/include\
             /usr/local/include/opencv4\
            /usr/local/include/opencv4/opencv2
LIBS +=/usr/local/lib/libopencv_*

DISTFILES +=
