/**
 * libvnn
 * Copyright (c) 2018 Nicolas Bertrand
 * Author:
 *
 * This file is part of libvnn.
 *
 * deepdetect is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * deepdetect is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with deepdetect.  If not, see <http://www.gnu.org/licenses/>.
 * 
 *
 * gst-launch-1.0 filesrc location="/home/nicolas/Vidéos/bbb_60.mkv" ! decodebin ! videoscale ! videoconvert ! video/x-raw,width=300,height=300, format=YUY2 ! xvimagesink
 *
 */





#include <cstdlib>

#include "inputconnectorcamera.h"
#include "inputconnectorfile.h"
#include "streamlibgstreamerdesktop.h"
#include "outputconnectordummy.h"
#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <chrono>
#include <iostream>
#include <sstream>

namespace vnn {



    typedef struct
    {
        GMainLoop *loop;
        GstElement *source;
        std::chrono::time_point<std::chrono::system_clock> timestamp;
        BufferCbFunc _buffercb;
    } ProgramData;
    ProgramData * _program_data;

    /* called when the appsink notifies us that there is a new buffer ready for
     * processing */
  int dummy_callback( long unsigned int size , unsigned char * data )
    {
        std::cout << "cb map.size =  " << size <<  std::endl;
        std::cout << "cb map.data =  " <<  static_cast<void*>(data) << std::endl;
      return 0;
    }

    static GstFlowReturn on_new_sample_from_sink (GstElement * elt, ProgramData * data)
    {
        GstSample *sample;
        GstBuffer *buffer;

        std::chrono::time_point<std::chrono::system_clock> timestamp;
        timestamp = std::chrono::system_clock::now();

        int elapsed_seconds = std::chrono::duration_cast<std::chrono::milliseconds>(timestamp - data->timestamp).count();
        std::cout << "elapsed " << elapsed_seconds <<"\n" ;
        std::cout << "FPS " << (1.0/elapsed_seconds)*1000 <<"\n" ;
        data->timestamp = timestamp;

        /* get the sample from appsink */
        sample = gst_app_sink_pull_sample (GST_APP_SINK (elt));
        buffer = gst_sample_get_buffer (sample);

        std::cout << "size =  " << gst_buffer_get_size(buffer) <<"\n" ;
        std::cout << "address =  " << static_cast<void*>(buffer) << std::endl;
        /* make a copy */
        //app_buffer = gst_buffer_copy (buffer);
        GstMapInfo map;
        gst_buffer_map (buffer, &map, GST_MAP_READ);
        std::cout << "map.size =  " << map.size <<"\n" ;
        std::cout << "map.data =  " <<  static_cast<void*>(map.data) <<"\n" ;
        data->_buffercb(map.size, map.data);

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


  template <class TInputConnectorStrategy, class TOutputConnectorStrategy>
    int StreamLibGstreamerDesktop<TInputConnectorStrategy, TOutputConnectorStrategy>::init()
    {
        ProgramData *data = NULL;
        GstBus *bus = NULL;
        GstElement *testsink = NULL;
        std::ostringstream launch_stream;
        GError *error = nullptr;
        std::string launch_string;
        std::string input_stream;


        this->_input.init();
        this->_output.init();


        gst_init(nullptr, nullptr);
        data = g_new0 (ProgramData, 1);

        data->loop = g_main_loop_new (NULL, FALSE);

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

  template <class TInputConnectorStrategy, class TOutputConnectorStrategy>
    int StreamLibGstreamerDesktop<TInputConnectorStrategy, TOutputConnectorStrategy>::run()
    {

        /* launching things */
        _program_data->timestamp = std::chrono::system_clock::now();
        gst_element_set_state (_program_data->source, GST_STATE_PLAYING);
        /* let's run !, this loop will quit when the sink pipeline goes EOS or when an
         * error occurs in the source or sink pipelines. */
        g_print ("Let's run!\n");
        g_main_loop_run (_program_data->loop);
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
      _program_data->_buffercb = buffercb;
    }



template class StreamLibGstreamerDesktop<InputConnectorCamera, OutputConnectorDummy>;
template class StreamLibGstreamerDesktop<InputConnectorFile, OutputConnectorDummy>;

  
}

