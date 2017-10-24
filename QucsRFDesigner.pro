HEADERS += Schematic/node.h Schematic/graphwidget.h qcustomplot.h \
    Schematic/component.h \
    Schematic/symbol.h \
    Schematic/wire.h \
    UI/preferencesdialog.h \
    UI/TuningBox.h \
    Filtering/CanonicalFilter.h \
    Filtering/Network.h \
    Filtering/EllipticFilter.h \
    SPAR/MathOperations.h \
    SPAR/sparengine.h \
    SPAR/mat.h \
    UI/QucsRFDesignerWindow.h \
    UI/smithchart.h \
    Filtering/DirectCoupledFilters.h \
    Filtering/LowpassPrototypeCoeffs.h \
    Filtering/EdgeCoupled.h


SOURCES += main.cpp \
    Schematic/component.cpp \
    Schematic/wire.cpp \
    UI/preferencesdialog.cpp \
    UI/TuningBox.cpp \
    Filtering/CanonicalFilter.cpp \
    Filtering/EllipticFilter.cpp \
    SPAR/MathOperations.cpp \
    SPAR/sparengine.cpp \
    SPAR/mat.cpp \
    UI/QucsRFDesignerWindow.cpp \
    UI/smithchart.cpp \
    UI/filterdesigngui.cpp \
    Filtering/DirectCoupledFilters.cpp \
    Filtering/LowpassPrototypeCoeffs.cpp \
    Filtering/EdgeCoupled.cpp

SOURCES += Schematic/node.cpp Schematic/graphwidget.cpp qcustomplot.cpp

QT += widgets
QT += sql
QT += svg
RESOURCES = resources.qrc

DISTFILES += \
    bitmaps/qucsRFdesigner.png
