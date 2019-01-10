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

#include "streamlibgstreamerdesktop.h"



#include <cstdlib>

#include <string.h>
#include "vnninputconnectorcamera.h"
#include "vnninputconnectorfile.h"
#include "vnnoutputconnectordummy.h"
#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <gst/pbutils/pbutils.h>
#include <iostream>
#include <sstream>
#include <queue>
#include <mutex>
#include <thread>


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
    int width=0;
    int height=0;
  };


    static std::deque<cv::Mat> static_decoded_frames;
    static std::mutex g_queue_mutex;

  unsigned int moving_average(
      const unsigned int x_average,
      const unsigned int x,
      const unsigned int alpha)
  {
    float tmp;
    unsigned int average_out;
    tmp =  static_cast<float> ( ((alpha -1) * x_average) + x) ;
    average_out =  static_cast<unsigned int> (tmp / alpha);
    return average_out;
  }


     /* called when the appsink notifies us that there is a new buffer ready for
     * processing */
  int dummy_callback( long unsigned int size , unsigned char * data )
    {
        std::cout << "cb map.size =  " << size <<  std::endl;
        std::cout << "cb map.data =  " <<  static_cast<void*>(data) << std::endl;
      return 0;
    }

  static gboolean print_field (GQuark field, const GValue * value, gpointer pfx) {
    gchar *str = gst_value_serialize (value);

    g_free (str);
    return TRUE;
  }

  static void print_caps (const GstCaps * caps, const gchar * pfx) {
    guint i;

    g_return_if_fail (caps != NULL);

    if (gst_caps_is_any (caps)) {
      g_print ("%sANY\n", pfx);
      return;
    }
    if (gst_caps_is_empty (caps)) {
      g_print ("%sEMPTY\n", pfx);
      return;
    }

    for (i = 0; i < gst_caps_get_size (caps); i++) {
      GstStructure *structure = gst_caps_get_structure (caps, i);

      gst_structure_foreach (structure, print_field, (gpointer) pfx);
    }
  }

  static void detect_size (const GstCaps * caps, gpointer data) {
    gint width, height;
    GstStructure *structure;
    std::string structure_name;
    Gstreamer_sys_t *_gstreamer_sys = (Gstreamer_sys_t *) data;

    g_return_if_fail (caps != NULL);

    if (gst_caps_is_any (caps)) {
      g_print ("Ouuuups ANY\n");
      return;
    }
    if (gst_caps_is_empty (caps)) {
      g_print ("Ouuuups EMPTY\n");
      return;
    }

    g_return_if_fail (gst_caps_is_fixed (caps));
    structure = gst_caps_get_structure (caps, 0);
    structure_name = gst_structure_get_name(structure);

    if ( structure_name.compare("video/x-raw") == 0)
    {
      if (!gst_structure_get_int (structure, "width", &width) ||
          !gst_structure_get_int (structure, "height", &height)) {
        g_print ("No width/height available\n");
        return;
      }

      g_queue_mutex.lock();
      _gstreamer_sys->width = width;
      _gstreamer_sys->height = height;
      g_queue_mutex.unlock();

      //g_print ("The video size of this set of capabilities is %dx%d\n",
      //    width, height);
    }
  }

  static void
    on_new_pad (GstElement *element,
        GstPad     *pad,
        gpointer    data)
    {

      (void) element;
      gchar *name;
      GstCaps *caps = NULL;
      Gstreamer_sys_t *_gstreamer_sys = (Gstreamer_sys_t *) data;

      name = gst_pad_get_name (pad);

      /* Retrieve negotiated caps (or acceptable caps if negotiation is not finished yet) */
      caps = gst_pad_get_current_caps (pad);
      if (!caps)
        caps = gst_pad_query_caps (pad, NULL);

      /* Print and free */
      detect_size(caps, _gstreamer_sys);
      gst_caps_unref (caps);
      g_free (name);

      /* here, you would setup a new pad link for the newly created pad */

    }

    static GstFlowReturn on_new_sample_from_sink (GstElement * elt, gpointer data)
    {

        /* TODO: may be here use static_cast ? */
        Gstreamer_sys_t *_gstreamer_sys = (Gstreamer_sys_t *) data;

        GstSample *sample;
        GstBuffer *buffer;
        GstCaps   *caps;
        const GstStructure *structure;
        int width, height;

        cv::Mat imgbuf ;
        cv::Mat rgbimgbuf ;



        std::chrono::time_point<std::chrono::system_clock> timestamp;
        timestamp = std::chrono::system_clock::now();

#if 0
        std::cout << "timestamp " << std::chrono::duration_cast<std::chrono::microseconds> (timestamp.time_since_epoch()).count() <<"\n" ;
        float f_fps =    (1.0/elapsed_us)*1000000;
        fps = static_cast<unsigned int>(f_fps);
        std::cout << "elapsed_us " << elapsed_us <<"\n" ;
#endif
       _gstreamer_sys->timestamp = timestamp;

     //     std::cout << "AVG FPS " << _gstreamer_sys->average_fps <<"\n" ;
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
          //_gstreamer_sys->_buffercb(width, height, map.data);
          /* push new recieved frame to decoded frames queue */
          rgbimgbuf = cv::Mat(cv::Size(width, height), CV_8UC3, (char*)map.data, cv::Mat::AUTO_STEP);
         // rgbimgbuf = cv::Mat(cv::Size(width, height), CV_8UC3, (char*)map.data, cv::Mat::AUTO_STEP);
          //cvtColor(imgbuf, rgbimgbuf, CV_YUV2BGR_YUY2);

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
        // std::cout << std::endl;
        return GST_FLOW_OK;
    }

    /* called when we get a GstMessage from the source pipeline when we get EOS */
    static gboolean on_gst_bus (GstBus * bus, GstMessage * message,gpointer data)
    {

     Gstreamer_sys_t *_gstreamer_sys = (Gstreamer_sys_t *) data;
      switch (GST_MESSAGE_TYPE (message)) {
        case GST_MESSAGE_EOS:

          g_print ("EOS \n");
          g_main_loop_quit (_gstreamer_sys->loop);
          break;
        case GST_MESSAGE_ERROR:
          g_print ("Received error\n");
          g_main_loop_quit (_gstreamer_sys->loop);
          break;
        case GST_MESSAGE_STATE_CHANGED:
            // DEBUG
            //g_print ("on_gst_bus_signal  %s %s\n",
            //    GST_MESSAGE_SRC_NAME(message),
            //    GST_MESSAGE_TYPE_NAME(message)
            //    );

          /* We are only interested in state-changed messages from the pipeline */
            GstState old_state, new_state, pending_state;
            gst_message_parse_state_changed (message, &old_state, &new_state, &pending_state);
            // DEBUG 
            //g_print ("Pipeline state changed from %s to %s:\n",
            //    gst_element_state_get_name (old_state), gst_element_state_get_name (new_state));
          break;
        default:
          break;
      }
      return TRUE;
    };


  template <class TVnnInputConnectorStrategy, class TVnnOutputConnectorStrategy>
    int StreamLibGstreamerDesktop<TVnnInputConnectorStrategy, TVnnOutputConnectorStrategy>::init()
    {
        GstBus *bus = NULL;
        GstElement *testsink = NULL , *input_elt = NULL;
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

        //std::cout << "input stream =  " << input_stream <<  std::endl;


        launch_stream
        << input_stream << " ! "
        << " videoscale !"
        << " videoconvert !"
        << " appsink caps=" << StreamLibGstreamerDesktop::output_caps().c_str()
        //<< " appsink caps=" << StreamLibGstreamerDesktop::_video_caps.c_str()
        << " name=testsink";

        launch_string = launch_stream.str();

//        g_print("Using launch string: %s\n", launch_string.c_str());

        _gstreamer_sys->source = gst_parse_launch (launch_string.c_str(), &error);
//         g_print( "pipeline == %p\n", _gstreamer_sys->source);

        if (_gstreamer_sys->source == NULL) {
            g_print ("Bad source\n");
            g_main_loop_unref (_gstreamer_sys->loop);
            g_free (_gstreamer_sys);
            return -1;
        }

                /* we use appsink in push mode, it sends us a signal when data is available
         * and we pull out the data in the signal callback. We want the appsink to
         * push as fast as it can, hence the sync=false */
        testsink = gst_bin_get_by_name (GST_BIN (_gstreamer_sys->source), "testsink");
        g_object_set (G_OBJECT (testsink), "emit-signals", TRUE, "sync", FALSE, NULL);
        g_signal_connect (testsink, "new-sample",
                G_CALLBACK (on_new_sample_from_sink), _gstreamer_sys);

        input_elt = gst_bin_get_by_name (GST_BIN (_gstreamer_sys->source), "decoder");
        g_signal_connect (input_elt, "pad-added",
                G_CALLBACK (on_new_pad), _gstreamer_sys);
        gst_object_unref (testsink);
        _gstreamer_sys->max_videoframe_buffer = this->_max_video_frame_buffer;

        /* to be notified of messages from this pipeline, mostly EOS */
        bus = gst_pipeline_get_bus(GST_PIPELINE(_gstreamer_sys->source));
//        g_print( "bus == %p\n", _gstreamer_sys->source);
        gst_bus_add_watch (bus,
            (GstBusFunc) &on_gst_bus,
            _gstreamer_sys);
       
       /// Run the main loop to receive messages from bus
       g_main_loop_thread_ = boost::thread(
           &StreamLibGstreamerDesktop<TVnnInputConnectorStrategy, TVnnOutputConnectorStrategy>
           ::RunningMainLoop, this);
        gst_object_unref (bus);


        return 0;
    };

  template <class TVnnInputConnectorStrategy, class TVnnOutputConnectorStrategy>
    int StreamLibGstreamerDesktop<TVnnInputConnectorStrategy, TVnnOutputConnectorStrategy>::run()
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
    int StreamLibGstreamerDesktop<TVnnInputConnectorStrategy, TVnnOutputConnectorStrategy>::run_async()
    {

        /* launching things */
        this->_gstreamer_sys->timestamp = std::chrono::system_clock::now();
        gst_element_set_state (this->_gstreamer_sys->source, GST_STATE_PLAYING);
        /* let's run !, this loop will quit when the sink pipeline goes EOS or when an
         * error occurs in the source or sink pipelines. */
        return 0;
    }


  template <class TVnnInputConnectorStrategy, class TVnnOutputConnectorStrategy>
    void StreamLibGstreamerDesktop<TVnnInputConnectorStrategy, TVnnOutputConnectorStrategy>
    ::RunningMainLoop()
    {
      GMainLoop* loop = g_main_loop_new(NULL, FALSE);
       g_main_loop_run(loop);
    }



  template <class TVnnInputConnectorStrategy, class TVnnOutputConnectorStrategy>
    int StreamLibGstreamerDesktop<TVnnInputConnectorStrategy, TVnnOutputConnectorStrategy>::stop()
    {
        gst_element_set_state (this->_gstreamer_sys->source, GST_STATE_NULL);
        return 0;
    }

  template <class TVnnInputConnectorStrategy, class TVnnOutputConnectorStrategy>
    bool StreamLibGstreamerDesktop<TVnnInputConnectorStrategy, TVnnOutputConnectorStrategy>::
    is_playing()
    {
        GstStateChangeReturn res;
        GstState gst_state;
        bool ret =false;
        res = gst_element_get_state (this->_gstreamer_sys->source, &gst_state, NULL, GST_CLOCK_TIME_NONE);
        switch (gst_state) {
          case GST_STATE_PLAYING:
            // DEBUG:
            // g_print ("gst state %s pad:\n", gst_element_state_get_name(gst_state));
            ret = true;
            break;
          default:
            break;
        }
        return ret;
    }

  template <class TVnnInputConnectorStrategy, class TVnnOutputConnectorStrategy>
    void StreamLibGstreamerDesktop<TVnnInputConnectorStrategy, TVnnOutputConnectorStrategy>::set_buffer_cb(BufferCbFunc &buffercb)
    {
      this->_buffercb = buffercb;
      this->_gstreamer_sys->_buffercb = buffercb;
    }

  template <class TVnnInputConnectorStrategy, class TVnnOutputConnectorStrategy>
    int StreamLibGstreamerDesktop<TVnnInputConnectorStrategy, TVnnOutputConnectorStrategy>
        ::get_video_buffer(cv::Mat &video_buffer)
    {
      if (  static_decoded_frames.empty() )
        return 0;

     g_queue_mutex.lock();
     video_buffer = static_decoded_frames.front();
     static_decoded_frames.pop_front();
     g_queue_mutex.unlock();
     return static_decoded_frames.size();
    }

  template <class TVnnInputConnectorStrategy, class TVnnOutputConnectorStrategy>
    std::string StreamLibGstreamerDesktop<TVnnInputConnectorStrategy, TVnnOutputConnectorStrategy>
        ::output_caps( )
    {
      std::ostringstream output;
      output << "video/x-raw,format=RGB"
      ",width=" << this->_scale_width<<
      ",height="<< this->_scale_height;
      return output.str();
    }


  template <class TVnnInputConnectorStrategy, class TVnnOutputConnectorStrategy>
    void StreamLibGstreamerDesktop<TVnnInputConnectorStrategy, TVnnOutputConnectorStrategy>
        ::set_scale_size( const int &width, const int &height)
    {
      StreamLib<TVnnInputConnectorStrategy, TVnnOutputConnectorStrategy>
        ::set_scale_size(width, height);
    }



  /* Structure to contain all our information, so we can pass it around */
  typedef struct _CustomData {
      GstDiscoverer *discoverer;
      GMainLoop *loop;
  } CustomData;

  /* Print a tag in a human-readable format (name: value) */
  static void print_tag_foreach (const GstTagList *tags, const gchar *tag, gpointer user_data) {
      GValue val = { 0, };
      gchar *str;
      gint depth = GPOINTER_TO_INT (user_data);

      gst_tag_list_copy_value (&val, tags, tag);

      if (G_VALUE_HOLDS_STRING (&val))
          str = g_value_dup_string (&val);
      else
          str = gst_value_serialize (&val);

      g_print ("%*s%s: %s\n", 2 * depth, " ", gst_tag_get_nick (tag), str);
      g_free (str);

      g_value_unset (&val);
  }

  /* Print information regarding a stream */
  static void print_stream_info (GstDiscovererStreamInfo *info, gint depth) {
      gchar *desc = NULL;
      GstCaps *caps;
      const GstTagList *tags;

      caps = gst_discoverer_stream_info_get_caps (info);

      if (caps) {
          if (gst_caps_is_fixed (caps))
              desc = gst_pb_utils_get_codec_description (caps);
          else
              desc = gst_caps_to_string (caps);
          gst_caps_unref (caps);
      }

      g_print ("%*s%s: %s\n", 2 * depth, " ", gst_discoverer_stream_info_get_stream_type_nick (info), (desc ? desc : ""));

      if (desc) {
          g_free (desc);
          desc = NULL;
      }

      tags = gst_discoverer_stream_info_get_tags (info);
      if (tags) {
          g_print ("%*sTags:\n", 2 * (depth + 1), " ");
          gst_tag_list_foreach (tags, print_tag_foreach, GINT_TO_POINTER (depth + 2));
      }
  }

  /* Print information regarding a stream and its substreams, if any */
  static void print_topology (GstDiscovererStreamInfo *info, gint depth) {
      GstDiscovererStreamInfo *next;

      if (!info)
          return;

      print_stream_info (info, depth);

      next = gst_discoverer_stream_info_get_next (info);
      if (next) {
          print_topology (next, depth + 1);
          gst_discoverer_stream_info_unref (next);
      } else if (GST_IS_DISCOVERER_CONTAINER_INFO (info)) {
          GList *tmp, *streams;

          streams = gst_discoverer_container_info_get_streams (GST_DISCOVERER_CONTAINER_INFO (info));
          for (tmp = streams; tmp; tmp = tmp->next) {
              GstDiscovererStreamInfo *tmpinf = (GstDiscovererStreamInfo *) tmp->data;
              print_topology (tmpinf, depth + 1);
          }
          gst_discoverer_stream_info_list_free (streams);
      }
  }

  /* This function is called every time the discoverer has information regarding
   * one of the URIs we provided.*/
  static void on_discovered_cb (GstDiscoverer *discoverer, GstDiscovererInfo *info, GError *err, CustomData *data) {
    (void) discoverer;
    (void) data;
    GstDiscovererResult result;
    const gchar *uri;
    const GstTagList *tags;
    GstDiscovererStreamInfo *sinfo;

    uri = gst_discoverer_info_get_uri (info);
    result = gst_discoverer_info_get_result (info);
    switch (result) {
      case GST_DISCOVERER_URI_INVALID:
        g_print ("Invalid URI '%s'\n", uri);
        break;
      case GST_DISCOVERER_ERROR:
        g_print ("Discoverer error: %s\n", err->message);
        break;
      case GST_DISCOVERER_TIMEOUT:
        g_print ("Timeout\n");
        break;
      case GST_DISCOVERER_BUSY:
        g_print ("Busy\n");
        break;
      case GST_DISCOVERER_MISSING_PLUGINS:{
                                            const GstStructure *s;
                                            gchar *str;

                                            s = gst_discoverer_info_get_misc (info);
                                            str = gst_structure_to_string (s);

                                            g_print ("Missing plugins: %s\n", str);
                                            g_free (str);
                                            break;
                                          }
      case GST_DISCOVERER_OK:
                                          g_print ("Discovered '%s'\n", uri);
                                          break;
    }

    if (result != GST_DISCOVERER_OK) {
          g_printerr ("This URI cannot be played\n");
          return;
      }

      /* If we got no error, show the retrieved information */

      g_print ("\nDuration: %" GST_TIME_FORMAT "\n", GST_TIME_ARGS (gst_discoverer_info_get_duration (info)));

      tags = gst_discoverer_info_get_tags (info);
      if (tags) {
          g_print ("Tags:\n");
          gst_tag_list_foreach (tags, print_tag_foreach, GINT_TO_POINTER (1));
      }

      g_print ("Seekable: %s\n", (gst_discoverer_info_get_seekable (info) ? "yes" : "no"));

      g_print ("\n");

      sinfo = gst_discoverer_info_get_stream_info (info);
      if (!sinfo)
          return;

      g_print ("Stream information:\n");

      print_topology (sinfo, 1);

      gst_discoverer_stream_info_unref (sinfo);

      g_print ("\n");
  }

  /* This function is called when the discoverer has finished examining
   * all the URIs we provided.*/
  static void on_finished_cb (GstDiscoverer *discoverer, CustomData *data) {
    (void) discoverer;
      g_print ("Finished discovering\n");

      g_main_loop_quit (data->loop);
  }

  template <class TVnnInputConnectorStrategy, class TVnnOutputConnectorStrategy>
    int StreamLibGstreamerDesktop<TVnnInputConnectorStrategy, TVnnOutputConnectorStrategy>
    ::get_original_width()
    {

      Gstreamer_sys_t *_gstreamer_sys = (Gstreamer_sys_t *) this->_gstreamer_sys;
      if  (_gstreamer_sys->width == 0)
        {
          //sleep a bit for video size

          std::this_thread::sleep_for(std::chrono::seconds(1));
        }
      return _gstreamer_sys->width;
    }

  template <class TVnnInputConnectorStrategy, class TVnnOutputConnectorStrategy>
    int StreamLibGstreamerDesktop<TVnnInputConnectorStrategy, TVnnOutputConnectorStrategy>
    ::get_original_height()
    {
      if  (_gstreamer_sys->height == 0)
        {
          //sleep a bit for video size

          std::this_thread::sleep_for(std::chrono::seconds(1));
        }
 
      Gstreamer_sys_t *_gstreamer_sys = (Gstreamer_sys_t *) this->_gstreamer_sys;
      return _gstreamer_sys->height;
    }



  template <class TVnnInputConnectorStrategy, class TVnnOutputConnectorStrategy>
      int StreamLibGstreamerDesktop<TVnnInputConnectorStrategy, TVnnOutputConnectorStrategy>::discoverer()
      {
          CustomData data;
          GError *err = NULL;
          gchar *uri = "https://www.freedesktop.org/software/gstreamer-sdk/data/media/sintel_trailer-480p.webm";


          /* Initialize cumstom data structure */
          memset (&data, 0, sizeof (data));

          /* Initialize GStreamer */
         gst_init(nullptr, nullptr);

          g_print ("Discovering '%s'\n", uri);

          /* Instantiate the Discoverer */
          data.discoverer = gst_discoverer_new (5 * GST_SECOND, &err);
          if (!data.discoverer) {
              g_print ("Error creating discoverer instance: %s\n", err->message);
              g_clear_error (&err);
              return -1;
          }

          /* Connect to the interesting signals */
          g_signal_connect (data.discoverer, "discovered", G_CALLBACK (on_discovered_cb), &data);
          g_signal_connect (data.discoverer, "finished", G_CALLBACK (on_finished_cb), &data);

          /* Start the discoverer process (nothing to do yet) */
          gst_discoverer_start (data.discoverer);

          /* Add a request to process asynchronously the URI passed through the command line */
          if (!gst_discoverer_discover_uri_async (data.discoverer, uri)) {
              g_print ("Failed to start discovering URI '%s'\n", uri);
              g_object_unref (data.discoverer);
              return -1;
          }

          /* Create a GLib Main Loop and set it to run, so we can wait for the signals */
          data.loop = g_main_loop_new (NULL, FALSE);
          g_main_loop_run (data.loop);

          /* Stop the discoverer process */
          gst_discoverer_stop (data.discoverer);

          /* Free resources */
          g_object_unref (data.discoverer);
          g_main_loop_unref (data.loop);

          return 0;

      }



template class StreamLibGstreamerDesktop<VnnInputConnectorCamera, VnnOutputConnectorDummy>;
template class StreamLibGstreamerDesktop<VnnInputConnectorFile, VnnOutputConnectorDummy>;

}
