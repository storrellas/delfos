#-------------------------------------------------
#
# Project created by QtCreator 2017-01-10T14:39:04
#
#-------------------------------------------------

QT       += core gui

CONFIG += c++11
CONFIG += console

INCLUDEPATH += "/usr/local/boost_1.62/"

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = coreclient
TEMPLATE = app

LIBS += -lwcore

# Output directories
OBJECTS_DIR    = $${_PRO_FILE_PWD_}/obj
UI_DIR         = $${_PRO_FILE_PWD_}/obj
MOC_DIR        = $${_PRO_FILE_PWD_}/obj
DESTDIR        = $${_PRO_FILE_PWD_}/bin

# Sources
SOURCES += src/main.cpp\
           src/mainwindow.cpp \
           src/rmform.cpp \
           src/pbxform.cpp \
           src/kpiform.cpp \           
           src/acmform.cpp \
           src/rtvform.cpp \
           src/gariform.cpp \
           src/lmform.cpp \

# Headers
HEADERS  += src/mainwindow.h \            
            src/commonparameters.h \            
            src/rmform.h \
            src/kpiform.h \
            src/pbxform.h \
            src/acmform.h \
            src/rtvform.h \
            src/gariform.h \
            src/lmform.h \

# Forms
FORMS    += src/mainwindow.ui \
            src/rmform.ui \
            src/pbxform.ui \
            src/kpiform.ui \
            src/acmform.ui \
            src/rtvform.ui \
            src/gariform.ui \
            src/lmform.ui
