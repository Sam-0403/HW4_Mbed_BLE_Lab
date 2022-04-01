from bluepy.btle import Peripheral, UUID
from bluepy.btle import Scanner, DefaultDelegate
import struct
INDIC_ON = struct.pack('BB', 0x02, 0x00)
NOTIF_ON = struct.pack('BB', 0x01, 0x00)
NOTIF_OFF = struct.pack('BB', 0x00, 0x00)

# user configuration
DEVICE_NAME = "Sam's Sensor"

HEART_SERVICE_UUID = 0x180d
HEART_RATE_UUID = 0x2a37
HEART_LOCATION_UUID = 0x2a38

MAG_SERVICE_UUID = 0xa000
MAG_X_RATE_UUID = 0xa001
MAG_Y_RATE_UUID = 0xa002
MAG_Z_RATE_UUID = 0xa003

# DefaultDelegate: to receive Bluetooth message asynchronously

def dataParsing(data):
    data_list = list(data)
    if data_list[3]==1:
        print("//--HeartRate--//")
    elif data_list[3]==2:
        print("//--MagXRate--//")
    elif data_list[3]==3:
        print("//--MagYRate--//")
    elif data_list[3]==4:
        print("//--MagZRate--//")

    if data_list[0]==0:
        final_data = int(data_list[2])*256+int(data_list[1])
        print("Data: ", final_data)
    elif data_list[0]==1:
        final_data = int(data_list[2])*256+int(data_list[1])-65536
        print("Data: ", final_data)

# ScanDelegate: to scan for BLE devices which are broadcasting data
class ScanDelegate(DefaultDelegate):
    def __init__(self):
        DefaultDelegate.__init__(self)
    def handleDiscovery(self, dev, isNewDev, isNewData):
        if isNewDev:
            print("Discovered device", dev.addr)
        elif isNewData:
            print("Received new data from", dev.addr)

# PeripheralDelegate: to handle notification from BLE server
class PeripheralDelegate(DefaultDelegate):
    def __init__(self, handle):
        DefaultDelegate.__init__(self)
        #print("handleNotification init")
        self.hndl = handle

    def handleNotification(self, cHandle, data):
        dataParsing(data)

# withDelegate: to stores a reference to a delegate object, which receives callbacks when broadcasts from devices are received
scanner = Scanner().withDelegate(ScanDelegate()) 
devices = scanner.scan(3.0)
n = 0
device_num = 0
for dev in devices:
    print ("%d: Device %s (%s), RSSI=%d dB" % (n, dev.addr, dev.addrType, dev.rssi))
    n += 1
    for (adtype, desc, value) in dev.getScanData():
        if (value == DEVICE_NAME):
            print('/////////////////////////////////////////////////////////////////')
            print('HERE IS THE NAME OF YOUR DEVICE:')
            print ("%s, %s" % (desc, value))
            print('/////////////////////////////////////////////////////////////////')
            device_num = n-1
        else:
            print ("%s, %s" % (desc, value))

print ("Connecting...")
dev = Peripheral(list(devices)[device_num].addr, 'random')

try:

    print ("Services...")
    for service in dev.services:
        print (str(service))
    heart_service = dev.getServiceByUUID(HEART_SERVICE_UUID)
    ch_heartrate = dev.getCharacteristics(uuid=UUID(HEART_RATE_UUID))[0]
    ch_heartlocation = dev.getCharacteristics(uuid=UUID(HEART_LOCATION_UUID))[0]

    mag_service = dev.getServiceByUUID(MAG_SERVICE_UUID)
    ch_mag_x_rate = dev.getCharacteristics(uuid=MAG_X_RATE_UUID)[0]
    ch_mag_y_rate = dev.getCharacteristics(uuid=MAG_Y_RATE_UUID)[0]
    ch_mag_z_rate = dev.getCharacteristics(uuid=MAG_Z_RATE_UUID)[0]

    print(str(ch_mag_x_rate.valHandle))
    print(str(ch_mag_y_rate.valHandle))
    print(str(ch_mag_z_rate.valHandle))

    ch_heartrate_handle_cccd = ch_heartrate.valHandle + 1
    ch_mag_x_handle_cccd = ch_mag_x_rate.valHandle + 1
    ch_mag_y_handle_cccd = ch_mag_y_rate.valHandle + 1
    ch_mag_z_handle_cccd = ch_mag_z_rate.valHandle + 1
    dev = dev.withDelegate(PeripheralDelegate(ch_heartrate_handle_cccd))
    dev.writeCharacteristic(ch_heartrate_handle_cccd, NOTIF_ON)
    dev.writeCharacteristic(ch_mag_x_handle_cccd, NOTIF_ON)
    dev.writeCharacteristic(ch_mag_y_handle_cccd, NOTIF_ON)
    dev.writeCharacteristic(ch_mag_z_handle_cccd, NOTIF_ON)

    # # ch_heartlocation_handle_cccd = ch_heartlocation.valHandle + 1
    # # dev = dev.withDelegate(PeripheralDelegate(ch_heartlocation_handle_cccd))

    # ch_mag_x_handle_cccd = ch_mag_x_rate.valHandle + 1
    # devX = dev.withDelegate(MagXRateDelegate(ch_mag_x_rate))
    # devX.writeCharacteristic(ch_mag_x_handle_cccd, NOTIF_ON)


    # ch_mag_y_handle_cccd = ch_mag_y_rate.valHandle + 1
    # devY = dev.withDelegate(MagYRateDelegate(ch_mag_y_rate))
    # devY.writeCharacteristic(ch_mag_y_handle_cccd, NOTIF_ON)

    # ch_mag_z_handle_cccd = ch_mag_z_rate.valHandle + 1
    # devZ = dev.withDelegate(MagZRateDelegate(ch_mag_z_rate))
    # devZ.writeCharacteristic(ch_mag_z_handle_cccd, NOTIF_ON)

    while True:
        if dev.waitForNotifications(3.0):
            continue

        print ("Waiting...")
    
finally:
    dev.disconnect()