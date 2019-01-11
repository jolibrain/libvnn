/**
 * libvnn
 * Copyright (c) 2018 JoliBrain
 * Author: Nicolas Bertrand <nicolas@davionbertrand.net>
 *
 * This file is part of libvnn.
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License
 */

#ifndef STREAMLIBGSTREAMERDESKTOP_H
#define STREAMLIBGSTREAMERDESKTOP_H

#include <string>
#include <chrono>
#include "streamlib.h"
#include <atomic>
#include <boost/thread.hpp>

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>

namespace vnn
{
typedef struct gstreamer_sys_t Gstreamer_sys_t;


  template <class TVnnInputConnectorStrategy, class TVnnOutputConnectorStrategy>
    class StreamLibGstreamerDesktop: public StreamLib<TVnnInputConnectorStrategy, TVnnOutputConnectorStrategy>
  {
    public:
      StreamLibGstreamerDesktop() {}
      ~StreamLibGstreamerDesktop() {}
      int init();
      int run();
      int run_async();
      int stop();

      void set_scale_size( const int &width, const int &height);
      void set_buffer_cb(BufferCbFunc &buffercb);
      int get_video_buffer(cv::Mat &video_buffer, long int  &timestamp);
      int get_original_width();
      int get_original_height();
      int discoverer();
      bool is_playing();
      BufferCbFunc _buffercb = nullptr;

    private:
      Gstreamer_sys_t *_gstreamer_sys;
      /* pipe to reproduct
       * gst-launch -v videotestsrc ! video/x-raw,width=320,height=240,format=UYVY ! xvimagesink
       */
      /* these are the caps we are going to pass through the appsink */
      std::string _video_caps =
        "video/x-raw,format=RGB";
      void RunningMainLoop();
       boost::thread g_main_loop_thread_;

      unsigned long _max_video_frame_buffer = MAX_VIDEOFRAME_BUFFER;
      bool init_done = false;

      std::string output_caps();

  };

#if 0
  class GstreamerTX2: StreamLib
  {};

  class GstreamerDesktop: StreamLib
  {};

  class OutputConnectorBuffer: OutputConnector
  {};
#endif

}
#endif
