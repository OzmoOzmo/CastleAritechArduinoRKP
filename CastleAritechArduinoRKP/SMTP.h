/* 
* SMTP.h
*
* Created: 4/5/2014 7:45:36 PM

*/


#ifndef __SMTP_H__
#define __SMTP_H__

#include <EthernetClient.h>

class SMTP
{
//variables
public:
	static int nEmailStage;
protected:
private:
	static EthernetClient client;
	static unsigned long mTimeout;
	static boolean bWaitForResponse;
	static IPAddress mSMTPServerIP;
	
//functions
public:
	static void Init(IPAddress smptServer, const char* sEmail);
	static void QueueEmail();
	static void SendEmailProcess();
	static boolean WaitForReplyLine();

protected:
private:
	SMTP();
	SMTP( const SMTP &c );
	SMTP& operator=( const SMTP &c );
	
	boolean waitForReplyLine();
	
}; //SMTP

#endif //__SMTP_H__
