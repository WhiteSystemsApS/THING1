/* THING1 WiFi Module Library 
White Systems ApS - http://www.white-systems.dk/wifi/ - info@white-systems.com 

License: Released under the Creative Commons Attribution Share-Alike 3.0 License. http://creativecommons.org/licenses/by-sa/3.0/

Target:  Arduino 
*/
#include "Thing1.h"

/* Public functions */
Thing1Class::Thing1Class()
{
	_currState = OFF;
	_currInLineState = LineUninitialized;
}

void Thing1Class::begin(uint8_t thingPowerPin, HardwareSerial* thingSerial, long bitsPerSeconds)
{
	_powerPin = thingPowerPin;
	_bitsPerSeconds = bitsPerSeconds;
	_serial = static_cast<SERIAL_CLASS*>(thingSerial);
	memset(_handle,-1,sizeof(_handle));
	
	Serial.write('\r'); Serial.write('\n'); Serial.write('^'); Serial.write('\r'); Serial.write('\n');
	reset();
	connectCmd();
	Serial.write('$'); Serial.write('\r'); Serial.write('\n');
	Serial.flush();
	_currState = STARTUP;
}

void Thing1Class::loop()
{
	Event event;

	if (_serial->available()>0)
	{
		readline(rcvBuf,RCV_BUF_LEN); /* Read Event */
		
		if (strcmp("OK",rcvBuf) == 0)
		{
			event = Ok;
		}
		else if (strcmp("ERR",rcvBuf) == 0)
		{
			event = Err;
		}
		else if (strcmp("TIMEOUT+ERR",rcvBuf) == 0)
		{
			event = TimeoutErr;
		}
		else if (strcmp("SWITCH_ON",rcvBuf) == 0)
		{
			event = SwitchOn;
		}
		else if (strcmp("WLAN_CONNECTED",rcvBuf) == 0)
		{
			event = WlanConnected;
		}
		else if (strcmp("WLAN_DISCONNECTED",rcvBuf) == 0)
		{
			event = WlanDisconnected;
		}
		else if (strcmp("IP_CONFIG",rcvBuf) == 0)
		{
			event = IpConfig;
		}
		else if (memcmp("tcp:cli:connected",rcvBuf,17) == 0)
		{
			event = TcpConnected;
		}
		else if (memcmp("tcp:cli:rcv",rcvBuf,11) == 0)
		{
			event = TcpCliRcv;
		}
		else
		{
			event = UnknownInput;
		}

		switch (event)
		{
		case WlanConnected:
			break;
		case TimeoutErr:
			setLed(GreenOff);
			setLed(RedOn);
			_currState = DISCONNECTED;
			break;
		case WlanDisconnected:
			_currState = DISCONNECTED;
			setLed(GreenOff);
			setLed(RedOn);			
			break;
		case IpConfig:
			_currState = CONNECTED;
			setLed(RedOff);
			setLed(GreenOn);
			if(ipConfigCallback) 
			{
				(*ipConfigCallback)(IpConfigC, rcvBuf);
			}
			break;
		case TcpCliRcv:
			if(tcpCliRcvCallback) 
			{
				/* tcp:cli:rcv <Handle> ”<Message>” */
				strtok (rcvBuf," "); //cmd
				char * phandle = strtok(NULL," "); // handle
				char * pmessage = strtok(NULL," "); // received message
				
				remove_all_chars(pmessage, '"');

				(*tcpCliRcvCallback)(TcpCliRcvC, phandle, pmessage);
			}
			break;
			
		case TcpConnected:
			if(TcpConnectedCallback) 
			{
				/* tcp:cli:connected <IP> <Port> <Handle> */
				strtok (rcvBuf," "); // cmd 
				char * pip = strtok (NULL," "); // ip of client
				char * pport = strtok (NULL," "); // port				
				char * phandle = strtok(NULL," "); // handle
				
				(*TcpConnectedCallback)(TcpConnectedC, pip, pport, phandle); //ip, port, handle
			}
			break;
		case SwitchOn:
			switch (_currState)
			{
			case CONNECTED:
				disconnectCmd();
				break;
			case DISCONNECTED:
				smartconfigCmd();
				break;
			default:
				break;
			}
			break;
		case UnknownInput:
			break;
		default:
			break;
		}
	}
}

void Thing1Class::reset()
{
	pinMode(_powerPin, OUTPUT);
	digitalWrite(_powerPin, LOW);
	delay(200);
	digitalWrite(_powerPin, HIGH);
	_serial->begin(_bitsPerSeconds);
	waitFor(READY);
}

void Thing1Class::setLed(LedCmd ledcmd)
{
	switch (ledcmd)
	{
	case GreenOn:
		sendCmd("led 1 1");
		break;
	case GreenOff:
		sendCmd("led 1 0");
		break;
	case RedOn:
		sendCmd("led 2 1");
		break;
	case RedOff:
		sendCmd("led 2 0");
		break;
	default:
		break;
	}
	waitFor(CRLF);
}

/* Private functions */
void Thing1Class::connectCmd()
{
	sendCmd("connect");
}

void Thing1Class::stateCmd()
{
	sendCmd("state");
}

void Thing1Class::smartconfigCmd()
{
	sendCmd("smartconfig");
}

void Thing1Class::disconnectCmd()
{
	sendCmd("disconnect");
}

