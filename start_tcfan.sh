#!/bin/bash

cpu_tmp_file="/sys/class/thermal/thermal_zone0/temp"
tc_start=36
current=$(cat $cpu_tmp_file)

#sudo kill -9 $(pidof tcfan_daemon)
#sudo kill -9 $(ps -ef|grep tcfan_daemon |awk '{print $2}')
#sudo pkill tcfan_daemon
#sudo killall tcfan_daemon

sudo /usr/local/sbin/tcfan_daemon -R -A -t $tc_start $cpu_tmp_file
