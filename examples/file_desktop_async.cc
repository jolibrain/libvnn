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

//#include "streamlib.h"
#include "streamlibgstreamerdesktop.h"
#include "vnninputconnectorfile.h"
#include "vnnoutputconnectordummy.h"
#include <iostream>
#include <chrono>
#include <thread>

using namespace vnn;

static long int frame_counter;

BufferCbFunc dummy_callback=[](int width , int height, unsigned char * data )
{
#if 0
  cv::Mat imgbuf ;
  cv::Mat rgbimgbuf ;
  std::ostringstream img_path;
  img_path << "/tmp/images/img" << frame_counter <<".jpg";
  std::cout << "img_path =" << img_path.str() << std::endl ;

  imgbuf = cv::Mat(cv::Size(width, height), CV_8UC2, (char*)data, cv::Mat::AUTO_STEP);
  rgbimgbuf = cv::Mat(cv::Size(width, height), CV_8UC3, (char*)data, cv::Mat::AUTO_STEP);
  cvtColor(imgbuf, rgbimgbuf, CV_YUV2BGRA_YUY2);
  imwrite(img_path.str(), rgbimgbuf);
  frame_counter++;
#endif
  return 0;
};

int main(int argc, char** argv)
{
  // set video patt given as argument
  std::string video_path = argv[1];
  //std::string video_path = "/home/nicolas/dev/jolibrain/samples/samples/bbb_60.mkv";
  int duration =10;
  frame_counter=0;
  std::chrono::time_point<std::chrono::system_clock> start, end;
  cv::Mat imgbuf ;
  int video_frame_count;
  std::ostringstream img_path;
  bool leave = false;
  int width, height;
  long  int timestamp;
  StreamLibGstreamerDesktop<VnnInputConnectorFile, VnnOutputConnectorDummy>  *my_streamlib =
  new StreamLibGstreamerDesktop<VnnInputConnectorFile, VnnOutputConnectorDummy>();

  my_streamlib->_input.set_filepath(video_path);
  my_streamlib->set_scale_size(200,200);
  my_streamlib->init();
  my_streamlib->set_buffer_cb(dummy_callback);
  start = std::chrono::system_clock::now();
  my_streamlib->run_async();

  width = my_streamlib->get_original_width();
  height = my_streamlib->get_original_height();
  std::cout
    << "Original Video size = "
    << width
    << "x"
    << height
    << std::endl ;


  while( leave == false)
  {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    img_path.str("");
    video_frame_count = my_streamlib->get_video_buffer(imgbuf, timestamp);
    img_path << "/tmp/images/img_" << ( long int ) timestamp <<".jpg";
    std::cout << "img_path =" << img_path.str() << "timestamp: " << ( long int ) timestamp <<  std::endl ;
    std::cout << "is_playing  =" << my_streamlib->is_playing() << std::endl ;
    imwrite(img_path.str(), imgbuf);
    frame_counter++;
    if  ( not( my_streamlib->is_playing()))  leave = true;
    imgbuf.release();
  }




  end = std::chrono::system_clock::now();
  my_streamlib->stop();

  std::cout << "is_playing  =" << my_streamlib->is_playing() << std::endl ;
  int elapsed_seconds = std::chrono::duration_cast<std::chrono::seconds>
	  (end-start).count();
  std::time_t end_time = std::chrono::system_clock::to_time_t(end);

  std::cout << "finished computation at " << std::ctime(&end_time)
	  << "elapsed time: " << elapsed_seconds << "s\n";

  std::this_thread::sleep_for(std::chrono::seconds(2));

    std::cout << "is_playing  =" << my_streamlib->is_playing() << std::endl ;
  return 0;

}
