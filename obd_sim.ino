#include <mcp_can.h>
#include <SPI.h>
#define INT32U unsigned long int

MCP_CAN CAN(9);                                      // Set CS to pin 9

INT32U canId = 0x000;

unsigned char len = 0;
unsigned char buf[8];

String BuildMessage="";
int MSGIdentifier=0;


void setup() {
  Serial.begin(115200);

START_INIT:

    if(CAN_OK == CAN.begin(CAN_500KBPS))                   // init can bus : baudrate = 500k
    {
        Serial.println("CAN BUS Shield init ok!");
    }
    else
    {
        Serial.println("CAN BUS Shield init fail");
        Serial.println("Init CAN BUS Shield again");
        delay(100);
        goto START_INIT;
    }
}

void loop()
{
    //Virtuel Sensors, just random values
    int intCoolantTemp=random(1,200);
    int intRPM=random(0,163); //factor 100, directly used by CAN, needs to be recalculated for OBD II
    int intSpeed=random(0,255);
    int intIAT=random(0,255);
    int intMAF=random(0,255);
    int intAmbientAirTemp=random(0,200);
    int intCAT1Temp=random(1,55);
    
    //SENSORS OBD II
    char rndCoolantTemp=intCoolantTemp;
    char rndRPM=intRPM;
    char rndSpeed=intSpeed;
    char rndIAT=intIAT;
    char rndMAF=intMAF;
    char rndAmbientAirTemp=intAmbientAirTemp;
    char rndCAT1Temp=intCAT1Temp;
    char obdRPM=(intRPM*400-224)/256; //recalculated RPM for OBD II
	
    //GENERAL ROUTINE for OBD II
    unsigned char SupportedPID[8] =       {1,2,3,4,5,6,7,8};
    unsigned char MilCleared[8] =         {4, 65, 63, 34, 224, 185, 147, 170}; 
	
    unsigned char CoolantTemp[8] =        {4, 65, 5, rndCoolantTemp, 0, 185, 147, 170};  
    unsigned char rpm[8] =                {4, 65, 12, obdRPM, 224, 170, 170, 170}; //{0x04, 0x41, 0x0C, rndRPM (factorA), 224 (factorB), 185 (fuellbyte), 147 (fuellbyte)};
    unsigned char vspeed[8] =             {4, 65, 13, rndSpeed, 224, 170, 170, 170}; //{0x04, 0x41, 0x0D, rndSpeed (factorA), 224 (fuellbyte), 185 (fuellbyte), 147 (fuellbyte)};
    unsigned char IATSensor[8] =          {4, 65, 15, rndIAT, 0, 185, 147, 170};
    unsigned char MAFSensor[8] =          {4, 65, 16, rndMAF, 0, 185, 147, 170};
    unsigned char AmbientAirTemp[8] =     {4, 65, 70, rndAmbientAirTemp, 0, 185, 147, 170};
    unsigned char CAT1Temp[8] =           {4, 65, 60, rndCAT1Temp, 224, 185, 147, 170};
    unsigned char CAT2Temp[8] =           {4, 65, 61, rndCAT1Temp, 224, 185, 147, 170};
    unsigned char CAT3Temp[8] =           {4, 65, 62, rndCAT1Temp, 224, 185, 147, 170};
    unsigned char CAT4Temp[8] =           {4, 65, 63, rndCAT1Temp, 224, 185, 147, 170};
	
    //SENSORS CAN
    char canTorque=random(0,255);
    char canVoltage=random(0,255);
    char canPressure=random(0,255);
   
    unsigned char testbench_000[8] =      {canVoltage, canTorque, rndSpeed, rndRPM, 0, 0, 0, 21};
    unsigned char ambient_500[8] =        {canPressure, rndAmbientAirTemp, 0, 0, 0, 0, 0, 21};

    //CAN
    CAN.sendMsgBuf(0, 0, 8, testbench_000);
    CAN.sendMsgBuf(500, 0, 8, ambient_500);
    
    //OBD II
    if(CAN_MSGAVAIL == CAN.checkReceive())  
    {
      CAN.readMsgBuf(&len, buf); 
        canId = CAN.getCanId();
        Serial.print("<");Serial.print(canId);Serial.print(",");

        for(int i = 0; i<len; i++)
        {  
          BuildMessage = BuildMessage + buf[i] + ",";
        }
        Serial.println(BuildMessage);
        
        //Check wich message was received.
        if(BuildMessage=="2,1,0,0,0,0,0,0,") {CAN.sendMsgBuf(0x7E8, 0, 8, SupportedPID);}
        if(BuildMessage=="2,1,1,0,0,0,0,0,") {CAN.sendMsgBuf(0x7E8, 0, 8, MilCleared);}
        
        //SEND SENSOR STATUSES
        if(BuildMessage=="2,1,5,0,0,0,0,0,") {CAN.sendMsgBuf(0x7E8, 0, 8, CoolantTemp);}
        if(BuildMessage=="2,1,12,0,0,0,0,0,"){CAN.sendMsgBuf(0x7E8, 0, 8, rpm);}
        if(BuildMessage=="2,1,13,0,0,0,0,0,"){CAN.sendMsgBuf(0x7E8, 0, 8, vspeed);}
        if(BuildMessage=="2,1,15,0,0,0,0,0,"){CAN.sendMsgBuf(0x7E8, 0, 8, IATSensor);}
        if(BuildMessage=="2,1,16,0,0,0,0,0,"){CAN.sendMsgBuf(0x7E8, 0, 8, MAFSensor);}
        if(BuildMessage=="2,1,70,0,0,0,0,0,"){CAN.sendMsgBuf(0x7E8, 0, 8, AmbientAirTemp);}
        if(BuildMessage=="2,1,60,0,0,0,0,0,"){CAN.sendMsgBuf(0x7E8, 0, 8, CAT1Temp);}
        if(BuildMessage=="2,1,61,0,0,0,0,0,"){CAN.sendMsgBuf(0x7E8, 0, 8, CAT2Temp);}
        if(BuildMessage=="2,1,62,0,0,0,0,0,"){CAN.sendMsgBuf(0x7E8, 0, 8, CAT3Temp);}
        if(BuildMessage=="2,1,63,0,0,0,0,0,"){CAN.sendMsgBuf(0x7E8, 0, 8, CAT4Temp);}
        BuildMessage="";
    }
}
