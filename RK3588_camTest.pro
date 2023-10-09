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
    Camera/general_camera.cpp \
    Camera/mpp_compressor.cpp \
    Camera/videoplayer.cpp \
    Camera/mpp_player.cpp \
    Camera/MvCamera.cpp \
    ImageProcess/image_processing.cpp \
    ImageProcess/imghdr.cpp \
    RKNN/rknn_inferencer.cpp \
    RKNN/utils/drawing.cpp \
    RKNN/utils/mpp_decoder.cpp \
    RKNN/utils/mpp_encoder.cpp \
    RKNN/utils/postprocess.cc \
    main.cpp \
    mainwindow.cpp \
    pictureview.cpp \

HEADERS += \
    Camera/general_camera.h \
    Camera/mpp_compressor.h \
    Camera/videoplayer.h \
    Camera/mpp_player.h \
    Camera/MvCamera.h \
    ImageProcess/image_processing.h \
    ImageProcess/imghdr.h \
    RKNN/rknn_inferencer.h \
    RKNN/utils/drawing.h \
    RKNN/utils/mpp_decoder.h \
    RKNN/utils/mpp_encoder.h \
    RKNN/utils/postprocess.h \
    mainwindow.h \
    pictureview.h \

FORMS += \
    mainwindow.ui

target.path = /home/toybrick/Works/CrossCompiled/$${TARGET}
INSTALLS += target

QMAKE_LFLAGS += -no-pie

#unix: CONFIG += link_pkgconfig
#unix: PKGCONFIG += opencv4 \
#                   libavcodec\
#                   libavformat \
#                   libavutil \
#                   libswresample \
#                   libswscale

unix:!macx: LIBS += \
-L//home/toybrick/opencv-build/install/lib/ -lopencv_world \
-L//home/toybrick/ffmpegInstall/lib/ -lavcodec \
-L//home/toybrick/ffmpegInstall/lib/ -lavformat \
-L//home/toybrick/ffmpegInstall/lib/ -lavutil \
-L//home/toybrick/ffmpegInstall/lib/ -lswresample \
-L//home/toybrick/ffmpegInstall/lib/ -lswscale \
-L/home/toybrick/librknn_api/lib/ -lrockchip_mpp \
-L/home/toybrick/librknn_api/lib/ -lrga \
-L/home/toybrick/librknn_api/lib/ -lmk_api \
-L/home/toybrick/librknn_api/lib/ -lrknnrt \
-L/opt/MVS/lib/aarch64 -lMvCameraControl \
-L/opt/MVS/bin \

INCLUDEPATH += \
/home/toybrick/ffmpegInstall/include \
/home/toybrick/opencv-build/install/include/opencv4 \
/home/toybrick/librknn_api/include \
/home/toybrick/3rdparty/mpp/include \
/home/toybrick/3rdparty/rga/RK3588/include \
/home/toybrick/3rdparty/zlmediakit/include \
/opt/MVS/include \
DEPENDPATH += \
/home/toybrick/ffmpegInstall/include \
/home/toybrick/opencv-build/install/include/opencv4 \
/home/toybrick/librknn_api/include \
/home/toybrick/3rdparty/mpp/include \
/home/toybrick/3rdparty/rga/RK3588/include \
/home/toybrick/3rdparty/zlmediakit/include \
/opt/MVS/include \

