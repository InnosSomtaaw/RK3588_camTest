QT       += core gui network xml
qtHaveModule(printsupport): QT += printsupport
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Camera/videoplayer.cpp \
    ImageProcess/cv_stereomatcher.cpp \
    ImageProcess/image_processing.cpp \
    ImageProcess/imghdr.cpp \
    RKNN/rknn_inferencer.cpp \
    RKNN/utils/drawing.cpp \
    RKNN/utils/mpp_decoder.cpp \
    RKNN/utils/mpp_encoder.cpp \
    RKNN/utils/postprocess.cc \
    main.cpp \
    mainwindow.cpp \
    pictureview.cpp

HEADERS += \
    Camera/videoplayer.h \
    ImageProcess/cv_stereomatcher.h \
    ImageProcess/image_processing.h \
    ImageProcess/imghdr.h \
    RKNN/rknn_inferencer.h \
    RKNN/utils/drawing.h \
    RKNN/utils/mpp_decoder.h \
    RKNN/utils/mpp_encoder.h \
    RKNN/utils/postprocess.h \
    mainwindow.h \
    pictureview.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

QMAKE_LFLAGS += -no-pie

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += opencv4 \
                   libavcodec\
                   libavformat \
                   libavutil \
                   libswresample \
                   libswscale

#unix:!macx: LIBS += -L$$PWD/../3rdparty/mpp/Linux/aarch64/ -lrockchip_mpp \
#                    -L$$PWD/../3rdparty/rga/RK3588/lib/Linux/aarch64/ -lrga \
#                    -L$$PWD/../3rdparty/zlmediakit/aarch64/ -lmk_api

unix:!macx: LIBS += -L/home/toybrick/Works/lib/ -lrockchip_mpp \
                    -L/home/toybrick/Works/lib/ -lrga \
                    -L/home/toybrick/Works/lib/ -lmk_api \
                    -L/home/toybrick/Works/lib/ -lrknnrt

INCLUDEPATH += $$PWD/../3rdparty/mpp/include \
               $$PWD/../3rdparty/rga/RK3588/include \
               $$PWD/../3rdparty/zlmediakit/include
DEPENDPATH += $$PWD/../3rdparty/mpp/include \
              $$PWD/../3rdparty/rga/RK3588/include \
              $$PWD/../3rdparty/zlmediakit/include

