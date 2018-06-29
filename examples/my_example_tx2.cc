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
#include "streamlibgstreamertx2.h"
#include "inputconnectorcamera.h"
#include "outputconnectordummy.h"

using namespace vnn;

int main ()
{

  StreamLibGstreamerTX2<InputConnectorCamera,OutputConnectorDummy>  my_streamlib;
 // StreamLib <InputConnectorCamera,OutputConnector>  my_streamlib;

  my_streamlib.init();
  my_streamlib.run();
  my_streamlib.stop();

  return 4;

}


