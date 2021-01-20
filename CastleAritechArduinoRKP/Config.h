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

//--------Configuration Start----------

#ifndef CONFIG_H_  //Dont touch
#define CONFIG_H_  //Dont touch

#define RKP_ID 1  //This is the Keypad id of the Arduinio - recommended 1 or 2 (0 is first keypad)

//The Arduino IP address and Port
//Here - the default settings are to use:  http://192.168.1.205:8383
#define IP_A 192
#define IP_B 168
#define IP_C 1
#define IP_D 205
//The IPPort for the server to listen on
#define IP_P 8383


//Set to 1 for normal operation - comment out will debug print useful logs - Only Useful for Leonardo
#define QUIET 1  //NOTE - For Arduino UNO - THIS MUST BE DEFINED (as we need the only Serial port for panel comms)

//Build Version (displayed on webpage)
#define sVersion "V4.04"







//Note: Gmail and Most other SMTP servers are no longer supported as it requires SSL - Use ESP32 version if Gmail is required
//You could setup a local SMTP server using an rpi.

//#define SENDEMAILS   //Define this to enable sending emails - Comment line out to disable sending emails (default setting = send no emails)

#define EMAIL_ADDR "?????????@gmail.com"  //Email to send to
#define USE_SMTP_PASSWORD                 //comment this out if your SMTP server doesnt need to logon before sending emails (eg. your ISP smtp server wont need login)
#define SMTP_USER "????????@gmx.com"           //SMTP account name to send from (www.gmx.com is a good option)
#define SMTP_PASS "??????????"       //SMTP account password to send from

//The Email server
//This is the IP for "smtp.gmx.com" a free smtp server you can use
// smtp.gmx.com = 74.208.5.1
#define SMTP_IP_A 74
#define SMTP_IP_B 208
#define SMTP_IP_C 5
#define SMTP_IP_D 1

#endif /* CONFIG_H_ */
