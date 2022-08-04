
Temperature Controled Fan Daemon  ver. 0.9.9.

IMPORTANT -- WiringPI Library is necessary

I:

Service installation ----

cd /tmp

sudo git clone https://github.com/bmchina/tcfan-daemon.git

cd tcfan-daemon

sudo ./install.sh

 
 

II:

Shell command ----

Usage: 

./tcfan_daemon [-gtsRvh] [filename]

	-g pin		GPIO output pin number(WiringPi format ONLY)
	
	-t numberic	Fan start temperature(1.00-99.99)
	
	-s numberic	Fan speed(1-9, 9 means full speed)
	
	-A 		Auto adjust fan speed('-s' ignored)
	
	-R 		Reverse level voltage(to control PNP MOSFET)
	
	-v		Show version informations
	
	-h		Show this help

	filename	Linux CPU temperature file
			default:/sys/class/thermal/thermal_zone0/temp)

Note:

	This program work under default settings as below:
	
	ONLY tested on NANOPI Duo2(without PWM output pin)
	 
	level -- default for high level to control PNP
	 
	temperature -- please specify celsius degree

Examples:

	Get help information:
	    ./tcfan_daemon -h

	Show version infomation:
	    ./tcfan_daemon -v

	Start TCFAN daemon:
	    ./tcfan_daemon -R -s 45 -g 15 /sys/class/thermal/thermal_zone0/temp

Version: 0.9.9  Copyleft by BH4DKU  2022/08/08


