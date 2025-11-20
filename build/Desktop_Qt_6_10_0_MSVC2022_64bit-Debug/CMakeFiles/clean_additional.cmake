# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "src\\3rdPart\\qcustomplot\\CMakeFiles\\qcustomplot_autogen.dir\\AutogenUsed.txt"
  "src\\3rdPart\\qcustomplot\\CMakeFiles\\qcustomplot_autogen.dir\\ParseCache.txt"
  "src\\3rdPart\\qcustomplot\\qcustomplot_autogen"
  "src\\app\\CMakeFiles\\Preventive_v2_0_autogen.dir\\AutogenUsed.txt"
  "src\\app\\CMakeFiles\\Preventive_v2_0_autogen.dir\\ParseCache.txt"
  "src\\app\\Preventive_v2_0_autogen"
  )
endif()
