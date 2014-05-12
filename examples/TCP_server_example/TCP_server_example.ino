/* TCP Server Example with THING1 WiFi Module

White Systems ApS - http://www.white-systems.dk/wifi/ - info@white-systems.com 

License: Released under the Creative Commons Attribution Share-Alike 3.0 License. http://creativecommons.org/licenses/by-sa/3.0/

Target:  Arduino 
*/

#include <Thing1.h>

/* Callback function header */
void IpConfigCallback(byte event, char* data);
void TcpCliRcvCallback(byte event, char* handle, char* message);  
void TcpConnectedCallback(byte event, char* ip, char* port, char* handle);


/* Init */
void setup()
{
  Serial.begin(57600);
  delay(2000);
  /* Star-up THING1 module and connect to Internet */
  Thing1.begin(A0, &Serial1, 57850);
  Thing1.attach(IpConfigC, IpConfigCallback);
  Thing1.attach(TcpCliRcvC, TcpCliRcvCallback);
  Thing1.attach(TcpConnectedC, TcpConnectedCallback);
}

/* loop */
void loop()
{
  Thing1.loop();
}


/* This function will be called when IP adress is assigned to THING1 module */ 
void IpConfigCallback(byte event, char* data)
{
   Thing1.startTCPServer(8080); 
}

/* When incoming TCP data is received this function is called */
void TcpCliRcvCallback(byte event, char* handle, char* message)
{
   PrepareTCPData (handle, message);
}

/* TCP connection is established (happens when another device connects as 
TCP client to the TCP server running in the THING1 WiFi Module) */
void TcpConnectedCallback(byte event, char* ip, char* port, char* handle)
{
    String _ip(ip);
    String _port(port);
    String _handle(handle);
    
    Serial.print("Connect IP: ");
    Serial.print (_ip);
    Serial.print (" Port: ");
    Serial.print (_port);
    Serial.print (" Handle: ");
    Serial.println (_handle);
}


/* This function is preparing TCP data */ 
void PrepareTCPData (char* handle, char* message)
{
  int sensorValue = 0;
  
  String _message(message);
  _message.trim();
  
  if  (_message == "0")
  {
    sensorValue = analogRead(A0);
    SendTCPData(handle, sensorValue); 
  }
  else if  (_message == "1")
  {
    sensorValue = analogRead(A1);
    SendTCPData(handle, sensorValue);
  }
  else if  (_message == "2")
  {
    sensorValue = analogRead(A2);
    SendTCPData(handle, sensorValue);
    
  }
  else if  (_message == "3")
  {
    sensorValue = analogRead(A3);
    SendTCPData(handle, sensorValue); 
  }
}

/* This function implementing TCP send function */ 
void SendTCPData(char* handle, int sensorValue)
{
  String stringOne = "Sensor value: ";
  String stringThree = stringOne + sensorValue;
  
  const char *p = stringThree.c_str();
  
  Thing1.sendingTCPData(handle,p);
}

