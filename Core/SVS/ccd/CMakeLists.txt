# ccd/CMakeLists.txt

set(CCD_SRC
  ccd.c
  mpr.c
  polytope.c
  support.c
  vec3.c
)

# Create a static library named ccd
add_library(ccd STATIC ${CCD_SRC})

# Needed because ccd is linked into the shared libSoar
set_target_properties(ccd PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Include the current directory as a public include path
target_include_directories(ccd PUBLIC 
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include>)

# Set C standard explicitly
target_compile_features(ccd PUBLIC c_std_99)

set_target_properties(ccd PROPERTIES LINKER_LANGUAGE C)

# Installation Structure
install(TARGETS ccd
  EXPORT SoarTargets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
)


