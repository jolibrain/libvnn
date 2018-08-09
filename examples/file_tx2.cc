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

#include "streamlibgstreamertx2.h"
#include "inputconnectorfiletx2.h"
#include "outputconnectordummy.h"
#include <iostream>
#include <chrono>

using namespace vnn;

BufferCbFunc dummy_callback=[]( long unsigned int size , unsigned char * data )
    {
      return 0;
    };

int main(int argc, char** argv)
{
  // set video patt given as argument
  std::string video_path = argv[1];
  std::string container;
  std::string decoder;
  int duration =10;
  std::chrono::time_point<std::chrono::system_clock> start, end;

  StreamLibGstreamerTX2<InputConnectorFileTX2, OutputConnectorDummy>  my_streamlib;

  my_streamlib._input.set_filepath(video_path);
  if (argc == 3) {
    container = argv[2];
    my_streamlib._input.set_container(container);
  } else if (argc == 4) {
    container = argv[2];
    decoder = argv[3];
    my_streamlib._input.set_container(container);
    my_streamlib._input.set_decoder(decoder);
  };


  my_streamlib.init();
  my_streamlib.set_buffer_cb(dummy_callback);
  start = std::chrono::system_clock::now();
  my_streamlib.run();
  end = std::chrono::system_clock::now();

  my_streamlib.stop();

  int elapsed_seconds = std::chrono::duration_cast<std::chrono::seconds>
	  (end-start).count();
  std::time_t end_time = std::chrono::system_clock::to_time_t(end);

  std::cout << "finished computation at " << std::ctime(&end_time)
	  << "elapsed time: " << elapsed_seconds << "s\n";
  return 0;
}
