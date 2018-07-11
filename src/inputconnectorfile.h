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

#ifndef INPUTCONNECTORFILE_H
#define INPUTCONNECTORFILE_H

#include "inputconnectorstrategy.h"

#include <string>
namespace vnn
{

  class InputConnectorFile: public InputConnectorStrategy
  {
    public:
      InputConnectorFile() {}
      InputConnectorFile(const std::string & file_path, const int & duration_s)
        : _file_path(file_path), _duration_s(duration_s) {}
      ~InputConnectorFile() {}

      void init() {};

      std::string get_input_stream();
    std::string _file_path;
    int _duration_s;
  };


}
#endif

