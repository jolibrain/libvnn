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

#ifndef VNNVIDEOSTREAM_H
#define VNNVIDEOSTREAM_H

namespace vnn
{
  template <class TVnnInputConnectorStrategy, class TVnnOutputConnectorStrategy>
    class VideoStreamNN
    {
      public:

        int init()
        {
          _input.init()
          _output.init()
        }

        int run()
        {
          _streamlib.run();
        }

        TVnnInputConnectorStrategy _input; /**< input connector strategy for channeling data in. */
        TVnnOutputConnectorStrategy _output; /**< output connector strategy for passing results back to API. */


    };
// TODO: create files for OutputConnector and StreamLib
#if 0
  class OutputConnector
  {
    int init()
    {};
  };

  class StreamLib
  {
    int init()
    {};
  };


  class GstreamerTX2: StreamLib
  {};

  class GstreamerDesktop: StreamLib
  {};

  class OutputConnectorBuffer: OutputConnector
  {};
#endif

}
#endif

