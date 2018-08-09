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

#ifndef INPUTCONNECTORCAMERATX2_H
#define INPUTCONNECTORCAMERATX2_H

#include "inputconnectorstrategy.h"

#include <cstdlib>
#include <string>
#include <sstream>
namespace vnn
{

  class InputConnectorCameraTX2: public InputConnectorStrategy
  {
    public:
      InputConnectorCameraTX2() {};
      ~InputConnectorCameraTX2() {};
      void init();

      std::string get_input_stream() {
        std::ostringstream input_stream;
        int w = 4208;
        int h = 3120;
        input_stream
          << "nvcamerasrc ! "
          << "video/x-raw(memory:NVMM), width="<< w <<", height="<< h <<", framerate=30/1 ! ";

        return input_stream.str();
      }
  };


}
#endif // INPUTCONNECTORCAMERATX2_H
