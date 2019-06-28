Region of interest Graphical User Interface
===========================================

Overview
--------
This component provides an interactive markers based graphical interface on rviz. It's focus is to manage, edit and define regions of interest for a specific module on the rc_visard sensor.

For more information go to the ROS Wiki page of this tutorial: http://wiki.ros.org/rc_visard/Tutorials/ManagingRegionsofInterest

Launch
------
This node can be started using the roslaunch command. For this, the sensor serial number, its ip address and the module to be utilized (rc_itempick or rc_boxpick) have to be given as arguments.

~~~
roslaunch rc_pick_client InteractiveRoiSelection.launch device:=serial_number host:=sensor_ip pick_module:=rc_module
~~~
