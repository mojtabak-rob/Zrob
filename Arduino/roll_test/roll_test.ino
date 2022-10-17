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

#define StepValue 150

const int SPI_CS_PIN = 10;

MCP_CAN CAN(SPI_CS_PIN);

long greenPos = 12000;
long blackPos = 25000;

unsigned char len = 0;
unsigned char buf[8];

long greenBot = 0x141;
long blackBot = 0x145;

// Use this to keep track of the bot in the startup procedure
long currentBot = 0;

long dir = 0x00;

const long up = 0x00;
const long down = 0x01;

// Define enum to specify if the green, black, or both robots will move
// These will be used in the "rhythm" arrays

enum HitType {
  green,
  black,
  both
};

// Moves a bot to a specified position in the defined position

void move (long bot, long dir, long pos) {
  buf[0]= 0xA6;
  buf[1]= dir;
  buf[2]= 0x20;
  buf[3]= 0x20;
  buf[4]= pos;
  buf[5]= pos >> 8;
  buf[6]= 0x00;
  buf[7]= 0x00;
  CAN.sendMsgBuf(bot,0,8,buf);

  while (true){
        if (CAN_MSGAVAIL == CAN.checkReceive()){
          CAN.readMsgBuf(&len,buf);
          break;  
        }
      }
}

void moveslow (long bot, long dir, long pos) {
  buf[0]= 0xA6;
  buf[1]= dir;
  buf[2]= 0x01;
  buf[3]= 0x01;
  buf[4]= pos;
  buf[5]= pos >> 8;
  buf[6]= 0x00;
  buf[7]= 0x00;
  CAN.sendMsgBuf(bot,0,8,buf);

  while (true){
        if (CAN_MSGAVAIL == CAN.checkReceive()){
          CAN.readMsgBuf(&len,buf);
          break;  
        }
      }
}

// Reads the current position of the bot

void readPos (long bot, long initPos) {
  buf[0]= 0x94;
  buf[1]= 0x00;
  buf[2]= 0x00;
  buf[3]= 0x00;
  buf[4]= 0x00;
  buf[5]= 0x00;
  buf[6]= 0x00;
  buf[7]= 0x00;
  CAN.sendMsgBuf(bot,0,8,buf);

  while (true) 
  {
    if (CAN_MSGAVAIL == CAN.checkReceive())
    {
      CAN.readMsgBuf(&len,buf);
  
      unsigned long canID = CAN.getCanId();
  
      if (buf[0] == 0x94) {
        // Determine position of bot, then move to starting position of 16000
        unsigned int currentPos = buf[6] + (buf[7] << 8);
        //SERIAL.println(currentPos);
        dir = (currentPos > initPos) ? down : up;
        moveslow(currentBot, dir, initPos);
        break;
      }
    }
  }
}

int hit (float freq, HitType hitType) {
  bool cyc = true;
  float timestep = 0;
  long dirGreen;
  long dirBlack;
  while (cyc) {
    timestep++;
    
    float stepValueGreen =  3000 * freq * sin(freq * timestep);
    float stepValueBlack = - 3000 * freq * sin(freq * timestep);
    
    //SERIAL.print("stepValue = " + String(stepValue));
    //SERIAL.println();
    
    if (stepValueGreen > 0) {
      dirGreen = up;
    }
    else if (stepValueGreen < 0) {
      dirGreen = down;
    }

    if (stepValueBlack > 0) {
      dirBlack = up;
    }
    else if (stepValueBlack < 0) {
      dirBlack = down;
    }

    

    if (hitType == green || hitType == both) {
      greenPos = greenPos + stepValueGreen;

      if (greenPos < 0) {
        greenPos = 0;
      }

      move(greenBot, dirGreen, greenPos);
    }

    if (hitType == black || hitType == both) {
      blackPos = blackPos + stepValueBlack;

      if (blackPos < 0) {
        blackPos = 0;
      }

      move(blackBot, dirBlack, blackPos);
    }
    if ((timestep * freq) >= 2 * PI) {
      cyc = false;
    }
  }
}

void playRhythm(HitType rhythm[], float freq) {
  int out = 0;

  for (int i = 0; i < 8; ++i) {
    int num = 0;
    float bar = 0;
    if(freq > 0.4) {
      exit(0);
    }

    num = i % 2;

    if (digitalRead(DOWN) == LOW) {
      break;
    }

    out = hit(freq, rhythm[num]);
  }
}

void setup() {
  SERIAL.begin(115200);
  delay(1000);
  while (CAN_OK != CAN.begin(CAN_1000KBPS))
  {
      SERIAL.println("CAN BUS Shield init fail");
      //SERIAL.println(CAN.begin(CAN_500KBPS));
      SERIAL.println("Init CAN BUS Shield again");
      delay(100);
  }
  SERIAL.println("CAN BUS Shield init ok!");
  SERIAL.println(CAN.checkError());

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

  // Read the position of both robots, then decide on a direction after
  // the position is received (>= 16000: up, < 16000: down)
  
}

void loop() {
  HitType rhythm1[2] = {both,both};
  HitType rhythm2[2] = {green,black};
  HitType rhythm3[2] = {green,green};

  currentBot = greenBot;
  readPos(greenBot,greenPos);

  currentBot = blackBot;
  readPos(blackBot,blackPos);
  delay(1000);

  while(true){


    
  
  
    if (digitalRead(UP)==LOW) {
      playRhythm(rhythm1, 0.005);
    }
    else if (digitalRead(LEFT)==LOW) {
      playRhythm(rhythm2, 0.005);
    }
    else if (digitalRead(RIGHT)==LOW) {
      //SERIAL.print("rhythm 3 selected");
      playRhythm(rhythm3, 0.005);
    }
  }

}

//SERIAL read:
//  while (!SERIAL.available());
//      freq = SERIAL.readString().toInt();






//  if (CAN_MSGAVAIL == CAN.checkReceive())
//  {
//    CAN.readMsgBuf(&len,buf);
//
//    unsigned long canID = CAN.getCanId();
//
//    for (int i = 0; i<len; i++)
//    {
//      SERIAL.print(buf[i],HEX);
//      SERIAL.print("\t");
//    }
//    SERIAL.print(greenPos);
//    SERIAL.print("\t");
//    SERIAL.print(blackPos);
//    SERIAL.print("\t");
//    SERIAL.print(canID,HEX);
//    SERIAL.println();
//  }
