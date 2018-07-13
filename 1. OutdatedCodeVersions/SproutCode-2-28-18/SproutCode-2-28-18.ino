#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <SD.h>
#include <SPI.h>

File myFile;

LiquidCrystal_I2C lcd(0x3F,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display

byte upDown[8] = {
  0b00100,
  0b01110,
  0b11111,
  0b00100,
  0b00100,
  0b11111,
  0b01110,
  0b00100
};
byte highlighted[] = {
  B00000,
  B00000,
  B01110,
  B01111,
  B01111,
  B01110,
  B00000,
  B00000
};

int pinCS = 2; // Pin 10 on Arduino Uno
int okay=A0;
int up=A2;
int down=8;

int btnStateOkay,btnStateDown,btnStateUp=0;
int cursorPosition=0;

int germDuration=8; //default (hrs)
int waterCycles=2; //default (times water fill/drain per day)
int nextWater=0;  //hrs to next water

int reservoirFillTime=2000;
int reservoirDrainTime=2000;
boolean fillReservoir,drainReservoir=false;

String state="@";
String stage="@";

unsigned long startTime = 0;
unsigned long currentMillis=0;
unsigned long timeToexecution = 0;
unsigned long interval=0;

unsigned long minutes,seconds=0;
boolean timerActive=false;



// ____ __________ _    ____ 
/// ___/  __/__ __/ \ //  __\
//|    |  \   / \ | | ||  \/|
//\___ |  /_  | | | \_/|  __/
//\____\____\ \_/ \____\_/  


void setup() 
{
  Serial.begin(9600);  //Start Serial monitor for debugging
  
  pinMode(pinCS, OUTPUT); //Pinmode for SD select
  pinMode(okay,INPUT);    //Button inputs
  pinMode(up,INPUT);      //
  pinMode(down,INPUT);    //
    
  lcd.init();                     // initialize the lcd 
  lcd.backlight();                //  
  lcd.createChar(0, upDown);      //Create LCD's custom Chars
  lcd.createChar(1, highlighted); //

  /////////////////////////////////////////////////////////////////////////////////////////
  lcd.setCursor(0,0);             //Display Welcome Screen
  lcd.print(" Sprouter  V1.0");
  lcd.setCursor(0,1);
  lcd.print("    Welcome!");
  delay(400);
  lcd.clear();

  SDCardVerify();  //verify sd is installed, check for data, load or create if not

  //Testing Variables
  //germDuration=8;
  //waterCycles=2;
  //nextWater=1;
  //startTimer(1);
  
  mainMenu();
}

void SDCardVerify()
{
  lcd.home();                     //Display SD checking notification
  lcd.print("  Checking for");
  lcd.setCursor(0,1);
  lcd.print("  User Data...");
  delay(400);
  lcd.clear();
  
  if(!checkSD())                  //Check to make sure SD Card is installed
  {
    lcd.home();                   //Print the error prompt
    lcd.print(" SD Card Missin");
    lcd.setCursor(0,1);
    lcd.print("   Check it");
    lcd.setCursor(4,1);
    
    while(!checkSD())             //Hold in screen until SD card detected
    {delay(1000);}
  }
  lcd.clear();                    //Clear input to LCD
  
  //////////////////////////////////////////////////////////////////////////
  if(!checkForData())             //Checks to see if data on SD
  {
    lcd.home();                   //Print info prompt to user
    lcd.print(" Data Not Found");
    lcd.setCursor(0,1);
    lcd.print("  Creating New");
    lcd.setCursor(4,1);
    delay(400);
    lcd.clear();                 //Clear input to LCD
    
    germDurationWindow();               //Allow user to input germination Duration
    waterCycleWindow();                 //Allow user to input waterCycles per day
    //TODO SAVE DATA
    ///////////////////////////////////////////////////////////////////
    if(sproutOrSeed()==1) //Start from Seed
    {
      loadSeedWindow();
      delay(500);
      lcd.clear();
      state="Ger";
      stage="FIL";
      fillReservoir=true;
    }
    else
    {
      nextWateringWindow();
      state="Spr";
      stage="DRY";
      if(nextWater==0)
      {
        fillReservoir=true;
      }
      else
      {
        startTimer(nextWater*60);
        //startTimer(.1); //Testing
      }
    }
    
  }
  else
  {
    lcd.home();
    lcd.print("  Data Found");
    lcd.setCursor(0,1);
    lcd.print("   Loading...");
    delay(2000);
    //TODO Load Data ******************
    
    if(sproutOrSeed()==1) //Start from Seed
    {
      loadSeedWindow();
      delay(500);
      lcd.clear();
      state="Ger";
      stage="FIL";      
      fillReservoir=true;


    }
    else
    {
      nextWateringWindow();
      state="Spr";
      stage="DRY";
      if(nextWater==0)
      {
        fillReservoir=true;
      }
      else
      {
        startTimer(nextWater*60);
        //startTimer(.1); //Testing
      }
    }
  }
}
boolean checkSD() //Checks to see if SD inserted
{
  
  if (SD.begin())
  {
    return true;
  } 
  else
  {
    return false;
  }
}

boolean checkForData() //Checks to see if data on SD
{
  
  myFile = SD.open("user.txt", FILE_READ);

  if (myFile) 
  {
    myFile.close(); // close the file
    return true;
  }
      // if the file didn't open, print an error:
  else 
  {
    return false;
  }
}

void germDurationWindow()
{
    lcd.home();                         //Have user select Germ Duration
    lcd.print(" Germ Duration:");
    lcd.setCursor(0,1);
    lcd.print("     ");
    lcd.write(byte(0)); //print up/down arrow char
    lcd.print("08HR");
    germDuration=8; //ensure default is set
    delay(1000);    

    btnStateOkay=digitalRead(okay);
    if(btnStateOkay==0)
    {
      Serial.println("Okay not pressed");
      while(btnStateOkay==0)
      {
        btnStateOkay=digitalRead(okay);
        btnStateUp=digitalRead(up);
        btnStateDown=digitalRead(down);
        
        if(btnStateUp==1)
          {
            delay(200);
            Serial.println("up pressed");

            if(germDuration!=99)
              {germDuration+=1;}
          }
      
       if(btnStateDown==1)
          {
            delay(200);
            Serial.println("down pressed");
            if(germDuration!=1)
              {germDuration-=1;}
          }
        
        lcd.setCursor(6,1);
        
        if(germDuration<10)
          {
            lcd.print("0");
          }
        lcd.print(germDuration);
        
        delay(200);
      }  
      
    }
    else
    {
     Serial.println("Okay pressed");
    }
    lcd.clear();
}
int sproutOrSeed()
{
    lcd.home();
    lcd.print("   Start from");
    lcd.setCursor(0,1);
    lcd.print("  Seed  Sprout");
    lcd.setCursor(1,1);
    lcd.write(1);
    cursorPosition=1;
    delay(2000);

    btnStateOkay=digitalRead(okay);
    if(btnStateOkay==0)
    {
      Serial.println("Okay not pressed");
      while(btnStateOkay==0)
      {
        btnStateOkay=digitalRead(okay);
        btnStateUp=digitalRead(up);
        btnStateDown=digitalRead(down);
        
        if(btnStateUp==1)
          {
            delay(200);
            Serial.println("up pressed");

            if(cursorPosition==1)
            {
              lcd.setCursor(1,1);
              lcd.print(" ");
              cursorPosition=7;
              lcd.setCursor(cursorPosition,1);
              lcd.write(1);
            }
            else
            {
              lcd.setCursor(7,1);
              lcd.print(" ");
              cursorPosition=1;
              lcd.setCursor(cursorPosition,1);
              lcd.write(1);

            }
            delay(200);
          }
      
       if(btnStateDown==1)
          {
            delay(200);
            Serial.println("down pressed");
            if(cursorPosition==1)
            {
              lcd.setCursor(1,1);
              lcd.print(" ");
              cursorPosition=7;
              lcd.setCursor(cursorPosition,1);
              lcd.write(1);

            }
            else
            {
              lcd.setCursor(7,1);
              lcd.print(" ");
              cursorPosition=1;
              lcd.setCursor(cursorPosition,1);
              lcd.write(1);

            }
            delay(200);
          }
       
        delay(200);
      }  
      
    }
    else
    {
     Serial.println("Okay pressed");
    }

    delay(1000);
    lcd.clear();
    return cursorPosition;
    
}
void waterCycleWindow()
{
  lcd.home();                         //Have user select Water Cycles
    lcd.print("  Water Cycles");
    lcd.setCursor(0,1);
    lcd.print("  Per Day: ");
    lcd.write(byte(0)); //print up/down arrow char
    lcd.print("2");
    waterCycles=2; //ensure default is set
    delay(1000); 

    btnStateOkay=digitalRead(okay);
    if(btnStateOkay==0)
    {
      Serial.println("Okay not pressed");
      while(btnStateOkay==0)
      {
        btnStateOkay=digitalRead(okay);
        btnStateUp=digitalRead(up);
        btnStateDown=digitalRead(down);
        
        if(btnStateUp==1)
          {
            delay(200);
            Serial.println("up pressed");

            if(waterCycles!=8)
              {waterCycles+=1;}
          }
      
       if(btnStateDown==1)
          {
            delay(200);
            Serial.println("down pressed");
            if(waterCycles!=1)
              {waterCycles-=1;}
          }
        
        lcd.setCursor(12,1);
        
        lcd.print(waterCycles);
        
        delay(200);
      }  
      
    }
    else
    {
     Serial.println("Okay pressed");
    }
    
    delay(1000);
    lcd.clear();
}
void loadSeedWindow()
{
  lcd.clear();
  lcd.home();
  lcd.print(" Load 2Tb Seeds");
  lcd.setCursor(0,1);
  lcd.print("       OK");
  lcd.setCursor(6,1);
  lcd.write(1);
  delay(200);

  btnStateOkay=digitalRead(okay);
  if(btnStateOkay==0)
  {
    Serial.println("Okay not pressed");
    while(btnStateOkay==0)
    {
      btnStateOkay=digitalRead(okay);
      delay(200);
    }    
  }
  else
  {
   Serial.println("Okay pressed");
  }
}
void nextWateringWindow()
{
      lcd.home();
      lcd.print(" Next Watering:");
      lcd.setCursor(0,1);
      lcd.print("     ");
      lcd.write(0);
      lcd.print("00Hr");
      delay(2000);
      nextWater=0;

    btnStateOkay=digitalRead(okay);
    if(btnStateOkay==0)
    {
      Serial.println("Okay not pressed");
      while(btnStateOkay==0)
      {
        btnStateOkay=digitalRead(okay);
        btnStateUp=digitalRead(up);
        btnStateDown=digitalRead(down);
        
        if(btnStateUp==1)
          {
            delay(200);
            Serial.println("up pressed");

            if(nextWater!=8)
              {nextWater+=1;}
          }
      
       if(btnStateDown==1)
          {
            delay(200);
            Serial.println("down pressed");
            if(nextWater!=0)
              {nextWater-=1;}
          }
        
        lcd.setCursor(7,1);
        
        lcd.print(nextWater);
        
        delay(200);
      }  
      
    }
    else
    {
     Serial.println("Okay pressed");
    }
}


// _     ____ _ _        _     ______     _    
/// \__//  _ / / \  /|  / \__//  __/ \  // \ /\
//| |\/|| / \| | |\ ||  | |\/||  \ | |\ || | ||
//| |  || |-|| | | \||  | |  ||  /_| | \|| \_/|
//\_/  \\_/ \\_\_/  \|  \_/  \\____\_/  \\____/

void mainMenu()
{
  lcd.clear();
  lcd.home();
  lcd.print("State:      ");
  lcd.write(1);
  lcd.print("Opt");
  lcd.setCursor(0,1);
  lcd.print(stage);
  lcd.setCursor(6,0);
  lcd.print(state);
  delay(200);
}
void event(String message)
{
  lcd.clear();
  lcd.home();
  lcd.print(message);
}
void startTimer(double dur)  //min
{
  Serial.println(" ");
  Serial.print("TimerStarted for ");
  Serial.print(dur);
  Serial.println(" min");
  currentMillis = millis();
  startTime=currentMillis;
  interval=dur*(60000);

  timerActive=true;
}
void getMinSec(unsigned long inTime)
{
  seconds=inTime/1000;
  minutes=0;
  String secs="";
  String mins="";
  
  if(seconds>=60)
  {
    minutes=seconds/60;
    seconds=seconds-(minutes*60);
  }

  if(seconds<10)
  {
    secs+="0";  
  }
  secs+=seconds;

  
  if(minutes<100)
  {
    mins+=" ";
    if(minutes<10)
    {
      mins+=" ";
    }
        
   }
  
  if(minutes!=0)
  {
      mins+=minutes;
      mins+="m";
  }
  else
  {
      mins="    ";
  }

  lcd.setCursor(9,1);
  lcd.print(mins);
  lcd.setCursor(13,1);
  lcd.print(secs);
  if(seconds!=0)
  {
    lcd.print("s");
  }
  else
  {
    if(minutes!=0)
    {
      lcd.print("s");
    }
    else
    {
      lcd.print(" ");
    }
  }
 
}
void motorState(int mNum,int onOff)
{
  //TODO make this do something
}
double getWaterCycleInterval()
{
  double result= 24.0/waterCycles;
  result=result*60; //convert to minutes
  return result;
}

void stageUpdate()
{
  Serial.println("Stage updating");
  if(state == "Spr")
        {
          if(stage == "FIL")
          {
            stage="SOA";
            //startTimer(2);
            startTimer(.1); //TESTING
          }
          else if(stage== "SOA")
          {
            stage="DRA";
            drainReservoir=true;
          }
          else if(stage== "DRA")
          {
            stage="DRY";
            startTimer(getWaterCycleInterval());
            //startTimer(.1); //TESTING
          }
          else //(stage== "DRY")
          {
            stage="FIL";
            fillReservoir=true;  
          }
        }
  else //(state == "Ger")
        {
          if(stage == "FIL")
          {
            stage="SOA";
            startTimer(germDuration*60);
            //startTimer(.1); //TESTING
          }
          else if(stage == "SOA")
          {
            stage="DRA";
            state="Spr";
            drainReservoir=true;
          }
        }
}


/// \  /  _ /  _ /  __\
//| |  | / \| / \|  \/|
//| |_/| \_/| \_/|  __/
//\____\____\____\_/   
                     
void loop() 
{ 
  
  ////////////////////////////Check for flag hits/////////////////////////////////
  if(fillReservoir==true)               
      { 
        Serial.println("Filling Reservoir");
        event("Filling Reservoir");
        motorState(1,1);
        motorState(2,0);
        delay(reservoirFillTime);
        fillReservoir=false;
        drainReservoir=false; //ensure it doesn't loop

        stageUpdate();
        
        mainMenu();
      }
  if(drainReservoir==true)
      {
        Serial.println("Drain Reservoir");
        event("Draining Reservoir");
        motorState(1,0);
        motorState(2,1);
        delay(reservoirDrainTime);
        fillReservoir=false;  //ensure it doesn't loop
        drainReservoir=false;

        stageUpdate();
        
        mainMenu();
      }
      
  if(timerActive)
  {
    currentMillis=millis();
    if (currentMillis - startTime >= interval) //Timer has completed
    {
        Serial.println("TIMER COMPLETE");
        timerActive=false;
        lcd.setCursor(13,1);
        lcd.print("     ");

        stageUpdate();

        //TODO print state/stage to LCD
    }
    else  //update timer
    {
      /////////////////////////////////////Update timer//////////////////////////////////
      timeToexecution=interval-(currentMillis - startTime); 

      getMinSec(timeToexecution);
      delay(1000);  //TODO when using buttons change this delay
    }
  }
  //when timer hits specific threshold, do required action
  //check for button presses
  //upon going into button press state and back, update timer accordingly
  
}
