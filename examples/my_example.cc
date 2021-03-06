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
#include "vnninputconnectorcamera.h"
#include "vnnoutputconnectordummy.h"
#include <iostream>

using namespace vnn;

BufferCbFunc dummy_callback=[](int width , int height, unsigned char * data )
    {
      return 0;
    };

int main ()
{

  StreamLibGstreamerDesktop<VnnInputConnectorCamera,VnnOutputConnectorDummy>  my_streamlib;

  my_streamlib.init();
  my_streamlib.set_buffer_cb(dummy_callback);
  my_streamlib.run();
  my_streamlib.stop();

  return 0;

}


