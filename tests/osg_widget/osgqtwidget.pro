TEMPLATE = lib
SOLUTION_ROOT = ./../..
DEV_ROOT = $$(SIMEX_DIR)_qt5
SIMEX_DIR_FULL = $$(SIMEX_DIR)_qt5

include($$SOLUTION_ROOT/base.pri)

INCLUDEPATH += $$SOLUTION_ROOT
#############
# include paths

INCLUDEPATH += ./
INCLUDEPATH += \
               $$(SIMEX_DIR_FULL)/src/_Include                \
               $$SOLUTION_ROOT/common          
               
INCLUDEPATH += $$INCLUDE_PATH
INCLUDEPATH += $$EXT_INCLUDE
INCLUDEPATH += $$_PRO_FILE_PWD_
INCLUDEPATH += $$SOLUTION_ROOT/tests/widget


LIBS += -L$$SIMEX_DIR_FULL/ext/lib/gl/$$PATH_SUFFIX
 
CONFIG(debug,debug|release){
   LIBS += -lvisual
} else {
   LIBS += -lvisual
}

win32 {
    LIBS += glew32s.lib
}

unix {
    LIBS += -lGLEW -lX11
}

CONFIG += qt
QT += core gui opengl

DEFINES += VISUAL_EXPORTS
# For spark only
DEFINES += QT_NO_EMIT
DEFINES += QT_OPENGL_LIB GLEW_STATIC

PRECOMPILED_HEADER = precompiled.h

HEADERS += \
    stdafx.h \
    OSGWidget.h \
    $$SOLUTION_ROOT\tests\widget\widget.h \
    $$SOLUTION_ROOT\tests\widget\core\core.h \
    $$SOLUTION_ROOT\tests\widget\core\widget\core_widget.h

SOURCES += \
    OSGWidget.cpp \
    $$SOLUTION_ROOT\tests\widget\widget.cpp \
    $$SOLUTION_ROOT\tests\widget\core\core.cpp \
    $$SOLUTION_ROOT\tests\widget\core\widget\core_widget.cpp
    

 