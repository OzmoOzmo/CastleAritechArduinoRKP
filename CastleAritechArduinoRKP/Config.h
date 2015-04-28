/*
 * Config.h
 *
 *
 *	This is all you should need edit to get a working system
 *
 *   Aritech Alarm Panel Arduino Internet Enabled Keypad -  CS350 - CD34 - CD72 - CD91 and more
 *
 *   For Arduino (UNO or Leonardo) with added Ethernet Shield
 *
 *   See Circuit Diagram for wiring instructions
 *
 *   Author: Ozmo
 *
 *   See: http://www.boards.ie/vbulletin/showthread.php?p=88215184
 *
*/


#ifndef CONFIG_H_
#define CONFIG_H_

//Set to 1 for normal operation - comment out will debug print logs - but might slow down the arduino too much for some panels
#define QUIET 1

//Build Version (displayed on webpage)
#define sVersion "V3.00"

//Define this to enable sending emails - undefining will same some memory - useful for debugging (cannot have QUIET off and SENDEMAILS on - not enough memory)
#define SENDEMAILS 1

//#define DISPLAY_ALL_PACKETS 1 //uncomment to send all RKP packets to debug serial

//The IPPort for the server to listen on
#define IP_P 8383


#endif /* CONFIG_H_ */
