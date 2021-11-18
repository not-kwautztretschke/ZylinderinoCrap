/**
 * @file zylWifi.cpp
 * @author Bernhard Stöffler
 * @brief 
 * @version 0.1
 * @date 2021-11-14
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "zylWifi.h"

//*************************** WIFI ********************************************

int zylWifi::init(zylWifiMode mode){
	if (mode == ZWM_CLIENT){				//! Something is broken here. There needs to be a Wifi.begin() before UDP init!!
		m_WAS.add(STASSID, STAPSWD);
		m_WAS.add(RESCUE_SSID, RESCUE_PSWD);
		while(m_WAS.scanAndConnect() < 0);//this has to succeed, otherwise we crash.
	}else if (mode == ZWM_HOST){
		WiFi.softAP(AP_NAME, AP_PSWD);IPAddress IP = WiFi.softAPIP();
		Serial.print("AP IP address: ");
		Serial.println(IP);
	}else{
		return 1; 			//wrong mode
	}
	return 0;
}

zylWifi::zylWifi() : m_WAS(WIFI_CONNECT_TIMEOUT) {};	//this calls the constructor of member m_WAS

int zylWifi::handle(){									//periodically check the wifi connection
	if(WiFi.status() != WL_CONNECTED) {					//and try to reconnect if broken
		Serial.print("Connecting wifi ");
		if(-1 < m_WAS.scanAndConnect()) {
			int connectedIndex = m_WAS.getConnectedIndex();
			Serial.print("to '");
			Serial.print(m_WAS.getSSID(connectedIndex));
			Serial.println("'. Done.");
		}else{
			Serial.println("failed.");
			return 1;
		}
	}
	return 0;
}

//*************************** UDP ********************************************

int zylUdp::init(int port){
	return m_Udp.begin(port);
}

int zylUdp::read(uint8_t* opcode, uint8_t* x, uint8_t* y, uint8_t* z){
	while (m_Udp.parsePacket()){
		int length = m_Udp.read(m_aPupBuffer, PUPBUFFER_SIZE);
		if (length && verifyPacket()) {
			*opcode = 	m_aPupBuffer[0];
			*x 		=	m_aPupBuffer[1];
			*y 		=	m_aPupBuffer[2];
			*z 		=	m_aPupBuffer[3];
		}
	}
	return 0;
}

#ifdef Z_LEO
int zylUdp::verifyPacket(){
	return (m_aPupBuffer[4] == PUP_SIGNATURE);
}
#endif

#ifdef Z_BERNIE
int zylUdp::verifyPacket(){
	return 1;		// No signature.
}
#endif


//*************************** Over the Air Update ********************
int initOTA(){
	ArduinoOTA.setHostname(OTA_HOSTNAME);
	#ifdef OTA_PASSWORDHASH
		ArduinoOTA.setPasswordHash("a1d237e74e5ce5c1f4584da49afeb5f7");
	#else
		ArduinoOTA.setPassword(OTA_PASSWORD);
	#endif
	ArduinoOTA.onStart([]() {
		Serial.println("OTA: Start");
		//digitalWrite(BOARD_LED,1);
	});
	ArduinoOTA.onEnd([]() {
		//digitalWrite(BOARD_LED,1);
		Serial.println("\nOTA: End");
	});
	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
		static bool state = false;
		//digitalWrite(BOARD_LED,state);
		state ^= true;
		Serial.printf("OTA: Progress: %u%%\r", (progress / (total / 100)));
	});
	ArduinoOTA.onError([](ota_error_t error) {
		Serial.printf("Error[%u]: ", error);
		if 		(error == OTA_AUTH_ERROR)      	{Serial.println("Auth failed");		return 1;}
		else if (error == OTA_BEGIN_ERROR)  	{Serial.println("Begin failed");	return 2;}
		else if (error == OTA_CONNECT_ERROR)	{Serial.println("Connect failed");	return 3;}
		else if (error == OTA_RECEIVE_ERROR)	{Serial.println("Receive failed");	return 4;}
		else if (error == OTA_END_ERROR)    	{Serial.println("End failed");		return 5;}
		else									{									return 0;}
	});
	Serial.printf("Beginning OTA...");
	ArduinoOTA.begin();
	Serial.printf("now\n");
	delay(1000);
	return 0;
}