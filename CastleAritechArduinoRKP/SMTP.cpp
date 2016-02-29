/*
  SMTP.cpp

  Created: 4/5/2014 7:45:36 PM
  Author: Ambrose

  SENDEMAILS defined in config.h determines if emails will be sent
*/

#include "limits.h"
#include "SMTP.h"
#include "Log.h"
#include "Config.h"
#include "WebSocket.h" //for the base64 stuff

int SMTP::nEmailStage = -1;

#ifndef SENDEMAILS

// protected constructor
SMTP::SMTP(){}
boolean SMTP::WaitForReplyLine(){}
void SMTP::QueueEmail(MSG a){}
void SMTP::Init(){}
void SMTP::SendEmailProcess(){}

#else

EthernetClient SMTP::client;
unsigned long SMTP::mTimeout = LONG_MAX;
boolean SMTP::bWaitForResponse = false;
unsigned long SMTP::mStartDelay = 0;
MSG SMTP::nMsgToSend = NA;

// protected constructor
SMTP::SMTP()
{
}

void SMTP::Init()
{
}

void SMTP::QueueEmail(MSG msgToSend)
{
  LogLn(F("Sending Email."));
  SMTP::nMsgToSend = msgToSend;
  nEmailStage = 0;
}

void SMTP::SendEmailProcess()
{
  unsigned long now = millis();
  if (now > SMTP::mTimeout)
  { //we have taken longer than 10 secs to send our email - reset and resend
    LogLn(F("Email Reset."));
    SMTP::mTimeout = LONG_MAX; //dont reset again
    nEmailStage = 0;
    client.println(F("QUIT"));
  }

  if (SMTP::mStartDelay != 0)
  {
    if (now < SMTP::mStartDelay)
      return;//after error wait 5 secs before trying again
    SMTP::mStartDelay = 0; //delay up;
  }

  if (SMTP::bWaitForResponse)
  { //We are awaiting a response
    if (SMTP::WaitForReplyLine())
      return;	//not yet - try next time
  }

  if (SMTP::nEmailStage == 0)
  { //Connect
    LogLn(F("--Start SendMail--"));
    SMTP::mTimeout = millis() + 20000; //20 sec to send...or will resend
    LogLn(F("Connecting..."));
    static byte server[] = {SMTP_IP_A, SMTP_IP_B, SMTP_IP_C, SMTP_IP_D};
    int r = SMTP::client.connect(server, 25);
    if (r != 1)
    {
      Log(F("connection failed.")); LogLn(r);
      SMTP::mStartDelay = millis() + 5000; //5 secs before we try again
      return;
    }
    LogLn(F("Confirmed Connected"));
    SMTP::bWaitForResponse = true;
    return;
  }

  if (nEmailStage == 1)
  {
    LogLn(F("Sending helo"));
#ifdef USE_SMTP_PASSWORD
    SMTP::client.println(F("ehlo 1.2.3.4"));  //use ehlo if password needs be sent
#else
    SMTP::client.println(F("helo 1.2.3.4"));  //use helo if no password needs be sent
#endif
    SMTP::bWaitForResponse = true;
    return;
  }

  //Stages 2 to 4 are the login with username and password stages
  if (nEmailStage == 2)
  {
    LogLn(F("Auth.."));
    SMTP::client.println(F("AUTH LOGIN"));
    SMTP::bWaitForResponse = true;
    return;
  }
  if (nEmailStage == 3)
  {
    LogLn(F("User.."));
    const char* pUSER = SMTP_USER;
    WebSocket::base64_encode(WebSocket::htmlline, (char*)pUSER, strlen(pUSER)); //reuse htmlline buffer
    SMTP::client.println(WebSocket::htmlline);	// insert base64 encoded username
    SMTP::bWaitForResponse = true;
    return;
  }
  if (nEmailStage == 4)
  {
    LogLn(F("Pswd.."));
    const char* pPASS = SMTP_PASS;
    WebSocket::base64_encode(WebSocket::htmlline, (char*)pPASS, strlen(pPASS)); //reuse htmlline buffer
    SMTP::client.println(WebSocket::htmlline);	// insert base64 encoded username
    SMTP::bWaitForResponse = true;
    return;
  }

  //Stages 5 onwards send the email
  if (nEmailStage == 5) //set to 20 to test retry
  {
    LogLn(F("From.."));	//(sender)
    SMTP::client.print("MAIL From: "); SMTP::client.println(F(SMTP_USER));
    SMTP::bWaitForResponse = true;
    return;
  }

  if (nEmailStage == 6)
  { //recipient address
    LogLn(F("To.."));
    SMTP::client.print("RCPT To: "); SMTP::client.println(F(EMAIL_ADDR));
    SMTP::bWaitForResponse = true;
    return;
  }

  if (nEmailStage == 7)
  {
    LogLn(F("Sending DATA"));
    SMTP::client.println("DATA");
    SMTP::bWaitForResponse = true;
    return;
  }

  if (nEmailStage == 8)
  {
    LogLn(F("Sending message"));
    SMTP::client.print("To: "); SMTP::client.println(F(EMAIL_ADDR));
    SMTP::client.print("From: TheHouse <"); SMTP::client.print(F(SMTP_USER)); SMTP::client.println(">");
    SMTP::client.println("Subject: House Calling. Alarm.\r\n");

    if (SMTP::nMsgToSend == START)
      SMTP::client.println("The House Alarm has just started\r\n");
    else if (SMTP::nMsgToSend == ALARM)
      SMTP::client.println("The House Alarm has gone off\r\n");
    SMTP::client.println("EndEmail");

    SMTP::client.println(".");
    SMTP::bWaitForResponse = true;
    return;
  }

  if (nEmailStage == 9)
  {
    SMTP::mTimeout = LONG_MAX; //cancel any timeout timer
    LogLn(F("Sending QUIT"));
    SMTP::client.println("QUIT");
    SMTP::bWaitForResponse = true;
    return;
  }

  if (nEmailStage == 10)
  {
    SMTP::client.stop();
    SMTP::nEmailStage = -1;
    LogLn(F("disconnected"));
    return;
  }
}

//return False and inc stage if we get a response - true if we are waiting
boolean SMTP::WaitForReplyLine()
{
  if (client.connected() && client.available())
  {
    byte thisByte = client.read();
    Log((char)thisByte);
    if (thisByte == '\n')
    {
      SMTP::nEmailStage++;
#ifndef USE_SMTP_PASSWORD
      if (SMTP::nEmailStage == 2) SMTP::nEmailStage = 5; //skip password login
#endif
      SMTP::bWaitForResponse = false;
      return false;	//Good!
    }
  }
  return true;
}

#endif
