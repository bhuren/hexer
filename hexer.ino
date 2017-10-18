#include <IRremote.h>
#include <RGBMood.h>



const byte select1[] = {2, 3, 4};
const byte select2[] = {5, 8, 7}; // pins to select 3021 x2
const byte redPin = 9;
const byte greenPin = 6;
const byte bluePin = 10; // RGB LED Pins
const byte RECV_PIN = 11; //Infrared Sensor Pin
RGBMood m(redPin, greenPin, bluePin);

int xp1 = 0;
int xp2 = 0;
int xp3 = 0; //xtra pots with dedicated analogPin's
byte control_num = 0; // Controller number in given channel
int value; // current reading of respective potentiometer
int readings[16]; // an array to store value
byte pin; // pins to be activated during 3021 read
byte analogPin; // analogPin to be chosen in the getValue function
unsigned long t2 = 0;


boolean ctrl_page_change = false; // flag to determine if remote controls controller pages (in sections of 16)
boolean controller_change = false; // flag to determine selection of individual numbers
boolean on = false;
boolean fine_tune;
boolean play = false;
boolean record = false;



// value of RGB Pins
byte c;
byte n; // controller number
byte x; 
byte mmc_flag = 0;
unsigned long t1 = 0;  //timer for first significant read

byte channel = 0xb0; // channel number
byte pre_controller = 0xb0; //previous chosen controller
byte controller = 0; // current controller;
byte pre_reading; // previous value

IRrecv irrecv(RECV_PIN);
decode_results results;



int getValue(byte controller)                   //function that reads value of specific controller
{

  for (byte bit = 0; bit < 3; bit++)
  {
    if (controller < control_num + 8) {
      pin = select1[bit];
      analogPin = 0;
    }
    else {
      pin = select2[bit];
      analogPin = 1;
    }

    byte isBitSet = bitRead(controller, bit); // true if given bit set in controller
    digitalWrite(pin, isBitSet);
  }
  return analogRead(analogPin);
}




void setup()
{

  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  irrecv.enableIRIn();


  m.setHoldingTime(0);  // Keep the same color for 4 seconds before fading again.
  m.setFadingSteps(200); // 200 steps.
  m.setFadingSpeed(5); // 25 ms * 200 steps = 5 seconds.
  m.setHSB(random(359), 120, 80);

  for (byte bit = 0; bit < 3; bit++) {
    pinMode(select1[bit], OUTPUT);
    pinMode(select2[bit], OUTPUT);
  }// set the three select pins to output


  fine_tune = false;
  mmc_flag = 0;

  Serial.begin(115200);


}


