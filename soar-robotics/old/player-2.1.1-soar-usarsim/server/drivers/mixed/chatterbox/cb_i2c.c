

// include Dave Hyland's Robostix interface code
#include "AvrInfo.h"
#include "i2c-dev.h"
#include "i2c-api.h"
#include "i2c-io-api.h"

#include "cb_i2c.h"

// i2c device
static int i2cDev = 0;

// address data structure
static led_t led[5][3];

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------

//----------------------------------------------------------------------------
/**
 * Initializes a pca9634 chip
 * @param addr i2c address of chip
 * @return 1 if successful, 0 otherwise
 */
int initPCA9634(unsigned char addr)
{ 
  unsigned char data[2];

  I2cSetSlaveAddress(i2cDev, addr, I2C_NO_CRC);

  // enable clock
  data[0] = 0x00;   // register address
  data[1] = 0x0f;   // register value
  if (I2cSendBytes(i2cDev, data, 2) != 0) {
    printf("Error initializing chip %d \n", addr);
    return 0; // error
  }

  // enable led output A
  data[0] = 0x0c;   // register address
  data[1] = 0xff;   // register value
  if (I2cSendBytes(i2cDev, data, 2) != 0) {
    printf("Error initializing chip %d \n", addr);
    return 0; // error
  }

  // enable led output B
  data[0] = 0x0d;   // register address
  data[1] = 0xff;   // register value
  if (I2cSendBytes(i2cDev, data, 2) != 0) {
    printf("Error initializing chip %d \n", addr);
    return 0; // error
  }

  return 1; // success  
}

//-----------------------------------------------------------------------------
/**
 * Resets all PCA9634 chips on the i2c bus
 * @return 1 if successful, 0 otherwise
 */
int resetPCA9634()
{
  unsigned char data[2];

  // Call ALL address
  I2cSetSlaveAddress(i2cDev, 0x03, I2C_NO_CRC);

  // reset all
  data[0] = 0xa5;   
  data[1] = 0x5a;   
  if (I2cSendBytes(i2cDev, data, 2) != 0) {
    printf("Error resetting pca9634 chips \n");
    return 0; // error
  }

  return 1; // success  
}
//----------------------------------------------------------------------------
/**
 * Set the 7 segment display to a given byte
 * @param value [0..255]
 * @return 1 if successful, 0 in case of an error
 */
int set7SegDisplay(unsigned char value)
{
  I2cSetSlaveAddress( i2cDev, ROBOSTIX_ADDR, I2C_USE_CRC );
  if (I2C_IO_WriteReg8(i2cDev, PORTA, value) == 0)
    return 0; // error

  return 1; // success
}
//-----------------------------------------------------------------------------
/**
 * Initializes the driver and hardware. Note this function has to be called
 * first to things to work properly
 * @return 1 if successful, 0 in case of an error
 */
