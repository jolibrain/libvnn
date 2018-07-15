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

//#include "streamlib.h"
#include "streamlibgstreamerdesktop.h"
#include "inputconnectorfile.h"
#include "outputconnectordummy.h"
#include <iostream>

using namespace vnn;

BufferCbFunc dummy_callback=[]( long unsigned int size , unsigned char * data )
    {
        std::cout << "cb map.size =  " << size <<  std::endl;
        std::cout << "cb map.data =  " <<  static_cast<void*>(data) << std::endl;
      return 0;
    };

int main(int argc, char** argv)
{
  // set video patt given as argument
  std::string video_path = argv[1];
  int duration =10;

  StreamLibGstreamerDesktop<InputConnectorFile, OutputConnectorDummy>  my_streamlib;

  my_streamlib._input.set_filepath(video_path);
  my_streamlib.init();
  my_streamlib.set_buffer_cb(dummy_callback);
  my_streamlib.run();
  my_streamlib.stop();

  return 0;

}


