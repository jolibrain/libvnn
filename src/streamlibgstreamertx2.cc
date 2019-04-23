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


#include <cstdlib>
#include "streamlibgstreamertx2.h"
#include "vnninputconnectorcamera.h"
#include "vnninputconnectorfiletx2.h"
#include "vnnoutputconnectordummy.h"
#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <chrono>
#include <iostream>
#include <sstream>
#include <queue>
#include <mutex>

namespace vnn {


  struct gstreamer_sys_t
  {
    GMainLoop *loop;
    GstElement *source;
    std::chrono::time_point<std::chrono::system_clock> timestamp;
    BufferCbFunc _buffercb;
    std::atomic<unsigned long> average_fps;
    std::queue<long int> decoded_frames;
    unsigned long max_videoframe_buffer;
    unsigned long idx_videoframe_buffer;
  };
    static std::deque<cv::Mat> static_decoded_frames;
    static std::mutex g_queue_mutex;


    /* called when the appsink notifies us that there is a new buffer ready for
     * processing */

    static GstFlowReturn on_new_sample_from_sink (GstElement * elt, gpointer data)
    {

      /* TODO: may be here use static_cast ? */
      Gstreamer_sys_t *_gstreamer_sys = (Gstreamer_sys_t *) data;

      GstSample *sample;
      GstBuffer *buffer;
      GstCaps   *caps;
      const GstStructure *structure;
      int width, height;
      char* dump_structure = new char;

      cv::Mat rgbimgbuf ;



      /* get the sample from appsink */
      sample = gst_app_sink_pull_sample (GST_APP_SINK (elt));
      if (sample) {

        caps = gst_sample_get_caps(sample);
        buffer = gst_sample_get_buffer (sample);

        GstMapInfo map;
        gst_buffer_map (buffer, &map, GST_MAP_READ);
        structure = gst_caps_get_structure (caps, 0);
        if (!gst_structure_get_int (structure, "width", &width) ||
            !gst_structure_get_int (structure, "height", &height)) {
          std::cout << "No width/height available\n" << std::endl;
          return GST_FLOW_OK ;
        }
        else
        {
          // TODO: store original widh and height
        }

        /* push new recieved frame to decoded frames queue */
        rgbimgbuf = cv::Mat(cv::Size(width, height), CV_8UC3, (char*)map.data, cv::Mat::AUTO_STEP);

        g_queue_mutex.lock();
        static_decoded_frames.push_front(cv::Mat());
        rgbimgbuf.copyTo(static_decoded_frames.front());
        g_queue_mutex.unlock();

        _gstreamer_sys->idx_videoframe_buffer ++;
        if  ( static_decoded_frames.size() >= _gstreamer_sys->max_videoframe_buffer)
        {
          for (int i =0; i <=10; i++)
          {
            // Remove old elemennts
            g_queue_mutex.lock();
            static_decoded_frames.pop_back();
            g_queue_mutex.unlock();
          }
        }

        /* we don't need the appsink sample anymore */
        gst_buffer_unmap(buffer, &map);
        gst_sample_unref (sample);

      }
      return GST_FLOW_OK;
    }

    /* called when we get a GstMessage from the source pipeline when we get EOS */
    static gboolean on_gst_bus (GstBus * bus, GstMessage * message,gpointer data)
        {
         GMainLoop *loop = (GMainLoop *) data;
            switch (GST_MESSAGE_TYPE (message)) {
                case GST_MESSAGE_EOS:
                    g_print ("EOS \n");
                    g_main_loop_quit (loop);
                    break;
                case GST_MESSAGE_ERROR:
                    g_print ("Received error\n");
                    g_main_loop_quit (loop);
                    break;
                default:
                    break;
            }
            return TRUE;
        };




