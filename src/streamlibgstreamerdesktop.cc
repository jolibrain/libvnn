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
#include "inputconnectorcamera.h"
#include "inputconnectorfile.h"
#include "outputconnectordummy.h"
#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <gst/pbutils/pbutils.h>
#include <iostream>
#include <sstream>


namespace vnn {
  struct node {
    char *key;
    char *value;
    struct node *next;
  };

  struct gstreamer_sys_t
  {
    GMainLoop *loop;
    GstElement *source;
    std::chrono::time_point<std::chrono::system_clock> timestamp;
    BufferCbFunc _buffercb;
    std::atomic<unsigned long> average_fps;
  };


    static long int frame_counter;

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

    static GstFlowReturn on_new_sample_from_sink (GstElement * elt, gpointer data)
    {

        /* TODO: may be here use static_cast ? */
        Gstreamer_sys_t *_gstreamer_sys = (Gstreamer_sys_t *) data;

        GstSample *sample;
        GstBuffer *buffer;
        GstCaps   *caps;
        const GstStructure *structure;
        unsigned int fps;
        unsigned int average = _gstreamer_sys->average_fps;
        int width, height;
        char* dump_structure = new char;



        std::chrono::time_point<std::chrono::system_clock> timestamp;
        timestamp = std::chrono::system_clock::now();

        auto elapsed_us = \
            std::chrono::duration_cast<std::chrono::microseconds>(timestamp - _gstreamer_sys->timestamp).count();
#if 0
        std::cout << "timestamp " << std::chrono::duration_cast<std::chrono::microseconds> (timestamp.time_since_epoch()).count() <<"\n" ;
        float f_fps =    (1.0/elapsed_us)*1000000;
        fps = static_cast<unsigned int>(f_fps);
        std::cout << "elapsed_us " << elapsed_us <<"\n" ;
#endif
        _gstreamer_sys->average_fps = moving_average(
            average,
            fps,
            128
            );
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
#if 0
uncomment for debug purpose
          dump_structure = gst_structure_to_string(structure);
          std::cout << "width =" << width << std::endl ;
          std::cout << "height =" << height << std::endl ;
          std::cout << "dump_structure: " << dump_structure << std::endl;
#endif
          frame_counter++;
          delete dump_structure ;

          _gstreamer_sys->_buffercb(width, height, map.data);

          /* we don't need the appsink sample anymore */
          gst_buffer_unmap(buffer, &map);
          gst_sample_unref (sample);
          if (frame_counter > 100) {
            return GST_FLOW_EOS;
          }
        }
        // std::cout << std::endl;
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


  template <class TInputConnectorStrategy, class TOutputConnectorStrategy>
    int StreamLibGstreamerDesktop<TInputConnectorStrategy, TOutputConnectorStrategy>::init()
    {
        GstBus *bus = NULL;
        GstElement *testsink = NULL;
        std::ostringstream launch_stream;
        GError *error = nullptr;
        std::string launch_string;
        std::string input_stream;
        Gstreamer_sys_t * _gstraemer_sys;

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
        << " videoscale !"
        << " videoconvert !"
        << " appsink caps=" << StreamLibGstreamerDesktop::_video_caps.c_str()
        << " name=testsink";

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
        g_object_set (G_OBJECT (testsink), "emit-signals", TRUE, "sync", FALSE, NULL);
        g_signal_connect (testsink, "new-sample",
                G_CALLBACK (on_new_sample_from_sink), _gstreamer_sys);
        gst_object_unref (testsink);

        return 0;
    };

  template <class TInputConnectorStrategy, class TOutputConnectorStrategy>
    int StreamLibGstreamerDesktop<TInputConnectorStrategy, TOutputConnectorStrategy>::run()
    {

        /* launching things */
        this->_gstreamer_sys->timestamp = std::chrono::system_clock::now();
        gst_element_set_state (this->_gstreamer_sys->source, GST_STATE_PLAYING);
        /* let's run !, this loop will quit when the sink pipeline goes EOS or when an
         * error occurs in the source or sink pipelines. */
        frame_counter =0;
        g_print ("Let's run!\n");
        g_main_loop_run (_gstreamer_sys->loop);
        g_print ("Going out\n");
        return 0;
    }

  template <class TInputConnectorStrategy, class TOutputConnectorStrategy>
    int StreamLibGstreamerDesktop<TInputConnectorStrategy, TOutputConnectorStrategy>::stop()
    {
        return 0;
    }
  template <class TInputConnectorStrategy, class TOutputConnectorStrategy>
    void StreamLibGstreamerDesktop<TInputConnectorStrategy, TOutputConnectorStrategy>::set_buffer_cb(BufferCbFunc &buffercb)
    {
      this->_buffercb = buffercb;
      this->_gstreamer_sys->_buffercb = buffercb;
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
      g_print ("Finished discovering\n");

      g_main_loop_quit (data->loop);
  }



  template <class TInputConnectorStrategy, class TOutputConnectorStrategy>
      int StreamLibGstreamerDesktop<TInputConnectorStrategy, TOutputConnectorStrategy>::discoverer()
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



template class StreamLibGstreamerDesktop<InputConnectorCamera, OutputConnectorDummy>;
template class StreamLibGstreamerDesktop<InputConnectorFile, OutputConnectorDummy>;

}

