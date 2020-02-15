#include <LiquidCrystal.h>
#include <OneWire.h> 
#include <DallasTemperature.h>

// defines pins numbers (This is for the distance sensor
const int trigPin = 8;
const int echoPin = 9;

//Initialization code for the te
#define ONE_WIRE_BUS 7 //Pin where temp sensor data pin is connected
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const int buttonPin = A2;

const int numTimerOptions = 6;
const int timerOptions [numTimerOptions] = {1, 5, 10, 15, 20, 30};

// defines variables
long duration;
int distance;

float temp;

//Variables used for keeping track which menu we are in
bool inSettings;
bool adjustingTimer;

//Keep track of the button presses
bool buttonPressed;
bool previousButtonPressed;

int selectedTimerIndex;
int buttonVal;

//Spray variables
int lastSpray; //Last time sprayed in minutes since start
int maxSprays = 2400;
int remainingSprays = maxSprays;

void setup() {
  lastSpray = -timerOptions[numTimerOptions-1]; //Initialize the max spray in such a way that we can immideatly use the device and we dont have to wait for a delay to pass
  
  selectedTimerIndex = 2; //Set the default timer index
  inSettings = false; //Default state is in normal mode
  adjustingTimer = false;
  
  Serial.begin(9600);

  buttonPressed = false;
  
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Turn off the cursor:
  lcd.noCursor();
  //Set the input pin for the buttons
  pinMode(buttonPin, INPUT);

  //Start the temperature sensor
  sensors.begin();
}

void loop() {
  
  //Keep track of the buttons, handled here since it's needed for both loops
  previousButtonPressed = buttonPressed;
  buttonVal = analogRead(buttonPin);
  buttonPressed = buttonVal < 1020;

  //Choose which loop we have to run
  if(inSettings)
    settingsMode();
  else
    normalMode();
}

//Loop used for normal/ in use mode
void normalMode()
{
  //Measure temp with temp sensor
  sensors.requestTemperatures(); // Send the command to get temperature readings
  temp = sensors.getTempCByIndex(0); //read the temperature value
  delay(100); //minimum delay needed for temp sensor = 25

  //////////////////////////////////////////
  //Type of toilet use detection code here//
  //////////////////////////////////////////

  //If we have pressed a button in the previous loop, we do not want to handle the input again
  if(!previousButtonPressed){
  //Button input detection
    if(buttonVal < 6){
      //Button 1 - Reset the amount of sprays (maybe add a timer that you have to hold the button down for 3 seconds to avoid accidental pressing
      remainingSprays = maxSprays;
    }
    else if (buttonVal < 520 && buttonVal > 500){
      //Button 2 - manual spray override
      spray();
    }
    else if (buttonVal < 700 && buttonVal > 670)
    {
      //Button 3 - go into the settings mode
      inSettings = true; 
    }
  }
  
  //Display remaining sprays
    lcd.setCursor(0,0);

    //Get the correct alignment for the text based on the spray number length
    if(remainingSprays > 999){
      lcd.print("Sprays left " + String(remainingSprays));
    }
    else if(remainingSprays > 99){
      lcd.print("Sprays left  " + String(remainingSprays));
    }
    else if(remainingSprays > 0){
      lcd.print("Sprays left   " + String(remainingSprays)); 
    }
    else
      lcd.print("Spray can empty");

  //Display temp
  lcd.setCursor(0,1);
  lcd.print("Temperature " + String(temp)); 
 
}

//Loop for the settings menu
void settingsMode(){
  if(adjustingTimer){
    //Adjusting timer
      if(!previousButtonPressed){ //Dont handle inputs if the button is already down
        
        if(buttonVal < 6){
          //Move through the timerIndices from right to left
            if(selectedTimerIndex > 0){
              selectedTimerIndex--;
            }
            
            else //If we are at 0 then move to the right-most index
              selectedTimerIndex = numTimerOptions-1;
          }
      
          else if (buttonVal < 520 && buttonVal > 500){
            //Confirm our current spray delay, and exit the adjusting mode
            adjustingTimer = false;
      
          }
          else if (buttonVal < 700 && buttonVal > 670)
          {
            //Move through the timerIndices left to right
            if(selectedTimerIndex < numTimerOptions - 1){
              selectedTimerIndex++;
            }
          else{ //If we reached the end of our array, go back to the first index
            selectedTimerIndex = 0;
          }
        }
     }
  
    //Display spray timer
    lcd.setCursor(0,0);
    lcd.print("Selecting  Delay");

    //Adjust the alignment based on the length of the delay text
    lcd.setCursor(0,1);
    if(selectedTimerIndex > 1)
      lcd.print("<      " + String(timerOptions[selectedTimerIndex]) + "      >");
    else
      lcd.print("<      " + String(timerOptions[selectedTimerIndex]) + "       >");
  }
  else{
    //Not adjusting the timer
    if(!previousButtonPressed){
        if(buttonVal < 6){
              //Return back to normal mode
              inSettings = false;
          }
         else if (buttonVal < 520 && buttonVal > 500){
            //Enter the mode in which we can change the spray delay
            adjustingTimer = true;
      
          }
        }
    lcd.setCursor(0,0);
    lcd.print("  Spray  Delay  ");

    lcd.setCursor(0,1);
    //Different alignment for different number lenghts
    if(selectedTimerIndex > 1)
      lcd.print("       " + String(timerOptions[selectedTimerIndex]) + "       ");
    else
      lcd.print("       " + String(timerOptions[selectedTimerIndex]) + "        ");
   }
}

void spray(){
  if(remainingSprays > 0)
  {
    lastSpray = millis() / 60000; //60k milisecs in a minute
    remainingSprays--;
  }
}
