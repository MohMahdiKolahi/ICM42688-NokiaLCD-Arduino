#include <Arduino.h>
#include <U8g2lib.h>
#include "ICM42688.h"


#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif
///LCD////////////
#define LCD_BACKLIGHT 7
#define LCD_CS 10
//////////////////


U8G2_STE2007_96X68_F_3W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* reset=*/ 8);

ICM42688 IMU(Wire, 0x68);


void u8g2_prepare(void) {
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.setFontRefHeightExtendedText();
  u8g2.setDrawColor(1);
  u8g2.setFontPosTop();
  u8g2.setFontDirection(0);
}



void interrupt_setup(){
  
cli();//stop interrupts

//set timer1 interrupt at 5Hz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 5hz increments
  OCR1A = 49999;// = (16*10^6) / (1*1024) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS12, CS11 and CS10 bits for 64 prescaler
  TCCR1B |= (0 << CS12) | (1 << CS11) | (1 << CS10);
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);


sei();//allow interrupts
}



void setup(void) {
  pinMode(LCD_BACKLIGHT, OUTPUT);
  digitalWrite(LCD_BACKLIGHT, HIGH);
  u8g2.begin();

  Serial.begin(115200);
  while(!Serial) {}

  // start communication with IMU
  int status = IMU.begin();
  if (status < 0) {
    Serial.println("IMU initialization unsuccessful");
    Serial.println("Check IMU wiring or try cycling power");
    Serial.print("Status: ");
    Serial.println(status);
    while(1) {}
  }

  // setting the accelerometer full scale range to +/-8G
  IMU.setAccelFS(ICM42688::gpm8);
  // setting the gyroscope full scale range to +/-500 deg/s
  IMU.setGyroFS(ICM42688::dps500);
  
  // set output data rate to 12.5 Hz
  IMU.setAccelODR(ICM42688::odr12_5);
  IMU.setGyroODR(ICM42688::odr12_5);

  interrupt_setup();
}

unsigned long timer = 0;
float roll = 0.00, pitch = 0.00 ,yaw = 0.00;  
float timeStep = 0.01;

char temp[10];
char temp1[10];
char temp2[10];
int i;


void loop(void) {

  // read the sensor
  timer = millis();
  IMU.getAGT();

  // display the data
  roll = atan2(IMU.accY() , IMU.accZ()) * 57.3;
  pitch = atan2((- IMU.accX()) , sqrt(IMU.accY() * IMU.accY() + IMU.accZ() * IMU.accZ())) * 57.3;
  yaw = (yaw + IMU.gyrZ()/100);
  
delay((timeStep*1000) - (millis() - timer));

}


ISR(TIMER1_COMPA_vect){


  u8g2.clearBuffer();

  u8g2_prepare();

  sprintf(temp, "roll = %d ",(int)roll);
  u8g2.drawStr( 0, 0, temp);

  sprintf(temp1, "pitch = %d ", (int)pitch);
  u8g2.drawStr( 0, 20, temp1);

  sprintf(temp2, "yaw = %d ", (int)yaw);
  u8g2.drawStr( 0, 40, temp2);


  u8g2.sendBuffer();
   
}
