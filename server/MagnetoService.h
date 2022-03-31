/* MBED_DEPRECATED */
#include <cstdint>
#warning "These services are deprecated and will be removed. Please see services.md for details about replacement services."

#ifndef MBED_BLE_MAGNETO_SENSOR_SERVICE_H__
#define MBED_BLE_MAGNETO_SENSOR_SERVICE_H__

#include "ble/BLE.h"
#include "ble/Gap.h"
#include "ble/GattServer.h"

#if BLE_FEATURE_GATT_SERVER

/**
 * BLE Heart Rate Service.
 *
 * @par purpose
 *
 * Fitness applications use the heart rate service to expose the heart
 * beat per minute measured by a heart rate sensor.
 *
 * Clients can read the intended location of the sensor and the last heart rate
 * value measured. Additionally, clients can subscribe to server initiated
 * updates of the heart rate value measured by the sensor. The service delivers
 * these updates to the subscribed client in a notification packet.
 *
 * The subscription mechanism is useful to save power; it avoids unecessary data
 * traffic between the client and the server, which may be induced by polling the
 * value of the heart rate measurement characteristic.
 *
 * @par usage
 *
 * When this class is instantiated, it adds a heart rate service in the GattServer.
 * The service contains the location of the sensor and the initial value measured
 * by the sensor.
 *
 * Application code can invoke updateHeartRate() when a new heart rate measurement
 * is acquired; this function updates the value of the heart rate measurement
 * characteristic and notifies the new value to subscribed clients.
 *
 * @note You can find specification of the heart rate service here:
 * https://www.bluetooth.com/specifications/gatt
 *
 * @attention The service does not expose information related to the sensor
 * contact, the accumulated energy expanded or the interbeat intervals.
 *
 * @attention The heart rate profile limits the number of instantiations of the
 * heart rate services to one.
 */
class MagnetoSensorService {
public:
    /**
     * Construct and initialize a heart rate service.
     *
     * The construction process adds a GATT heart rate service in @p _ble
     * GattServer, sets the value of the heart rate measurement characteristic
     * to @p hrmCounter and the value of the body sensor location characteristic
     * to @p location.
     *
     * @param[in] _ble BLE device that hosts the heart rate service.
     * @param[in] hrmCounter Heart beats per minute measured by the heart rate
     * sensor.
     * @param[in] location Intended location of the heart rate sensor.
     */
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

    /**
     * Update the heart rate that the service exposes.
     *
     * The server sends a notification of the new value to clients that have
     * subscribed to updates of the heart rate measurement characteristic; clients
     * reading the heart rate measurement characteristic after the update obtain
     * the updated value.
     *
     * @param[in] hrmCounter Heart rate measured in BPM.
     *
     * @attention This function must be called in the execution context of the
     * BLE stack.
     */
    void updateSensorRate(int16_t* magCounter) {
        valueXBytes.updateMagRate((uint16_t)magCounter[0]);
        valueYBytes.updateMagRate((uint16_t)magCounter[1]);
        valueZBytes.updateMagRate((uint16_t)magCounter[2]);
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
        static const unsigned MAX_VALUE_BYTES = 3;
        static const unsigned FLAGS_BYTE_INDEX = 0;

        static const unsigned VALUE_FORMAT_BITNUM = 0;
        static const uint8_t  VALUE_FORMAT_FLAG = (1 << VALUE_FORMAT_BITNUM);

        MagRateValueBytes(int16_t magCounter) : valueBytes()
        {
            updateMagRate(magCounter);
        }

        void updateMagRate(uint16_t magCounter)
        {
            // uint16_t u_magCounter = magCounter+32768;
            if (magCounter <= 255) {
                valueBytes[FLAGS_BYTE_INDEX] &= ~VALUE_FORMAT_FLAG;
                valueBytes[FLAGS_BYTE_INDEX + 1] = magCounter;
            } else {
                valueBytes[FLAGS_BYTE_INDEX] |= VALUE_FORMAT_FLAG;
                valueBytes[FLAGS_BYTE_INDEX + 1] = (uint8_t)(magCounter & 0xFF);
                valueBytes[FLAGS_BYTE_INDEX + 2] = (uint8_t)(magCounter >> 8);
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
            if (valueBytes[FLAGS_BYTE_INDEX] & VALUE_FORMAT_FLAG) {
                return 1 + sizeof(uint16_t);
            } else {
                return 1 + sizeof(uint8_t);
            }
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
