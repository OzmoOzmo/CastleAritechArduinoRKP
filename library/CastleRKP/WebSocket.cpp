/*
 * WebSocket.cpp - Websocket Implementation - works on most browsers and Mobile Phones (but notably Not on IE8)
 *
 * Created: 3/30/2014 9:57:39 PM
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

#include "sha1.h"
#include "Base64.h"
#include "RKP.h"	//for nKeyToSend
#include "LOG.h"
#include "websocket.h"

#ifdef AtmelStudio
  //Other modules
  #include <SPI.cpp>
  #include <Ethernet.cpp>
  #include <EthernetClient.cpp>
  #include <EthernetServer.cpp>
  #include <utility\socket.cpp>
  #include <utility\w5100.cpp>
#else
  #include <SPI.h>
  #include <Ethernet.h>
  #include <EthernetClient.h>
  #include <EthernetServer.h>
  #include <util.h>
#endif


//Workaround if needed for http://gcc.gnu.org/bugzilla/show_bug.cgi?id=34734
#ifdef PROGMEM
#undef PROGMEM
#define PROGMEM __attribute__((section(".progmem.data")))
#endif



//--Sockets
//use this as password - pick random port - set code tamper on alarm also.
EthernetServer ethServer(IP_P);
//EthernetServer ethServerHTML(80);
EthernetClient ethClient;


//"*" will be replaced with button
char htmlSite[] PROGMEM=
"<!DOCTYPE html PUBLIC '-//W3C//DTD XHTML 1.0 Transitional//EN'>"
"<html><head><title>Castle</title>"
"<meta name='viewport' content='width=320, initial-scale=1.2, user-scalable=no'>"
"<style>.long{height: 64px;} button{height: 35px;width: 35px;}</style>"
"<script src='http://goo.gl/m3GB3M' type='text/javascript'></script>"
"</head><body>"
"<div style='border: 5px solid black; width: 180px;'>&nbsp;<div id=msg1 style='float:left'></div><div id=msg2 style='float:right'></div></div>"
"<table>"
"<tr><td><button>1</button></td><td><button>2</button></td><td><button>3</button></td><td rowspan=2><button class=long>Y</button></td></tr>"
"<tr><td><button>4</button></td><td><button>5</button></td><td><button>6</button></td></tr>"
"<tr><td><button>7</button></td><td><button>8</button></td><td><button>9</button></td><td rowspan=2><button class=long>N</button></td></tr>"
"<tr><td><button>*</button></td><td><button>0</button></td><td><button>#</button></td></tr>"
"</table>"
"<script>var ws;$(document).ready(function(){"
"try{"
	"ws = new WebSocket('ws://'+location.hostname+':8383/sock');"
	"ws.onmessage = function (evt) {var d=evt.data.split('|');$('#msg1').text(d[0]);$('#msg2').text(d[1]);};"
	"ws.onerror = function (evt) {$('#msg').append('ERR:' + evt.data);};"
	"ws.onclose = function (evt) {$('#msg').text('Closed');};"
	"ws.onopen = function () { };"
"} catch (ex) {alert(ex.message);}"
"$(document).keypress(function (e) {ws.send(e.which);});"
"$(':button').click(function (e) { ws.send(e.target.innerText.charCodeAt(0));});"
"});</script></body></html>";


void WebSocket::EtherPoll()
{
	// Should be called for each loop.
	EthernetClient cli;
	if (cli = ethServer.available())
	{
		RKPClass::loop_PanelMon(); //most important we poll this or RKP will loose data
		//LogLn(F("browser req"));
		if (cli == true)
		{
			if (ethClient != cli)
			{//New connection
				//Secrisk
				ethClient = cli;
				WebSocket_doHandshake();
			}
			else
			{//Existing connection
				if (WebSocket_getFrame() == false)
				{//Request to end comms (rarely happens)
					RKPClass::loop_PanelMon(); //most important we poll this or RKP will loose data

					//Got bad frame, disconnect
					Log(F("Disconnect{"));
					while(ethClient.available()>0)
						ethClient.read();

					ethClient.flush();
					ethClient.stop();
				}
			}
		}
	}

/*	if (cli = ethServerHTML.available())
	{
		//Log(cli.available());
		if (cli == true)
			SendHTMLSite();
	}
*/
}

