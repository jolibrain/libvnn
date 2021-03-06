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

#ifndef VNNOUTPUTCONNECTORDUMMY_H
#define VNNOUTPUTCONNECTORDUMMY_H



#include "vnnoutputconnectorstrategy.h"
#include <string>
#include <functional>

namespace vnn
{
  typedef std::function<int (void)> SizeFunc;


  class VnnOutputConnectorDummy: public VnnOutputConnectorStrategy
  {
    public:
      VnnOutputConnectorDummy(){}
      VnnOutputConnectorDummy(SizeFunc &func);
      ~VnnOutputConnectorDummy() {}

      void init();
      /**
       * \brief return output command to apply to streamlib
       */
      std::string output_command();
    protected:
      SizeFunc _szfunc = nullptr;
  };



}
#endif

