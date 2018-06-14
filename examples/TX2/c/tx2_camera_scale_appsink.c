/* libvnn
 *
 * tx2_camera_scale_appsink.c: example for TX2 camera with
 * appsink.
 *
 * Copyright (C) 2018 Nicolas Bertrand <nicolas@davionbertrand.net>
 *
 * origin sample: gstreamer/gst-plugins-base/tests/examples/app/appsink-src.c
*/

#include <gst/gst.h>

#include <string.h>

#include <gst/app/gstappsink.h>

#include <iostream>
#include <chrono>
#include <ctime>

/* these are the caps we are going to pass through the appsink */
const gchar *video_caps =
    "video/x-raw(memory:NVMM), width=(int)300, height=(int)300, format=(string)RGBA";


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
  GstBuffer *app_buffer, *buffer;
  GstElement *source;
  GstFlowReturn ret;
  GstMemory *memory;

  std::chrono::time_point<std::chrono::system_clock> timestamp;
  timestamp = std::chrono::system_clock::now();
  std::time_t ts_time = std::chrono::system_clock::to_time_t(timestamp);

  int elapsed_seconds = std::chrono::duration_cast<std::chrono::milliseconds>(timestamp - data->timestamp).count();

  std::cout << "elapsed " << elapsed_seconds <<"\n" ;
  std::cout << "FPS " << (1.0/elapsed_seconds)*1000 <<"\n" ;
  data->timestamp = timestamp;
  /* get the sample from appsink */
  sample = gst_app_sink_pull_sample (GST_APP_SINK (elt));
  buffer = gst_sample_get_buffer (sample);

  std::cout << "size =  " << gst_buffer_get_size(buffer) <<"\n" ;
  /* make a copy */
  app_buffer = gst_buffer_copy (buffer);

  //g_print ("*");
  /* we don't need the appsink sample anymore */
  gst_sample_unref (sample);

  return GST_FLOW_OK;
}

/* called when we get a GstMessage from the source pipeline when we get EOS */
static gboolean
on_source_message (GstBus * bus, GstMessage * message, ProgramData * data)
{
  GstElement *source;

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

int
main (int argc, char *argv[])
{
  gchar *filename = NULL;
  ProgramData *data = NULL;
  gchar *string = NULL;
  GstBus *bus = NULL;
  GstElement *testsink = NULL;

  gst_init (&argc, &argv);

  data = g_new0 (ProgramData, 1);

  data->loop = g_main_loop_new (NULL, FALSE);

  /* setting up source pipeline, we read from a file and convert to our desired
   * caps. */
  string =
      g_strdup_printf
      //("videotestsrc num_buffers=15 ! appsink caps=\"%s\" name=testsink",
      ("nvcamerasrc  num-buffers=15 ! video/x-raw(memory:NVMM), width=4208, height=3120, format=(string)NV12, framerate=(fraction)30/1 ! nvvidconv ! appsink caps=\"%s\" name=testsink",
      video_caps);
  g_free (filename);
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
  g_object_set (G_OBJECT (testsink), "emit-signals", TRUE, "sync", TRUE, NULL);
  g_signal_connect (testsink, "new-sample",
      G_CALLBACK (on_new_sample_from_sink), data);
  gst_object_unref (testsink);

  /* launching things */
  data->timestamp = std::chrono::system_clock::now();
  gst_element_set_state (data->source, GST_STATE_PLAYING);

  /* let's run !, this loop will quit when the sink pipeline goes EOS or when an
   * error occurs in the source or sink pipelines. */
  g_print ("Let's run!\n");
  g_main_loop_run (data->loop);
  g_print ("Going out\n");

  gst_element_set_state (data->source, GST_STATE_NULL);

  gst_object_unref (data->source);
  g_main_loop_unref (data->loop);
  g_free (data);

  return 0;
}
