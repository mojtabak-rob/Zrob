#include <mcp_can.h>
#include <SPI.h>

#ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE
  #define SERIAL SerialUSB
#else
  #define SERIAL Serial
#endif

#define UP    A1
#define DOWN  A3
#define LEFT  A2
#define RIGHT A5
#define CLICK A4

#define LED2 8
#define LED3 7

#define StepValue 20



const int SPI_CS_PIN = 10;

MCP_CAN CAN(SPI_CS_PIN);

void setup() {
  // put your setup code here, to run once:
  SERIAL.begin(115200);
  SERIAL.setTimeout(1);
  delay(1000);
  while (CAN_OK != CAN.begin(CAN_1000KBPS))
  {
      //SERIAL.println("CAN BUS Shield init fail");
      //SERIAL.println(CAN.begin(CAN_500KBPS));
      //SERIAL.println("Init CAN BUS Shield again");
      delay(100);
  }
  //SERIAL.println("CAN BUS Shield init ok!");
  //SERIAL.println(CAN.checkError());

  pinMode(UP, INPUT);
  pinMode(DOWN, INPUT);
  pinMode(LEFT, INPUT);
  pinMode(RIGHT, INPUT);
  pinMode(LED2, INPUT);
  pinMode(LED3, INPUT);

  digitalWrite(UP, HIGH);
  digitalWrite(DOWN, HIGH);
  digitalWrite(LEFT, HIGH);
  digitalWrite(RIGHT, HIGH);

  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);

}
long slim =0x01;
long dir=0x00;
long dir2=0x00;
long GenPos = 12600;
long GenPoss = 22600;
long GenPos2 = 0;
long GenPoss2 = 0;
int freq;
int out;
float timestep=0;
float freq1;
long StepValue1;
void loop() {
  // put your main code here, to run repeatedly:
  unsigned char len = 0;
  unsigned char buf[8];
  unsigned char buf1[8];
  unsigned char buff[2];

  while (true){

    
    buf[0]= 0x94;
    buf[1]= 0x00;
    buf[2]= 0x00;
    buf[3]= 0x00;
    buf[4]= 0x00;
    buf[5]= 0x00;
    buf[6]= 0x00;
    buf[7]= 0x00;
    delay(500);
    CAN.sendMsgBuf(0x141,0,8,buf);
    

    while (true){
      if (CAN_MSGAVAIL == CAN.checkReceive()){
        CAN.readMsgBuf(&len,buf1);
        break;  
      }
    }
    GenPos2= buf1[6]+buf1[7]*16*16;

    buf[0]= 0x94;
    buf[1]= 0x00;
    buf[2]= 0x00;
    buf[3]= 0x00;
    buf[4]= 0x00;
    buf[5]= 0x00;
    buf[6]= 0x00;
    buf[7]= 0x00;
    delay(500);
    CAN.sendMsgBuf(0x145,0,8,buf);
    

    while (true){
      if (CAN_MSGAVAIL == CAN.checkReceive()){
        CAN.readMsgBuf(&len,buf1);
        break;  
      }
    }
    GenPoss2= buf1[6]+buf1[7]*16*16;
    
    GenPos = 12600;
    GenPoss = 22600;
    //while (!SERIAL.available());
    //GenPos = SERIAL.readString().toInt();
    if (GenPos<5000||GenPos>30000){
      SERIAL.print(0000);
      break;
    }
    if (GenPoss<5000||GenPoss>30000){
      SERIAL.print(0000);
      break;
    }

    if (GenPos2-GenPos<0){
      dir=0x00;
    }
    else{
      dir=0x01;
    }
    if (GenPos>35999){
      GenPos=max(GenPos-35999,0);
    }
    if (GenPos<0){
      GenPos=min(GenPos + 35999,35999);
    }

    if ((GenPos2-GenPos)*(GenPos2-GenPos)>90000){
      buf[0]= 0xA6;
      buf[1]= dir;
      buf[2]= slim;
      buf[3]= slim;
      buf[4]= GenPos;
      buf[5]= GenPos >>8;
      buf[6]= 0x00;
      buf[7]= 0x00;
      CAN.sendMsgBuf(0x141,0,8,buf);

      while (true){
        if (CAN_MSGAVAIL == CAN.checkReceive()){
          CAN.readMsgBuf(&len,buf);
          break;  
        }
      }
    }


    
    if (GenPoss2-GenPoss<0){
      dir2=0x00;
    }
    else{
      dir2=0x01;
    }
    if (GenPoss>35999){
      GenPoss=max(GenPoss-35999,0);
    }
    if (GenPoss<0){
      GenPoss=min(GenPoss + 35999,35999);
    }

    if ((GenPoss2-GenPoss)*(GenPoss2-GenPoss)>90000){
      buf[0]= 0xA6;
      buf[1]= dir2;
      buf[2]= slim;
      buf[3]= slim;
      buf[4]= GenPoss;
      buf[5]= GenPoss >>8;
      buf[6]= 0x00;
      buf[7]= 0x00;
      CAN.sendMsgBuf(0x145,0,8,buf);

      while (true){
        if (CAN_MSGAVAIL == CAN.checkReceive()){
          CAN.readMsgBuf(&len,buf);
          break;  
        }
      }
    }

    while(true){

      int d=0;

      while (!SERIAL.available());
      freq = SERIAL.readString().toInt();
    
      freq1 = float(freq)/1000;
      if (freq1<0.01||freq1>0.3){
        SERIAL.print(freq1);
        d=1;
      }
      if (d==0){
        //out=hit(freq1);
        delay(500);
        out=hit2(freq1);
        SERIAL.print(freq1);
      }
    }

    

    
//    while (true){
//      buf[0]= 0x94;
//      buf[1]= 0x00;
//      buf[2]= 0x00;
//      buf[3]= 0x00;
//      buf[4]= 0x00;
//      buf[5]= 0x00;
//      buf[6]= 0x00;
//      buf[7]= 0x00;
//      CAN.sendMsgBuf(0x141,0,8,buf);
//
//      while (true){
//        if (CAN_MSGAVAIL == CAN.checkReceive()){
//          CAN.readMsgBuf(&len,buf);
//          break;  
//        }
//      }
//      GenPos2= buf[6]+buf[7]*16*16;
//      if (pow((GenPos2-GenPos),2)<20){
//        break;
//      }
//    }
//    SERIAL.print(GenPos2);


  }
}