void WebSocket::SendHTMLSite(/*EthernetClient& cli*/)
{
	//Log(F("ConnectWWW{"));
	//	while(ethClient.available()>0)
	//	Log((char)cli.read());
	//LogLn(F("}"));
	ethClient.println(F("HTTP/1.0 200 OK"));	//
	ethClient.println(F("Content-Type: text/html"));
	ethClient.println(F("Connnection: close"));
	ethClient.println();
	char* p = &htmlSite[0];
	for(int n=0;n<2000;n++)
	{
		char c = pgm_read_byte(p++);
		if (c==0) break;
		//Can use tokens to compress the HTML a bit
		//if (c=='*')
		//	ethClient.print(F("button"));
		//else
		ethClient.print(c);
		if (n%100==0)
			RKPClass::loop_PanelMon(); //most important we poll this when doing other stuff or RKP will loose data
	}
	RKPClass::loop_PanelMon(); //most important we poll this or RKP will loose data

	// close the connection so browser gets data
	ethClient.flush();
	//delay(1);
	ethClient.stop();
	LogLn("Sent Page");
}


// Create a Websocket server
void WebSocket::WebSocket_EtherInit( IPAddress ip, IPAddress gateway )
{
	//IPAddress ip( 192, 168, 1 , 205);			//Give the device a unique IP
	//IPAddress gateway( 192, 168, 1, 1 );	//Gateway (the Router)

	IPAddress subnet( 255, 255, 255, 0 );	//typically dont need change
	// this sequence must be unique on your lan
	byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0x59, 0x67 };	//typically dont need change


	//Start Ethernet
	Ethernet.begin(mac, ip, gateway, gateway, subnet);

	ethServer.begin();
	//ethServerHTML.begin();
	Log(F("server is at ")); LogLn(Ethernet.localIP());

	delay(150); // Settle time
}

//Send something to connected browser
bool WebSocket::WebSocket_send(char* data, byte length)
{
	if (!ethClient.connected())
	{
		LogLn(F("No Client."));
		return false;
	}
	//int length = strlen(data);
	ethClient.write(0x81);// string type

	if (length > 125) {
		ethClient.write(126);
		ethClient.write((uint8_t) (length >> 8));
		ethClient.write((uint8_t) (length && 0xFF));
	}
	else
		ethClient.write((uint8_t) length);

	for (int i=0; i<length; ++i)
		ethClient.write(data[i]);

	//LogLn(F("Sent OK."));
	return true;
}

