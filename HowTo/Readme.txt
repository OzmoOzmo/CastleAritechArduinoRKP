
See http://www.boards.ie/vbulletin/showthread.php?p=88215184 for the details...


Basically this allows you to access an Aritech Alarm from a mobile phone by emulating one of its keypads using an arduino.


There are two options for the electronics

1. Straight connect TX of Arduino and RX of the Alarm panel - and use a Diode and connect the RX of Arduino to TX of the Alarm panel

2. Do it using a two transistor interface board - connects to 12v ABCD keypad socket

3. Do it propperly using a 4 transistor board which completly isolates the arduino when not needed (emulates the original keypad hardware more closely)



Plans for 1 and 2 are published


