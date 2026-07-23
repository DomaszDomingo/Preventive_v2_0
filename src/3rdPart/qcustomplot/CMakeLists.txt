qt_add_library(qcustomplot SHARED
        qcustomplot.cpp
)

target_link_libraries(qcustomplot PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::PrintSupport)
target_compile_definitions(qcustomplot PRIVATE QCUSTOMPLOT_COMPILE_LIBRARY)
target_compile_definitions(qcustomplot INTERFACE QCUSTOMPLOT_USE_LIBRARY)
target_include_directories(qcustomplot INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
