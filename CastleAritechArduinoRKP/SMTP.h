/* 
* SMTP.h
*
* Created: 4/5/2014 7:45:36 PM
*/


#ifndef __SMTP_H__
#define __SMTP_H__

#include <EthernetClient.h>

enum MSG{NA,START,ALARM,WARNING};

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
	static MSG nMsgToSend;
	static unsigned long mStartDelay;

//functions
public:
	static void Init();
	static void QueueEmail(MSG msgToSend);
	static void SendEmailProcess();
	static boolean WaitForReplyLine();
        
protected:
private:
	SMTP();
}; //SMTP

#endif //__SMTP_H__
