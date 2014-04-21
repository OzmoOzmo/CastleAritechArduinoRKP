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


//This is the Keypad id of the Arduinio - recommended 1 or 2 (0 is first keypad)
#define RKP_ID 2

//Set to 1 for normal operation - comment out will debug print logs - but might slow down the arduino too much for some panels
#define QUIET 1




#define sEmail "ozmo@example.com"	//change this to your email address - Or comment out to Not send emails at all



//---To Send emails You NEED get the IP address of the SMTP server from the list below that matches your Internet Provider
//No DNS lookup as Ive removed that from my libs to save space...

//"smtp.mysmart.ie"							Smart Telecom Outgoing SMTP Server
//"smtp.irishbroadband.ie."					Irish Broadband Outgoing SMTP Server
//mail1.eircom.net OR mail2.eircom.net		Eircom Outgoing SMTP Server
//smtp.magnet.ie							Magnet Outgoing SMTP Server
//smtp.upcmail.ie							NTL and UPC Outgoing SMTP Server
//mail.icecomms.net							Ice Broadband Outgoing SMTP Server
//mail.vodafone.ie							Vodafone Outgoing SMTP Server
//smtp.o2.ie								O2 Outgoing SMTP Server
//smtp.clearwire.ie / mail.clearwire.ie		Clearwire Outgoing SMTP Server
//smtp.digiweb.ie							Digiweb Outgoing SMTP Server
//mail.imagine.ie OR mail.gaelic.ie			Imagine Broadband Outgoing SMTP Server
//mail.perlico.ie							Perlico Outgoing SMTP Server
//mail-relay.3ireland.ie					3 Outgoing SMTP Server: Mobile broadband with 3 mobile Ireland

//eg. this is the IP for "smtp.upcmail.ie" (will only work if you are a UPC customer)
#define SMTP_IP_A 213
#define SMTP_IP_B 46
#define SMTP_IP_C 255
#define SMTP_IP_D 2


#endif /* CONFIG_H_ */
