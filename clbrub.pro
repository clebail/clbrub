#-------------------------------------------------
#
# Project created by QtCreator 2019-03-15T10:36:11
#
#-------------------------------------------------

QT       += core gui opengl concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = clbrub
TEMPLATE = app
CONFIG += link_pkgconfig
PKGCONFIG += python3

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

FLEXSOURCES = mouvements.l
BISONSOURCES = mouvements.y

flex.commands = flex -o ${QMAKE_FILE_OUT} --c++ ${QMAKE_FILE_IN}
flex.input = FLEXSOURCES
flex.output = mouvements.l.cpp
flex.variable_out = SOURCES
flex.depends = mouvements.y.hpp
flex.name = flex
QMAKE_EXTRA_COMPILERS += flex

bison.commands = bison -o ${QMAKE_FILE_OUT} -d ${QMAKE_FILE_IN}
bison.input = BISONSOURCES
bison.output = mouvements.y.cpp
bison.variable_out = SOURCES
bison.name = bison
QMAKE_EXTRA_COMPILERS += bison

bisonheader.commands = @true
bisonheader.input = BISONSOURCES
bisonheader.output = mouvements.y.hpp
bisonheader.variable_out = HEADERS
bisonheader.name = bison header
bisonheader.depends = mouvements.y.cpp
QMAKE_EXTRA_COMPILERS += bisonheader

OTHER_FILES += \
    $$BISONSOURCES \
    $$FLEXSOURCES

SOURCES += \
        main.cpp \
        CMainWindow.cpp \
        C3dView.cpp \
        CRubik.cpp \
        CMouvement.cpp

HEADERS += \
        CMainWindow.h \
        C3dView.h \
        CRubik.h \
        CMouvement.h \
        CScanner.h

FORMS += \
        CMainWindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
        clbrub.qrc
