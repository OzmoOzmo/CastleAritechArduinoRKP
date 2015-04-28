/* 
* SMTP.cpp
*
* Created: 4/5/2014 7:45:36 PM
* Author: Ambrose
*/

#include "limits.h"
#include "SMTP.h"
#include "Log.h"
#include "Config.h"


int SMTP::nEmailStage=-1;

#ifndef SENDEMAILS

// protected constructor
SMTP::SMTP(){}
boolean SMTP::WaitForReplyLine(){}
void SMTP::QueueEmail(){}
void SMTP::Init( IPAddress smptServerIP, const char* sEmail ){}
void SMTP::SendEmailProcess(){}

#else


//for IPAddresss
//#include "w5100.h"
//#include "Ethernet.h"
#include "IPAddress.h"


EthernetClient SMTP::client;
unsigned long SMTP::mTimeout = LONG_MAX;
boolean SMTP::bWaitForResponse = false;

IPAddress SMTP::mSMTPServerIP;
const char* msEmail = NULL;

// protected constructor
SMTP::SMTP()
{
}

void SMTP::Init( IPAddress smptServerIP, const char* sEmail )
{
	mSMTPServerIP = smptServerIP;
	msEmail = sEmail;
}

void SMTP::QueueEmail()
{
	if(msEmail != NULL)
	{
		LogLn(F("Sending Email."));
		nEmailStage=0;
	}
	else
		LogLn(F("Email Disabled."));
}

void SMTP::SendEmailProcess()
{
	if (msEmail == NULL)
		return;
	if(millis() > SMTP::mTimeout)
	{//we have taken longer than 10 secs to send our email - reset and resend
		LogLn(F("Email Reset."));
		mTimeout=LONG_MAX; //dont reset again
		nEmailStage=0;
		client.println(F("QUIT"));
	}

	if (SMTP::bWaitForResponse)
	{//We are awaiting a response
		if (SMTP::WaitForReplyLine())
			return;	//not yet - try next time
	}
	
	if (SMTP::nEmailStage==0)
	{//Connect
		SMTP::mTimeout = millis() + 10000; //10 sec to send...or will resend

		LogLn(F("--Start SendMail--"));
		
		Log(F("Cleaning buffers..{"));while(client.available()) Log(client.read()); LogLn("}");
		
		LogLn(F("Connecting..."));
		if(!client.connect(SMTP::mSMTPServerIP,25) /*|| !client.connected()*/)
		{
			LogLn(F("connection failed"));
			return;
		}
		LogLn(F("Confirmed Connected"));
		SMTP::bWaitForResponse=true;
		return;
	}
	
	if (nEmailStage==1)
	{
		LogLn(F("Sending helo"));
		SMTP::client.println("helo 1.2.3.4");
		SMTP::bWaitForResponse=true;
		return;
	}

	if (nEmailStage==2) //set to 20 to test retry
	{
		LogLn(F("From.."));	//(sender)
		SMTP::client.print("MAIL From: "); SMTP::client.println(msEmail);
		SMTP::bWaitForResponse=true;
		return;
	}

	if (nEmailStage==3)
	{//recipient address
		LogLn(F("To.."));
		SMTP::client.print("RCPT To: "); SMTP::client.println(msEmail);
		SMTP::bWaitForResponse=true;
		return;
	}

	if (nEmailStage==4)
	{
		LogLn(F("Sending DATA"));
		SMTP::client.println("DATA");
		SMTP::bWaitForResponse=true;
		return;
	}
	
	if (nEmailStage==5)
	{
		LogLn(F("Sending message"));
		SMTP::client.print("To: "); SMTP::client.println(msEmail);
		SMTP::client.print("From: TheHouse <"); SMTP::client.print(msEmail); SMTP::client.println(">");
		SMTP::client.println("Subject: House Calling. Alarm.\r\n");
	
		{
			SMTP::client.println("The House Alarm has gone off\r\n");
			SMTP::client.println("EndEmail");
		}
		
		SMTP::client.println(".");
		SMTP::bWaitForResponse=true;
		return;
	}

	if (nEmailStage==6)
	{
		SMTP::mTimeout = LONG_MAX; //cancel any timeout timer
		LogLn(F("Sending QUIT"));
		SMTP::client.println("QUIT");
		SMTP::bWaitForResponse=true;
		return;
	}
	
	if (nEmailStage==7)
	{
		SMTP::client.stop();
		SMTP::nEmailStage=-1;
		LogLn(F("disconnected"));
		return;
	}
}

//return False and inc stage if we get a response - true if we are waiting
boolean SMTP::WaitForReplyLine()
{
	if(client.connected() && client.available())
	{
		byte thisByte = client.read();
		//Log(thisByte);
		if (thisByte == '\n')
		{
			SMTP::nEmailStage++;
			SMTP::bWaitForResponse = false;
			return false;	//Good!
		}
	}
	return true;
}

#endif
