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

#ifndef VNNINPUTCONNECTORFILETX2_H
#define VNNINPUTCONNECTORFILETX2_H

#include "vnninputconnectorstrategy.h"

#include <string>
#include <unordered_map>

namespace vnn
{

  static std::unordered_map<std::string, std::string> TX2_CONTAINERS {
	{"qt","qtdemux"},
	{"mkv","matroskademux"}
  };

  static std::unordered_map<std::string, std::string> TX2_DECODERS {
	{"h264","h264parse ! omxh264dec"},
	{"h265", "h265parse ! omxh265dec"}
  };


  class VnnInputConnectorFileTX2: public VnnInputConnectorStrategy
  {
    public:
      VnnInputConnectorFileTX2():
        _container(TX2_CONTAINERS["qt"]),
        _decoder(TX2_DECODERS["h264"])
      {}


      VnnInputConnectorFileTX2(const std::string & file_path, const int & duration_s)
        : _file_path(file_path), _duration_s(duration_s),
        _container(TX2_CONTAINERS["qt"]),
        _decoder(TX2_DECODERS["h264"])
      {}
      ~VnnInputConnectorFileTX2() {}

      void init() {};
      void set_filepath(std::string &filepath) {
        _file_path = filepath;
      }
      void set_container(std::string &container) {
        _container = TX2_CONTAINERS[container];
      }
      void set_decoder(std::string &decoder) {
        _decoder = TX2_DECODERS[decoder];
      }


      std::string get_input_stream();

      std::string _file_path;
      std::string _container;
      std::string _decoder;
      int _duration_s;
  };


}
#endif // INPUTCONNECTORFILETX2_H

