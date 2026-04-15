# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/DungeonRealm_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/DungeonRealm_autogen.dir/ParseCache.txt"
  "DungeonRealm_autogen"
  )
endif()
