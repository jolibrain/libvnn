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


#include "inputconnectorfile.h"

#include <cstdlib>
#include <iostream>
#include <sstream>

namespace vnn {

  std::string InputConnectorFile::get_input_stream() {
    std::ostringstream input_stream;
    input_stream
      << "filesrc location=" << this->_file_path << " ! "
      << "decodebin ";
    return input_stream.str();
  }
}
