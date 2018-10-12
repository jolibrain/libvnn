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

namespace vnn {

    static long int frame_counter;

    typedef struct
    {
        GMainLoop *loop;
        GstElement *source;
        std::chrono::time_point<std::chrono::system_clock> timestamp;
        BufferCbFunc _buffercb = nullptr;
    } ProgramData;
    ProgramData * _program_data;

    /* called when the appsink notifies us that there is a new buffer ready for
     * processing */
    static GstFlowReturn on_new_sample_from_sink (GstElement * elt, ProgramData * data)
    {
        GstSample *sample;
        GstBuffer *buffer;

        std::chrono::time_point<std::chrono::system_clock> timestamp;
        timestamp = std::chrono::system_clock::now();

        int elapsed_seconds = std::chrono::duration_cast<std::chrono::milliseconds>(timestamp - data->timestamp).count();
#if 0
        std::cout << "elapsed " << elapsed_seconds <<"\n" ;
        std::cout << "FPS " << (1.0/elapsed_seconds)*1000 <<"\n" ;
#endif
        data->timestamp = timestamp;

        /* get the sample from appsink */
        sample = gst_app_sink_pull_sample (GST_APP_SINK (elt));
        buffer = gst_sample_get_buffer (sample);

        //std::cout << "size =  " << gst_buffer_get_size(buffer) <<"\n" ;
        frame_counter++;
        /* make a copy */
        //app_buffer = gst_buffer_copy (buffer);

        /* we don't need the appsink sample anymore */
        gst_sample_unref (sample);

        return GST_FLOW_OK;
    }

    /* called when we get a GstMessage from the source pipeline when we get EOS */
    static gboolean
        on_source_message (GstBus * bus, GstMessage * message, ProgramData * data)
        {

            switch (GST_MESSAGE_TYPE (message)) {
                case GST_MESSAGE_EOS:
                    g_print ("EOS \n");
                    g_main_loop_quit (data->loop);
                    break;
                case GST_MESSAGE_ERROR:
                    g_print ("Received error\n");
                    g_main_loop_quit (data->loop);
                    break;
                default:
                    break;
            }
            return TRUE;
        };


  template <class TVnnInputConnectorStrategy, class TVnnOutputConnectorStrategy>
    int StreamLibGstreamerTX2<TVnnInputConnectorStrategy, TVnnOutputConnectorStrategy>::init()
    {
        ProgramData *data = NULL;
        GstBus *bus = NULL;
        GstElement *testsink = NULL;
        std::ostringstream launch_stream;
        GError *error = nullptr;
        std::string input_stream;
        std::string launch_string;
        int w_scale = 300;
        int h_scale = 300;

        gst_init(nullptr, nullptr);
        data = g_new0 (ProgramData, 1);

        data->loop = g_main_loop_new (NULL, FALSE);

        input_stream = this->_input.get_input_stream();
        std::cout << "input stream =  " << input_stream <<  std::endl;


        launch_stream
          << input_stream << " ! "
          << "nvvidconv ! "
          << "video/x-raw, format=RGBA, width="<< w_scale <<", height="<< h_scale <<" ! "
          << "appsink name=testsink";


        launch_string = launch_stream.str();

        g_print("Using launch string: %s\n", launch_string.c_str());

        data->source = gst_parse_launch (launch_string.c_str(), &error);

        if (data->source == NULL) {
            g_print ("Bad source\n");
            g_main_loop_unref (data->loop);
            g_free (data);
            return -1;
        }

        /* to be notified of messages from this pipeline, mostly EOS */
        bus = gst_element_get_bus (data->source);
        gst_bus_add_watch (bus, (GstBusFunc) on_source_message, data);
        gst_object_unref (bus);

        /* we use appsink in push mode, it sends us a signal when data is available
         * and we pull out the data in the signal callback. We want the appsink to
         * push as fast as it can, hence the sync=false */
        testsink = gst_bin_get_by_name (GST_BIN (data->source), "testsink");
        g_object_set (G_OBJECT (testsink), "emit-signals", TRUE, "sync", FALSE, NULL);
        g_signal_connect (testsink, "new-sample",
                G_CALLBACK (on_new_sample_from_sink), data);
        gst_object_unref (testsink);

        _program_data = data;
        return 0;
    };

  template <class TVnnInputConnectorStrategy, class TVnnOutputConnectorStrategy>
    int StreamLibGstreamerTX2<TVnnInputConnectorStrategy, TVnnOutputConnectorStrategy>::run()
    {
        /* launching things */
        _program_data->timestamp = std::chrono::system_clock::now();
	frame_counter = 0;
        gst_element_set_state (_program_data->source, GST_STATE_PLAYING);
        /* let's run !, this loop will quit when the sink pipeline goes EOS or when an
         * error occurs in the source or sink pipelines. */
        g_print ("Let's run!\n");
        g_main_loop_run (_program_data->loop);

        std::cout << "Frame count " << frame_counter << std::endl;;
        g_print ("Going out\n");
        return 0;
    }

  template <class TVnnInputConnectorStrategy, class TVnnOutputConnectorStrategy>
    int StreamLibGstreamerTX2<TVnnInputConnectorStrategy, TVnnOutputConnectorStrategy>::stop()
    {
        return 0;
    }

  template <class TVnnInputConnectorStrategy, class TVnnOutputConnectorStrategy>
    void StreamLibGstreamerTX2<TVnnInputConnectorStrategy, TVnnOutputConnectorStrategy>::
    set_buffer_cb(BufferCbFunc &buffercb)
    {
      this->_buffercb = buffercb;
      _program_data->_buffercb = buffercb;
    }


template class StreamLibGstreamerTX2<VnnInputConnectorCamera, VnnOutputConnectorDummy>;
template class StreamLibGstreamerTX2<VnnInputConnectorFileTX2, VnnOutputConnectorDummy>;
}

