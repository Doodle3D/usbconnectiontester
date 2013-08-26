usbconnectiontester
===================

Doodle3D USB connection tester

A lot of routers that have the ar9331 chip, like the TP-Link MR3020, have usb communication issues. This only affects the non high speed (less than 480Mb/s) communication. Like Arduino's for example.  
It's probably caused by the WiFi module it's radio signals. See the following forum thread for more details.
https://forum.openwrt.org/viewtopic.php?id=39956

The simplest solution is to use a high speed (usb 2) hub that translates the full speed (12Mb/s) signals to highspeed (480Mb/s). 

To easilly test the communication we made this package that will keep sending a message and a Arduino applet that responds. As long as the RX and TX lights keep lighting up the communcation is fine. When it stops it failed.
