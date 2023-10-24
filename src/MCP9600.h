#ifndef __MCP9600_H
#define __MCP9600_H

#include <Arduino.h>
#include <TwoWireDevice.h>

#define MCP9600_ADDRESS_DEFAULT        (0x67) // VCC
#define MCP9600_ADDRESS_ALT            (0x60) // GND

class MCP9600 : public TwoWireDevice
{
    public:
		MCP9600(TwoWire& wire, const uint8_t addr = MCP9600_ADDRESS_DEFAULT)  : TwoWireDevice(wire, addr) {};
		MCP9600(const uint8_t addr = MCP9600_ADDRESS_DEFAULT) : TwoWireDevice(addr) {};
        ~MCP9600() {};

        bool begin();

        void sleep(bool sleep = true);

        int32_t readADC();
        float readThermocouple();
        float readColdJunction();

        void setFilter(const uint8_t);
        void setAlertTemperature(uint8_t alertnum, const float temp, const uint8_t hysteresis);
        void configureAlert(const uint8_t alert, bool enabled, bool rising,
            bool alertColdJunction = false, bool activeHigh = false,
            bool interruptMode = false);

        typedef enum : uint8_t
        {
            TYPE_K                  = 0x0,
            TYPE_J                  = 0x1,
            TYPE_T                  = 0x2,
            TYPE_N                  = 0x3,
            TYPE_S                  = 0x4,
            TYPE_E                  = 0x5,
            TYPE_B                  = 0x6,
            TYPE_R                  = 0x7,
        } thermocouple_t;
        void setThermocoupleType(const thermocouple_t);

        typedef enum : uint8_t
        {
            ADC_18BIT               = 0x0,
            ADC_16BIT               = 0x1,
            ADC_14BIT               = 0x2,
            ADC_12BIT               = 0x3
        } resolution_t;
        void setADCresolution(const resolution_t);

        union status_reg_t
        {
            struct 
            {
                uint8_t BURSTCOMPLETE       : 1;
                uint8_t TH_UPDATE           : 1;
                uint8_t : 1;
                uint8_t INPUT_RANGE         : 1;
                uint8_t ALERT4_STATUS       : 1;
                uint8_t ALERT3_STATUS       : 1;
                uint8_t ALERT2_STATUS       : 1;
                uint8_t ALERT1_STATUS       : 1;
            } bits;
            uint8_t value;
        };
        status_reg_t getStatus();
    
    protected:
        uint8_t _device_id = 0;

    private:
        union config_reg_t
        {
            uint8_t value;
            struct
            {
                uint8_t COLDJUNCRES : 1;
                uint8_t ADCRES      : 2;
                uint8_t BURSTMODE   : 3;
                uint8_t SHUTDOWN    : 2;
            } bits;
        } _reg_config;

        union sensor_reg_t
        {
            uint8_t value;
            struct
            {
                uint8_t             : 1;
                uint8_t TYPE        : 3;
                uint8_t             : 1;
                uint8_t FILTER      : 2;
            } bits;
        } _reg_sensor;

        // config_reg_t _reg_config;
        // sensor_reg_t _reg_sensor;
};

#endif //__MCP9600_H