void WebSocket::WebSocket_doHandshake()
{
	LogLn("HS");
	char htmlline[128];	//TODO: there are 3 buffers used - htmlline, key and sha - maybe could merge them
	char key[80];

	bool hasKey = false;
	bool bReqWebPage = false;

	byte counter = 0;

	while(ethClient.available()>0)
	{//Read each line
		byte bite = ethClient.read();
		//Log(bite);
		htmlline[counter++] = bite;
		if (counter > 127)
		{
			LogLn(F("HandShake Overflow{"));
			while(ethClient.available()>0)
				LogHex(ethClient.read());
			LogLn(F("}"));
			//htmlline[127] = 0;
			//LogHex(htmlline);
			counter=0;
			continue;
		}

		if (bite == '\n')
		{ // Parse the line
			htmlline[counter - 2] = 0; // Terminate string before CRLF

			bool bFound = (strstr_P(htmlline, PSTR("Sec-WebSocket-Key:")) != NULL);
			if (bFound)
			{
				hasKey = true;
				strtok(htmlline, " ");
				strcpy(key, strtok(NULL, " "));	//TODO: Not safe - need specify max 80 limit
			}

			if (strstr_P(htmlline, PSTR("GET / HTTP")) != NULL)
			{
				LogLn(F("Page Req"));
				bReqWebPage=true;
			}

			counter = 0; // Start saving new header string

			//Each Line - check if there is any comms to do
			RKPClass::loop_PanelMon(); //most important we poll this or RKP will loose data
		}
	}

	//Log("Got header: ");LogHex((byte*)htmlline,counter);

	if (hasKey)
	{
		LogLn(F("WS Req"));
		strcat_P(key, PSTR("258EAFA5-E914-47DA-95CA-C5AB0DC85B11")); // Magic Number GUID
		Sha1.init();
		Sha1.print(key);
		uint8_t* hash = Sha1.result();
		base64_encode(htmlline, (char*)hash, 20); //reuse htmlline buffer
		ethClient.print(F("HTTP/1.1 101 Switching Protocols\r\n"));
		ethClient.print(F("Upgrade: websocket\r\n"));
		ethClient.print(F("Connection: Upgrade\r\n"));
		ethClient.print(F("Sec-WebSocket-Accept: "));
		ethClient.print(htmlline);  //eg. VoNhf1LMVVTziHWxjiajVem5DB4=
		ethClient.print(F("\r\n\r\n"));

		LogLn(F("Connected"));

		//Send any display we might have
		RKPClass::bScreenHasUpdated = true; //RKPClass::SendDisplayToBrowser();
	}
	else if (bReqWebPage)
	{// Nope, Not a websocket request - send the main webpage
		//LogLn(F("WebPageReq"));
		SendHTMLSite();
	}
	else
	{
		ethClient.println(F("HTTP/1.0 404 File Not Found"));	//
		ethClient.println(F("Content-Type: text/html"));
		ethClient.println(F("Connnection: close"));
		ethClient.println();
		//delay(1);
		ethClient.stop();
	}
	return;
}

struct Frame {
	bool isMasked;
	bool isFinal;
	byte opcode;
	byte mask[4];
	byte length;
	char data[64+1];
} frame;

byte WebSocket::ReadNext()
{
	byte bite = ethClient.read();
	//LogHex(bite);
	return bite;
}

bool WebSocket::WebSocket_getFrame()
{
	// Get opcode
	byte bite = ReadNext(); if (bite==0xFF) return false;

	frame.opcode = bite & 0xf; // Opcode
	frame.isFinal = bite & 0x80; // Final frame?
	// Determine length (only accept <= 64 for now)
	bite = ReadNext(); if (bite==0xFF) return false;

	frame.length = bite & 0x7f; // Length of payload
	if (frame.length >= 64)
	{//Unlikely to happen
		Log(F("Frame Too Big")); LogLn(bite);
		return false;
	}
	// Client should always send mask, but check just to be sure
	frame.isMasked = bite & 0x80;
	if (frame.isMasked) {
		frame.mask[0] = ReadNext();
		frame.mask[1] = ReadNext();
		frame.mask[2] = ReadNext();
		frame.mask[3] = ReadNext();
	}

	// Get message bytes and unmask them if necessary
	int i = 0;
	for (; i < frame.length; i++)
	{
		bite = ReadNext();
		if (frame.isMasked)
			frame.data[i] = bite ^ frame.mask[i % 4];
		else
			frame.data[i] = bite;
	}
	frame.data[i]=0;

	// Frame complete!
	if (!frame.isFinal)
	{	// We don't handle fragments! Close and disconnect.
		LogLn(F("Unsurp"));
		return false; //RejectBroswerMsg();
	}

	if (frame.opcode== 0x01)
	{// Txt frame
		Log(F("Data: ")); //Got Data
		LogLn(frame.data);
		RKPClass::PushKey(atoi(frame.data));
		return true;
	}

	if (frame.opcode== 0x08)
	{
		// Close frame. Answer with close and terminate tcp connection
		LogLn(F("Close")); //Close frame
		ethClient.write((uint8_t) 0x08);
		return false;
	}

	// Unexpected. Ignore?
	LogLn(F("Ex.")); //Unhandled frame ignored
	return false;
}


