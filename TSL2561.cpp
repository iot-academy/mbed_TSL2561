
#include "TSL2561.h"
Serial DEBUG(USBTX, USBRX);

#define DEBUG_PRINTX(z,x)             if(z==1) DEBUG.printf(x);
#define DEBUG_PRINTLNX(z,x)           if(z==1) {DEBUG.printf(x);        DEBUG.printf("\r\n");}
#define DEBUG_PRINTXY(z,x, y)         if(z==1) DEBUG.printf(x, y);
#define DEBUG_PRINTLNXY(z,x, y)       if(z==1) {DEBUG.printf(x, y);     DEBUG.printf("\r\n");}

TSL2561::TSL2561():i2c(TSL2561_I2C_PINNAME_SDA,TSL2561_I2C_PINNAME_SCL){
    i2c.frequency (300);
    _addr = TSL2561_ADDR_FLOAT<<1;
    _initialized = false;
    _integration = TSL2561_INTEGRATIONTIME_13MS;
    _gain = TSL2561_GAIN_16X;    
}

TSL2561::TSL2561(uint8_t addr):i2c(TSL2561_I2C_PINNAME_SDA,TSL2561_I2C_PINNAME_SCL) {

  _addr = addr<<1;
  _initialized = false;
  _integration = TSL2561_INTEGRATIONTIME_13MS;
  _gain = TSL2561_GAIN_16X;
  // we cant do wire initialization till later, because we havent loaded Wire yet
}

TSL2561::TSL2561(PinName sda, PinName scl):i2c(sda, scl) {

  _addr = TSL2561_ADDR_FLOAT<<1;
  _initialized = false;
  _integration = TSL2561_INTEGRATIONTIME_13MS;
  _gain = TSL2561_GAIN_16X;
  // we cant do wire initialization till later, because we havent loaded Wire yet
}

TSL2561::TSL2561(PinName sda, PinName scl, uint8_t addr):i2c(sda, scl) {

  _addr = addr<<1;
  _initialized = false;
  _integration = TSL2561_INTEGRATIONTIME_13MS;
  _gain = TSL2561_GAIN_16X;
  // we cant do wire initialization till later, because we havent loaded Wire yet
}

uint16_t TSL2561::getLuminosity (uint8_t channel) {

  uint32_t x = getFullLuminosity();

  if (channel == 0) {
    // Reads two byte value from channel 0 (visible + infrared)
    
    return (x & 0xFFFF);
  } else if (channel == 1) {
    // Reads two byte value from channel 1 (infrared)
    
    return (x >> 16);
  } else if (channel == 2) {
    // Reads all and subtracts out just the visible!
    
    return ( (x & 0xFFFF) - (x >> 16));
  }
  
  // unknown channel!
  return 0;
}

uint32_t TSL2561::getFullLuminosity (void)
{
  if (!_initialized) begin();

  // Enable the device by setting the control bit to 0x03
  enable();

  // Wait x ms for ADC to complete
  switch (_integration)
  {
    case TSL2561_INTEGRATIONTIME_13MS:
      wait_ms(14);
      break;
    case TSL2561_INTEGRATIONTIME_101MS:
      wait_ms(102);
      break;
    default:
      wait_ms(403);
      break;
  }

DEBUG_PRINTLNXY(1," Integration:= %d",_integration);

  uint32_t x;
  x = read16(TSL2561_COMMAND_BIT | TSL2561_WORD_BIT | TSL2561_REGISTER_CHAN1_LOW);
  
  DEBUG_PRINTLNXY(1," x:= %d",x);
  
  x <<= 16;
  x |= read16(TSL2561_COMMAND_BIT | TSL2561_WORD_BIT | TSL2561_REGISTER_CHAN0_LOW);

  DEBUG_PRINTLNXY(1," x:= %d",x);
  
    wait(3);
  disable();

  return x;
}


bool TSL2561::begin(void) {

    char reg = TSL2561_REGISTER_ID;
    char read;
    
    i2c.start();
    i2c.write(_addr);
    i2c.write(reg);
    
    //i2c.start();
    //i2c.write(reg);
    read = i2c.read(1);    
    i2c.stop();   
    
  if (read & 0x0A ) {
    DEBUG_PRINTLNXY(1,"Read 0x%x => Found TSL2561",read);
  } else {
    return false;
  } 
    _initialized = true;
     
    // Set default integration time and gain
    setTiming(_integration);
    setGain(_gain);
  
    // Note: by default, the device is in power down mode on bootup
    disable();    
    
    return true;
 }
 
 uint16_t TSL2561::read16(uint8_t reg)
{
    uint16_t x; uint16_t t;
    char _x;char _t;char r[1];
    r[0] = reg;
 
    i2c.start();
    i2c.write(_addr);
    i2c.write(reg);
    
    //i2c.start();
    //i2c.write(reg);
    _t = i2c.read(1);
    _x = i2c.read(1);
    
    DEBUG_PRINTLNXY(0,"%x",_x);
    DEBUG_PRINTLNXY(0,"%x",_t);
    
    i2c.stop();
   
    t=(uint16_t)&_t;
    x=(uint16_t)&_x;
    x <<= 8;
    x |= t;
    return x;
}
 
 void TSL2561::write8 (uint8_t reg, uint8_t value)
{ 
    i2c.start();
    i2c.write(_addr);
    i2c.write(reg);
    i2c.write(value);
    i2c.stop(); 
}

void TSL2561::setTiming(tsl2561IntegrationTime_t integration){

if (!_initialized) begin();

else DEBUG_PRINTLNX(1,"--------------Set Timing---------");

  enable();
  
  _integration = integration;
  
  DEBUG_PRINTLNXY(1,"Integration: 0x%x",_integration);
  DEBUG_PRINTLNXY(1,"Gain: 0x%x",_gain);
  DEBUG_PRINTLNXY(1,"Integration | Gain: 0x%x",_integration | _gain);
  
  write8(TSL2561_COMMAND_BIT | TSL2561_REGISTER_TIMING, _integration | _gain);  
  
  disable();
  
  DEBUG_PRINTLNX(1,"--------------Complete Set Timing-------------");
  
  wait(1);

}

void TSL2561::setGain(tsl2561Gain_t gain) {

if (!_initialized) begin();
else    DEBUG_PRINTLNX(1,"-------------Set Gain--------------");


  enable();
  
  DEBUG_PRINTLNXY(1,"Intergration: 0x%x",_integration);
  DEBUG_PRINTLNXY(1,"Gain: 0x%x",_gain);
  DEBUG_PRINTLNXY(1,"Intergration | Gain: 0x%x",_integration | _gain);
  
  _gain = gain;
  write8(TSL2561_COMMAND_BIT | TSL2561_REGISTER_TIMING, _integration | _gain);  
  //write8(TSL2561_COMMAND_BIT | TSL2561_REGISTER_TIMING,  _gain);  
  disable();
  
  DEBUG_PRINTLNX(1,"---------------Complete Set Gain----------------");
  wait(1);
  
}

void TSL2561::enable(void)
{
  if (!_initialized) begin();

  // Enable the device by setting the control bit to 0x03
  DEBUG_PRINTLNX(1," Power On");
  write8(TSL2561_COMMAND_BIT | TSL2561_REGISTER_CONTROL, TSL2561_CONTROL_POWERON);
}

void TSL2561::disable(void)
{
  if (!_initialized) begin();

  // Disable the device by setting the control bit to 0x03
  DEBUG_PRINTLNX(1," Power Off");
  write8(TSL2561_COMMAND_BIT | TSL2561_REGISTER_CONTROL, TSL2561_CONTROL_POWEROFF);
}