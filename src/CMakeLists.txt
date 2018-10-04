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

set(vnn_SOURCES  vnnvideostream.h
                    inputconnectorstrategy.h
		    inputconnectorcamera.h
		    inputconnectorcamera.cc
		    inputconnectorcameratx2.h
		    inputconnectorcameratx2.cc
		    inputconnectorfile.h
		    inputconnectorfile.cc
		    inputconnectorfiletx2.h
		    inputconnectorfiletx2.cc
		    streamlib.h
		    streamlibgstreamerdesktop.h
		    streamlibgstreamerdesktop.cc
		    outputconnectorstrategy.h
		    outputconnectordummy.h
		    outputconnectordummy.cc
		    streamlibgstreamertx2.h
		    streamlibgstreamertx2.cc
		    )

add_library(vnn ${vnn_SOURCES})

install(TARGETS vnn DESTINATION lib)
install(FILES streamlib.h DESTINATION include)
