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

#ifndef STREAMLIBGSTREAMERDESKTOP_H
#define STREAMLIBGSTREAMERDESKTOP_H

#include <string>
#include "streamlib.h"

namespace vnn
{
  template <class TInputConnectorStrategy, class TOutputConnectorStrategy>
    class StreamLibGstreamerDesktop: public StreamLib<TInputConnectorStrategy, TOutputConnectorStrategy>
  {
    public:
      StreamLibGstreamerDesktop() {}
      ~StreamLibGstreamerDesktop() {}
      int init();
      int run();
      int stop();

    private:

      /* pipe to reproduct
       * gst-launch -v videotestsrc ! video/x-raw,width=320,height=240,format=UYVY ! xvimagesink
       */
      /* these are the caps we are going to pass through the appsink */
      std::string _video_caps =
        "video/x-raw,width=300,height=300,format=YUY2";
  };

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