int init()
{ 
  I2C_IO_Info_t   info;

  // open I2C device
  if (( i2cDev = open( I2C_DEV_NAME, O_RDWR )) < 0 ) {
    printf("Error  opening '%s': %s\n", I2C_DEV_NAME, strerror( errno ));
    return 0;  // error
  }
 
  //*****************************************
  // Robostix stuff

  // lets talk to robostix
  I2cSetSlaveAddress( i2cDev, ROBOSTIX_ADDR, I2C_USE_CRC );

  // check if software version robostix is high enough
  if ( !I2C_IO_GetInfo( i2cDev, &info )) {
    printf("Unable to retrieve information from i2c address %d\n", 
	    ROBOSTIX_ADDR );
    return 0; // error
  }
  if ( info.version < 2 ) {
    printf("i2c-io.hex on the robostix is too old and needs to be updated\n" );
    return 0;
  }

  // configure DDRA port as output 
  if (I2C_IO_WriteReg8(i2cDev, DDRA, 0xFF) == 0)
    return 0; // error

  // clear 7 seg display
  set7SegDisplay(0);

  // configure ir enable pin as output
  if (I2C_IO_SetGPIODir( i2cDev, 2, 4, 4 ) == 0)
    return 0; // error

  //****************************************
  // RGB stuff

  led[0][RED].chipAddr   = LED0_R_ADDR;
  led[0][RED].ledAddr    = LED0_R_PORT;
  led[0][GREEN].chipAddr = LED0_G_ADDR;
  led[0][GREEN].ledAddr  = LED0_G_PORT;
  led[0][BLUE].chipAddr  = LED0_B_ADDR;
  led[0][BLUE].ledAddr   = LED0_B_PORT;

  led[1][RED].chipAddr   = LED1_R_ADDR;
  led[1][RED].ledAddr    = LED1_R_PORT;
  led[1][GREEN].chipAddr = LED1_G_ADDR;
  led[1][GREEN].ledAddr  = LED1_G_PORT;
  led[1][BLUE].chipAddr  = LED1_B_ADDR;
  led[1][BLUE].ledAddr   = LED1_B_PORT;

  led[2][RED].chipAddr   = LED2_R_ADDR;
  led[2][RED].ledAddr    = LED2_R_PORT;
  led[2][GREEN].chipAddr = LED2_G_ADDR;
  led[2][GREEN].ledAddr  = LED2_G_PORT;
  led[2][BLUE].chipAddr  = LED2_B_ADDR;
  led[2][BLUE].ledAddr   = LED2_B_PORT;

  led[3][RED].chipAddr   = LED3_R_ADDR;
  led[3][RED].ledAddr    = LED3_R_PORT;
  led[3][GREEN].chipAddr = LED3_G_ADDR;
  led[3][GREEN].ledAddr  = LED3_G_PORT;
  led[3][BLUE].chipAddr  = LED3_B_ADDR;
  led[3][BLUE].ledAddr   = LED3_B_PORT;

  led[4][RED].chipAddr   = LED4_R_ADDR;
  led[4][RED].ledAddr    = LED4_R_PORT;
  led[4][GREEN].chipAddr = LED4_G_ADDR;
  led[4][GREEN].ledAddr  = LED4_G_PORT;
  led[4][BLUE].chipAddr  = LED4_B_ADDR;
  led[4][BLUE].ledAddr   = LED4_B_PORT;

  // reset all chips and then initialize them
  if (resetPCA9634() == 0)
    return 0; // error

  if (initPCA9634(CHIP_A_ADDR) == 0)
    return 0; // error

  if (initPCA9634(CHIP_B_ADDR) == 0)
    return 0; // error

  return 1; // succes

}

//----------------------------------------------------------------------------
/**
 * Set the rgb value for a given led. The layout is
 *  0 - front right
 *  1 - front left
 *  2 - left
 *  3 - back
 *  4 - right
 * @param id of led to set
 * @param red [0..255]
 * @param green [0..255]
 * @param blue [0..255]
 * @return 1 successfull, 0 error
 */
int setLed(unsigned char id, unsigned char red, unsigned char green, 
           unsigned char blue)
{
  unsigned char data[2];

  if (id > 4) {
    printf("Error: led id out of bounds [0..4] %d \n",id);
    return 0; // error
  }

  //*************************************
  // send red channel data
  I2cSetSlaveAddress(i2cDev, led[id][RED].chipAddr, I2C_NO_CRC);

  data[0] = led[id][RED].ledAddr;
  data[1] = red;
  if (I2cSendBytes(i2cDev, data, 2) != 0) {
    printf("Error setting color red of led %d to %d \n", id, red);
    return 0; // error
  }

  //*************************************
  // send green channel data
  I2cSetSlaveAddress(i2cDev, led[id][GREEN].chipAddr, I2C_NO_CRC);

  data[0] = led[id][GREEN].ledAddr;
  data[1] = green;
  if (I2cSendBytes(i2cDev, data, 2) != 0) {
    printf("Error setting color red of led %d to %d \n", id, red);
    return 0; // error
  }

  //*************************************
  // send blue channel data
  I2cSetSlaveAddress(i2cDev, led[id][BLUE].chipAddr, I2C_NO_CRC);

  data[0] = led[id][BLUE].ledAddr;
  data[1] = blue;
  if (I2cSendBytes(i2cDev, data, 2) != 0) {
    printf("Error setting color red of led %d to %d \n", id, red);
    return 0; // error
  }

  return 1; // success
}

//-----------------------------------------------------------------------------
/**
 * Sets the 7 segment display to show a given decimal and the dot
 * @param n decimal to display [0..9]
 * @param dot 1 display dot otherwise do not display dot
 * @return 1 if successful, 0 in case of an error
 */
