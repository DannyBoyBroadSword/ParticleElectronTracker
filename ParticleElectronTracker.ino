// This #include statement was automatically added by the Particle IDE.
#include <Adafruit_SSD1306.h>

/* -----------------------------------------------------------
This example shows a lot of different features. As configured here
it will check for a good GPS fix every 10 minutes and publish that data
if there is one. If not, it will save you data by staying quiet. It also
registers 3 Particle.functions for changing whether it publishes,
reading the battery level, and manually requesting a GPS reading.
---------------------------------------------------------------*/

// Getting the library
#include "AssetTracker.h"

bool hasFix FALSE; 

std::string myData;

#define OLED_RESET D7
Adafruit_SSD1306 display(OLED_RESET);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2


#define LOGO16_GLCD_HEIGHT 16 
#define LOGO16_GLCD_WIDTH  16 
static const unsigned char logo16_glcd_bmp[] =
{ 0B00000000, 0B11000000,
  0B00000001, 0B11000000,
  0B00000001, 0B11000000,
  0B00000011, 0B11100000,
  0B11110011, 0B11100000,
  0B11111110, 0B11111000,
  0B01111110, 0B11111111,
  0B00110011, 0B10011111,
  0B00011111, 0B11111100,
  0B00001101, 0B01110000,
  0B00011011, 0B10100000,
  0B00111111, 0B11100000,
  0B00111111, 0B11110000,
  0B01111100, 0B11110000,
  0B01110000, 0B01110000,
  0B00000000, 0B00110000 };

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif



// Set whether you want the device to publish data to the internet by default here.
// 1 will Particle.publish AND Serial.print, 0 will just Serial.print
// Extremely useful for saving data while developing close enough to have a cable plugged in.
// You can also change this remotely using the Particle.function "tmode" defined in setup()
int enablePin = D7;

//bool fixFlag = TRUE;

int transmittingData = 1;

// Used to keep track of the last time we published data
long lastPublish = 0;

// How many minutes between publishes? 10+ recommended for long-time continuous publishing!
int delaySeconds = 1200;

// Creating an AssetTracker named 't' for us to reference
AssetTracker t = AssetTracker();

// A FuelGauge named 'fuel' for checking on the battery state
FuelGauge fuel;

// setup() and loop() are both required. setup() runs once when the device starts
// and is used for registering functions and variables and initializing things
void setup() {
    
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  // init done
   display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.println("Waiting for DATA!");
    display.display();
  
  
  
  
    pinMode(enablePin,OUTPUT);
    
    digitalWrite(enablePin, HIGH);
    // Sets up all the necessary AssetTracker bits
    t.begin();

    // Enable the GPS module. Defaults to off to save power.
    // Takes 1.5s or so because of delays.
    t.gpsOn();

    // Opens up a Serial port so you can listen over USB
    Serial.begin(9600);
    
   
    

    // These three functions are useful for remote diagnostics. Read more below.
    Particle.function("tmode", transmitMode);
    Particle.function("batt", batteryStatus);
    Particle.function("gps", gpsPublish);
    Particle.function("alt",altPublish);
    Particle.function("acc",accPublish);
    Particle.function("SP",speedPublish);
    Particle.function("D",dPublish);
    Particle.function("F",chgFreq);
    Particle.function("S",sendLoc);
    Particle.function("SD",sendData);
    
    Particle.subscribe("hook-response/D", readLoc, MY_DEVICES);
    

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.println("Waiting for Data");
    display.println(String::format("Battery Voltage"+String::format("%.2f",fuel.getVCell())));
    display.println(String::format("Battery Percent"+String::format("%.2f",fuel.getSoC())+"%"));
    display.display();
    
    
        
    //delay(5000);
    
    //digitalWrite(enablePin,LOW);
}

// loop() runs continuously
void loop() {
    // You'll need to run this every loop to capture the GPS output
    t.updateGPS();

    // if the current time - the last time we published is greater than your set delay...
    if (millis()-lastPublish > delaySeconds*1000) {
        digitalWrite(enablePin, HIGH);
        
        //delay(15000);
        
        
        // Remember when we published
        lastPublish = millis();

        //String pubAccel = String::format("%d,%d,%d", t.readX(), t.readY(), t.readZ());
        //Serial.println(pubAccel);
        //Particle.publish("A", pubAccel, 60, PRIVATE);

        // Dumps the full NMEA sentence to serial in case you're curious
        Serial.println(t.preNMEA());

        // GPS requires a "fix" on the satellites to give good data,
        // so we should only publish data if there's a fix
        
        
        if (t.gpsFix()) {
            // Only publish if we're in transmittingData mode 1;
            if (transmittingData) {
                // Short publish names save data!
                hasFix = TRUE;
                Particle.publish("G", t.readLatLon(), 60, PRIVATE);
                Particle.publish("D",  String::format("%.6f",t.readLatDeg())+","+String::format("%.6f",t.readLonDeg())+","+String::format("%.2f",fuel.getVCell()) +","+
          String::format("%.2f",fuel.getSoC())+","+String::format("%.2f",t.getGpsAccuracy())+","+String::format("%.2f",t.getSpeed())+","+String::format("%.2f",t.getAltitude())
         , 60, PRIVATE);
                //Particle.publish("SP", String::format("%.2f",t.getSpeed()), 60, PRIVATE);
                
                
                //digitalWrite(enablePin,LOW);
            }
            // but always report the data over serial for local development
            //Serial.println(t.readLatLon());
        } else{
            hasFix = FALSE;
        }
        
        //digitalWrite(enablePin, HIGH);
    } else if (hasFix == FALSE){
            display.clearDisplay();
            display.setTextSize(1);
            display.setTextColor(WHITE);
            display.setCursor(0,0);
            display.println("Waiting for Data");
            display.println(String::format("Battery Voltage:"));
            display.println(String::format("%.2f",fuel.getVCell()));
            display.println(String::format("Battery Percent:"));
            display.println(String::format("%.2f",fuel.getSoC())+" Percent");
            display.display();
    }
}

