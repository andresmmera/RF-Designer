HEADERS += Schematic/node.h Schematic/graphwidget.h qcustomplot.h \
    Schematic/component.h \
    Schematic/symbol.h \
    Schematic/wire.h \
    UI/preferencesdialog.h \
    Filtering/CanonicalFilter.h \
    Schematic/Network.h \
    Filtering/EllipticFilter.h \
    SPAR/MathOperations.h \
    SPAR/sparengine.h \
    SPAR/mat.h \
    UI/QucsRFDesignerWindow.h \
    Filtering/DirectCoupledFilters.h \
    Filtering/LowpassPrototypeCoeffs.h \
    UI/FilterDesignTool.h \
    UI/PowerCombiningTool.h \
    PowerCombining/PowerCombinerDesigner.h \
    general.h \
    Filtering/QuarterWaveFilters.h \
    Filtering/SteppedImpedanceFilter.h \
    Filtering/EndCoupled.h \
    Filtering/CapacitivelyCoupledShuntResonatorsFilter.h \
    Filtering/CoupledLineBandpassFilter.h \
    Filtering/CoupledLineHarmonicRejectionSIRBandpassFilter.h \
    UI/AttenuatorDesignTool.h \
    Schematic/SchematicContent.h \
    Schematic/structures.h \
    AttenuatorDesign/TeeAttenuator.h \
    AttenuatorDesign/PiAttenuator.h \
    AttenuatorDesign/BridgedTeeAttenuator.h \
    AttenuatorDesign/ReflectionAttenuator.h \
    AttenuatorDesign/QW_SeriesAttenuator.h \
    AttenuatorDesign/QW_ShuntAttenuator.h \
    Schematic/infoclasses.h \


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
    Filtering/DirectCoupledFilters.cpp \
    Filtering/LowpassPrototypeCoeffs.cpp \
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
    Filtering/QuarterWaveFilters.cpp \
    Filtering/SteppedImpedanceFilter.cpp \
    Filtering/EndCoupled.cpp \
    Filtering/CapacitivelyCoupledShuntResonatorsFilter.cpp \
    Filtering/CoupledLineBandpassFilter.cpp \
    Filtering/CoupledLineHarmonicRejectionSIRBandpassFilter.cpp \
    UI/AttenuatorDesignTool.cpp \
    AttenuatorDesign/TeeAttenuator.cpp \
    AttenuatorDesign/PiAttenuator.cpp \
    AttenuatorDesign/BridgedTeeAttenuator.cpp \
    AttenuatorDesign/ReflectionAttenuator.cpp \
    AttenuatorDesign/QW_SeriesAttenuator.cpp \
    AttenuatorDesign/QW_ShuntAttenuator.cpp \
    Schematic/SchematicContent.cpp

SOURCES += Schematic/node.cpp Schematic/graphwidget.cpp qcustomplot.cpp

QT += widgets
QT += widgets printsupport
QT += sql
QT += svg
RESOURCES = resources.qrc

DISTFILES += \
    bitmaps/qucsRFdesigner.png \
    TODO \
    ZverevTables.sql
