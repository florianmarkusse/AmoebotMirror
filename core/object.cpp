/* Copyright (C) 2020 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

#include "core/object.h"

Object::Object(const Node& node, const bool isTraversable, const bool anchor)
    : _node(node), _isTraversable(isTraversable), _anchor(anchor)
{
}
