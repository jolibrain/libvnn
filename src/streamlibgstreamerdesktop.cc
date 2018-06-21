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
 */


#include "streamlibgstreamerdesktop.h"
#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <chrono>
#include <iostream>
namespace vnn {
    /* pipe to reproduct
     * gst-launch -v videotestsrc ! video/x-raw,width=320,height=240,format=UYVY ! xvimagesink
     */

    /* these are the caps we are going to pass through the appsink */
    const gchar *video_caps =
        "video/x-raw,width=300,height=300,format=YUY2";


    typedef struct
    {
        GMainLoop *loop;
        GstElement *source;
        std::chrono::time_point<std::chrono::system_clock> timestamp;
    } ProgramData;

    /* called when the appsink notifies us that there is a new buffer ready for
     * processing */
    static GstFlowReturn
        on_new_sample_from_sink (GstElement * elt, ProgramData * data)
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
        }

    /* called when we get a GstMessage from the sink pipeline when we get EOS, we
     * exit the mainloop and this testapp. */
    static gboolean
        on_sink_message (GstBus * bus, GstMessage * message, ProgramData * data)
        {
            /* nil */
            switch (GST_MESSAGE_TYPE (message)) {
                case GST_MESSAGE_EOS:
                    g_print ("Finished playback\n");
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
        }


    int StreamLibGstreamerDesktop::init()
    {
        int argc;
        char **argv[10];
        ProgramData *data = NULL;
        gchar *string = NULL;
        GstBus *bus = NULL;
        GstElement *testsink = NULL;


        data = g_new0 (ProgramData, 1);

        data->loop = g_main_loop_new (NULL, FALSE);

        /* setting up source pipeline, we read from a file and convert to our desired
         * caps. */
        string =
            g_strdup_printf
            //("videotestsrc num_buffers=15 ! appsink caps=\"%s\" name=testsink",
            ("v4l2src  num_buffers=15 ! video/x-raw,format=YUY2,width=1280,height=720,framerate=10/1 ! videoscale ! appsink caps=\"%s\" name=testsink",
             video_caps);
        data->source = gst_parse_launch (string, NULL);
        g_free (string);

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

        /* launching things */
        data->timestamp = std::chrono::system_clock::now();
        gst_element_set_state (data->source, GST_STATE_PLAYING);


        gst_init(&argc, argv);
        return 0 ;
    };

}
