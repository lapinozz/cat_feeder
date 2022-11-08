#include "HX711.h"
#include <Servo.h> 
#include <Stepper.h>

#include "shared/shared.h"

#include <SoftwareSerial.h>
SoftwareSerial espSerial(2, 3); // RX, TX

Stream* mainSerial = nullptr;

constexpr int wheelServoPin = 6;
constexpr int wheelOpenAngle = 55;
constexpr int wheelCloseTightAngle = 160;
constexpr int wheelCloseLooseAngle = wheelCloseTightAngle - 7;
constexpr int wheelCloseLooserAngle = wheelCloseLooseAngle - 7;
constexpr int wheelAngles[WheelPosition::_COUNT] = {wheelOpenAngle, wheelCloseLooserAngle, wheelCloseLooseAngle, wheelCloseTightAngle};

constexpr int splitterServoPin = 7;
constexpr int splitterLeftAngle = 0;
constexpr int splitterRightAngle = 130;
constexpr int splitterAngles[SplitterPosition::_COUNT] = {splitterLeftAngle, splitterRightAngle};

constexpr int splitterLeftMaxWeight = 0;

Servo servo; 

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 14;
const int LOADCELL_SCK_PIN = 15;

HX711 scale;
float lastScaleValue = 0;

float leftFillWeight = 850;
float rightFillWeight = leftFillWeight * 2;

void setWheelPosition(WheelPosition pos, int delayTime = 500)
{
    servo.attach(wheelServoPin);
    servo.write(wheelAngles[pos]);
    delay(delayTime);
    servo.detach();
}

void setSplitterPosition(SplitterPosition pos, int delayTime = 500)
{
    servo.attach(splitterServoPin);
    servo.write(splitterAngles[pos]);
    delay(delayTime);
    servo.detach();
}

bool updateScale(bool tare = false)
{
    if (scale.is_ready())
    {
      //scale.power_down();
/*
      //setWheelPosition(WheelPosition::CLOSE_TIGHT, 100);
      for(int x = 0; x < 5; x++)
      {
        setWheelPosition(WheelPosition::CLOSE_LOOSE, 200);
        setWheelPosition(WheelPosition::CLOSE_LOOSER, 200);
      }

      delay(1000);
  */  
      //scale.power_up();
      //delay(500);

      if(tare)
      {
        //scale.tare(1);
        scale.tare(1);
        lastScaleValue = 0;
      }
      else
      {
        //lastScaleValue = scale.get_units(1);
        //delay(2000);
        lastScaleValue = scale.get_units(1);
      }

      return true;
    }

    return false;
}

void dispense(int turns = 10)
{
  const int stepsPerRevolution = 2038;
  
  const int stepperPin1 = 8;
  const int stepperPin2 = 10;
  const int stepperPin3 = 9;
  const int stepperPin4 = 11;
  
  Stepper stepper = Stepper(stepsPerRevolution, stepperPin1, stepperPin2, stepperPin3, stepperPin4);
  for(int x = 0; x < turns; x++)
  {
    stepper.setSpeed(10);
    stepper.step(-stepsPerRevolution / 4);
    
    stepper.setSpeed(10);
    stepper.step(200);
  }
  
  pinMode(stepperPin1, INPUT);
  pinMode(stepperPin2, INPUT);
  pinMode(stepperPin3, INPUT);
  pinMode(stepperPin4, INPUT);
}

void fillAndDrop()
{
  setWheelPosition(WheelPosition::CLOSE_LOOSE);
  
  updateScale(true);

  setSplitterPosition(SplitterPosition::LEFT);
  while(lastScaleValue < leftFillWeight)
  {
    dispense();
    updateScale();
    mainSerial->println(lastScaleValue, 5);
  }
  
  setSplitterPosition(SplitterPosition::RIGHT);
  while(lastScaleValue < rightFillWeight)
  {
    dispense();
    updateScale();
    mainSerial->println(lastScaleValue, 5);
  }

  setWheelPosition(WheelPosition::OPEN);
}

void setup() {
  Serial.begin(57600);
  Serial.println("Hello, world?");
  
  espSerial.begin(9600);
  //espSerial.println("Hello, world?");
  
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  const float callibrationFactor = 286;
  const float callibrationWeight = 13.5;
 
  //scale.set_scale(callibrationFactor / callibrationWeight);
  //scale.tare();
  //scale.set_gain(128);

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

void parseCommands()
{
  long c = -1;
  if(!expectInt(c))
  {
    return;
  }
  
  Serial.println((int)c);
  if(c == 0)
  {
    return;
  }

  bool success = true;
  
  if(c == Commands::ARD_SetWheelPosition)
  {
    WheelPosition pos = WheelPosition::OPEN;

    if(expectComma())
    {
      long ret;
      if(expectInt(ret))
      {
        pos = static_cast<WheelPosition>(ret);
      }
    }
    
    setWheelPosition(pos);
  }
  else if(c == Commands::ARD_SetSplitterPositio)
  {
    SplitterPosition pos = SplitterPosition::LEFT;

    if(expectComma())
    {
      long ret;
      if(expectInt(ret))
      {
        pos = static_cast<SplitterPosition>(ret);
      }
    }
    
    setSplitterPosition(pos);
  }
  else if(c == Commands::ARD_Weight)
  {
    updateScale();
    mainSerial->println(lastScaleValue, 5);
  }
  else if(c == Commands::ARD_Tare)
  {
    updateScale(true);
  }
  else if(c == Commands::ARD_ResetTare)
  {
    scale.set_scale();
  }
  else if(c == Commands::ARD_Dispense)
  {
    dispense();
  }
  else if(c == -1)
  {
    if (scale.is_ready())
    {
      scale.set_scale();    
      mainSerial->println("Tare... remove any weights from the scale.");
      delay(5000);
      scale.tare(20);
      mainSerial->println("Tare done...");
      mainSerial->print("Place a known weight on the scale...");
      delay(5000);
      long reading = scale.get_units(20);
      mainSerial->print("Result: ");
      mainSerial->println(reading);
    } 
    else
    {
      mainSerial->println("HX711 not found.");
    }
    delay(1000);
  }
  else if(c == 11)
  {
    fillAndDrop();
  }
  
  mainSerial->println("READY");
}

void loop()
{
  parseCommands();
}
