qt_add_executable(Preventive_v2_0
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    controller/simulationcontroller.cpp
    controller/simulationcontroller.h
    dataloader/dataloader.cpp
    dataloader/dataloader.h
    datamodel/DataPoint.h
    simulation/simulator.cpp
    simulation/simulator.h
    simulation/concretestrategies.cpp
    simulation/concretestrategies.h
    simulation/ISimulationStrategy.h
    simulation/strategyfactory.cpp
    simulation/strategyfactory.h
    utils/dataanalyzer.cpp
    utils/dataanalyzer.h
    infowindow.h infowindow.cpp
    components/chartslot.h components/chartslot.cpp
    menu/filemenu.h menu/filemenu.cpp
    menu/editmenu.h menu/editmenu.cpp
    menu/aboutmenu.h menu/aboutmenu.cpp
    dialogs/limitsdialog.h dialogs/limitsdialog.cpp
    datamodel/analysisresult.h datamodel/analysisresult.cpp

)

target_link_libraries(Preventive_v2_0 PRIVATE
    qcustomplot
    Qt6::Widgets
    Qt6::PrintSupport)



