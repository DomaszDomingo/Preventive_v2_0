qt_add_executable(Preventive_v2_0
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    simwindow.h
    simwindow.cpp
    simwindow.ui
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

)

target_link_libraries(Preventive_v2_0 PRIVATE
    qcustomplot
    Qt6::Widgets
    Qt6::PrintSupport)