int hit(float freqq) {
  unsigned char len = 0;
  unsigned char buf[8];
  bool cyc=1;
  timestep=0;
  freqq=freqq/5;
  while (cyc){

    StepValue1=-4000*freqq*sin(freqq*timestep);
    
    
    GenPos = GenPos + StepValue1;
    
    if (GenPos<0){
      GenPos=0;
    }
    
    if (StepValue1>0){
      dir = 0x00;
    }
    
    if (StepValue1<0){
      dir = 0x01;
    }

    if (GenPos<3000||GenPos>30000){
      SERIAL.print(0000);
      break;
    }

    buf[0]= 0xA6;
    buf[1]= dir;
    buf[2]= 0x20;
    buf[3]= 0x20;
    buf[4]= GenPos;
    buf[5]= GenPos >>8;
    buf[6]= 0x00;
    buf[7]= 0x00;
    
    CAN.sendMsgBuf(0x141,0,8,buf);

    if (CAN_MSGAVAIL == CAN.checkReceive()){
      CAN.readMsgBuf(&len,buf);
      unsigned long canID = CAN.getCanId();
    }
    timestep++;
    
    if (timestep*freqq>=2*PI*8){
      cyc=0;
    }
  }
  return 0;
}


int hit2(float freqq) {
  unsigned char len = 0;
  unsigned char buf[8];
  bool cyc=1;
  timestep=0;
  freqq=freqq/5;
  while (cyc){

    StepValue1=-2000*freqq*sin(freqq*timestep);
    
    
    GenPoss = GenPoss + StepValue1;
    
    if (GenPoss<0){
      GenPoss=0;
    }
    
    if (StepValue1>0){
      dir2 = 0x00;
    }
    
    if (StepValue1<0){
      dir2 = 0x01;
    }

    if (GenPoss<6000||GenPoss>30000){
      SERIAL.print(0000);
      break;
    }

    buf[0]= 0xA6;
    buf[1]= dir2;
    buf[2]= 0x20;
    buf[3]= 0x20;
    buf[4]= GenPoss;
    buf[5]= GenPoss >>8;
    buf[6]= 0x00;
    buf[7]= 0x00;
    
    CAN.sendMsgBuf(0x145,0,8,buf);

    if (CAN_MSGAVAIL == CAN.checkReceive()){
      CAN.readMsgBuf(&len,buf);
      unsigned long canID = CAN.getCanId();
    }
    timestep++;
    
    if (timestep*freqq>=2*PI*8){
      cyc=0;
    }
  }
  return 0;
}
