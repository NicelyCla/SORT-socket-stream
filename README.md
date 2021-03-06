# SORT-socket-stream
***
The assignment refers to track A).<br />
The HUB process receives a chosen number of temperatures from J sensors. For each sensor temperature, a child is created that takes care of the transmission to the hub. The hub waits for J connected processes and then the temperatures are sent to the server which takes care of forwarding them to the actuators already listening. Then, they calculate the average of the measures and the user is asked to unsubscribe the actuators.

## Application launch
First launch:<br />
$ ./hub "J" (default J = 2)<br />
$ ./server<br />
subsequently the actuators are registered:<br />
$ ./attuatori "name" "(i/d)"<br /><br />

Tgoal and the list of sensor names will be requested iteratively. e.g .:<br />

------------------------------------
$ ./attuatori a1 i<br />
Tgoal = 23<br />
Enter 0 to stop ...<br />
Sensor name to subscribe: t1<br />
Sensor name to subscribe: t2<br />
Sensor name to subscribe: 0<br /><br />

$ ./attuatori a2 i<br />
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
The sensor processes end the execution by receiving the number of actuators that have subscribed to the measure. For terminate the actuators, it is required the subscription by the user.<br />
So, one or more actuators are launched in unsubscribe mode. e.g .:<br />

------------------------------------
$ ./attuatori a3 d<br />
Enter 0 to stop ...<br />
Actuator name to unsubscribe: a1<br />
Actuator name to unsubscribe: a2<br />
Actuator name to unsubscribe: 0<br /><br />

------------------------------------
Then, the server send an empty string to actuators and the reception will end.
