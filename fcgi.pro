QT += core
QT -= gui

CONFIG += c++14

TARGET = fcgi
CONFIG += console
CONFIG -= app_bundle

#QMAKE_CXXFLAGS+= -fsanitize=address -fno-omit-frame-pointer
#QMAKE_CFLAGS+= -fsanitize=address -fno-omit-frame-pointer
#QMAKE_LFLAGS+= -fsanitize=address

INCLUDEPATH += /home/swap/fcgi/include/

TEMPLATE = app

SOURCES += main.cpp \
    libfcgi/fcgio.cpp \
    libfcgi/fcgi_stdio.c \
    libfcgi/fcgiapp.c \
    libfcgi/os_unix.cpp

HEADERS += \
    include/fastcgi.h \
    include/fcgi_stdio.h \
    include/fcgiapp.h \
    include/fcgio.h \
    include/fcgios.h
