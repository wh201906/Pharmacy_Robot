QT       += core gui serialport

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
    main.cpp \
    mainwindow.cpp \
    module/detectors.cpp \
    module/facerecognizer.cpp \
    module/movecontroller.cpp \
    module/rfid.cpp \
    module/servodriver.cpp \
    rfidtestdialog.cpp \
    servotestdialog.cpp

HEADERS += \
    mainwindow.h \
    module/detectors.hpp \
    module/facerecognizer.h \
    module/movecontroller.h \
    module/rfid.h \
    module/servodriver.h \
    rfidtestdialog.h \
    servotestdialog.h

FORMS += \
    mainwindow.ui \
    rfidtestdialog.ui \
    servotestdialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

INCLUDEPATH += \
    /opt/intel/openvino/opencv/include/opencv2 \
    /opt/intel/openvino/opencv/include \
    /opt/intel/openvino/deployment_tools/inference_engine/include \
    /opt/intel/openvino/deployment_tools/inference_engine/samples/common \
    /opt/intel/openvino/deployment_tools/ngraph/include \
    /usr/include/python3.6 \

LIBS += \
    /opt/intel/openvino/opencv/lib/libopencv*.so \
    /opt/intel/openvino_2020.4.287/deployment_tools/inference_engine/lib/intel64/*.so \
    /opt/intel/openvino_2020.4.287/deployment_tools/ngraph/lib/libngraph.so \
    /usr/lib/python3.6/config-3.6m-x86_64-linux-gnu/libpython3.6.so \
#    /opt/intel/openvino_2020.4.287/deployment_tools/inference_engine/external/tbb/lib/libtbb.so.2 \
#    /opt/intel/openvino_2020.4.287/opencv/lib/libopencv_core.so.4.4

DISTFILES +=