void Thing1Class::ipConfig()
{
	sendCmd("ipconfig");
}

void Thing1Class::frimwareVersion()
{
	sendCmd("version");
}

void Thing1Class::createTCPClient(char* ip, unsigned short port)
{
	char str[80];
	/* tcp:cli:connect <IP> <Port> */
	sprintf(str, "tcp:cli:connect %s %u", ip, port);
	sendCmd(str);
}

void Thing1Class::sendingTCPData(char* handle,  const char* message)
{
	char str[80];
	/* tcp:cli:send <Handle> “<Message>” */
	sprintf(str, "tcp:cli:send %s \"%s\"", handle, message);
	sendCmd(str);
}

void Thing1Class::startTCPServer(unsigned short port)
{ 
	char str[80];
	/* tcp:srv:start <Port> */
	sprintf(str, "tcp:srv:start %u", port);
	sendCmd(str);
}

void Thing1Class::stopTCPServer()
{ 
	/* tcp:srv:stop */
	sendCmd("tcp:srv:stop");
}


void Thing1Class::startUDPServer(unsigned short port)
{ 
	char str[80];
	/* udp:srv:start Port */
	sprintf(str, "udp:srv:start %u", port);
	sendCmd(str);
}

void Thing1Class::stopUDPServer()
{ 
	/* udp:srv:stop */
	sendCmd("udp:srv:stop");
}


/* generic callbacks */
void Thing1Class::attach(byte command, callbackFunction newFunction)
{
	switch(command) 
	{
		case IpConfigC:	ipConfigCallback = newFunction; break;
		case WlanDisconnectedC:	wlanDisconnectCallback = newFunction; break;
		case WlanConnectedC:	wlanConnectedCallback = newFunction; break;
		case TcpSrvRcvC:	tcpSrvRcvCallback = newFunction; break;
		case UdpCliRcvC:	udpCliRcvCallback = newFunction; break;
		case UdpSrvTcvC:	udpSrvTcvCallback = newFunction; break;
	}
}

void Thing1Class::attach(byte command, ConnectDisconnectcallbackFunction newFunction)
{
	switch(command) 
	{
		case TcpConnectedC:	TcpConnectedCallback = newFunction; break;
	}
}

void Thing1Class::attach(byte command, tcpCliRcvCallbackFunction newFunction)
{
	switch(command) 
	{
		case TcpCliRcvC: tcpCliRcvCallback = newFunction; break;
	}
}

void Thing1Class::sendCmd(const char* pCommand)
{	

	uint8_t reSend = 0;

	do{
		write(pCommand, strlen(pCommand));
		Serial.write(pCommand);
		writeCRLF();
	
		if (_serial->available() > 0)
		{
			 /* Check return code from Module */
			readline(rcvBuf,RCV_BUF_LEN);
			
			if (strcmp("OK",rcvBuf) == 0)
			{
				reSend = 1;
			}
			else
			{
				reSend = 0;
			}
		}
	
	}while(reSend != 1);
}

int Thing1Class::write(const char* data, uint32_t len)
{
	uint32_t i;
	for (i = 0; i < len; ++i, ++data)
	{
		switch (*data)
		{
		case ESC: case CR: case LF:
			_serial->write(ESC);
			/* Fall through */
		default:
			_serial->write(*data);
		}
		Serial.write(*data);
	}
	return i;
}

int Thing1Class::writeCRLF()
{
	_serial->write(CR);
	_serial->write(LF);
	_serial->flush();

    Serial.write(CR); 
    Serial.write(LF); 
}

int Thing1Class::readUntil(char cStop, char* buf, uint32_t buflen)
{
	uint32_t i;
	int c;

	i = 0;
	while (i < buflen)
	{
		if (_serial->available() > 0)
		{
			c = _serial->read();
			//Serial.write(c);
			switch (c)
			{
			case ESC:
				while (_serial->available() == 0) ;
				c = _serial->read();
				/* Fall through */
			default:
				buf[i] = c;
				++i;
				if (c == cStop)
					break;
			}

			if (c == cStop)
				break;
			buf[i] = c;
			++i;
		}
	}
	return i;
}

int Thing1Class::readline(char* buf, uint32_t buflen)
{
	uint32_t i;
	int c;

	i = 0;
	while (i < buflen)
	{
		if (_serial->available() > 0)
		{
			c = _serial->read();
			//Serial.write(c);
			switch (c)
			{
			case CR:
				while (_serial->available() == 0);
				c = _serial->read(); /* Read LF */
				//Serial.write(c);
				goto done; /* EOL found */
			case ESC:
				while (_serial->available() == 0) ;
				c = _serial->read();
				/* Fall through */
			default:
				buf[i] = c;
				++i;
			}
		}
	}
done:
	buf[i] = '\0'; /* Null terminate buf */
	return i;
}

void Thing1Class::waitFor(const char* s)
{
	uint32_t i = 0;
	int c;

	while ( s[i] != '\0' )
	{
		if (_serial->available() > 0)
		{
			c = _serial->read();
		   // Serial.write(c); 
			if (c==s[i])
				++i;
			else
				i = 0;
		}
	}
}

void Thing1Class::remove_all_chars(char* str, char c) 
{
    char *pr = str, *pw = str;
    while (*pr) {
        *pw = *pr++;
        pw += (*pw != c);
    }
    *pw = '\0';
}



Thing1Class Thing1;
