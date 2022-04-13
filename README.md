# SORT-socket-stream
***
The assignment refers to track A).
The HUB process receives a chosen number of temperatures from J sensors. For each sensor temperature, a child is created that takes care of the transmission to the hub. The hub waits for J connected processes and then the temperatures are sent to the server which takes care of forwarding them to the actuators already listening. They calculate the average and the user is asked to unsubscribe.

## Summary
1. [Launch of the application] (# Launch)
### Application launch
***
The executables are launched first:
$ ./hub "J" (default J = 2)
$ ./server
subsequently the actuators are registered:
$ ./actuators "name" "(i / d)"

Tgoal and the list of sensor names to be entered will be requested iteratively. e.g .:
------------------------------------
$ ./actuators a1 i
Tgoal = 23
Enter 0 to stop ...
Sensor name to subscribe: t1
Sensor name to subscribe: t2
Sensor name to subscribe: 0

$ ./ a2 actuators i
Tgoal = 18
Enter 0 to stop ...
Sensor name to subscribe: t1
Sensor name to subscribe: 0

------------------------------------
The actuators will subscribe to the server and wait for the temperatures to be subscribed.
Successively it is required to run J processes (J terminals) sensors "name" "n° measures". e.g.:
Successively it is required to run J processes (J terminals) sensors "name" "n ° measures". e.g .:
------------------------------------
$ ./sensors t1 3
$ ./sensors t2 4
------------------------------------
Each process will send temperatures to the hub as required by the assignment. The hub will take care of the transmission to the server and the server will only forward the requested measures to the enrollment actuators.
The actuators will calculate the averages and decide accordingly whether to turn the heaters on or off.
The sensor processes end the execution by receiving the number of actuators that have subscribed to the measure. The actuators for termination require to be unsubscribed by the user.
Finally, one or more actuators are launched in unsubscribe mode. e.g .:
------------------------------------
$ ./ a3 actuators d
Enter 0 to stop ...
Actuator name to unsubscribe: a1
Actuator name to unsubscribe: a2
Actuator name to unsubscribe: 0
------------------------------------
The server will then send an empty string to a1 and a2 which will finish receiving and close.
