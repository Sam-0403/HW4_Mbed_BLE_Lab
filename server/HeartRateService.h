#include <cstdint>
#warning "These services are deprecated and will be removed. Please see services.md for details about replacement services."

#ifndef MBED_BLE_HEART_RATE_SERVICE_H__
#define MBED_BLE_HEART_RATE_SERVICE_H__

#include "ble/BLE.h"
#include "ble/Gap.h"
#include "ble/GattServer.h"

#if BLE_FEATURE_GATT_SERVER

class HeartRateService {
public:
    /**
     * Intended location of the heart rate sensor.
     */
    enum BodySensorLocation {
        /**
         * Other location.
         */
        LOCATION_OTHER = 0,

        /**
         * Chest.
         */
        LOCATION_CHEST = 1,

        /**
         * Wrist.
         */
        LOCATION_WRIST = 2,

        /**
         * Finger.
         */
        LOCATION_FINGER,

        /**
         * Hand.
         */
        LOCATION_HAND,

        /**
         * Earlobe.
         */
        LOCATION_EAR_LOBE,

        /**
         * Foot.
         */
        LOCATION_FOOT,
    };

public:
    HeartRateService(BLE &_ble, uint16_t hrmCounter, BodySensorLocation location) :
        ble(_ble),
        valueBytes(hrmCounter),
        hrmRate(
            GattCharacteristic::UUID_HEART_RATE_MEASUREMENT_CHAR,
            valueBytes.getPointer(),
            valueBytes.getNumValueBytes(),
            HeartRateValueBytes::MAX_VALUE_BYTES,
            GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY
        ),
        hrmLocation(
            GattCharacteristic::UUID_BODY_SENSOR_LOCATION_CHAR,
            reinterpret_cast<uint8_t*>(&location)
        )
    {
        setupService();
    }

    void updateHeartRate(uint16_t hrmCounter) {
        valueBytes.updateHeartRate(hrmCounter, 1);
        ble.gattServer().write(
            hrmRate.getValueHandle(),
            valueBytes.getPointer(),
            valueBytes.getNumValueBytes()
        );
    }

protected:
    /**
     * Construct and add to the GattServer the heart rate service.
     */
    void setupService() {

        GattCharacteristic *charTable[] = {
            &hrmRate,
            &hrmLocation
        };
        GattService hrmService(
            GattService::UUID_HEART_RATE_SERVICE,
            charTable,
            sizeof(charTable) / sizeof(charTable[0])
        );
        ble.gattServer().addService(hrmService);
    }

protected:
    /*
     * Heart rate measurement value.
     */
    struct HeartRateValueBytes {
        /* 1 byte for the Flags, and up to two bytes for heart rate value. */
        static const unsigned MAX_VALUE_BYTES = 4;
        static const unsigned FLAGS_BYTE_INDEX = 0;

        static const unsigned VALUE_FORMAT_BITNUM = 0;
        static const uint8_t  VALUE_FORMAT_FLAG = (1 << VALUE_FORMAT_BITNUM);

        HeartRateValueBytes(uint16_t hrmCounter) : valueBytes()
        {
            updateHeartRate(hrmCounter, 0);
        }

        void updateHeartRate(uint16_t hrmCounter, uint8_t id)
        {
            valueBytes[FLAGS_BYTE_INDEX] = 0;
            valueBytes[FLAGS_BYTE_INDEX + 1] = (uint8_t)(hrmCounter & 0xFF);
            valueBytes[FLAGS_BYTE_INDEX + 2] = (uint8_t)(hrmCounter >> 8);
            valueBytes[FLAGS_BYTE_INDEX + 3] = id;
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
    HeartRateValueBytes valueBytes;
    GattCharacteristic hrmRate;
    ReadOnlyGattCharacteristic<uint8_t> hrmLocation;
};

#endif // BLE_FEATURE_GATT_SERVER

#endif /* #ifndef MBED_BLE_HEART_RATE_SERVICE_H__*/
