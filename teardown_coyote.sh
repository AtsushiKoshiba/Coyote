#!/usr/bin/env bash

sudo rmmod coyote_drv
sudo bash sw/util/hot_reset.sh "e1:00.0"

# xilinx-shell "vivado -mode batch -nolog -nojournal -source program_fpga.tcl -tclargs $1"

