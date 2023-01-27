#include "shared/shared.h"

#include <SoftwareSerial.h>
SoftwareSerial espSerial(8, 9); // RX, TX

Stream* mainSerial = nullptr;

constexpr int paddlePin1 = 5;
constexpr int paddlePin2 = 6;

constexpr int positionSensorPin = 2;

constexpr int chuteLaserPin = 12;
constexpr int reservoirLaserPin = A2;

constexpr int chuteLaserSensorPin = 3;
constexpr int reservoirLaserSensorPin = A4;

volatile bool positionPinLow = false;
volatile bool chuteSensorTriggered = false;

constexpr int shiftLatchPin = 11;
constexpr int shiftClockPin = 12;
constexpr int shiftDataPin = 10;

uint8_t shifterOutputs = 0;

void setShifterOutput(ShiftOutputs output, bool state)
{
  bitWrite(shifterOutputs, output, state);

  Serial.println(shifterOutputs);

  digitalWrite(shiftLatchPin, LOW);
  shiftOut(shiftDataPin, shiftClockPin, MSBFIRST, shifterOutputs);
  digitalWrite(shiftLatchPin, HIGH);
}

void rotateLeft()
{
  setShifterOutput(ShiftOutputs::MotorA, true);
  setShifterOutput(ShiftOutputs::MotorB, false);
}

void rotateRight()
{
  setShifterOutput(ShiftOutputs::MotorA, false);
  setShifterOutput(ShiftOutputs::MotorB, true);
}

void rotateStop()
{
  setShifterOutput(ShiftOutputs::MotorA, false);
  setShifterOutput(ShiftOutputs::MotorB, false);
}

struct Laser
{
  int laserPin;
  int sensorPin;
  int threshold;
  //bool active = false;
  bool visible;
};

Laser chuteLaser = {
  chuteLaserPin,
  chuteLaserSensorPin
};

Laser reservoirLaser = {
  reservoirLaserPin,
  reservoirLaserSensorPin
};

Laser lasers[Lasers::_COUNT] = {chuteLaser, reservoirLaser};

bool updateLaser(Lasers id)
{
  Laser& laser = lasers[id];

  digitalWrite(laser.laserPin, HIGH);
  delay(5);

  //int value = analogRead(laser.sensorPin);
  int value = digitalRead(laser.sensorPin);
  Serial.print('0' + id);
  Serial.print(": ");
  Serial.println(value);
  laser.visible = value >= laser.threshold;

  //digitalWrite(laser.laserPin, LOW);

  return laser.visible;
}

void updateLasers()
{
  for(int x = 0; x < Lasers::_COUNT; x++)
  {
    updateLaser(x);
  }
}

void movePaddle(PaddleActions action)
{
  setShifterOutput(ShiftOutputs::ChuteLaser, true);

  chuteSensorTriggered = false;

  rotateRight();

  DispensingResult result = DispensingResult::Ok;

  int retryCount = 0;
  auto start = millis();
  const auto updateTimeout = [&]()
  {
    const auto now = millis();
    if(now - start > 800)
    {
      rotateLeft();
    
      delay(100);

      rotateRight();
    
      delay(100);
      
      start = millis();

      retryCount++;

      if(retryCount >= 10)
      {
        result = DispensingResult::PaddleStuck;
      }
    }
  };

  int openCount = 0;

  while(result == DispensingResult::Ok)
  {
    if(!digitalRead(positionSensorPin))
    {
      openCount++;
    }
    else
    {
      openCount = 0;
    }

    if(openCount >= 100)
    {
      break;
    }
    
    delay(1);
    
    updateTimeout();
  }

  positionPinLow = false;
  
  while(!positionPinLow && result == DispensingResult::Ok)
  {
    updateTimeout();
  }

  rotateLeft();
  
  delay(20);

  rotateStop();

  setShifterOutput(ShiftOutputs::ChuteLaser, false);

  if(!chuteSensorTriggered)
  {
    result = DispensingResult::NoOutput;
  }

  Serial.println(result);
}

void onPositionPinLow()
{
  positionPinLow = true;
}

void onChuteSensorTriggered()
{
  chuteSensorTriggered = true;
}

void setup() {
  Serial.begin(57600);
  Serial.println("Hello, world?");
  
  espSerial.begin(9600);
  //espSerial.println("Hello, world?");

  pinMode(reservoirLaserPin, OUTPUT);
  pinMode(chuteLaserPin, OUTPUT);

  pinMode(paddlePin1, OUTPUT);
  pinMode(paddlePin2, OUTPUT);

  pinMode(positionSensorPin, INPUT);
  
  pinMode(shiftDataPin, OUTPUT);
  pinMode(shiftClockPin, OUTPUT);  
  pinMode(shiftLatchPin, OUTPUT);

  setShifterOutput(0, 0);

  attachInterrupt(digitalPinToInterrupt(positionSensorPin), onPositionPinLow, RISING);

  attachInterrupt(digitalPinToInterrupt(chuteLaserSensorPin), onChuteSensorTriggered, FALLING);
  
  while(mainSerial == nullptr)
  {
    if(espSerial.available() > 0)
    {
      mainSerial = &espSerial;
    }
    else if(Serial.available() > 0)
    {
      mainSerial = &Serial;
    }
  }
}

bool waitForInput()
{
  while(mainSerial->available() <= 0)
  {
  }

  return true;
}

bool expectComma()
{
  waitForInput();
  if(mainSerial->peek() == ',')
  {
    mainSerial->read();
    return true;
  }

  return false;
}

bool expectInt(long& ret, bool trash = true)
{
  if(trash)
  {
    while(waitForInput() && !isDigit(mainSerial->peek()))
    {
      mainSerial->read();
    }
  }
  
  waitForInput();
  if(!isDigit(mainSerial->peek()))
  {
    return false;
  }
  
  ret = mainSerial->parseInt(SKIP_NONE);
  return true;
}

void dispense()
{
  movePaddle(PaddleActions::CLOCKWISE);
}

void parseCommands()
{
  long c = -1;
  if(!expectInt(c))
  {
    return;
  }

  bool success = true;
  
  if(c == Commands::ARD_Dispense)
  {
    mainSerial->println("dispense");
    dispense();
  }
  
  mainSerial->println("");
}

void loop()
{
  parseCommands();
}
