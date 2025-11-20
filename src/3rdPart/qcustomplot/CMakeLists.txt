qt_add_library(qcustomplot SHARED
        qcustomplot.cpp
)

target_link_libraries(qcustomplot PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::PrintSupport)
target_include_directories(qcustomplot INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
