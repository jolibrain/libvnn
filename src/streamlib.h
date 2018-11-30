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

  typedef std::function<int (int, int, unsigned char * )> BufferCbFunc;

  template <class TVnnInputConnectorStrategy, class TVnnOutputConnectorStrategy>
    class StreamLib
    {
      public:
        StreamLib() {}
        ~StreamLib() {}

        int init( )
        {  return 0;};

        int run()
        {  return 0;};
        int run_async();

        int stop()
        {  return 0;};
        TVnnInputConnectorStrategy _input; /**< input connector strategy for channeling data in. */
        TVnnOutputConnectorStrategy _output; /**< output connector strategy for passing results back to API. */

        void set_max_video_buffer( const int  &max_video_buffer){
            _max_video_buffer = max_video_buffer;
        };
        void set_scale_size( const int &width, const int &height){
          _scale_width = width;
          _scale_height = height;
        }
      protected:
        int _scale_width =300;
        int _scale_height=300;

      private:
        int _max_video_buffer = 100;

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

