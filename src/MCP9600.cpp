#include "MCP9600.h"

#include "tools-log.h"

enum : uint8_t
{
    REG_HOTJUNCTION         = 0x00,
    REG_JUNCTIONDELTA       = 0x01,
    REG_COLDJUNCTION        = 0x02,
    REG_RAWDATAADC          = 0x03,
    REG_STATUS              = 0x04,
    REG_SENSOR              = 0x05,
    REG_CONFIG              = 0x06,
    REG_ALERTCONFIG_1       = 0x08,
    REG_ALERTHYST_1         = 0x0C,
    REG_ALERTLIMIT_1        = 0x10,
    REG_DEVICEID            = 0x20,
};

bool MCP9600::begin() 
{   
    if(!TwoWireDevice::begin())
	{
		// ERROR("TwoWireDevice.begin() failed.");
		return false;
	};

    /* Check for MCP9600 device ID and revision register (0x20), high byte should
     * be 0x40. */
    uint16_t id_reg = readreg16_ML(REG_DEVICEID);
    if(id_reg >> 8 != 0x40)
    {
        // ERROR("Device id not correct.");
        return false;
    };

    // config register defaults
    _reg_config.bits.COLDJUNCRES = 0b0;     // 0.0625C CJ resolution
    _reg_config.bits.ADCRES = ADC_18BIT;    // 18-bit
    _reg_config.bits.BURSTMODE = 0b000;     // Burst n = 1
    _reg_config.bits.SHUTDOWN = 0b00;       // Normal operation
    writereg8(REG_CONFIG, _reg_config.value);

    // sensor register defaults
    _reg_sensor.bits.TYPE = TYPE_K;
    _reg_sensor.bits.FILTER = 0;
    writereg8(REG_SENSOR, _reg_sensor.value);

    return true;
};

float MCP9600::readThermocouple() 
{
    uint16_t twos = readreg16_ML(REG_HOTJUNCTION);
    float temp = (twos & 0x7FFF) * 0.0625;
    if(twos & 0x8000)
        temp *= -1;
    return temp;
};

float MCP9600::readColdJunction(void) 
{
    uint16_t twos = readreg16_ML(REG_COLDJUNCTION);
    float temp = (twos & 0x0FFF) * 0.0625;
    if(twos & 0x1000)
        temp *= -1;
    return temp;
};

int32_t MCP9600::readADC(void) 
{
  uint32_t val = readreg24_ML(REG_RAWDATAADC);
  // sign extended 
  if(val & 0x04000000)
    return val | 0xFC000000;
  return val;
};

void MCP9600::sleep(bool sleep) 
{
    if(sleep)
        _reg_config.bits.SHUTDOWN = 0x01;
    else
        _reg_config.bits.SHUTDOWN = 0x00;
    writereg8(REG_CONFIG, _reg_config.value);
};

void MCP9600::setADCresolution(const resolution_t res) 
{
    _reg_config.bits.ADCRES = res;
    writereg16_ML(REG_CONFIG, _reg_config.value);
};


void MCP9600::setThermocoupleType(const thermocouple_t tctype) 
{
    _reg_sensor.bits.TYPE = tctype;
    writereg8(REG_SENSOR, _reg_sensor.value);
};

void MCP9600::setFilter(const uint8_t filter) 
{
    _reg_sensor.bits.FILTER = filter & 0x03;
    writereg8(REG_SENSOR, _reg_sensor.value);
};

void MCP9600::setAlertTemperature(uint8_t alertnum, const float temp, const uint8_t hysteresis) 
{
    alertnum--;
    if (alertnum > 3)
        return;

    int16_t limit = temp / 0.0625; // 0.0625*C per LSB!
    writereg16_ML(REG_ALERTLIMIT_1 + alertnum, limit);

    writereg8(REG_ALERTHYST_1 + alertnum, hysteresis & 0x0F);
};

/**************************************************************************/
/*!
    @brief Configure temperature alert
    @param alert Which alert output we're getting, can be 1 to 4
    @param enabled Whether this alert is on or off
    @param rising True if we want an alert when the temperature rises above.
    False for alert on falling below.
    @param alertColdJunction Whether the temperature we're watching is the
    internal chip temperature (true) or the thermocouple (false). Default is
    false
    @param activeHigh Whether output pin goes high on alert (true) or low
   (false)
    @param interruptMode Whether output pin latches on until we clear it (true)
    or comparator mode (false)
*/
/**************************************************************************/
void MCP9600::configureAlert(uint8_t alertnum, bool enabled, 
    bool rising, bool alertColdJunction, bool activeHigh,
    bool interruptMode) 
{
    alertnum--;
    if (alertnum > 3)
        return;

    union
    {
        uint8_t value;
        struct
        {
            uint8_t CLEARINT        : 1;
            uint8_t                 : 2;
            uint8_t COLDJ           : 1;
            uint8_t RISEFALL        : 1;
            uint8_t ACTIVE_HIGH     : 1;
            uint8_t MODE            : 1;
            uint8_t ENABLE          : 1;
        } bits;
    } c;
    c.value = 0;
    c.bits.ENABLE = enabled;
    c.bits.MODE = interruptMode;
    c.bits.ACTIVE_HIGH = activeHigh;
    c.bits.RISEFALL = rising;
    c.bits.COLDJ = alertColdJunction;
    writereg8(REG_ALERTCONFIG_1 + alertnum, c.value);
};

MCP9600::status_reg_t MCP9600::getStatus() 
{
    status_reg_t status;
    status.value = readreg8(REG_STATUS);
    return status;
};
