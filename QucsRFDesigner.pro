HEADERS += Schematic/node.h Schematic/graphwidget.h qcustomplot.h \
    Schematic/component.h \
    Schematic/symbol.h \
    Schematic/wire.h \
    UI/preferencesdialog.h \
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
    Filtering/EdgeCoupled.h \
    UI/FilterDesignTool.h \
    UI/PowerCombiningTool.h \
    PowerCombining/PowerCombinerDesigner.h \
    general.h \
    UI/InterceptPointsTool.h \
    UI/SmithChartTool.h


SOURCES += main.cpp \
    Schematic/component.cpp \
    Schematic/wire.cpp \
    UI/preferencesdialog.cpp \
    Filtering/CanonicalFilter.cpp \
    Filtering/EllipticFilter.cpp \
    SPAR/MathOperations.cpp \
    SPAR/sparengine.cpp \
    SPAR/mat.cpp \
    UI/QucsRFDesignerWindow.cpp \
    UI/smithchart.cpp \
    Filtering/DirectCoupledFilters.cpp \
    Filtering/LowpassPrototypeCoeffs.cpp \
    Filtering/EdgeCoupled.cpp \
    UI/FilterDesignTool.cpp \
    UI/PowerCombiningTool.cpp \
    PowerCombining/PowerCombinerDesigner.cpp \
    PowerCombining/Wilkinson2way.cpp \
    general.cpp \
    PowerCombining/MultistageWilkinson.cpp \
    PowerCombining/ChebyshevTaper.cpp \
    PowerCombining/TJunction.cpp \
    PowerCombining/Branchline.cpp \
    PowerCombining/DoubleBoxBranchline.cpp \
    PowerCombining/Bagley.cpp \
    PowerCombining/Gysel.cpp \
    PowerCombining/Lim_Eom.cpp \
    PowerCombining/Wilkinson3way_ImprovedIsolation.cpp \
    PowerCombining/Recombinant3WayWilkinson.cpp \
    UI/InterceptPointsTool.cpp \
    UI/SmithChartTool.cpp

SOURCES += Schematic/node.cpp Schematic/graphwidget.cpp qcustomplot.cpp

QT += widgets
QT += sql
QT += svg
RESOURCES = resources.qrc

DISTFILES += \
    bitmaps/qucsRFdesigner.png \
    TODO \
    ZverevTables.sql
