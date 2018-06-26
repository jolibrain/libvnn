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

#ifndef INPUTCONNECTORSTRATEGY_H
#define INPUTCONNACTORSTRATEGY_H
#include <string>

namespace vnn
{
  class InputConnectorStrategy
  {
    public:
      InputConnectorStrategy() {};
      ~InputConnectorStrategy() {};

      /**
       * \brief return input command to apply to streamlib
       */
      std::string input_command();
  };

}
#endif

