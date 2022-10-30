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

long greenPos = 13000;
long blackPos = 23000;

const long greenContactPoint = 3000;
const long blackContactPoint = 13000;

unsigned char len = 0;
unsigned char buf[8];

long greenBot = 0x141;
long blackBot = 0x145;

// Use this to keep track of the bot in the startup procedure
long currentBot = 0;

const long up = 0x00;
const long down = 0x01;

// Define enum to specify if the green, black, or both robots will move
// These will be used in the "rhythm" arrays

enum HitType {
  green,
  black,
  both
};

HitType rhythm1[2] = {green,black};
HitType rhythm2[2] = {green,black};
HitType rhythm3[2] = {green,green};

// Moves a bot to a specified position in the defined position
void move (long bot, long dir, long pos, bool slow = false) {
  buf[0]= 0xA6;
  buf[1]= dir;
  buf[2]= slow ? 0x01: 0x20;
  buf[3]= slow ? 0x01: 0x20;
  buf[4]= pos;
  buf[5]= pos >> 8;
  buf[6]= 0x00;
  buf[7]= 0x00;
  CAN.sendMsgBuf(bot,0,8,buf);

  while (true) {
    if (CAN_MSGAVAIL == CAN.checkReceive()) {
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
        long dir = (currentPos > initPos) ? down : up;
        move(bot, dir, initPos, true);
        break;
      }
    }
  }
}

int hit (float greenAmp[], float greenFreq[], float blackAmp[], float blackFreq[], HitType hitType, int harmonics, int delayTime = 10) {
  bool cyc = true;
  float timestep = 0;
  long dirGreen;
  long dirBlack;
  while (cyc) {
    timestep++;

    if (hitType == green || hitType == both) {
      int stepValueGreen = 0;
      for (int i = 0; i < harmonics; ++i) {
        stepValueGreen -= greenAmp[i] * greenFreq[i] * sin(greenFreq[i] * timestep);
        if ((timestep * greenFreq[i]) >= 2 * PI) {
          cyc = false;
        }
      }

      greenPos = greenPos + stepValueGreen;
      if (greenPos < 0) {
        greenPos = 0;
      }

      if (stepValueGreen > 0) {
        dirGreen = up;
      }
      else if (stepValueGreen < 0) {
        dirGreen = down;
      }

      move(greenBot, dirGreen, greenPos);
    }

    //delay(delayTime);

    if (hitType == black || hitType == both) {
      int stepValueBlack = 0;
      for (int i = 0; i < harmonics; ++i) {
        stepValueBlack -= blackAmp[i] * blackFreq[i] * sin(blackFreq[i] * timestep);
        if ((timestep * blackFreq[i]) >= 2 * PI) {
          cyc = false;
        }
      }

      blackPos = blackPos + stepValueBlack;
      if (blackPos < 0) {
        blackPos = 0;
      }

      if (stepValueBlack > 0) {
        dirBlack = up;
      }
      else if (stepValueBlack < 0) {
        dirBlack = down;
      }

      move(blackBot, dirBlack, blackPos);
    }

    //delay(delayTime);
  }
}

void playRhythm(HitType rhythm[], float greenAmp[], float greenFreq[], float blackAmp[], float blackFreq[],
                int harmonics, int greenBias = 0, int blackBias = 0)
{
//  if(greenFreq[0] > 0.4 || blackFreq[0] > 0.4) {
//    exit(0);
//  }

  // initialization according to the AMP - it can be done by adding a constant value to the Pos
  if (sizeof(greenFreq) != 0) {
    int initialPos = greenContactPoint + greenBias + (greenAmp[0] * 2);
    long dir = (initialPos > greenPos) ? up : down;
    move(greenBot, dir, initialPos, true);
    greenPos = initialPos;
    delay(500);
  }
  if (sizeof(blackFreq) != 0) {
    int initialPos = blackContactPoint + blackBias + (blackAmp[0] * 2);
    long dir = (initialPos > blackPos) ? up : down;
    move(blackBot, dir, initialPos, true);
    blackPos = initialPos;
    delay(500);
  }

  int out = 0;
  for (int i = 0; i < 8; ++i) {
    int num = 0;
    num = i % 2;

    if (digitalRead(DOWN) == LOW) {
      break;
    }

    out = hit(greenAmp, greenFreq, blackAmp, blackFreq, rhythm[num], harmonics);
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

  // Read the position of both robots, then decide on a direction after
  // the position is received (>= 16000: up, < 16000: down)

  currentBot = greenBot;
  readPos(greenBot,greenPos);

  currentBot = blackBot;
  readPos(blackBot,blackPos);
  delay(1000);
}

void loop() {
  while (!SERIAL.available());
  unsigned long freq = Serial.read();
  while (!SERIAL.available());
  unsigned long AMP1 = Serial.read();
  float freq1 = float(freq)/1000;
  AMP1 = AMP1*20;

  if ((freq1 > 0.01) && (freq1 < 0.04) && (AMP1 > 0) && (AMP1 < 5000)) {
    float greenAmp[] = {AMP1};
    float greenFreq[] = {freq1};
    float blackAmp[] = {AMP1};
    float blackFreq[] = {freq1};
    playRhythm(rhythm1, greenAmp, greenFreq, blackAmp, blackFreq, 1);
  }
}
