/**
 * libvnn
 * Copyright (c) 2018 Nicolas Bertrand
 * Author:
 *
 * This file is part of libvnn.
 *
 * deepdetect is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * deepdetect is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with deepdetect.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef VNNVIDEOSTREAM_H
#define VNNVIDEOSTREAM_H

namespace vnn
{
  template <class TInputConnectorStrategy, class TOutputConnectorStrategy>
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

        TInputConnectorStrategy _input; /**< input connector strategy for channeling data in. */
        TOutputConnectorStrategy _output; /**< output connector strategy for passing results back to API. */


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

