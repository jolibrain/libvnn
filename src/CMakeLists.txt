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
                    vnninputconnectorstrategy.h
		    vnninputconnectorcamera.h
		    vnninputconnectorcamera.cc
		    vnninputconnectorcameratx2.h
		    vnninputconnectorcameratx2.cc
		    vnninputconnectorfile.h
		    vnninputconnectorfile.cc
		    vnninputconnectorfiletx2.h
		    vnninputconnectorfiletx2.cc
		    streamlib.h
		    streamlibgstreamerdesktop.h
		    streamlibgstreamerdesktop.cc
		    vnnoutputconnectorstrategy.h
		    vnnoutputconnectordummy.h
		    vnnoutputconnectordummy.cc
		    streamlibgstreamertx2.h
		    streamlibgstreamertx2.cc
		    )

add_library(vnn ${vnn_SOURCES})

target_link_libraries(vnn ${OpenCV_LIBS})

install(TARGETS vnn DESTINATION lib)
install(FILES streamlib.h
	vnninputconnectorstrategy.h
	vnninputconnectorfile.h
	vnnoutputconnectordummy.h
	streamlibgstreamerdesktop.h
	DESTINATION include)
