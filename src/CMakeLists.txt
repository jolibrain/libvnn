# main library
if (NOT CUDA_FOUND)
  add_definitions(-DCPU_ONLY)
endif()
if (USE_DD_SYSLOG)
  add_definitions(-DUSE_DD_SYSLOG)
endif()

if (GSTREAMER_FOUND)
 message(STATUS "Coucou gstereamer found")
endif()

set(libvnn_SOURCES  vnnvideostream.h
                    inputconnectorstrategy.h
		    inputconnectorcamera.h
		    inputconnectorcamera.cc
		    streamlib.h
		    streamlibgstreamerdesktop.h
		    streamlibgstreamerdesktop.cc
		    )

add_library(libvnn ${libvnn_SOURCES})