void loop () {



  if (irrecv.decode(&results)) {                    // if IR Sensor detects a valid signal

    switch (results.value) {

      case 2704:                                   //Power button on Remote
        on = !on;
        break;

      case 1488:                                   //button besides it
        fine_tune = !fine_tune;
        break;

      case 2320:                                    //button '0' -'9'
        pre_controller = 0;
        break;

      case 16:
        if (controller_change) {
          pre_controller = 1;
        }
        else if (ctrl_page_change) control_num = 0;
        else channel = 0xb0;
        break;

      case 2064:
        if (controller_change) {
          pre_controller = 2;
        }
        else if (ctrl_page_change) control_num = 16;
        else channel = 0xb1;
        break;

      case 1040:

        if (controller_change) {
          pre_controller = 3;
        }
        else if (ctrl_page_change) control_num = 32;
        else channel = 0xb2;
        break;


      case 3088:

        if (controller_change) {
          pre_controller = 4;
        }
        else if (ctrl_page_change) control_num = 48;
        else channel = 0xb3;
        break;

      case 528:

        if (controller_change) {
          pre_controller = 5;
        }
        else if (ctrl_page_change) control_num = 64;
        else channel = 0xb4;
        break;


      case 2576:
        if (controller_change) {
          pre_controller = 6;
        }
        else if (ctrl_page_change) control_num = 80;
        else channel = 0xb5;
        break;

      case 1552:
        if (controller_change) {
          pre_controller = 7;
        }
        else if (ctrl_page_change) control_num = 96;
        else channel = 0xb6;
        break;

      case 3600:
        if (controller_change) {
          pre_controller = 8;
        }
        else if (ctrl_page_change) control_num = 112;
        else channel = 0xb7;
        break;

      case 272:
        if (controller_change) {
          pre_controller = 9;
        }
        else channel = 0xb8;
        break;

      case 144:                                                            //Program change UP
        if (ctrl_page_change && control_num <= 96) control_num += 16;
        else if (controller_change) pre_controller++;
        else if (channel < 0xbf) channel++;
        break;

      case 2192:
        if (ctrl_page_change && control_num >= 16) control_num -= 16;       //Program change DOWN
        else if (controller_change) pre_controller--;
        else if (channel > 0xb0) channel--;
        break;

      case 3728:                                                            //controller change button
        ctrl_page_change = false;
        controller_change = true;
        break;

      case 2640:                                                           //controller page change button
        ctrl_page_change = true;
        controller_change = false;
        break;

      case 464:                                                            //channel change - disables controller and page change
        ctrl_page_change = false;
        controller_change = false;
        break;

      case 1168:                                                           // controller value UP

        if (c < 127) {
          c++;
          Serial.write(byte(channel));
          Serial.write(byte(pre_controller));
          Serial.write(byte(c));
          m.setRGB(c, 127-c, 0);
          t1 = millis();
        }

        break;

      case 3216:                                                            // controller value DOWN
        if (c > 0) {
          c--;
          Serial.write(byte(channel));
          Serial.write(byte(pre_controller));
          Serial.write(byte(c));
          m.setRGB(c, 127-c, 0);
          t1 = millis();
        }

        break;


      case 2872:  // to stop         //MMC Messages
        mmc_flag = 0x01;
        play = false;
        break;

      case 824: // to play
         mmc_flag = 0x02;
         play = !play;
        break;

      case 1848:  // to start record
        mmc_flag = 0x06;  
        record = !record;
        break;

      case 3896:  // to stop record
        mmc_flag = 0x07;
        record = false;
        break;

      case 720: // forward
        mmc_flag = 0x05;
        break;

      case 3280: // rewind
        mmc_flag = 0x04;
        break;


    }
    if (mmc_flag != 0) {                         //Send the MMC Commands
      
      Serial.write(byte(0xf0));
      Serial.write(byte(0x7f));
      Serial.write(byte(0x00));
      Serial.write(byte(0x06));
      Serial.write(byte(mmc_flag));
      Serial.write(byte(0xf7));
      mmc_flag = 0;
    }

    irrecv.resume(); // Receive the next value
  }

  if (on) {   //

    xp1 =  analogRead(A2);
    xp2 =  analogRead(A3);
    xp3 =  analogRead(A4);     // read the xpots


    if ((millis() - t1 >= 2000)) {   // 2 seconds wait before default RGB initiation
      if (record || play) {

        unsigned long currentMillis = millis();

        if (currentMillis - t2 >= 1000) {            // 1 second invert for LEDs
          t2 = currentMillis;
          if (x >= 120) x = 0;
          else x = 120;

          if (record) m.fadeRGB(x, 0, 0);            // if record - glow the RED LED  
          else if (play) m.fadeRGB(0, x, 0);  
          else if (record && play) m.fadeRGB(x, 0, 0);        //if play - glow the GREEN LED                                 
                                            
        }

      }
      // if record is on; LED Glows and diffuses RED every second -else Indicate Channel numbers on RGB LED
      else {
        
        switch (channel) {
          case 0xb0:
            m.setMode(RGBMood::BLUE_MODE);
            break;
          case 0xb1:
            m.setMode(RGBMood::ORANGE_MODE);
            break;
          case 0xb2:
            m.setMode(RGBMood::AQUA_MODE);
            break;
          case 0xb3:
            m.setMode(RGBMood::LIME_MODE);
            break;
          case 0xb4:
            m.setMode(RGBMood::VIOLET_MODE);
            break;
          case 0xb5:
            m.setMode(RGBMood::GREEN_MODE);
            break;
          case 0xb6:
            m.setMode(RGBMood::PURPLE_MODE);
            break;
          case 0xb7:
            m.setMode(RGBMood::YELLOW_MODE);
            break;
          case 0xb8:
           m.setMode(RGBMood::RED_MODE);
            break;

        }



      }
    }
    else m.setRGB(c, 127-c, 0);                    // If 2 seconds not over after the significant read; indicate the Value using RGB LED


    for (controller = control_num; controller < control_num + 16 ; controller++)         //start reading the values one-by-one of each controller
    {
      value = getValue(controller);                                                      //get the value of that specific controller
      n = controller - control_num;                                                      // controller number in given page

      if ((sq(value - readings[n]) > 64) && (pre_reading - value / 8) != 0)              // if there is a significant reading change and previous value is not equal to incoming value
      {


        pre_reading = readings[n] / 8;
        readings[n] = value;

        if (pre_controller != controller) pre_controller = controller;                    // if it is the first single read on a controller; do not print the value!

        else {                                                                            // if it is not the first-read, then document the read

          if (fine_tune) readings[n] = map(readings[n], 0, 1023, xp3, xp2);               //during fine tune, values of controllers are limited between value of xpot 2 & 3

          c = readings[n] / 8;
          
          Serial.write(byte(channel));
          Serial.write(byte(controller));
          Serial.write(byte(c));
          
          m.setRGB(c, 127-c, 0);
          t1 = millis();
          //pre_controller = controller;                               //EXP*******

        }

      }

    }

  }
  else if (!on) {

    m.setMode(RGBMood::RANDOM_HUE_MODE);
   
  }

  m.tick();
}

