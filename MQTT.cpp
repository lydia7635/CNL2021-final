#include "MQTT.h"

 // check connection state of TCP Client, return true if connected
bool MQTT::isConnected(void){
    return this->_mClient.connected();
}

MQTT::ConnectionState MQTT::connectionState(void) const{
    return this->_mConnectionState;
}
/*
            IP address of MQTT broker
            Port used by MQTT Client
            Keep alive = time duration used by MQTT Client
            Callback = publish received callback
*/

MQTT::MQTT(IPAddress IP, uint16_t Port, uint16_t KeepAlive, Publish_Callback Callback){
    this->_init(IP, Port, KeepAlive, Callback);
}

// Deconstructor. Stops the timer and close the network connection.
MQTT::~MQTT(){
    if(this->_mPingTimer != NULL){
        this->_mPingTimer->dispose();
        delete this->_mPingTimer;
    }

    if(this->isConnected()){
        this->Disonnect();
    }
}
/*
    Client ID: a user
    CleanSession: The Client and Server can store Session state to enable reliable messaging to continue across a sequence of Network Connections.
    User: Pointer to user settings object.
    return Error code
*/
MQTT::Error MQTT::Connect(const char* ClientID, bool CleanSession, MQTT::User* User){
    uint16_t Length = MQTT_FIXED_HEADER_SIZE;

    if(ClientID == NULL){
        return INVALID_PARAMETER;
    }

    if(!this->isConnected()){
        if(_mClient.connect(this->_mIP, this->_mPort)){
            uint8_t Flags = 0x00;
            this->_mCurrentMessageID = 0x01;

            // // Set the protocol name and the protocol level
            // #if(MQTT_VERSION == MQTT_VERSION_3_1)
            //     const uint8_t Header[] = {0x00, 0x06, 'M', 'Q', 'I', 's', 'd', 'p', MQTT_VERSION};
            // #endif

            memcpy(this->_mBuffer + MQTT_FIXED_HEADER_SIZE, Header, sizeof(Header));
            Length += sizeof(Header);

            Flags |= CleanSession << 0x01;

            // send message/ topic that wanted to subcribed, 但不確定你們要怎麼處理（？

            // if(Will){
            //     if(!(Will->Message) || (!(Will->Topic))){
            //         return INVALID_PARAMETER;
            //     }

            //     Flags |= (((uint8_t)Will->Retain) << 0x05) | (((uint8_t)Will->QoS) << 0x03) | (0x01 << 0x02);
            // }

            if(User){
                if(!(User->Name)){
                    return INVALID_PARAMETER;
                }
                Flags |= (0x01 << 0x07);
                if(User->Password){
                    Flags |= (0x01 << 0x06);
                }
            }

            // Set the flags
            this->_mBuffer[Length++] = Flags;

            // Set keep alive
            this->_mBuffer[Length++] = (this->_mKeepAlive >> 0x08);
            this->_mBuffer[Length++] = (this->_mKeepAlive & 0xFF);

            // Set the client ID
            this->_copyString(ClientID, &Length);

            // Set the will configuration
            if(Will){
                this->_copyString(Will->Topic, &Length);
                this->_copyString(Will->Message, &Length);
            }

            // Set the user configuration
            if(User){
                this->_copyString(User->Name, &Length);

                if(User->Password){
                    // ！！！ 之後要改，現在先用隨便數字
                    this->_mBuffer[Length++] = (User->PasswordLength >> 0x08);
                    this->_mBuffer[Length++] = (User->PasswordLength & 0xFF);

                    if((Length + User->PasswordLength) > MQTT_BUFFER_SIZE){
                        return BUFFER_OVERFLOW;
                    }

                    for(uint16_t i = 0x00; i < User->PasswordLength; i++){
                        this->_mBuffer[Length++] = User->Password[i];
                    }
                }
            }

            // Transmit the buffer
            if(this->_writeMessage(CONNECT, 0x00, Length - MQTT_FIXED_HEADER_SIZE)){
                return TRANSMISSION_ERROR;
            }

            // Wait for the broker
            uint32_t TimeLastAction = millis();
            while(!_mClient.available()){
                if((millis() - TimeLastAction) > (this->_mKeepAlive * 1000UL)){
                    this->_mClient.stop();
                    return TIMEOUT;
                }
            }

            uint16_t Temp;
            if((this->_readMessage(&Length, &Temp)) || (this->_mBuffer[0] != (CONNACK << 0x04))){
                return TRANSMISSION_ERROR;
            }

            // Save the connection state
            this->_mConnectionState = (MQTT::ConnectionState)this->_mBuffer[3];

            // 如果還有時間的話 ToDo: Add more detailed error message
            if(this->_mConnectionState == ACCEPTED){
                this->_mPingTimer->start();
                return NO_ERROR;
            }
            return HOST_UNREACHABLE;
        }
        return CLIENT_ERROR;
    }
    else{
        return CONNECTION_IN_USE;
    }
}
// Close the connection with the MQTT broker.
void MQTT::Disonnect(void){
    this->_mBuffer[0] = (DISCONNECT << 0x04);
    this->_mBuffer[1] = 0x00;
    _mClient.write(this->_mBuffer, 0x02);
    this->_mClient.stop();
    this->_mPingTimer->stop();
}
/*
    Set the IP address and the port for the communication with the broker.
    IP address of the broker, Port used by client
*/

void MQTT::SetBroker(IPAddress IP, uint16_t Port){
    this->_mIP = IP;
    this->_mPort = Port;
}
/*
    keep alive time for the communication with the broker.
*/
void MQTT::SetKeepAlive(uint16_t KeepAlive){
    this->_mKeepAlive = KeepAlive;
}

/*
    keep alive time for the communication with the broker.
*/
void MQTT::SetCallback(Publish_Callback Callback){
    this->_mCallback = Callback;
}

//read single byte from tcp client
uint8_t MQTT::_readByte(void){
    while(!this->_mClient.available());
    return this->_mClient.read();
}

//initialize all variables needed by client
void MQTT::_init(IPAddress IP, uint16_t Port, uint16_t KeepAlive, Publish_Callback Callback){
    this->_mIP = IP;
    this->_mPort = Port;
    this->_mKeepAlive = KeepAlive;
    this->_mCallback = Callback;

    this->_mPingTimer = new Timer(this->_mKeepAlive * 1000UL, &MQTT::_sendPing, *this);
    this->_mPingTimer->stop();
}


// send a ping control packet to the broker.
void MQTT::_sendPing(void){
    if(this->isConnected()){
        if(this->_mWaitForHostPing){
            this->_mClient.stop();
        }

        // Send new ping
        this->_mBuffer[0] = (PINGREQ << 0x04);
        this->_mBuffer[1] = 0x00;
        this->_mClient.write(this->_mBuffer, 0x02);
        this->_mWaitForHostPing = true;
    }
}