int set7SegNumber(unsigned char n, char dot)
{
  unsigned char byteCode;

  switch (n) {
  case 0:
    byteCode = 0xFC;
    break;
  case 1:
    byteCode = 0x60;
    break;
  case 2:
    byteCode = 0xDA;
    break;
  case 3:
    byteCode = 0xF2;
    break;
  case 4:
    byteCode = 0x66;
    break;
  case 5:
    byteCode = 0xB6;
    break;
  case 6:
    byteCode = 0xBE;
    break;
  case 7:
    byteCode = 0xE0;
    break;
  case 8:
    byteCode = 0xFE;
    break;
  case 9:
    byteCode = 0xF6;
    break;

  default:
    printf("Error: number out of bounds [0..9] \n");
    return 0; // error

  } // switch

  // should we turn the dot on ?
  if (dot == 1)
    byteCode |= 0x01;


  return set7SegDisplay(byteCode);
}
//-----------------------------------------------------------------------------
/**
 * Shows a rotating segment bar and progresses it one position
 * @return 1 if successful, 0 in case of an error
 */
int rotateStep()
{
  unsigned char byteCode;
  static int state = 0;

  switch (state) {
  case 0:
    byteCode = 0x80;
    break;
  case 1:
    byteCode = 0x40;
    break;
  case 2:
    byteCode = 0x20;
    break;
  case 3:
    byteCode = 0x10;
    break;
  case 4:
    byteCode = 0x08;
    break;
  case 5:
    byteCode = 0x04;
    state = -1;
    break;
  default:
    byteCode = 0;
    state = 0;

  } // switch

  state++;

  return set7SegDisplay(byteCode);
}
//-----------------------------------------------------------------------------
/**
 * Enables or disables the ir sensors
 * @param enable = 1 enable, disable otherwise
 * @return 1 if successful, 0 in case of an error
 */
int enableIr(char enable)
{ 
  I2cSetSlaveAddress( i2cDev, ROBOSTIX_ADDR, I2C_USE_CRC );
  
  if (enable == 1) {
    if ( I2C_IO_SetGPIO(i2cDev, 2, 2, 1) == 0) {
      printf("Error: while enabling IR \n");
      return 0; // error
    }
  }
  else {
    if ( I2C_IO_SetGPIO(i2cDev, 2, 2, 0) == 0) {
      printf("Error: while disabling IR \n");
      return 0; // error
    }
  }

  return 1; // success
}
//-----------------------------------------------------------------------------
/**
 * Checks if the IR sensors are enabled
 * @return 1 enabled, 0 disabled, -1 error
 */
int isIrEnabled()
{
  unsigned char pinVal;

  I2cSetSlaveAddress( i2cDev, ROBOSTIX_ADDR, I2C_USE_CRC );
  if (I2C_IO_GetGPIO( i2cDev, 2, &pinVal) == 0) {
    printf("Error: failed to read ir status \n");
    return -1; // error
  }
 
  return pinVal & 0x02;
 
}
//-----------------------------------------------------------------------------
/**
 * Reads the 10bits from the adc
 * @param id of ADC [0..7]
 * @return 10 bit value or -1 in case of an error
 */
int readAdc(unsigned char id)
{
  unsigned short adcVal;

  if (id > 7) {
    printf("ADC id out of bounds [0..7] %d \n", id);
    return -1;
  }
  I2cSetSlaveAddress( i2cDev, ROBOSTIX_ADDR, I2C_USE_CRC );

  if ( I2C_IO_GetADC( i2cDev, id, &adcVal ) == 0) {
    printf("Error while reading from ADC %d \n", id);
    return -1;
  }
  return adcVal;
}
//-----------------------------------------------------------------------------
/**
 * Reads the distance from an ir sensor. The layout is:
 *  0 - front center
 *  1 - front left
 *  2 - left
 *  3 - back center
 *  4 - right
 *  5 - front right
 * @param id of ir sensor [0..5]
 * @return distance [m]
 * @return voltage [V]
 * @return 1 if successfull, 0 otherwise
 */
int readDistance(unsigned char id, float* distance, float* voltage)
{
  float u;
  float v;
  float range;
  unsigned char adcId = 0;

  if (id > 5) {
    printf("IR id out of bounds [0..5] %d \n", id);
    return 0;  // error
  }

  switch (id) {
  case 0:      // front center
    adcId = 7;
    break;
  case 1:      // front left
    adcId = 4;
    break;
  case 2:      // left side
    adcId = 3; 
    break;
  case 3:      // back center
    adcId = 6; 
    break;
  case 4:      // right side
    adcId = 2; 
    break;
  case 5:      // front right
    adcId = 5;
    break;
  } // switch

  u = (float)(readAdc(adcId)) / 1024.0 * VREF;
  
  v = u;
  range = K5;
  range += K4 * v;
  v = v * u; 
  range += K3 * v;
  v = v * u; 
  range += K2 * v;
  v = v * u; 
  range += K1 * v;
  v = v * u; 
  range += K0 * v;

  *distance = range;
  *voltage  = u;
  return 1; // success
}
