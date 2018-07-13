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

String state="@";


void setup() 
{
  Serial.begin(9600);
  
  pinMode(pinCS, OUTPUT);
  
  pinMode(okay,INPUT);
  pinMode(up,INPUT);
  pinMode(down,INPUT);
    
  lcd.init();                 // initialize the lcd 
  lcd.backlight();
  lcd.createChar(0, upDown);
  lcd.createChar(1, highlighted);


  //Display Welcom Screen
  lcd.setCursor(0,0);
  lcd.print(" Sprouter  V1.0");
  lcd.setCursor(0,1);
  lcd.print("    Welcome!");
  delay(400);
  lcd.clear();

  //Display sd checking notification
  lcd.home();
  lcd.print("  Checking for");
  lcd.setCursor(0,1);
  lcd.print("  User Data...");
  delay(400);
  lcd.clear();

  if(!checkSD()) //Check to make sure SD Card is installed
  {
    //Print the error prompt
    lcd.home();
    lcd.print(" SD Card Missin");
    lcd.setCursor(0,1);
    lcd.print("   Check it");
    lcd.setCursor(4,1);
    
    while(!checkSD())
    {
      delay(1000);
    }
  }
  lcd.clear();

  if(!checkForData()) //Checks to see if data on SD
  {
    //Print informational prompt to user
    lcd.home();
    lcd.print(" Data Not Found");
    lcd.setCursor(0,1);
    lcd.print("  Creating New");
    lcd.setCursor(4,1);
    delay(400);
    lcd.clear();

    //Have user select Germ Duration
    lcd.home();
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

    //Have user select Germ Duration
    lcd.home();
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

    //TODO SAVE DATA***********************************
    if(sproutOrSeed()==1) //Start from Seed
    {
      loadSeed();
      delay(500);
      lcd.clear();
      state="G";
    }
    else
    {
      nextWatering();
      state="S";
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
      loadSeed();
      delay(500);
      lcd.clear();
      state="G";

    }
    else
    {
      nextWatering();
      state="S";
    }
  }
  mainMenu();
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
void loadSeed()
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
void nextWatering()
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
    return nextWater;
}
void mainMenu()
{
  lcd.clear();
  lcd.home();
  lcd.print("State:  TTW:   m");
  lcd.setCursor(0,1);
  lcd.print("    Opt   New");
  lcd.setCursor(6,0);
  lcd.print(state);
  delay(200);
}
void loop() 
{
  

}
