#-------------------------------------------------
#
# Project created by QtCreator 2019-03-15T10:36:11
#
#-------------------------------------------------

QT       += core gui opengl concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = clbrub
TEMPLATE = app
CONFIG += link_pkgconfig configA
PKGCONFIG += python3-embed

configA {
    LIBS += -lqscintilla2_qt5
}

configB {
    LIBS += -lqt5qscintilla2
}

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11 debug

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
    $$FLEXSOURCES \
    genTrain.py

SOURCES += \
        main.cpp \
        CMainWindow.cpp \
        C3dView.cpp \
        CRubik.cpp \
        CMouvement.cpp \
        CCubeCore.cpp \
    CMapView.cpp

HEADERS += \
        CMainWindow.h \
        C3dView.h \
        CRubik.h \
        CMouvement.h \
        CCubeCore.h \
        CScanner.h \
    CMapView.h

FORMS += \
        CMainWindow.ui

# Module Python autonome rubikcore (sans Qt), construit par setup.py à chaque make si ses sources changent.
# Interpréteur modifiable : qmake PYTHON=/chemin/vers/python3
isEmpty(PYTHON): PYTHON = python3
RUBIKCORE_SO = $$PWD/rubikcore$$system($$PYTHON -c \"import sysconfig; print(sysconfig.get_config_var(\'EXT_SUFFIX\'))\")

rubikcore.target = $$RUBIKCORE_SO
# --force : make décide déjà quand reconstruire, et le .so doit être daté du build
rubikcore.commands = cd $$PWD && $$PYTHON setup.py build_ext --inplace --force
rubikcore.depends = $$PWD/rubikcore.cpp $$PWD/CCubeCore.cpp $$PWD/CCubeCore.h $$PWD/setup.py

# Alias : make rubikcore
rubikcorealias.target = rubikcore
rubikcorealias.depends = $$RUBIKCORE_SO
rubikcorealias.commands = @echo "rubikcore à jour : $$RUBIKCORE_SO"
rubikcorealias.CONFIG = phony

QMAKE_EXTRA_TARGETS += rubikcore rubikcorealias
PRE_TARGETDEPS += $$RUBIKCORE_SO
QMAKE_CLEAN += $$RUBIKCORE_SO

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
        clbrub.qrc

DISTFILES += \
    train.py \
    resolvpy.py \
    setup.py \
    rubikcore.cpp
