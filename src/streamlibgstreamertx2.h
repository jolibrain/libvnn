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


#ifndef STREAMLIBGSTREAMERTX2_H
#define STREAMLIBGSTREAMERTX2_H

#include <string>
#include "streamlib.h"

namespace vnn
{
  template <class TVnnInputConnectorStrategy, class TVnnOutputConnectorStrategy>
    class StreamLibGstreamerTX2: public StreamLib<TVnnInputConnectorStrategy, TVnnOutputConnectorStrategy>
  {
    public:
      StreamLibGstreamerTX2() {}
      ~StreamLibGstreamerTX2() {}
      int init();
      int run();
      void set_buffer_cb(BufferCbFunc &buffercb);
      BufferCbFunc _buffercb = nullptr;

     int stop();

    private:

      /* pipe to reproduct
       * gst-launch -v videotestsrc ! video/x-raw,width=320,height=240,format=UYVY ! xvimagesink
       */
      /* these are the caps we are going to pass through the appsink */
      std::string _video_caps =
        "video/x-raw,width=300,height=300,format=YUY2";
  };


}
#endif

