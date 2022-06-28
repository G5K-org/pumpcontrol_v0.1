#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/i2c.h"
#include "driverlib/sysctl.h"


#include "i2c.h"
#include "ssd1306.h"

// I2C
#define GPIO_PD0_I2C3SCL 0x00030003
#define GPIO_PD1_I2C3SDA 0x00030403

void I2C_3_config(){ // Configure I2C3 for pins PD0 and PD1
  SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C3);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
  GPIOPinConfigure(GPIO_PD0_I2C3SCL);
  GPIOPinConfigure(GPIO_PD1_I2C3SDA);
  GPIOPinTypeI2CSCL(GPIO_PORTD_BASE, GPIO_PIN_0);
  GPIOPinTypeI2C(GPIO_PORTD_BASE, GPIO_PIN_1);
  I2CMasterInitExpClk(I2C3_BASE, SysCtlClockGet(), false);
  
  // Pull up I2C3 by PB6 and PB7 pins
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
  GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, GPIO_PIN_6);
  GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, GPIO_PIN_7);
  GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_6, GPIO_STRENGTH_12MA, GPIO_PIN_TYPE_STD_WPU);
  GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_7, GPIO_STRENGTH_12MA, GPIO_PIN_TYPE_STD_WPU);
}

void I2C_sendSingleByte(uint8_t slave_addr, char data){
  I2CMasterSlaveAddrSet(I2C3_BASE, slave_addr, false);
  I2CMasterDataPut(I2C3_BASE, data);
  I2CMasterControl(I2C3_BASE, I2C_MASTER_CMD_SINGLE_SEND);
  while(I2CMasterBusy(I2C3_BASE));
}

void I2C_sendMultipleBytes(uint8_t slave_addr, uint8_t numOfBytes, char by[]){
  uint8_t i;
  I2CMasterSlaveAddrSet(I2C3_BASE, slave_addr, false);
  I2CMasterDataPut(I2C3_BASE, by[0]);
  I2CMasterControl(I2C3_BASE, I2C_MASTER_CMD_BURST_SEND_START);
  while(I2CMasterBusy(I2C3_BASE));
  for(i = 1; i < numOfBytes - 1; i++){
    I2CMasterDataPut(I2C3_BASE, by[i]);
    I2CMasterControl(I2C3_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
    while(I2CMasterBusy(I2C3_BASE));
  }
  I2CMasterDataPut(I2C3_BASE, by[numOfBytes - 1]);
  I2CMasterControl(I2C3_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
  while(I2CMasterBusy(I2C3_BASE));
}
//_______________________________PUMPCONTROL_______________________________
void GPIO_PORTE_CONFIG()
{
 SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE); //RCGC
 //---INPUTS---
 GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, GPIO_PIN_0); //SENSOR DRYRUN                         (Trockenlauffehler)
 GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, GPIO_PIN_1); //SENSOR WATERLVL 0
 GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, GPIO_PIN_2); //SENSOR WATERLVL 1
 GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, GPIO_PIN_3); //SENSOR WATERLVL 3                     (Hochwasserfehler)
 GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, GPIO_PIN_4); //SENSOR WSK                            (Wicklungsschutzkontakt) BI-METAL Pumpe 1
 GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, GPIO_PIN_5); //SENSOR WSK                            (Wicklungsschutzkontakt) BI-METAL Pumpe 2
 //---PIN CONFIG---
 GPIODirModeSet(GPIO_PORTE_BASE, GPIO_PIN_1, GPIO_DIR_MODE_IN);
 GPIODirModeSet(GPIO_PORTE_BASE, GPIO_PIN_2, GPIO_DIR_MODE_IN);
 GPIODirModeSet(GPIO_PORTE_BASE, GPIO_PIN_3, GPIO_DIR_MODE_IN);
 GPIODirModeSet(GPIO_PORTE_BASE, GPIO_PIN_4, GPIO_DIR_MODE_IN);
 GPIODirModeSet(GPIO_PORTE_BASE, GPIO_PIN_5, GPIO_DIR_MODE_IN);
 //---PAD CONFIG---
 GPIOPadConfigSet(GPIO_PORTE_BASE, GPIO_PIN_0, GPIO_STRENGTH_12MA, GPIO_PIN_TYPE_STD_WPU);
 GPIOPadConfigSet(GPIO_PORTE_BASE, GPIO_PIN_1, GPIO_STRENGTH_12MA, GPIO_PIN_TYPE_STD_WPU);
 GPIOPadConfigSet(GPIO_PORTE_BASE, GPIO_PIN_2, GPIO_STRENGTH_12MA, GPIO_PIN_TYPE_STD_WPU);
 GPIOPadConfigSet(GPIO_PORTE_BASE, GPIO_PIN_3, GPIO_STRENGTH_12MA, GPIO_PIN_TYPE_STD_WPU);
 GPIOPadConfigSet(GPIO_PORTE_BASE, GPIO_PIN_4, GPIO_STRENGTH_12MA, GPIO_PIN_TYPE_STD_WPU);
 GPIOPadConfigSet(GPIO_PORTE_BASE, GPIO_PIN_5, GPIO_STRENGTH_12MA, GPIO_PIN_TYPE_STD_WPU);

}

void GPIO_PORTF_CONFIG()
{
SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1);
GPIODirModeSet(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_DIR_MODE_OUT);
GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD);
//GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);
}


