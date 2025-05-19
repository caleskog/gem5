# Copyright (c) 2025
# Authors: Christoffer Åleskog (caleskog)

from m5.params import *
from m5.proxy import *
from m5.SimObject import SimObject


class SystolicArray(SimObject):
    type = 'SystolicArray'
    cxx_header = "custom/systolic_array.hh"
    cxx_class = "gem5::SystolicArray"
    cores = Param.Int(1, "Cores/Threads using this device.")
