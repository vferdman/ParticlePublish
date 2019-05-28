#include "application.h"
#line 1 "c:/Users/Vladimir/Documents/Particle_IO/Photon/Publish/src/Publish.ino"
// -----------------------------------------
// Publish and Dashboard with Photoresistors
// -----------------------------------------
// This app will publish an event when the beam of light between the LED and the photoresistor is broken.
// It will publish a different event when the light is intact again.

void setup();
void loop();
#line 7 "c:/Users/Vladimir/Documents/Particle_IO/Photon/Publish/src/Publish.ino"
int led = D0;       // This is where your LED is plugged in. The other side goes to a resistor connected to GND.
int boardLed = D7;  // This is the LED that is already on your device.

int photoresistor = A0; // This is where your photoresistor is plugged in. The other side goes to the "power" pin (below).

int power = A5; // This is the other end of your photoresistor. The other side is plugged into the "photoresistor" pin (above).

// The following values get set up when your device boots up and calibrates:
int intactValue;  // This is the average value that the photoresistor reads when the beam is intact.
int brokenValue;  // This is the average value that the photoresistor reads when the beam is broken.
int beamThreshold;  // This is a value halfway between ledOnValue and ledOffValue, above which we will assume the led is on and below which we will assume it is off.

bool beamBroken = false; // This flag will be used to mark if we have a new status or now. We will use it in the loop.

#define REPORT_INTERVAL 500
int loopCount = REPORT_INTERVAL;  // interval at wich to report photoresistor value


void setup() {
  pinMode(led,OUTPUT); // Our LED pin is output (lighting up the LED)
  pinMode(boardLed,OUTPUT); // Our on-board LED is output as well
  pinMode(photoresistor,INPUT);  // Our photoresistor pin is input (reading the photoresistor)
  pinMode(power,OUTPUT); // The pin powering the photoresistor is output (sending out consistent power)

  // Next, write the power of the photoresistor to be the maximum possible, which is 4095 in analog.
  digitalWrite(power,HIGH);

  // Start by calibrating our photoresistor.
  // This one is going to require some input from the user!

  // First, the D7 LED will go on to tell you to put your hand in front of the beam.
  digitalWrite(boardLed,HIGH);
  delay(5000);

  // Then, the D7 LED will go off and the LED will turn on.
  digitalWrite(boardLed,LOW);
  digitalWrite(led,HIGH);
  delay(500);

  // Now we'll take some readings...
  int off_1 = analogRead(photoresistor); // read photoresistor
  delay(200); // wait 200 milliseconds
  int off_2 = analogRead(photoresistor); // read photoresistor
  delay(300); // wait 300 milliseconds

  // Now flash to let us know that you've taken the readings...
  digitalWrite(boardLed,HIGH);
  delay(500);
  digitalWrite(boardLed,LOW);
  delay(500);
  digitalWrite(boardLed,HIGH);
  delay(500);
  digitalWrite(boardLed,LOW);
  delay(500);

  // Now the D7 LED will go on to tell you to remove your hand...
  digitalWrite(boardLed,HIGH);
  delay(3000);

  // The D7 LED will turn off...
  digitalWrite(boardLed,LOW);

  // ...And we will take two more readings.
  int on_1 = analogRead(photoresistor); // read photoresistor
  delay(200); // wait 200 milliseconds
  int on_2 = analogRead(photoresistor); // read photoresistor
  delay(1000); // wait 1 second

  // Now flash the D7 LED on and off three times to let us know that we're ready to go!
  digitalWrite(boardLed,HIGH);
  delay(500);
  digitalWrite(boardLed,LOW);
  delay(500);
  digitalWrite(boardLed,HIGH);
  delay(500);
  digitalWrite(boardLed,LOW);
  delay(500);
  digitalWrite(boardLed,HIGH);
  delay(500);
  digitalWrite(boardLed,LOW);


  // Now we average the "on" and "off" values to get an idea of what the resistance will be when the LED is on and off
  intactValue = (on_1+on_2)/2;
  brokenValue = (off_1+off_2)/2;

  // Let's also calculate the value between ledOn and ledOff, above which we will assume the led is on and below which we assume the led is off.
  beamThreshold = (intactValue+brokenValue)/2;


  // Send a publish...
  Particle.publish("Intact Value",String(intactValue),60,PRIVATE);
  Particle.publish("Broken Value",String(brokenValue),60,PRIVATE);
  Particle.publish("Beam Threshold",String(beamThreshold),60,PRIVATE);

}

//
// Main Loop
//

void loop() {

  int photoresistorValue;
  
  /* In this loop function, we're going to check to see if the beam has been broken.
  When the status of the beam changes, we'll send a Particle.publish() to the cloud
  so that if we want to, we can check from other devices when the LED is on or off.

  We'll also turn the D7 LED on when the Photoresistor detects a beam breakagse.
  */
  photoresistorValue = analogRead(photoresistor);
  
  // report the value if the interval has passed
  if (loopCount-- == 0) {
    Particle.publish("Photoresistor Value",String(photoresistorValue),60,PRIVATE);
    loopCount = REPORT_INTERVAL;
  }

  if (photoresistorValue>beamThreshold) {

    /* If you are above the threshold, we'll assume the beam is intact.
    If the beam was intact before, though, we don't need to change anything.
    We'll use the beamBroken flag to help us find this out.
    This flag monitors the current status of the beam.
    After the beam is broken, it is set TRUE
    and when the beam reconnects it is set to FALSE.
    */

    if (beamBroken==true) {
        // If the beam was broken before, then this is a new status.
        // We will send a publish to the cloud and turn the LED on.

        // Send a publish to your devices...
        Particle.publish("beamStatus","intact",60,PRIVATE);
        // And flash the on-board LED on and off.
        digitalWrite(boardLed,LOW);

        // Finally, set the flag to reflect the current status of the beam.
        beamBroken=false;
    }
  }
  else {
      // If you are below the threshold, the beam is probably broken.
      if (beamBroken==false) {

        // Send a publish...
        Particle.publish("beamStatus","broken",60,PRIVATE);
        // And flash the on-board LED on and off.
        digitalWrite(boardLed,HIGH);

        // Finally, set the flag to reflect the current status of the beam.
        beamBroken=true;
      }
  }
}
