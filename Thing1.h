/* THING1 WiFi Module Library 
White Systems ApS - http://www.white-systems.dk/wifi/ - info@white-systems.com 

License: Released under the Creative Commons Attribution Share-Alike 3.0 License. http://creativecommons.org/licenses/by-sa/3.0/

Target:  Arduino 
*/
#ifndef thing1_h
#define thing1_h

#include <inttypes.h>
#include <string.h>

#include "Arduino.h"

#ifdef _SAM3XA_ 			// Arduino Due
	#include "USARTClass.h"
#define SERIAL_CLASS USARTClass
#else						// Rest
  #include "HardwareSerial.h"
#define SERIAL_CLASS HardwareSerial
#endif

#define MAX_SOCK_NUM 1
#define RCV_BUF_LEN 127

#define IpConfigC               	0x00
#define WlanConnectedC          	0x01
#define WlanDisconnectedC        	0x02
#define TcpCliRcvC               	0x03 
#define TcpSrvRcvC              	0x04 
#define UdpCliRcvC               	0x05  
#define UdpSrvTcvC               	0x06
#define TcpConnectedC               0x07  

extern "C" {
	/* general callback */
    typedef void (*callbackFunction)(byte event, char* data); 
	/* client connected callback - IP, Port, Handle */
	typedef void (*ConnectDisconnectcallbackFunction)(byte event, char* ip , char* port, char* handle); 
	/* Incoming TCP data callback - Handle, Message */
	typedef void (*tcpCliRcvCallbackFunction)(byte event, char* handle, char* message); 
 }

const char ESC = 0x1B;
const char CR  = '\r'; // 0x0D
const char LF  = '\n'; // 0x0A
const char CRLF[] = "\r\n";
const char READY[] = "READY\r\n";
const char OK[] = "OK\r\n";

#if 0
const char pszCommand[][] =
{
	"connect",
	"disconnect",
	"reset",
	"smartconfig",
	"state"
};

const char pzResponse[][] =
{
	"OK",
	"ERR",
	"TIMEOUT+ERR"
};

const char pzUnsolicitedEvents[][] =
{
	"WLAN_CONNECTED",
	"IP_CONFIG",
	"WLAN_DISCONNECTED",
	"SMARTCONFIG"
};
#endif

class Thing1Class {
private:
	enum State {
		OFF,
		STARTUP,
		CONNECTED,
		DISCONNECTING,
		DISCONNECTED,
		SMARTCONFIG
	};

	enum Event {
		Init,
		Ready,
		Ok,
		Err,
		TimeoutErr,
		SwitchOn,
		WlanConnected,
		WlanDisconnected,
		IpConfig,
		TcpCliRcv,
		TcpSrvRcv,
		TcpConnected,
		UdpCliRcv,
		UdpSrvTcv,
		UnknownInput
	};


	enum Action {
		BeginConnect,
		GreenLedOn,
		GreenLedOff,
		RedLedOn,
		RedLedOff,
		Disconnect,
		SendSmartConfig
	};

	enum LineState {
		LineUninitialized,
		LineIdle,
		LineBusy,
		LineCmd,
		LineCmdArg,
		LineRspArg,
		LineRspRC,
		LineRspData,
		LineEvt,
		LineEvtArg,
		LineEvtData
	};

	enum LineEvent {
		lineCR,
		lineLF,
		lineESC,
		lineData
	};

	uint8_t _powerPin;
	long _bitsPerSeconds;
	SERIAL_CLASS* _serial;
	uint32_t _count;
	char rcvBuf[RCV_BUF_LEN+1];
	State _currState;
	LineState _currInLineState;
	uint8_t _handle[MAX_SOCK_NUM];

	void connectCmd();     		/* Connect to Wifi if configured */	
	void smartconfigCmd();      /* Do SmartConfig - No key specified for now!!! */
	void disconnectCmd();       /* Disconnect from Wifi if connected */
	void sendCmd(const char* command);
	int write(const char* data, uint32_t len);
	int writeCRLF();
	int readUntil(char cStop, char* buf, uint32_t buflen);
	int readline(char* buf, uint32_t buflen);
	void waitFor(const char* s);
	void ipConfig();
	bool startsWith (char* base, char* str);
	void remove_all_chars(char* str, char c);
	
	callbackFunction ipConfigCallback;
	callbackFunction wlanDisconnectCallback;
	callbackFunction wlanConnectedCallback;
	callbackFunction tcpSrvRcvCallback;
	callbackFunction udpCliRcvCallback;
	callbackFunction udpSrvTcvCallback;
	ConnectDisconnectcallbackFunction TcpConnectedCallback;
	tcpCliRcvCallbackFunction tcpCliRcvCallback;

public:
	enum LedCmd {
		RedOn,
		RedOff,
		GreenOn,
		GreenOff
	};

	Thing1Class();
	void begin(uint8_t thingPowerPin, HardwareSerial* thingSerial, long _bitsPerSeconds);
	void loop();
	void reset();
	void setLed(LedCmd ledcmd);
	void frimwareVersion();		//Retrieve firmware Version from THING1 
	void startTCPServer(unsigned short port);
	void stopTCPServer();
	void startUDPServer(unsigned short port);
	void stopUDPServer();
	void attach(byte command, callbackFunction newFunction);
	void attach(byte command, ConnectDisconnectcallbackFunction newFunction);
	void attach(byte command, tcpCliRcvCallbackFunction newFunction);
	void createTCPClient(char* ip, unsigned short port);
	void sendingTCPData(char* handle, const char* message);
	void stateCmd();            // Request module state (WLAN_CONNECTED or WLAN_DISCONNECTED)	

	friend class Thing1Client;
};

extern Thing1Class Thing1;

#endif
