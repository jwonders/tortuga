/*
 * Copyright (C) 2007 Robotics at Maryland
 * Copyright (C) 2007 Daniel Hakim
 * All rights reserved.
 *
 * Author: Daniel Hakim
 * File:  packages/vision/include/ProcessList.h
 */

#ifndef RAM_VISION_PROCESS_LIST_H_06_11_2007
#define RAM_VISION_PROCESS_LIST_H_06_11_2007

// STD Includes
#include <iostream>
#include <string>
#include <list>
#include "vision/include/Export.h"

namespace ram {
namespace vision {

typedef std::list<std::string> StringList;
typedef StringList::iterator StringListIterator;

class RAM_EXPORT ProcessList
{
 public:
  ProcessList();
  void	addStep(std::string);
  StringList toCall;
 private:
};
} // namespace vision
} // namespace ram
    
#endif // RAM_VISION_PROCESS_LIST_H_06_11_2007