    template <class TVnnInputConnectorStrategy, class TVnnOutputConnectorStrategy>
      int StreamLibGstreamerTX2<TVnnInputConnectorStrategy, TVnnOutputConnectorStrategy>::init()
      {
        GstBus *bus = NULL;
        GstElement *testsink = NULL;
        std::ostringstream launch_stream;
        GError *error = nullptr;
        std::string launch_string;
        std::string input_stream;
        Gstreamer_sys_t * _gstreamer_sys;

        this->_input.init();
        this->_output.init();


        gst_init(nullptr, nullptr);
        this->_gstreamer_sys = g_new0 (Gstreamer_sys_t, 1);
        _gstreamer_sys = this->_gstreamer_sys;


        _gstreamer_sys->loop = g_main_loop_new (NULL, FALSE);
        _gstreamer_sys->average_fps = 0;

        input_stream = this->_input.get_input_stream();

        std::cout << "input stream =  " << input_stream <<  std::endl;

        launch_stream
          << input_stream << " ! "
          << "nvvidconv ! "
          << "video/x-raw, format=RGB, width="<< this->_scale_width <<
          ", height="<< this->_scale_height <<" ! "
          << "appsink name=testsink";


        launch_string = launch_stream.str();

        g_print("Using launch string: %s\n", launch_string.c_str());

        _gstreamer_sys->source = gst_parse_launch (launch_string.c_str(), &error);

        if (_gstreamer_sys->source == NULL) {
          g_print ("Bad source\n");
          g_main_loop_unref (_gstreamer_sys->loop);
          g_free (_gstreamer_sys);
          return -1;
        }

        /* to be notified of messages from this pipeline, mostly EOS */
        bus = gst_element_get_bus (_gstreamer_sys->source);
        gst_bus_add_watch (bus,
            (GstBusFunc) &on_gst_bus,
            _gstreamer_sys->loop);
        gst_object_unref (bus);

        /* we use appsink in push mode, it sends us a signal when data is available
         * and we pull out the data in the signal callback. We want the appsink to
         * push as fast as it can, hence the sync=false */
        testsink = gst_bin_get_by_name (GST_BIN (_gstreamer_sys->source), "testsink");
        g_object_set (G_OBJECT (testsink), "emit-signals", TRUE, "sync", _scalesink_sync ? TRUE : FALSE, NULL);
        g_signal_connect (testsink, "new-sample",
            G_CALLBACK (on_new_sample_from_sink), _gstreamer_sys);
        gst_object_unref (testsink);
        _gstreamer_sys->max_videoframe_buffer = this->_max_video_frame_buffer;

        return 0;
      };


  template <class TVnnInputConnectorStrategy, class TVnnOutputConnectorStrategy>
    int StreamLibGstreamerTX2<TVnnInputConnectorStrategy, TVnnOutputConnectorStrategy>::run()
  {

        /* launching things */
        this->_gstreamer_sys->timestamp = std::chrono::system_clock::now();
        gst_element_set_state (this->_gstreamer_sys->source, GST_STATE_PLAYING);
        /* let's run !, this loop will quit when the sink pipeline goes EOS or when an
         * error occurs in the source or sink pipelines. */
        g_print ("Let's run!\n");
        g_main_loop_run (_gstreamer_sys->loop);
        g_print ("Going out\n");
        return 0;
    }

  template <class TVnnInputConnectorStrategy, class TVnnOutputConnectorStrategy>
    int StreamLibGstreamerTX2<TVnnInputConnectorStrategy, TVnnOutputConnectorStrategy>::run_async()
    {

        /* launching things */
        this->_gstreamer_sys->timestamp = std::chrono::system_clock::now();
        gst_element_set_state (this->_gstreamer_sys->source, GST_STATE_PLAYING);
        /* let's run !, this loop will quit when the sink pipeline goes EOS or when an
         * error occurs in the source or sink pipelines. */
        return 0;
    }



  template <class TVnnInputConnectorStrategy, class TVnnOutputConnectorStrategy>
    int StreamLibGstreamerTX2<TVnnInputConnectorStrategy, TVnnOutputConnectorStrategy>::
    stop()
    {
        gst_element_set_state (this->_gstreamer_sys->source, GST_STATE_NULL);
        return 0;
    }

  template <class TVnnInputConnectorStrategy, class TVnnOutputConnectorStrategy>
    void StreamLibGstreamerTX2<TVnnInputConnectorStrategy, TVnnOutputConnectorStrategy>::
    set_buffer_cb(BufferCbFunc &buffercb)
    {
      this->_buffercb = buffercb;
      this->_gstreamer_sys->_buffercb = buffercb;
    }

  template <class TVnnInputConnectorStrategy, class TVnnOutputConnectorStrategy>
    int StreamLibGstreamerTX2<TVnnInputConnectorStrategy, TVnnOutputConnectorStrategy>::
    get_video_buffer(cv::Mat &video_buffer)
    {
      Gstreamer_sys_t *_gstreamer_sys = (Gstreamer_sys_t *) this->_gstreamer_sys;
      if (  static_decoded_frames.empty() )
        return 0;

      g_queue_mutex.lock();
      video_buffer = static_decoded_frames.front();
      static_decoded_frames.pop_front();
      g_queue_mutex.unlock();
      return static_decoded_frames.size();
    }

  template <class TVnnInputConnectorStrategy, class TVnnOutputConnectorStrategy>
    void StreamLibGstreamerTX2<TVnnInputConnectorStrategy, TVnnOutputConnectorStrategy>::
        set_scale_size( const int &width, const int &height)
    {
      StreamLib<TVnnInputConnectorStrategy, TVnnOutputConnectorStrategy>
        ::set_scale_size(width, height);
    }



template class StreamLibGstreamerTX2<VnnInputConnectorCamera, VnnOutputConnectorDummy>;
template class StreamLibGstreamerTX2<VnnInputConnectorFileTX2, VnnOutputConnectorDummy>;
}

