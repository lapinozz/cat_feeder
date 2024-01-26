#include "shared/shared.h"

#include <SoftwareSerial.h>
#include <Servo.h>

SoftwareSerial espSerial(6, 7); // RX, TX

Stream* mainSerial = nullptr;

constexpr int positionSensorPin = 2;

constexpr int chuteLaserPin = A5;
constexpr int chuteLaserSensorPin = 3;

constexpr int reservoirLaserPin = 4; 
constexpr int reservoirLaserSensorPin = A4;

volatile bool positionPinLow = false;
volatile bool chuteSensorTriggered = false;

constexpr int motorPinA = 11;
constexpr int motorPinB = 12;

constexpr int servoPin = 9;
constexpr int servoPositions[PositionCount] = {1, 180 / 2 - 8, 180};

constexpr auto servoMin = MIN_PULSE_WIDTH + 30;
constexpr auto servoMax = MAX_PULSE_WIDTH + 120;

Servo servo;

void rotateLeft(int speed = 255)
{
  analogWrite(motorPinA, 255 - speed);
  digitalWrite(motorPinB, HIGH);
}

void rotateRight(int speed = 255)
{
  analogWrite(motorPinA, speed);
  digitalWrite(motorPinB, LOW);
}

void rotateStop()
{
  digitalWrite(motorPinA, LOW);
  digitalWrite(motorPinB, LOW);
}

void moveServo(int pos, int waitDelay = 500)
{      
  if(pos < 0 || pos > PositionCount)
  {
    return;
  }

  servo.attach(servoPin, servoMin, servoMax);
  servo.write(servoPositions[pos]);
  delay(waitDelay);
  servo.detach();
}

void onPositionPinLow()
{
  positionPinLow = true;
}

void onChuteSensorTriggered()
{
  chuteSensorTriggered = true;
}

bool isReservoirLow()
{
    digitalWrite(reservoirLaserPin, LOW);
    delay(10);

    bool signal = false;
    for(int x = 0; x < 10; x++)
    {
      signal = signal || digitalRead(reservoirLaserSensorPin);
      delay(1);
    }

    digitalWrite(reservoirLaserPin, HIGH);
    
    return signal;
}

