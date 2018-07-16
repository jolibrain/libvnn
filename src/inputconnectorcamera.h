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

#ifndef INPUTCONNECTORCAMERA_H
#define INPUTCONNECTORCAMERA_H

#include "inputconnectorstrategy.h"

#include <string>
namespace vnn
{

  class InputConnectorCamera: public InputConnectorStrategy
  {
    public:
      InputConnectorCamera() {};
      ~InputConnectorCamera() {};
      void init();

      std::string get_input_stream() {
        return "v4l2src num_buffers=10 ! video/x-raw,format=YUY2,width=1280,height=720,framerate=10/1";
      }
  };


}
#endif //INPUTCONNECTORCAMERA_H

