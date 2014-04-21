/*
 * RKP.cpp - All the Remote Keypad comms and display commands
 *
 * Created: 3/30/2014 10:04:10 PM
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

//change network settings to yours
#include <Arduino.h>
#include "RKP.h"
#include "LOG.h"
#include "Websocket.h"
#include "SMTP.h"
#include "Config.h"


//Workaround if needed for http://gcc.gnu.org/bugzilla/show_bug.cgi?id=34734
#ifdef PROGMEM
#undef PROGMEM
#define PROGMEM __attribute__((section(".progmem.data")))
#endif

#define nSerialBaudKP_RX 1953
#define ixMaxPanel 40	//40 bytes enough


const prog_uchar allmonths[] = {"JANFEBMARAPRMAYJUNJULAUGSEPOCTNOVDEC"};
const prog_uchar alldays[] = {"SUNMONTUEWEDTHUFRISAT"};
	
String RKPClass::msKeyToSend="";
bool RKPClass::dateFlash=true;
bool RKPClass::mbIsPanelWarning=false;
bool RKPClass::mbIsPanelAlarm=false;

#define DISP_BUF_LEN 16+1+2		//16 characters - space - AW (will be followed by a Terminator 0)
byte RKPClass::dispBuffer[DISP_BUF_LEN+1];


//const char msgOff[] PROGMEM = "KeyPad OFF!";
//const char msgOn[] PROGMEM = "KeyPad ON!";



RKPClass::RKPClass()
{
	dispBuffer[16]='|';
	dispBuffer[DISP_BUF_LEN]=0;
}

void RKPClass::loop_PanelMon(void)
{
	static int msgbufLen = 0;
	static byte msgbuf[ixMaxPanel];
	static bool bReceivingPacketNow = false;
	
	static byte lastByte=-1;

	//Note - nothing here is to be blocking 
	//- so sending emails must not block receiving serial logs
	
	//Software UART not great at receiving for some reason :/ - ok for sending.
	if (Serial1.available()==0)
		return;

	byte rx = Serial1.read();
	//Log((char)rx);
	
	if (bReceivingPacketNow == false)
	{
		if(lastByte == 0)
		{//last char was a zero - this might be our header...
			//It may be for us or not - but get whole message before testing so we stay in sync
			bReceivingPacketNow = true;
			msgbufLen=0;
			//not necessary to balnk - but good for debug
			for(int n=0;n<ixMaxPanel;n++)
				msgbuf[n]=0xFF;
			msgbuf[msgbufLen++]=rx;
		}
		lastByte = rx;
		return; //wait for byte#3 (0,id,..)
	}
	lastByte = rx;	//needed in case cs error
	
	msgbuf[msgbufLen++]=rx;
	if (rx!=0)
	{//wasn't end of packet - is buffer full?
		if (msgbufLen>=ixMaxPanel)
		{//packet never terminated - bytes lost :(
			Log("Buf overflow");
			bReceivingPacketNow = false;
			return;
		}
	}
	else
	{//End of the packet
		bReceivingPacketNow = false;

		byte idDev = (msgbuf[0] & 0xf0)>>4; //ID of remote that message is for 0x0-0xf are valid ids
		
		//Uncomment to Display all packets
		//LogHex(msgbuf,msgbufLen);Log(F("DevID:"));LogLn(idDev);

		byte cs = 0;
		for(int n=0;n<msgbufLen;n++)
		{
			byte rx = msgbuf[n];
			if (n<msgbufLen-2) //dont sum cs or terminator
				cs+=rx;
			//Log((char)rx);
		}
		if (cs != msgbuf[msgbufLen-2])
		{
			Log(F("CS Fail :( "));
			//LogHex(msgbuf,msgbufLen);
			Log(F("Dev:"));LogLn(idDev);
			return;
		}
		//LogLn(F(""));

		lastByte=-1; //good message - so waiting again for next zero

		{//Send Email if alarm lights come on
			bool bIsPanelWarning = (msgbuf[1] & 0x04) != 0;
			if (bIsPanelWarning == true && RKPClass::mbIsPanelWarning == false)
				SMTP::QueueEmail();
			RKPClass::mbIsPanelWarning = bIsPanelWarning;
			
			bool bIsPanelAlarm = (msgbuf[1] & 0x02) != 0;
			if (bIsPanelAlarm == true && RKPClass::mbIsPanelAlarm == false)
				SMTP::QueueEmail();
			RKPClass::mbIsPanelAlarm = bIsPanelAlarm;
		}
		
		//for us?
		if (idDev==RKP_ID)
		{
			DisplayScreen(msgbuf, msgbufLen); //try display what device 0 sees
			bool bAck = (msgbuf[0] & 0x04) != 0;	//required for good comms
			SendToPanel(RKP_ID, bAck);
			LogLn("-OK-");
		}
		else
		{
			//LogLn("-Not for us-");
		}
	}
	
}

void RKPClass::DisplayScreen( byte* msgbuf, int msgbufLen )
{
	byte bufix=0;
	//Force Screen clear each time
	//for(int m=0;m<DISP_BUF_LEN;m++)
	//	dispBuffer[m]='!';
		
	//Checksum so can see if changes and need update client
	static int previousCS =-1;
					
	int ixMsgbuf=2;//skip 2 header bytes
	msgbufLen-=2; //remove checksum and zero terminator

	while(ixMsgbuf<msgbufLen)
	{
		byte rx = msgbuf[ixMsgbuf++];
		if (rx>=0 && rx < 0x0f)
		{//Switch language - not implemented - ignore this command
		}
		else if (rx == 0x13)
		{//means will be no more text (but can be commands) - so do nothing?
		}
		else if (rx == 0x1b)
		{// 0x1b=Next character is a Non-Eng characters - not implemented - ignore this command
		}
		else if (rx>= 0x20 && rx <= 0x7F)
		{//Normal ASCII
			if (bufix < DISP_BUF_LEN)
				dispBuffer[bufix++]=(char)rx;
		}
		else if (rx>= 0x80 && rx <= 0x8F)
		{//Date in encoded format
			int b0=rx;
			int b1=msgbuf[ixMsgbuf++];
			int b2=msgbuf[ixMsgbuf++];
			int b3=msgbuf[ixMsgbuf++];
			
			byte nMonth= (b0 & 0x0f)-1;
			byte day = (b1 & (128+64+32))>> 5;
			byte date = (b1 & (31));
			byte h1=(b2 & 0xf0)>>4; if(h1==0x0A) h1=0;
			byte h2=(b2 & 0x0f); if(h2==0x0A) h2=0;
			byte m1=(b3 & 0xf0)>>4; if(m1==0x0A) m1=0;
			byte m2=(b3 & 0x0f); if(m2==0x0A) m2=0;
			
			memcpy_P(dispBuffer+0,alldays+(day*3),3);
			dispBuffer[3]=' ';
			dispBuffer[4]=('0'+(int)(date/10));
			dispBuffer[5]=('0'+(date%10));
			dispBuffer[6]=' ';
			
			memcpy_P(dispBuffer+7,allmonths+(nMonth*3),3);
			dispBuffer[10]=' ';
			dispBuffer[11]='0'+h1;
			dispBuffer[12]='0'+h2;
			//if (dateFlash)
			dispBuffer[13]= ':';
			//else
			//	buffer[13]= F(' ');
			//dateFlash=!dateFlash;
			//buffer[13]= ((millis()/500)&1) ==0? ':':' ';
			dispBuffer[14]='0'+m1;
			dispBuffer[15]='0'+m2;
			bufix=0;
		}
		else if (rx == 0x90)
		{//CLS
			bufix=0; 
			for(int m=0;m<DISP_BUF_LEN;m++)
				dispBuffer[m]=' ';
		}
		else if (rx == 0x91)
		{//HOME Maybe??
			bufix=0;
		}
		else if (rx >= 0xA0 && rx <= 0xAf)
		{//MOVE cursor to position x
			bufix = (rx & 0x0f); //-1 gives us 2 *'s  but without -1 we go off screen at Login ***
		}
		else if (rx >= 0xB0 && rx <= 0xBF)
		{//{BLINK_N}"	Bxh Blink x chars starting at current cursor position
			//int nChars = (rx & 0x0f)-1;
			//if (dateFlash)
			//	buffer[i]= ':';
			//else
			//	buffer[i]= ' ';
			//dateFlash=!dateFlash;
		}
		else if (rx >= 0xC0 && rx <= 0xCf)
		{// Set position to x and clear all chars to right
			int i = (rx & 0x0f);
			if (i < DISP_BUF_LEN)
				bufix = i;
			for(int n=bufix;n<DISP_BUF_LEN;n++)
				dispBuffer[bufix++]=' ';
		}
		else if (rx>= 0xE0 && rx <= 0xFF)
		{// Special Characters Arrows and foreign chars
			int i = (rx & 0x0f);
							
			char c=0;
			if (i==4)	c= '*';
			else if (i==5)	c= '#';
			else if (i==7)	c= '>';
							
			if (c>0)
				if (bufix < DISP_BUF_LEN)
					dispBuffer[bufix++]=c;
		}
		else
		{//unknown command
			Log(F("{"));LogHex(rx);Log(F("}"));
		}
		
		//TODO: there are quite a few codes in Engineer menu to deal with flashing cursors and characters - cannot do easily in html
		//might look at later.
	}

	dispBuffer[16]='|';	//this may overwrite a char sometimes...
	dispBuffer[17]=(RKPClass::mbIsPanelAlarm)?'A':' ';
	dispBuffer[18]=(RKPClass::mbIsPanelWarning)?'W':' ';
				
	int cs=0;
	for(int n=0;n<DISP_BUF_LEN;n++)
		cs+=(char)dispBuffer[n];

	if (cs != previousCS)
	{
		////Display as hex
		LogHex(msgbuf, msgbufLen);
		////Display as characters
		for(int n=0;n<DISP_BUF_LEN;n++)	Log((char)dispBuffer[n]); LogLn(".");
			
		previousCS=cs;
		SendDisplayToBrowser();
	}
					
}

char RKPClass::NextKeyPress()
{
	if (RKPClass::msKeyToSend.length() == 0)
		return -1; //no keys
	char next = RKPClass::msKeyToSend[0];
	return next;
}

char RKPClass::PopKeyPress()
{
	char next = NextKeyPress();
	if (next!=0xFF)//RKPClass::mnKeyToSend=0xff;
		RKPClass::msKeyToSend = RKPClass::msKeyToSend.substring(1);
	return next;
}

void RKPClass::PushKey( char key )
{
	if (RKPClass::msKeyToSend.length()<10)
		RKPClass::msKeyToSend += key;
}

void RKPClass::SendToPanel( int id, bool bAck )
{
	//Log(F("Send: "));
	#define H1_LIDTAMP 1
	#define H1_BASE 2 //always set
	#define H1_CD3008 0 //keypad with buzzer
	#define H1_CD9038 4 //Keypad with 4 zones
	#define H1_CD9041 8 //Glassbreak Expander with no keypad - works on cd34 but not cd72
	static byte nH2Previous=-1;

	byte h1 = H1_BASE + H1_CD3008 + (id<<4); //kKKK (Capital K is an Extender enabled keypad)
	byte h2 = 0;

	if (bAck)
	//if (true)	//TODO: put back
	{
		//LogLn("Ack");
		char nKeyToSend = PopKeyPress();
		if (nKeyToSend!=0xff)
		{
			//nKeyToSend &= 0x20; //tolower
			int nNumb=0x0e;
			if (nKeyToSend >='1' && nKeyToSend <= '9')
			nNumb = (nKeyToSend-'0');
			else if (nKeyToSend == '0')	nNumb = 0x0a;
			else if (nKeyToSend == 'f'||nKeyToSend == '*')	nNumb = 0x0b;	//UP  (* for IPhone)
			else if (nKeyToSend == 'v'||nKeyToSend == '#')	nNumb = 0x0c;	//DOWN (# for IPhone)
			else if (nKeyToSend == 'p')	nNumb = 0x0d;	//UP + DOWN (Panic)
			else if (nKeyToSend == 'x'||nKeyToSend == ';'||nKeyToSend == 'n'||nKeyToSend == 'N')	nNumb = 0x0e;	//UP + 0 or X(reject) (WAIT on IPhone numpad)
			else if (nKeyToSend == 13||nKeyToSend == '+'||nKeyToSend == 'y'||nKeyToSend == 'Y')	nNumb = 0x0f;	//UP + 0 or *(accept) (+ on IPhone numpad)

			Log(F("Sent: ")); LogHex(nNumb);LogLn(F("."));

			h2 = nNumb<<4;
		}
		nH2Previous = h2;
	}
	else
	{
		Log(F("Resend: ")); LogHex(nH2Previous>>4);LogLn(F("!"));
		h2 = nH2Previous;
	}

	byte h3=0; byte h4=(byte)(h1+h2+h3);

	Serial1.write((byte)(h1));
	Serial1.write((byte)(h2));
	Serial1.write((byte)(h3));
	Serial1.write((byte)(h4));
	//LogHex(h1); LogHex(h2); LogHex(h3); LogHex(h4); LogLn(".");
}

void RKPClass::Init()
{
	Serial1.begin(nSerialBaudKP_RX, SERIAL_8N2);	//RKP Comms
}

void RKPClass::SendDisplayToBrowser()
{
	WebSocket::WebSocket_send((char*)dispBuffer,DISP_BUF_LEN);
}
