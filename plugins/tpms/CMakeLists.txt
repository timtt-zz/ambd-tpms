if(tpms_plugin)

  include_directories(${CMAKE_SOURCE_DIR}/lib ${include_dirs})

  pkg_check_modules(libusb REQUIRED libusb-1.0)
  set(link_libraries ${link_libraries} ${libusb_LIBRARIES})
  include_directories(${libusb_INCLUDE_DIRS})

  set(tpmsplugin_headers tpmsplugin.h)
  set(tpmsplugin_sources tpmsplugin.cpp)

  add_library(tpmsplugin MODULE ${tpmsplugin_sources})
  set_target_properties(tpmsplugin PROPERTIES PREFIX "")
  target_link_libraries(tpmsplugin amb -L${CMAKE_CURRENT_BINARY_DIR}/lib ${link_libraries})

endif(tpms_plugin)