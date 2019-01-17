include_directories(${Boost_INCLUDE_DIRS})
include_directories(${Gobject_INCLUDE_DIRS})
#link_directories(${CAFFE_LIB_DIR} ${XGBOOST_LIB_DIR} ${TF_LIB_DIR} ${TSNE_LIB_DIR})

if (CUDA_FOUND)
  set(CUDA_LIB_DEPS ${CUDA_LIBRARIES} ${CUDA_CUBLAS_LIBRARIES} ${CUDA_curand_LIBRARY})
else()
  set(CUDA_LIB_DEPS "")
  add_definitions(-DCPU_ONLY)
endif()
set(CAFFE_LIB_DEPS -lleveldb -lsnappy -llmdb -lhdf5_hl -lhdf5 -lopenblas -lcaffe -lprotobuf)
if (USE_CAFFE2)
  list(APPEND CAFFE2_LIB_DEPS -lcaffe2)
  if (CUDA_FOUND)
    list(APPEND CAFFE2_LIB_DEPS -lcaffe2_gpu)
  endif()
endif()
if (USE_TF)
  set(TF_LIB_DEPS -ltensorflow_cc)
else()
  set(TF_LIB_DEPS)
endif()

if (USE_XGBOOST)
  if (CUDA_FOUND AND USE_XGBOOST_GPU)
    set(XGBOOST_LIB_DEPS -Wl,--whole-archive ${CMAKE_BINARY_DIR}/xgboost/src/xgboost/lib/libxgboost.a -Wl,--no-whole-archive ${CMAKE_BINARY_DIR}/xgboost/src/xgboost/build/dmlc-core/libdmlc.a ${CMAKE_BINARY_DIR}/xgboost/src/xgboost/build/librabit.a ${CMAKE_BINARY_DIR}/xgboost/src/xgboost/build/CMakeFiles/gpuxgboost.dir/plugin/updater_gpu/src/gpuxgboost_generated_updater_gpu.cu.o ${CMAKE_BINARY_DIR}/xgboost/src/xgboost/build/nccl/libnccl.a)
  else()
    set(XGBOOST_LIB_DEPS -Wl,--whole-archive ${CMAKE_BINARY_DIR}/xgboost/src/xgboost/lib/libxgboost.a -Wl,--no-whole-archive ${CMAKE_BINARY_DIR}/xgboost/src/xgboost/build/dmlc-core/libdmlc.a ${CMAKE_BINARY_DIR}/xgboost/src/xgboost/build/librabit.a)
  endif()
else()
  set(XGBOOST_LIB_DEPS)
endif()
if (USE_TSNE)
  set(TSNE_LIB_DEPS -ltsne_multicore)
endif()


set (CMAKE_CXX_FLAGS "-g -Wall -Wextra -std=c++11 -pthread")

add_executable (my_example my_example.cc)
target_link_libraries (my_example vnn ${CUDA_LIB_DEPS} ${GSTREAMER_APP_LIB_DEPS} )

add_executable (my_example_tx2 my_example_tx2.cc)
target_link_libraries (my_example_tx2 vnn ${CUDA_LIB_DEPS} ${GSTREAMER_APP_LIB_DEPS} )

add_executable (file_desktop file_desktop.cc)
target_link_libraries (file_desktop vnn ${CUDA_LIB_DEPS} ${GSTREAMER_APP_LIB_DEPS} )

add_executable (file_tx2 file_tx2.cc)
target_link_libraries (file_tx2 vnn ${CUDA_LIB_DEPS} ${GSTREAMER_APP_LIB_DEPS} )

add_executable (file_desktop_async file_desktop_async.cc)
target_link_libraries (file_desktop_async vnn ${CUDA_LIB_DEPS} ${GSTREAMER_APP_LIB_DEPS} )

add_executable (discoverer discoverer.cc)
target_link_libraries (discoverer vnn ${CUDA_LIB_DEPS} ${GSTREAMER_APP_LIB_DEPS} )

add_executable (webcam webcam.cc)
target_link_libraries (webcam vnn ${CUDA_LIB_DEPS} ${GSTREAMER_APP_LIB_DEPS} )
