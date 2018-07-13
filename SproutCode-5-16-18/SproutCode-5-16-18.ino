#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <SPI.h>

LiquidCrystal_I2C lcd(0x3F, 16, 2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

//Custom characters generated for LCD
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

const int up    =A2;                        //Controller button pin values
const int down  =A3;                        //"
const int okay  =A0;                        //"
const int back  =A1;                        //"
int btnStateOkay,btnStateDown,btnStateUp,btnStateBack=0; //Button state values

const int motor1Pin1  =4; //Fill motor
const int motor1Pin2  =5;

const int motor2Pin1  =6; //Drain Motor
const int motor2Pin2  =7;

int cursorPosition=0; //Stores position of user nonQuantified LCD selections

int germDuration=8; //hours seed requires for germination
int waterCycles =2; //amount of times water fills&drains per day
int nextWater   =0; //Hours until next water, used once during initial setup

int reservoirFillTime =2000; 
int reservoirDrainTime=2000;

boolean fillReservoir,drainReservoir=false; //Flags for stage events
boolean timerActive                 =false; //Flag to show if timer is available for update

String state="@";   //Stores state of seed (Germination/Sprouting)
String stage="@";   //Stores stage of state (Fil/Drain/Soak/Dry)

unsigned long startTime       =0;   //Holds millisecond system start time
unsigned long interval        =0;   //Holds total Milliseconds of current stage
unsigned long currentMillis   =0;   //Used to hold current milliseconds of system since start
unsigned long timeToexecution =0;   //Holds milliseconds between current time and estimated execution time
unsigned long minutes,seconds =0;   //Holds millisecond conversions for LCD display


// __      _               
/// _\ ___| |_ _   _ _ __  
//\ \ / _ \ __| | | | '_ \ 
//_\ \  __/ |_| |_| | |_) |
//\__/\___|\__|\__,_| .__/ 
//                  |_|   

void setup() 
{
  Serial.begin(9600);     //Start Serial monitor for debugging
  
  pinMode(okay,INPUT);
  pinMode(up,INPUT);
  pinMode(down,INPUT);
  pinMode(back,INPUT);
  pinMode(motor1Pin1,OUTPUT);   
  pinMode(motor1Pin2,OUTPUT);   
  pinMode(motor2Pin1,OUTPUT);   
  pinMode(motor2Pin2,OUTPUT);   
    
  lcd.begin();      
  lcd.backlight();  
  
  lcd.createChar(0, upDown);      //Creates LCD's custom Chars
  lcd.createChar(1, highlighted); 

  ///////////////////////////////Display windows///////////////////////////////////////
  
  welcomeWindow();        //Display welcome screen
  germDurationWindow();   //Allow user to input germination Duration
  waterCycleWindow();     //Allow user to input waterCycles per day

  //germDuration=8;   //TestingVariables
  //waterCycles=2;    //"
  //nextWater=1;      //"
  //startTimer(1);    //"

  //Window prompting user to start cycle from sprout(7) or seed(1)
  if(sproutOrSeedWindow()==1) //Cursor position is 1, Start from Seed
    {
      loadSeedWindow();    
      state="Ger";stage="FIL";fillReservoir=true; //Set program position determined by result of previous window
    }
  else                       //Cursor position is 7, Start from Sprout
    {
      nextWateringWindow(); //Ask user when the next watering is
      state="Spr";stage="DRY";  //Set program position determined by result of previous window
      
      if(nextWater==0)      //User wants to water immediately
      {
        fillReservoir=true;
        stage="FIL"; 
       }
      else                  //User has set delay on watering 
      {
        startTimer(nextWater*60);
        //startTimer(.1); //TestingVariable
      }
    }
  
  mainMenu(); //Start main menu to display current variable and timers
}
void welcomeWindow()
{
  lcd.setCursor(0,0);             //Set cursor to first line
  lcd.print(" Sprouter  V1.0");   
  lcd.setCursor(0,1);             //Set cursor to second line
  lcd.print("    Welcome!");      
  delay(400);                     
  lcd.clear();                    
}
void germDurationWindow()
{
    //Allows user to set duration of soad for premature seeds
    lcd.home();                         
    lcd.print(" Germ Duration:");       
    lcd.setCursor(0,1);                 
    lcd.print("     ");                
    lcd.write(byte(0));                 //print up/down arrow char
    lcd.print("08HR");                  //print temporary value for user to change
    germDuration=8;                     //ensure default is set
    delay(400);                            

    btnStateOkay=digitalRead(okay);     //Read current state of okay button
    
    if(btnStateOkay==0)                 //Okay button is not pressed
    {
      Serial.println("Okay not pressed");
      while(btnStateOkay==0)            //While button isnt pressed, Read user up/down buttons
      {
        btnStateOkay=digitalRead(okay); //Grab button values
        btnStateDown=digitalRead(down); //"
        btnStateUp=digitalRead(up);     //"
        
        if(btnStateUp==1)               //User is attempting to increase germ Duration
          {
            delay(200);                 
            Serial.println("up pressed");

            if(germDuration!=99)        //Increase value of variable if below max threshold
              {germDuration+=1;}
          }
      
        if(btnStateDown==1)              //User is attempting to dencrease germ Duration
          {
            delay(200);
            Serial.println("down pressed");
            if(germDuration!=1)       //Decrease value of variable if above main threshold
              {germDuration-=1;}
          }
        
        lcd.setCursor(6,1);           //Set LCD cursor to print current germDuration value
        if(germDuration<10)           //Ensure germ duration displays cleanly on LCD
          {lcd.print("0");}
        lcd.print(germDuration);      //Print value to LCD
        delay(200);
      }  
    }
    else
    {
     Serial.println("Okay pressed");
    }
    lcd.clear();   
    delay(400);    //Display change to user so they depress Okay for next window   
}
int sproutOrSeedWindow()
{
    //Display Window prompts user to start cycle from sprout(7) or seed(1)
    lcd.home();                   //Go to start of the first line
    lcd.print("   Start from");   //Print 1st line
    lcd.setCursor(0,1);           //Go to the start of the second line
    lcd.print("  Seed  Sprout");  //Print options for user selection
    
    lcd.setCursor(1,1);   //Set cursor initial position
    lcd.write(1);         //"
    cursorPosition=1;     //"
    
    delay(400);           //Display text for atleast .4seconds           

    btnStateOkay=digitalRead(okay);         //Read in current state of Okay Button
    if(btnStateOkay==0)                     
    {
      Serial.println("Okay not pressed");
      while(btnStateOkay==0)                //While button isnt pressed, Read user up/down buttons
      {
        btnStateOkay=digitalRead(okay);     //Read in button values
        btnStateUp=digitalRead(up);         //"
        btnStateDown=digitalRead(down);     //"
        
        if(btnStateUp==1)
          {
            delay(200);
            Serial.println("up pressed");

            if(cursorPosition==1)         //Change cursor location and correspoding variable
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
        if(btnStateDown==1)                 //Change cursor location and correspoding variable
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

    lcd.clear();   //Clear LCD for future input
    delay(400);    //Display change to user so they depress Okay for next window   
    return cursorPosition;  //Return value of position to indicate State starting position
}
void waterCycleWindow()
{
    //Have user select Water Cycles
    lcd.home();                     //Go to start of the first line    
    lcd.print("  Water Cycles");    //Print first line
    lcd.setCursor(0,1);             //Go to start of the second line
    lcd.print("  Per Day: ");       //Print partial second line
    lcd.write(byte(0));             //print up/down arrow char
    lcd.print("2");                 //print initial value
    waterCycles=2;                  //ensure variable is set
    delay(400);                     //display screen for atleast .4 seconds 

    btnStateOkay=digitalRead(okay); //Read initial button state
    
    if(btnStateOkay==0)
    {
      Serial.println("Okay not pressed");
      while(btnStateOkay==0)              //While okay is not pressed, read up/down button presses
      {
        btnStateOkay=digitalRead(okay);   //Read in button values
        btnStateUp=digitalRead(up);       //"
        btnStateDown=digitalRead(down);   //"
        
        if(btnStateUp==1)
          {
            delay(200);
            Serial.println("up pressed");

            if(waterCycles!=8)          //Increase water cycle value if below max
              {waterCycles+=1;}
          }
       if(btnStateDown==1)
          {
            delay(200);
            Serial.println("down pressed");
            if(waterCycles!=1)          //Decrease water cycle if above min
              {waterCycles-=1;}
          }
        
        lcd.setCursor(12,1);    //Update to LCD current water cycle value
        lcd.print(waterCycles); //"
      }  
    }
    else
    {
     Serial.println("Okay pressed");
    }
    
    lcd.clear();   //Clear LCD for future input
    delay(400);    //Display change to user so they depress Okay for next window   
}
void loadSeedWindow()
{
  //Prompt user to load seeds into chamber
  lcd.home();                   //Go to start of 1st line
  lcd.print(" Load 2Tb Seeds"); //Print 1st line
  lcd.setCursor(6,1);           //Go to desired symbol location
  lcd.write(1);                 //Print indicator symbol
  lcd.print("OK");              //Print remainder of second line
  delay(400);                   //Display window for atleast .4 seconds

  btnStateOkay=digitalRead(okay); //Read in current ok button state
  
  if(btnStateOkay==0)
  {
    Serial.println("Okay not pressed");
    while(btnStateOkay==0)            //Wait for user to press okay
    {
      btnStateOkay=digitalRead(okay); //Read in current state of button
      delay(200);
    }    
  }
  else
  {
   Serial.println("Okay pressed");
  }
  lcd.clear();   //Clear LCD for future input
  delay(400);    //Display change to user so they depress Okay for next window   
}
void nextWateringWindow()
{
    //When user already has sprouts in chamber, ask them when they want to water next
    lcd.home();                   //Go to the start of the 1st line
    lcd.print(" Next Watering:"); //print first line
    lcd.setCursor(5,1);           //Go to desired position of second line starting
    lcd.write(byte(0));                 //Write updown symbol
    lcd.print("00Hr");            //print default value to LCD
    nextWater=0;                  //Ensure variable is set correctly
    delay(400);                   //Display window for atleast .4 seconds

    btnStateOkay=digitalRead(okay); //Read inital state of Okay button
    
    if(btnStateOkay==0)
    {
      Serial.println("Okay not pressed");
      while(btnStateOkay==0)                //While okay is not pressed, check up/down button changes
      {
        btnStateOkay=digitalRead(okay);     //Read in button values
        btnStateUp=digitalRead(up);         //"
        btnStateDown=digitalRead(down);     //"
        
        if(btnStateUp==1)
          {
            delay(200);
            Serial.println("up pressed");

            if(nextWater!=8)      //Increase variable value if below max
              {nextWater+=1;}
          }
        if(btnStateDown==1)
          {
            delay(200);
            Serial.println("down pressed");
            if(nextWater!=0)      //Decrease variable value if below max
              {nextWater-=1;}
          }
        
        lcd.setCursor(7,1);   //Print current value to LCD
        lcd.print(nextWater); //"
        delay(200);
      }  
    }
    else
    {
     Serial.println("Okay pressed");
    }
    
    lcd.clear();   //Clear LCD for future input
    delay(400);    //Display change to user so they depress Okay for next window 
}


//  /\/\   __ _(_)_ __     /\/\   ___ _ __  _   _ 
// /    \ / _` | | '_ \   /    \ / _ \ '_ \| | | |
/// /\/\ \ (_| | | | | | / /\/\ \  __/ | | | |_| |
//\/    \/\__,_|_|_| |_| \/    \/\___|_| |_|\__,_|

void mainMenu()
{
  lcd.clear();                //Clear old value of mainMenu
  lcd.home();                 //Go to the start of the first line
  lcd.print("State:      ");  //Print partial 1st line
  lcd.write(1);               //Print cursor to show user option
  lcd.print("Opt");           //User selection opt
  lcd.setCursor(0,1);         //Go to the start of second line
  lcd.print(stage);           //Print stage value
  lcd.setCursor(6,0);         //Go to start of second line
  lcd.print(state);           //Print state value
  delay(200);                 //Display window for atleast .2 seconds
}
void event(String message)
{
  //Notification of state transition
  lcd.clear();        //Clear old LCD value
  lcd.home();         //Go to the start of first line
  lcd.print(message); //Print message
}
void startTimer(double dur)
{
  //Starts a timer for X minutes
  Serial.println(" ");
  Serial.print("TimerStarted for ");
  Serial.print(dur);
  Serial.println(" min");
  
  currentMillis = millis(); //Read in current system ontime
  startTime=currentMillis;  //Declare startTime of timer
  interval=dur*(60000);     //Calculate millisecond interval of timer duration

  timerActive=true; //Declare flag that tells loop() to update timer
}
void getMinSec(unsigned long inTime)
{
  //Converts timer status from milliseconds to min/sec
  seconds=inTime/1000;  //Convert milliseconds to seconds
  minutes=0;            //ensure minutes value 0
  String secs="";       //String result variable to print to LCD
  String mins="";       //"
  
  if(seconds>=60) //Converts total seconds into min/seconds
  {
    minutes=seconds/60;           //Declare minutes value from seconds
    seconds=seconds-(minutes*60); //Update new seconds value
  }

  if(seconds<10)  //Cleans single second values to clean LCD write
  {secs+="0";}
  secs+=seconds;  //Write seconds string to variable

  if(minutes==0)
  {
    mins="    ";
  }
  else
  {
    if(minutes<100) //Cleans minutes values to clean LCD write
    {
      mins+=" ";
      if(minutes<10)
      {mins+=" ";}   
    }
    mins+=minutes;//Write actual min value to string variable
    mins+="m";    //Write units
  }

  lcd.setCursor(9,1);   //Go to LCD min location
  lcd.print(mins);      //Print String value of minutes
  lcd.setCursor(13,1);  //Go to LCD sec location
  lcd.print(secs);      //Print String value of seconds
  
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
      lcd.setCursor(9,1);   //Go to LCD timer start location
      lcd.print("       "); //Clear data
    }
  }
}
void motorState(int mNum,int onOff)
{
  if(mNum==1)
  {
    if(onOff==1)//Turn on
    {
      Serial.println("m1ON");
      digitalWrite(motor1Pin1,LOW);
      digitalWrite(motor1Pin2,HIGH);
    }
    else        //Turn off
    {
      Serial.println("m1OFF");
      digitalWrite(motor1Pin1,LOW);
      digitalWrite(motor1Pin2,LOW);
    }
  }
  else //mNum==2
  {
    if(onOff==1)//Turn on
    {
      Serial.println("m2ON");
      digitalWrite(motor2Pin1,HIGH);
      digitalWrite(motor2Pin2,LOW);
    }
    else        //Turn off
    {
      digitalWrite(motor2Pin1,LOW);
      digitalWrite(motor2Pin2,LOW);
    }
  }
}
void motorBoardTest()
{
      digitalWrite(motor1Speed,HIGH);
      digitalWrite(motor1Pin1,HIGH);
      digitalWrite(motor1Pin2,HIGH);
      
      digitalWrite(motor2Pin1,HIGH);
      digitalWrite(motor2Pin2,HIGH);
      digitalWrite(motor2Speed,HIGH);
}
double getWaterCycleInterval()
{
  //Calculate dry state interval from water cycles per day 
  double result= 24.0/waterCycles;  //Divide hours in day by number of intervals
  result=result*60;                 //Convert to minutes
  return result;                    
}
void stageUpdate()
{
  //Updates program state to tell program what is next after completed timer
  Serial.print("Stage updating");
  Serial.print(state);Serial.print(stage);Serial.print("=>");
  if(state == "Spr")
        {
          if(stage == "FIL")
          {
            stage="SOA";    
            //startTimer(2); //Set timer for time sprouts soak in water before draining
            startTimer(.1); //TESTING
          }
          else if(stage== "SOA")
          {
            stage="DRA";
            drainReservoir=true;  //Set flag to indicate need for drain
          }
          else if(stage== "DRA")
          {
            stage="DRY";
            //startTimer(getWaterCycleInterval());
            startTimer(.1); //TestingVariable
          }
          else //(stage== "DRY")
          {
            stage="FIL";
            fillReservoir=true;  //Set flag to indicate need for fill
          }
        }
  else //(state == "Ger")
        {
          if(stage == "FIL")
          {
            stage="SOA";
            //startTimer(germDuration*60); //Set timer for time seeds germinate in water
            startTimer(.1); //TestingVariable
          }
          else if(stage == "SOA")
          {
            stage="DRA";
            state="Spr";
            drainReservoir=true;  //Set flag to indicate need for drain
          }
        }
   Serial.print(state);Serial.println(stage);
}


//   __                   
//  / /  ___   ___  _ __  
// / /  / _ \ / _ \| '_ \ 
/// /__| (_) | (_) | |_) |
//\____/\___/ \___/| .__/ 
//                 |_|     
                     
void loop() 
{ 
  delay(1000);
  ////////////////////////////Check for flag hits/////////////////////////////////
  if(fillReservoir==true)               
      { 
        Serial.println("Filling Chamber");
        event("Filling Chamber");   //Print notification of action
        motorState(1,1);            //Turn motor1 On      
        motorState(2,0);            //Ensure motor2 Off
        delay(reservoirFillTime);   //Delay for determined fill time
        fillReservoir=false;        //Deactivate flag
        drainReservoir=false;       //ensure it doesn't loop
        motorState(1,0);            //Turn motor1 Off     
        motorState(2,0);            //Ensure motor2 Off

        stageUpdate();  //Update state of program
        mainMenu();     //Return to main menu
      }
  if(drainReservoir==true)
      {
        Serial.println("Drainin Chamber");
        event("Draining Chamber");  //Print notification of action
        motorState(1,0);            //Ensure motor1 Off
        motorState(2,1);            //Turn motor2 On
        delay(reservoirDrainTime);  //Delay for determed drain time
        fillReservoir=false;        //ensure it doesn't loop
        drainReservoir=false;       //Deactivate flag
        motorState(1,0);            //Turn motor1 Off     
        motorState(2,0);            //Ensure motor2 Off

        stageUpdate();  //Update state of program
        mainMenu();     //Return to main menu
      }
  /////////////////////////////////////////////////////////////////////////////////
     
  if(timerActive)
  {
    currentMillis=millis(); //Get current system time
    
    if (currentMillis - startTime >= interval) //Timer has completed
    {
        Serial.println("TIMER COMPLETE");
        timerActive=false;    //Deactivate flag
        lcd.setCursor(13,1);  //Go to start of time
        lcd.print("     ");   //Clear timer residue

        stageUpdate();  //Update state of program
    }
    else  //update timer
    {
      timeToexecution=interval-(currentMillis - startTime); //Store remaining time of timer in milliseconds to variable 

      getMinSec(timeToexecution); //Relay timer change to LCD
      delay(1000);                //Delay 1second between updates
      //TODO when using buttons change previous delay
    }
  }
  //check for button presses
  //upon going into button press state and back, update timer accordingly
}
