QT += core network
QT -= gui

CONFIG += c++11

CONFIG -= app_bundle #Please apple, don't make a bundle

DESTDIR = bin
OBJECTS_DIR = build
MOC_DIR = build
TARGET = fftw_load_test
TEMPLATE = app

HEADERS +=

SOURCES += fftw_load_test_main.cpp

#FFTW
LIBS += -fopenmp -lfftw3 -lfftw3_threads
