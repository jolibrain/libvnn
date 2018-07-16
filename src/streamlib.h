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

#ifndef STREAMLIB_H
#define STREAMLIB_H

#include <functional>

namespace vnn
{

  typedef std::function<int (long unsigned int, unsigned char * )> BufferCbFunc;

  template <class TInputConnectorStrategy, class TOutputConnectorStrategy>
    class StreamLib
    {
      public:
        StreamLib() {}
        ~StreamLib() {}

        int init( )
        {  return 0;};

        int run()
        {  return 0;};
        int stop()
        {  return 0;};
        TInputConnectorStrategy _input; /**< input connector strategy for channeling data in. */
        TOutputConnectorStrategy _output; /**< output connector strategy for passing results back to API. */
    };
/**
     * \brief sets the gradient function, if available.
     * @param gfunc gradient function
     */

#if 0
  class GstreamerTX2: StreamLib
  {};

  class GstreamerDesktop: StreamLib
  {};

  class OutputConnectorBuffer: OutputConnector
  {};
#endif

}
#endif