void setup()
{
  Serial.begin(57600);
  Serial.println("Hello, world?");
  
  espSerial.begin(9600);

  //espSerial.println("Hello, world?");

  pinMode(reservoirLaserPin, OUTPUT);

  pinMode(chuteLaserPin, OUTPUT);
  digitalWrite(chuteLaserPin, LOW);

  pinMode(chuteLaserSensorPin, INPUT);

  pinMode(positionSensorPin, INPUT);

  pinMode(motorPinA, OUTPUT);
  pinMode(motorPinB, OUTPUT);
  
  rotateStop();

  attachInterrupt(digitalPinToInterrupt(positionSensorPin), onPositionPinLow, RISING);

  attachInterrupt(digitalPinToInterrupt(chuteLaserSensorPin), onChuteSensorTriggered, FALLING);

  for (int pos = 0; pos < PositionCount; pos++)
  {
    moveServo(pos);
  }

  Serial.println(isReservoirLow() ? "low" : "not low");

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

  Serial.println("Connected");
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

DispensingResult dispense()
{
  DispensingResult result = DispensingResult::Ok;
    
  digitalWrite(chuteLaserPin, HIGH);
  chuteSensorTriggered = false;

  delay(100);

  const auto chuteJammed = !digitalRead(chuteLaserSensorPin);
  if(chuteJammed)
  {
    return DispensingResult::FoodJammed;
  }

  int retryCount = 0;
  auto start = millis();
  const auto updateTimeout = [&]()
  {
    const auto now = millis();
    if(now - start > 4200)
    {
      rotateLeft();
    
      delay(200);

      rotateRight();
    
      delay(200);
      
      start = millis();

      retryCount++;

      if(retryCount >= 5)
      {
        result = DispensingResult::PaddleStuck;
      }
    }
  };

  int openCount = 0;
  bool clearedInitialPosition = false;
  bool inFinalPosition = false;
  const auto updatePositionDetection = [&]()
  {
    if(!clearedInitialPosition)
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
        clearedInitialPosition = true;
        positionPinLow = false;
      }
    }
    else
    {
      if(positionPinLow)
      {
        inFinalPosition = true;        
      }
    }
  };
  
  /*
  const int movementStepsCount = 4;
  const int movementStepsDirection[movementStepsCount]  = {1,     1,    -1,    0};
  const int movementStepsDuration[movementStepsCount]   = {50,    200,   30,   600};
  const int movementStepsSpeed[movementStepsCount]      = {255,   255/2, 255,  0};
  */
  
  /*
  const int movementStepsCount = 2;
  const int movementStepsDirection[movementStepsCount]  = {1,     0};
  const int movementStepsDuration[movementStepsCount]   = {80,    500};
  const int movementStepsSpeed[movementStepsCount]      = {255,   0};
  int currentMovementStep = 0;
  */
  
  const int movementStepsCount = 1;
  const int movementStepsDirection[movementStepsCount]  = {1};
  const int movementStepsDuration[movementStepsCount]   = {3000};
  const int movementStepsSpeed[movementStepsCount]      = {255};
  int currentMovementStep = 0;
  
  auto lastMovementUpdate = millis();
  const auto updateMovement = [&]()
  {
    const auto duration = movementStepsDuration[currentMovementStep];

    const auto now = millis();
    if(now - lastMovementUpdate > duration)
    {
      lastMovementUpdate = now;
      currentMovementStep++;
      if(currentMovementStep >= movementStepsCount)
      {
        currentMovementStep = 0;
      }
    }

    const auto movement = movementStepsDirection[currentMovementStep];
    const auto speed = movementStepsSpeed[currentMovementStep];

    if(movement == 1)
    {
      rotateRight(speed);
    }
    else if(movement == -1)
    {
      rotateLeft(speed);
    }
    else if(movement == 0)
    {
      rotateStop();
    }
  };

  while(result == DispensingResult::Ok && !inFinalPosition)
  {
    delay(1);

    updateTimeout();
    updatePositionDetection();
    updateMovement();
  }

  rotateLeft();
  
  delay(20);

  rotateStop();
  
  delay(1000);

  if(!chuteSensorTriggered)
  {
    result = DispensingResult::NoOutput;
  }

  digitalWrite(chuteLaserPin, LOW);

  delay(100);
  
  return result;
}

void parseCommands()
{
  long c = -1;
  if(!expectInt(c))
  {
    return;
  }

  if(c == Commands::ARD_Dispense)
  {
    long amount = 0;
    expectInt(amount);

    long positionsPacked = 0;
    expectInt(positionsPacked);

    int positions[PositionCount] = {};

    int actualPositionCount = 0;
    for(int x = 0; x < PositionCount; x++)
    {
        if((positionsPacked >> x) & 1)
        {
            positions[actualPositionCount++] = x;
        }
    }
    
    for (int x = 0; x < actualPositionCount - 1; x++)
    {
        const auto index = random(0, actualPositionCount - x);

        const auto temp = positions[index];
        positions[index] = positions[x];
        positions[x] = temp;
    }

    mainSerial->print(+Commands::ARD_Dispense);
    mainSerial->print(" ");
    mainSerial->println(+DispensingResult::Started);

    DispensingResult errors[3];

    for (int x = 0; x < actualPositionCount; x++)
    {
      moveServo(positions[x]);

      for(int y = 0; y < amount; y++)
      {
        const auto result = dispense();
        if(result != DispensingResult::Ok)
        {
          const auto errorIndex = result - (DispensingResult::Ok + 1);
          errors[errorIndex] = errors[errorIndex] + 1;
        }
      }
    }

    mainSerial->print(+Commands::ARD_Dispense);
    for(int x = 0; x < 3; x++)
    {
      mainSerial->print(" ");
      mainSerial->print(+errors[x]);
    }

    mainSerial->println("");
  }
  else if(c == Commands::ARD_Reservoir)
  {
    const auto isLow = isReservoirLow();

    mainSerial->print(+Commands::ARD_Reservoir);
    mainSerial->print(" ");
    mainSerial->println(isLow ? 0 : 1);
  }
}

void loop()
{
  parseCommands();
}
