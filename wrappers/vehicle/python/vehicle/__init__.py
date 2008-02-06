# Copyright (C) 2007 Maryland Robotics Club
# Copyright (C) 2007 Joseph Lisee <jlisee@umd.edu>
# All rights reserved.
#
# Author: Joseph Lisee <jlisee@umd.edu>
# File:  wrappers/vehicle/python/vehicle/__init__.py

# STD Imports
import sys
import StringIO

# Capture stderr, to suppress unwanted warnings
stderr = sys.stderr
sys.stderr = StringIO.StringIO()

try:
    # Core more first because of Boost.Python wrapping dependencies
    import ext.core

    from ext._vehicle import *

finally:
    sys.stderr = stderr

