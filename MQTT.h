
#include "application.h"
class MQTT{
    public:
        
        // state for the connection
        typedef enum{
            ACCEPTED = 0x00,							       
            UNACCEPTABLE_PROCOTOL = 0x01,				        
            ID_REJECT = 0x02,							        
            SERVER_UNAVAILALE = 0x03,							
            BAD_USER_PASSWORD = 0x04,							
            NOT_AUTHORIZED = 0x05,							    
        } ConnectionState;
        
        typedef enum
        {
            NO_ERROR = 0x00,							        
            CONNECTION_IN_USE = 0x01,							
            NOT_CONNECTED = 0x02,							   
            CLIENT_ERROR = 0x03,							    
            INVALID_PARAMETER = 0x04,							
            TRANSMISSION_ERROR = 0x05,							
            TIMEOUT = 0x06,							           
            BUFFER_OVERFLOW = 0x07,							    
            HOST_UNREACHABLE = 0x08,						   
        } Error;

       typedef struct
        {
            const char* Name;							        
            const uint8_t* Password;							
            const uint16_t PasswordLength;						
        } User;

        bool isConnected(void);
        MQTT::ConnectionState connectionState(void) const;

        
        MQTT(IPAddress IP, uint16_t Port, uint16_t KeepAlive, Publish_Callback Callback);
        
        
        ~MQTT();

        MQTT::Error Connect(const char* ClientID, bool CleanSession, MQTT::Will* Will, MQTT::User* User);


        void Disonnect(void);

        void SetBroker(IPAddress IP, uint16_t Port);

        void SetKeepAlive(uint16_t KeepAlive);

        void SetCallback(Publish_Callback Callback);


        MQTT::Error Subscribe(const char* Topic, MQTT::QoS QoS);

    private:
        #define MQTT_FIXED_HEADER_SIZE                  0x05

        //MQTT control packets.

        typedef enum
        {
            CONNECT = 0x01,
            CONNACK = 0x02,
            PUBLISH = 0x03,
            PUBACK = 0x04,
            PUBREC = 0x05,
            PUBREL = 0x06,
            PUBCOMP = 0x07,
            SUBSCRIBE = 0x08,
            SUBACK = 0x09,
            UNSUBSCRIBE = 0x0A,
            UNSUBACK = 0x0B,
            PINGREQ = 0x0C,
            PINGRESP = 0x0D,
            DISCONNECT = 0x0E,
        } ControlPacket;

        Timer* _mPingTimer;

        TCPClient _mClient;
        IPAddress _mIP;
        ConnectionState _mConnectionState;
        
        uint8_t _mBuffer[MQTT_BUFFER_SIZE];

        uint16_t _mPort;
        uint16_t _mKeepAlive;
        uint16_t _mCurrentMessageID;

        bool _mWaitForHostPing;

        Publish_Callback _mCallback;

        void _init(IPAddress IP, uint16_t Port, uint16_t KeepAlive, Publish_Callback Callback);
        void _sendPing(void);


}


