#!/bin/bash

rpi-rw
sudo gcc tcfan_daemon.c  -o tcfan_daemon -lwiringPi -lpthread
sudo chmod a+x ./tcfan_daemon
sudo cp -f ./tcfan.service /lib/systemd/system
sudo cp -f ./tcfan_daemon /usr/local/sbin
sudo cp -f ./start_tcfan.sh /etc
sudo systemctl enable tcfan.service
