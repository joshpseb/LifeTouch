#include <Wire.h>
#include <Adafruit_MMA8451.h>
#include <SD.h>
#include <TMRpcm.h>
#include <SoftwareSerial.h>

#define ACC_ADDR 0x1D 
#define SD_ChipSelectPin 4 
#define SPEAKER_PIN 9 
#define BUTTON_PIN_1 2
#define BUTTON_PIN_2 3 
Adafruit_MMA8451 accelerometer = Adafruit_MMA8451();
TMRpcm audioPlayer;
SoftwareSerial bluetooth(10, 11);


unsigned long buttonPressTime = 0;
bool button1Pressed = false;
bool button2Pressed = false;

void setup() {
  Serial.begin(9600);
  bluetooth.begin(9600);
  pinMode(BUTTON_PIN_1, INPUT_PULLUP);
  pinMode(BUTTON_PIN_2, INPUT_PULLUP);

  if (!accelerometer.begin()) {
    Serial.println("Could not find accelerometer, check wiring!");
    while (1);
  }
  accelerometer.setRange(MMA8451_RANGE_2_G);

  if (!SD.begin(SD_ChipSelectPin)) {
    Serial.println("SD initialization failed!");
    return;
  }

  audioPlayer.speakerPin = SPEAKER_PIN;
  audioPlayer.setVolume(6);
}

void loop() {
  checkButtons();

  sensors_event_t event;
  accelerometer.getEvent(&event);
  float acceleration = sqrt(event.acceleration.x * event.acceleration.x +
                            event.acceleration.y * event.acceleration.y +
                            event.acceleration.z * event.acceleration.z);

  if (acceleration < 6.0) {
    //playSound("track1.wav");
  } else if (acceleration >= 6.0 && acceleration < 9.0) {
    playSound("track2.wav");
    bluetooth.println("Do you want to call EMS?");
    checkButtons();
  } else {
    playSound("track1.wav");
    bluetooth.println("EMS are on their way");
  }

  delay(1000);
}

void checkButtons() {
  if (digitalRead(BUTTON_PIN_1) == HIGH && button1Pressed && !button2Pressed) {
    buttonPressTime = millis();
    button1Pressed = true;
    Serial.println("Button 1 pressed");
  }

  if (digitalRead(BUTTON_PIN_2) == HIGH && button2Pressed && !button1Pressed) {
    buttonPressTime = millis();
    button2Pressed = true;
    Serial.println("Button 2 pressed");
  }

  if ((button1Pressed || button2Pressed) && millis() - buttonPressTime >= 10000) {
    if (button1Pressed || button2Pressed) {
      playSound("track2.wav");
      bluetooth.println("Cancelling the EMS Call");
    } else {
      playSound("track1.wav");
      bluetooth.println("Calling EMS");
    }
    button1Pressed = false;
    button2Pressed = false;
  }
}

void playSound(const char* soundName) {
  if (!SD.exists(soundName)) {
    Serial.println("File not found!");
    return;
  }

  if (!audioPlayer.isPlaying()) {
    audioPlayer.play(soundName);
    Serial.print("Playing ");
    Serial.println(soundName);
    while (audioPlayer.isPlaying()) {
    }
  }
}
