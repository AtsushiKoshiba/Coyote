#!/usr/bin/env bash

sudo bash sw/util/hot_reset.sh "e1:00.0"
sudo insmod driver/coyote_drv.ko