uint8_t DIGI_SENS()
{
    int8_t data = GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5);
    return data;
}


uint8_t FAIL_SET(uint8_t sensor_input)
{
    //SWITCH CASE FAIL STATE RECOCGITION PLAUSIBLE CHECK
    /*
     * 0x00 --ALL OK
     * 0x01 --DRYRUN PROTECTION
     * 0x02 --SENSOR ERROR (NOT PLAUSIBLE)
     * 0x03 --DRYRUN | SENSOR ERROR
     * 0x04 --HIGH WATER
     * 0x05 --SENSOR ERROR | HIGH WATER
     * 0x06 --DRYRUN | SENSOR ERROR | HOCHWASSER
     * 0x07 --ERROR
     * 0x08 --CHECK WSK!
     *
     *
     * */
    uint8_t WSK_ONLY = sensor_input>>4;

    if(WSK_ONLY == 0x01 || WSK_ONLY == 0x2)
            {
            return 0x08;
            }


    uint8_t sensor_input_without_wsk= sensor_input & ~(3<<4);
    switch(sensor_input_without_wsk)
    {
    case 0x00:
              return 0x04; //0b 0000 0000 Hochwasser

    case 0x01:
              return 0x05; //0b 0000 0001 Sensor + Hochwasser

    case 0x02:
              return 0x05; //0b 0000 0010 Sensor + Hochwasser

    case 0x03:
              return 0x05; //0b 0000 0011 Sensor + Hochwasser

    case 0x04:
              return 0x05; //0b 0000 0100 Sensor + Hochwasser

    case 0x05:
              return 0x05; //0b 0000 0101 Sensor + Hochwasser

    case 0x06:
              return 0x05; //0b 0000 0110 Sensor + Hochwasser

    case 0x07:
              return 0x06; //0b 0000 0111 Dryrun + Sensor + Hochwasser

    case 0x08:
              return 0x00; //0b 0000 1000 OK

    case 0x09:
              return 0x02; //0b 0000 1001 Sensor

    case 0x0A:
              return 0x02; //0b 0000 1010 Sensor

    case 0x0B:
              return 0x02; //0b 0000 1011 Sensor

    case 0x0C:
              return 0x00; //0b 0000 1100 OK

    case 0x0D:
              return 0x03; //0b 0000 1101 Dryrun + Sensor

    case 0x0E:
              return 0x00; //0b 0000 1110 OK

    case 0x0F:
              return 0x00; //0b 0000 1111 OK

    default:
              return 0x07;
    }

}

uint8_t WATER_LVL(uint8_t sense, uint8_t analog, bool is_analog)
{
    uint8_t waterlvl = sense & ~(3<<4); //Cut of WSK Bits
    switch(is_analog)
    {
    case false:
            return waterlvl;
    case true:
            return analog;
    default:
            return waterlvl;
    }
}

uint8_t DIGI_OUT(uint8_t failstate,uint8_t waterlvl)
{

}

void printscreen(uint8_t waterlvl, uint8_t active_actors, uint8_t error_codes)
{
    switch(waterlvl)
    {
    case 0xF:
            OLED_sendStrXY("Waterlvl: 0m", 1, 0);
            break;
    case 0xE:
            OLED_sendStrXY("Waterlvl: 1m", 1, 0);
            break;
    case 0xC:
            OLED_sendStrXY("Waterlvl: 2m", 1, 0);
            break;
    case 0x8:
            OLED_sendStrXY("Waterlvl: 3m", 1, 0);
            break;
    }

    switch(error_codes)
    {
    case 0x00:
              OLED_sendStrXY("ERR: noting    ", 3, 0);
              break;
    case 0x01:
              OLED_sendStrXY("ERR: dryrun    ", 3, 0);
              break;
    case 0x02:
              OLED_sendStrXY("ERR:dry+sens   ", 3, 0);
              break;
    case 0x03:
              OLED_sendStrXY("ERR:dry+sens   ", 3, 0);
              break;
    case 0x04:
              OLED_sendStrXY("ERR:highwater  ", 3, 0);
              break;
    case 0x05:
              OLED_sendStrXY("ERR:sens+highw ", 3, 0);
              break;
    case 0x06:
              OLED_sendStrXY("ERR:dry+sens+hw", 3, 0);
              break;
    case 0x07:
              OLED_sendStrXY("ERR: ERR       ", 3, 0);
              break;
    case 0x08:
              OLED_sendStrXY("ERR: check WSK!", 3, 0);
              break;
    }

    OLED_sendStrXY("   >Digimode<", 0, 0);
    OLED_sendStrXY("DigiOut:", 2, 0);

}


void main (void){
  SysCtlClockSet(SYSCTL_SYSDIV_2_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN); // 80 MHz
  GPIO_PORTF_CONFIG();
  GPIO_PORTE_CONFIG();
  I2C_3_config();
  OLED_Init();
  OLED_clearDisplay();
  OLED_sendStrXY("Pumpcontrol v0.1", 0, 0);
  SysCtlDelay(50000000);
  OLED_clearDisplay();

  while(1)
  {
  uint8_t ERROR_CODE = FAIL_SET(DIGI_SENS());
  uint8_t water_lvl_is = WATER_LVL(DIGI_SENS(),0, false);
  uint8_t actor_active = DIGI_OUT(ERROR_CODE,water_lvl_is);
  printscreen(water_lvl_is, 0, ERROR_CODE);
  }

}






