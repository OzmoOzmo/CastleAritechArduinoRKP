/*
 * WebSocket.h
 *
 * Created: 3/30/2014 11:54:30 PM Ozmo
 */

#ifndef WEBSOCKET_H_
#define WEBSOCKET_H_

#include <IPAddress.h>

class WebSocket
{
	public:
		// Create a Websocket server
		void static WebSocket_EtherInit(IPAddress ip, IPAddress gateway);

		//Send something to connected browser
		bool static WebSocket_send(char* data, byte length);

		void static WebSocket_doHandshake();

		byte static ReadNext();
		bool static RejectBroswerMsg();
		bool static WebSocket_getFrame();
		void static EtherPoll();
	private:
		void static SendHTMLSite();
};

#endif /* WEBSOCKET_H_ */
