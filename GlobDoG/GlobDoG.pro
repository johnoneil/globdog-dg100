TEMPLATE = app
QT = gui core webkit
CONFIG += qt warn_on console debug
DESTDIR = bin
OBJECTS_DIR = build
MOC_DIR = build
UI_DIR = build
FORMS = ui/dialog.ui ui/aboutbox.ui
HEADERS = src/dialogimpl.h \
 src/serial.h \
 ../common/dg100.h \
 ../common/Queue.h \
 ../common/nmea.h \
 src/aboutboximpl.h
SOURCES = src/dialogimpl.cpp \
 src/main.cpp \
 src/serial.cpp \
 ../common/dg100.cpp \
 ../common/Queue.cpp \
 ../common/nmea.cpp \
 src/aboutboximpl.cpp
RESOURCES += assets/assets.qrc
TARGET = globdog
