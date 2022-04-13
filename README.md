# SORT-socket-stream
***
The assignment refers to track A).
The HUB process receives a chosen number of temperatures from J sensors. For each sensor temperature, a child is created that takes care of the transmission to the hub. The hub waits for J connected processes and then the temperatures are sent to the server which takes care of forwarding them to the actuators already listening. They calculate the average and the user is asked to unsubscribe.

## Application launch
***
The executables are launched first:<br />
$ ./hub "J" (default J = 2)<br />
$ ./server<br />
subsequently the actuators are registered:<br />
$ ./actuators "name" "(i / d)"<br /><br />

Tgoal and the list of sensor names to be entered will be requested iteratively. e.g .:<br />

------------------------------------
$ ./actuators a1 i<br />
Tgoal = 23<br />
Enter 0 to stop ...<br />
Sensor name to subscribe: t1<br />
Sensor name to subscribe: t2<br />
Sensor name to subscribe: 0<br /><br />

$ ./ a2 actuators i<br />
Tgoal = 18<br />
Enter 0 to stop ...<br />
Sensor name to subscribe: t1<br />
Sensor name to subscribe: 0<br /><br />

------------------------------------
The actuators will subscribe to the server and wait for the temperatures to be subscribed.<br />
Successively it is required to run J processes (J terminals) sensors "name" "n° measures". e.g.:<br />

------------------------------------
$ ./sensors t1 3<br />
$ ./sensors t2 4<br /><br />

------------------------------------
Each process will send temperatures to the hub as required by the assignment. The hub will take care of the transmission to the server and the server will only forward the requested measures to the enrollment actuators.<br />
The actuators will calculate the averages and decide accordingly whether to turn the heaters on or off.<br />
The sensor processes end the execution by receiving the number of actuators that have subscribed to the measure. The actuators for termination require to be unsubscribed by the user.<br />
Finally, one or more actuators are launched in unsubscribe mode. e.g .:<br />

------------------------------------
$ ./ a3 actuators d<br />
Enter 0 to stop ...<br />
Actuator name to unsubscribe: a1<br />
Actuator name to unsubscribe: a2<br />
Actuator name to unsubscribe: 0<br /><br />

------------------------------------
The server will then send an empty string to a1 and a2 which will finish receiving and close.
