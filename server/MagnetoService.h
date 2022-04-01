#include <cstdint>
#warning "These services are deprecated and will be removed. Please see services.md for details about replacement services."

#ifndef MBED_BLE_MAGNETO_SENSOR_SERVICE_H__
#define MBED_BLE_MAGNETO_SENSOR_SERVICE_H__

#include "ble/BLE.h"
#include "ble/Gap.h"
#include "ble/GattServer.h"

#if BLE_FEATURE_GATT_SERVER

class MagnetoSensorService {
public:
    const static uint16_t UUID_MAG_SENSOR_SERVICE = 0xA000;
    const static uint16_t UUID_MAG_X_RATE_MEASUREMENT_CHAR = 0xA001;
    const static uint16_t UUID_MAG_Y_RATE_MEASUREMENT_CHAR = 0xA002;
    const static uint16_t UUID_MAG_Z_RATE_MEASUREMENT_CHAR = 0xA003;
    MagnetoSensorService(BLE &_ble, int16_t* magCounter) :
        ble(_ble),
        valueXBytes(magCounter[0]),
        valueYBytes(magCounter[1]),
        valueZBytes(magCounter[2]),
        magXRate(
            MagnetoSensorService::UUID_MAG_X_RATE_MEASUREMENT_CHAR,
            valueXBytes.getPointer(),
            valueXBytes.getNumValueBytes(),
            MagRateValueBytes::MAX_VALUE_BYTES,
            GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY
        ),
        magYRate(
            MagnetoSensorService::UUID_MAG_Y_RATE_MEASUREMENT_CHAR,
            valueYBytes.getPointer(),
            valueYBytes.getNumValueBytes(),
            MagRateValueBytes::MAX_VALUE_BYTES,
            GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY
        ),
        magZRate(
            MagnetoSensorService::UUID_MAG_Z_RATE_MEASUREMENT_CHAR,
            valueZBytes.getPointer(),
            valueZBytes.getNumValueBytes(),
            MagRateValueBytes::MAX_VALUE_BYTES,
            GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY
        )
    {
        setupService();
    }

    void updateSensorRate(int16_t* magCounter) {
        valueXBytes.updateMagRate((uint16_t)magCounter[0], 2);
        valueYBytes.updateMagRate((uint16_t)magCounter[1], 3);
        valueZBytes.updateMagRate((uint16_t)magCounter[2], 4);
        ble.gattServer().write(
            magXRate.getValueHandle(),
            valueXBytes.getPointer(),
            valueXBytes.getNumValueBytes()
        );
        ble.gattServer().write(
            magYRate.getValueHandle(),
            valueYBytes.getPointer(),
            valueYBytes.getNumValueBytes()
        );
        ble.gattServer().write(
            magZRate.getValueHandle(),
            valueZBytes.getPointer(),
            valueZBytes.getNumValueBytes()
        );

    }

protected:
    /**
     * Construct and add to the GattServer the heart rate service.
     */
    void setupService() {
        GattCharacteristic *charTable[] = {
            &magXRate,
            &magYRate,
            &magZRate
        };
        GattService magService(
            MagnetoSensorService::UUID_MAG_SENSOR_SERVICE,
            charTable,
            sizeof(charTable) / sizeof(charTable[0])
        );
        ble.gattServer().addService(magService);
    }

protected:
    struct MagRateValueBytes {
        /* 1 byte for the Flags, and up to two bytes for mag rate value. */
        static const unsigned MAX_VALUE_BYTES = 4;
        static const unsigned FLAGS_BYTE_INDEX = 0;

        static const unsigned VALUE_FORMAT_BITNUM = 0;
        static const uint8_t  VALUE_FORMAT_FLAG = (1 << VALUE_FORMAT_BITNUM);

        MagRateValueBytes(int16_t magCounter) : valueBytes()
        {
            updateMagRate(magCounter, 0);
        }

        void updateMagRate(uint16_t magCounter, uint8_t id)
        {
            if(magCounter>=32768){
                valueBytes[FLAGS_BYTE_INDEX] = 1;
                valueBytes[FLAGS_BYTE_INDEX + 1] = (uint8_t)(magCounter & 0xFF);
                valueBytes[FLAGS_BYTE_INDEX + 2] = (uint8_t)(magCounter >> 8);
                valueBytes[FLAGS_BYTE_INDEX + 3] = id;
            }
            else{
                valueBytes[FLAGS_BYTE_INDEX] = 0;
                valueBytes[FLAGS_BYTE_INDEX + 1] = (uint8_t)(magCounter & 0xFF);
                valueBytes[FLAGS_BYTE_INDEX + 2] = (uint8_t)(magCounter >> 8);
                valueBytes[FLAGS_BYTE_INDEX + 3] = id;
            }
        }

        uint8_t *getPointer()
        {
            return valueBytes;
        }

        const uint8_t *getPointer() const
        {
            return valueBytes;
        }

        unsigned getNumValueBytes() const
        {
            return MAX_VALUE_BYTES;
        }

    private:
        uint8_t valueBytes[MAX_VALUE_BYTES];
    };

protected:
    BLE &ble;
    MagRateValueBytes valueXBytes;
    MagRateValueBytes valueYBytes;
    MagRateValueBytes valueZBytes;
    GattCharacteristic magXRate;
    GattCharacteristic magYRate;
    GattCharacteristic magZRate;
};

#endif // BLE_FEATURE_GATT_SERVER

#endif /* #ifndef MBED_BLE_MAGNETO_SENSOR_SERVICE_H__*/