int sendLoc(String command){
    return 1;
}

int sendData(String command){
    hasFix = TRUE;
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.println(command);
    display.display();
    return 1;
}

void readLoc(const char *event, const char *data) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.println(data);
    display.println(String::format("Battery Voltage:"));
    display.println(String::format("%.2f",fuel.getVCell()));
    display.println(String::format("Battery Percent:"));
    display.println(String::format("%.2f",fuel.getSoC())+" Percent");
    display.display();
    Particle.publish("S",data);
}

// Allows you to remotely change whether a device is publishing to the cloud
// or is only reporting data over Serial. Saves data when using only Serial!
// Change the default at the top of the code.
int transmitMode(String command) {
    transmittingData = atoi(command);
    return 1;
}

int chgFreq(String command){
    delaySeconds = atoi(command);
    return 1;
}

// Actively ask for a GPS reading if you're impatient. Only publishes if there's
// a GPS fix, otherwise returns '0'
int gpsPublish(String command) {
    //digitalWrite(enablePin, HIGH);
    //delay(4000);
    
    if (t.gpsFix()) {
        Particle.publish("G", t.readLatLon(), 60, PRIVATE);
        //delay(1000);
        //digitalWrite(enablePin, LOW);
        //fixFlag = FALSE;
        // uncomment next line if you want a manual publish to reset delay counter
        // lastPublish = millis();
        return 1;
    } else {
      //delay(1000);
      //fixFlag = TRUE;
      //digitalWrite(enablePin, HIGH);
      return 0;
    }
}

int dPublish(String command) {
    //digitalWrite(enablePin, HIGH);
    //delay(4000);
    
    if (t.gpsFix()) {
        Particle.publish("D",  String::format("%.6f",t.readLatDeg())+","+String::format("%.6f",t.readLonDeg())+","+String::format("%.2f",fuel.getVCell()) +","+
          String::format("%.2f",fuel.getSoC())+","+String::format("%.2f",t.getGpsAccuracy())+","+String::format("%.2f",t.getSpeed())+","+String::format("%.2f",t.getAltitude())
         , 60, PRIVATE);
        //delay(1000);
        //digitalWrite(enablePin, LOW);
        //fixFlag = FALSE;
        // uncomment next line if you want a manual publish to reset delay counter
        // lastPublish = millis();
        return 1;
    } else {
      //delay(1000);
      //fixFlag = TRUE;
      //digitalWrite(enablePin, HIGH);
      return 0;
    }
}

int altPublish(String command) {
    //digitalWrite(enablePin, HIGH);
    //delay(4000);
    
    if (t.gpsFix()) {
        Particle.publish("A", String::format("%.2f",t.getAltitude()), 60, PRIVATE);
        //delay(1000);
        //digitalWrite(enablePin, LOW);
        //fixFlag = FALSE;
        // uncomment next line if you want a manual publish to reset delay counter
        // lastPublish = millis();
        return 1;
    } else {
      //delay(1000);
      //fixFlag = TRUE;
      //digitalWrite(enablePin, HIGH);
      return 0;
    }
}

int speedPublish(String command) {
    //digitalWrite(enablePin, HIGH);
    //delay(4000);
    
    if (t.gpsFix()) {
        Particle.publish("SP", String::format("%.2f",t.getSpeed()), 60, PRIVATE);
        //delay(1000);
        //digitalWrite(enablePin, LOW);
        //fixFlag = FALSE;
        // uncomment next line if you want a manual publish to reset delay counter
        // lastPublish = millis();
        return 1;
    } else {
      //delay(1000);
      //fixFlag = TRUE;
      //digitalWrite(enablePin, HIGH);
      return 0;
    }
}

int accPublish(String command) {
    //digitalWrite(enablePin, HIGH);
    //delay(4000);
    
    if (t.gpsFix()) {
        Particle.publish("AC", String::format("%.2f",t.getGpsAccuracy()),60,PRIVATE);
        //delay(1000);
        //digitalWrite(enablePin, LOW);
        //fixFlag = FALSE;
        // uncomment next line if you want a manual publish to reset delay counter
        // lastPublish = millis();
        return 1;
    } else {
      //delay(1000);
      //fixFlag = TRUE;
      //digitalWrite(enablePin, HIGH);
      return 0;
    }
}



// Lets you remotely check the battery status by calling the function "batt"
// Triggers a publish with the info (so subscribe or watch the dashboard)
// and also returns a '1' if there's >10% battery left and a '0' if below
int batteryStatus(String command){
    // Publish the battery voltage and percentage of battery remaining
    // if you want to be really efficient, just report one of these
    // the String::format("%f.2") part gives us a string to publish,
    // but with only 2 decimal points to save space
    Particle.publish("B",
          "V:"+String::format("%.2f",fuel.getVCell()) +"C:"+
          String::format("%.2f",fuel.getSoC()),60, PRIVATE
    );
    // if there's more than 10% of the battery left, then return 1
    if (fuel.getSoC()>10){ return 1;}
    // if you're running out of battery, return 0
    else { return 0;}
}


