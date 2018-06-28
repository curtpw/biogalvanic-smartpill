/*
 * M3 available I/O
 * P2 :exposed pad - vibration motor side between KX022 and battery
 * P18 :exposed pad - next to OLED pads
 * P7, P6, P3: next to each other in that order, HR sensor plug
 */
/********************************************************************************************************/
/************************ INCLUDES **********************************************************************/
/********************************************************************************************************/
#include <SPI.h>
#include <BLEPeripheral.h>
#include <BLEUtil.h>
#include "Arduino_nRF5x_lowPower.h"

/********************************************************************************************************/
/************************ CONSTANTS / SYSTEM VAR ********************************************************/
/********************************************************************************************************/
bool  debug = true;        //turn serial on/off to get data or turn up sample rate
bool  debug_time = false;    //turn loop component time debug on/off

int   speedHz = 1; //throttled loop speed - native max loop speed is about 35 ms or 28Hz
float speedMs = 1000 / speedHz;  //native max loop speed is 62.5 ms or 16Hz

bool  awake = true;
bool  wakeAttached = true;

/********************************************************************************************************/
/************************ DEFINITIONS *******************************************************************/
/********************************************************************************************************/
//log to USB serial monitor
#define VERBOSE_SERIAL

//I use USB serial for debugging. This is diconnection in final version.
#define PIN_SERIAL_TX           13 
//wakeup device when super capacitors are fully charged (attached to positive lead from capacitor bank)
#define SLEEP_INT               18
//ADC for measureing cell voltage (attached to copper cathode)
#define SENSE_VOLTAGE           2


/********************************************************************************************************/
/************************ VARIABLES *********************************************************************/
/********************************************************************************************************/

  //Timestamp
  float     clocktime;
  int       secondCounter =     0;
  int       tenSecondCounter =  0;
    
  //sense
  int cellVoltage = 0;

  //sleep
  volatile int cycles = 0;
  volatile bool int1 = false;


/********************************************************************************************************/
/************************ DECLARATIONS ******************************************************************/
/********************************************************************************************************/
//Bluetooth
// create peripheral instance, see pinouts above
BLEPeripheral blePeripheral =   BLEPeripheral();

// create service
BLEService customService =      BLEService("a000");

// create command i/o characteristics
//BLECharCharacteristic           ReadOnlyArrayGattCharacteristic  = BLECharCharacteristic("a001", BLERead);
//BLECharCharacteristic           WriteOnlyArrayGattCharacteristic = BLECharCharacteristic("a002", BLEWrite);


void wakeUp() {
    cycles = 0;
    int1 = true;
}


/********************************************************************************************************/
/************************ SETUP *************************************************************************/
/********************************************************************************************************/

void setup() 
{
  //power mgmt
  nRF5x_lowPower.powerMode(POWER_MODE_LOW_POWER);

  //turn on serial debugging for development
#ifdef VERBOSE_SERIAL
    Serial.begin(115200); 
    if(debug) Serial.print("STARTING\t");
    delay(50);
#endif

  //disable UART to save power when serial debugging isn't needed
#ifndef VERBOSE_SERIAL
 /* http://discuss.redbear.cc/t/power-reduce-in-ble-nano/1237/18 */
    NRF_POWER->DCDCEN = 0x00000001;
    NRF_UART0->TASKS_STOPTX = 1;
    NRF_UART0->TASKS_STOPRX = 1;
    NRF_UART0->ENABLE = 0;
#endif

  //low power sleep interrupt
    pinMode(SLEEP_INT, INPUT);
    attachInterrupt(digitalPinToInterrupt(SLEEP_INT), wakeUp, RISING);
    nRF5x_lowPower.enableWakeupByInterrupt(SLEEP_INT, RISING);
 

  /************** INIT BLUETOOTH BLE instantiate BLE peripheral *********/
    // set LED pin to output mode
   // set advertised local name and service UUID
    blePeripheral.setLocalName("data");
    blePeripheral.setDeviceName("smartpill");
    blePeripheral.setAdvertisedServiceUuid(customService.uuid());
    blePeripheral.setAppearance(0xFFFF);
  
    // add attributes (services, characteristics, descriptors) to peripheral
    blePeripheral.addAttribute(customService);

    // begin initialization
    blePeripheral.begin();

    if(debug) Serial.println("BLE MOBILE APP PERIPHERAL");

  delay(500);  
}

/********************************************************************************************************/
/************************ LOOP **************************************************************************/
/********************************************************************************************************/

void loop()
{     
      if(wakeAttached == false){
    //  attachInterrupt(digitalPinToInterrupt(SLEEP_INT), wakeUp, HIGH); 
      wakeAttached = true;
    }

  /********************************************************************************** 
  ******************** LOOP SPEED CONTROL *******************************************
  ***********************************************************************************/
 if(  (clocktime + speedMs < millis())  ){
    /***************** Timestamp ****************************************************/
    clocktime = millis();
#ifdef VERBOSE_SERIAL
    Serial.println(" "); Serial.print("TIME: "); Serial.print( clocktime/1000 ); Serial.println(" s"); 
#endif

    /******************* Measure Cell Power *******************/
    cellVoltage = analogRead(SENSE_VOLTAGE);
#ifdef VERBOSE_SERIAL
    Serial.print("cell voltage: "); 
    Serial.println(cellVoltage);
#endif


    /******************* Bluetooth Transmit *******************************************/
    blePeripheral.end();
    char buffer [8];
    itoa( int(cellVoltage), buffer, 10);  //change from 10 to 16 if want to use hex to save space
    blePeripheral.setLocalName(buffer);
    blePeripheral.begin();

#ifdef VERBOSE_SERIAL
    Serial.print("TIME LOOP: "); Serial.print(millis() - clocktime); Serial.print("\tSeconds: "); Serial.print(secondCounter); 
#endif
        
  } //end loop speed
} //end infinate loop




/*********************************************************************
*************** ETC **************************************************
*********************************************************************/

void __delay(uint32_t timeout)
{
  uint32_t start;
  start = millis();

 do
 {
   __WFE();
 }
   while ((millis() - start) >= timeout);
}


int hex_to_int(char c){
  int first;
  int second;
  int value;
  
  if (c >= 97) {
    c -= 32;
  }
  first = c / 16 - 3;
  second = c % 16;
  value = first * 10 + second;
  if (value > 9) {
    value--;
  }
  return value;
}

int hex_to_ascii(char c, char d){
  int high = hex_to_int(c) * 16;
  int low = hex_to_int(d);
  return high+low;
}

