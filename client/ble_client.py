from bluepy.btle import Peripheral, UUID
from bluepy.btle import Scanner, DefaultDelegate, AssignedNumbers
import struct
INDIC_ON = struct.pack('BB', 0x02, 0x00)
NOTIF_ON = struct.pack('BB', 0x01, 0x00)
NOTIF_OFF = struct.pack('BB', 0x00, 0x00)

# user configuration
# DEVICE_NAME = "AQUOS V"
# SERVICE_UUID = 0xfff0
# CHAR_UUID = 0xfff4
DEVICE_NAME = "Sam's Sensor"

HEART_SERVICE_UUID = 0x180d
HEART_RATE_UUID = 0x2a37
HEART_LOCATION_UUID = 0x2a38

MAG_SERVICE_UUID = 0xa000
MAG_X_RATE_UUID = 0xa001
MAG_Y_RATE_UUID = 0xa002
MAG_Z_RATE_UUID = 0xa003

# DefaultDelegate: to receive Bluetooth message asynchronously

def dataParsing(data, type):
    data_list = list(data)
    if data_list[0]==0:
        final_data = int(data_list[2])*256+int(data_list[1])
        print("Data: ", final_data)
    elif data_list[0]==1:
        final_data = int(data_list[2])*256+int(data_list[1])-65536
        print(type, ": ", final_data)

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
class HeartRateDelegate(DefaultDelegate):
    def __init__(self, handle):
        DefaultDelegate.__init__(self)
        #print("handleNotification init")
        self.hndl = handle

    def handleNotification(self, cHandle, data):
        dataParsing(data, "HeartRate")

class MagXRateDelegate(DefaultDelegate):
    def __init__(self, handle):
        DefaultDelegate.__init__(self)
        #print("handleNotification init")
        self.hndl = handle

    def handleNotification(self, cHandle, data):
        dataParsing(data, "Magneto X")

class MagYRateDelegate(DefaultDelegate):
    def __init__(self, handle):
        DefaultDelegate.__init__(self)
        #print("handleNotification init")
        self.hndl = handle

    def handleNotification(self, cHandle, data):
        dataParsing(data, "Magneto Y")

class MagZRateDelegate(DefaultDelegate):
    def __init__(self, handle):
        DefaultDelegate.__init__(self)
        #print("handleNotification init")
        self.hndl = handle

    def handleNotification(self, cHandle, data):
        dataParsing(data, "Magneto Z")

# withDelegate: to stores a reference to a delegate object, which receives callbacks when broadcasts from devices are received
scanner = Scanner().withDelegate(ScanDelegate()) 
devices = scanner.scan(3.0)
n = 0
for dev in devices:
    print ("%d: Device %s (%s), RSSI=%d dB" % (n, dev.addr, dev.addrType, dev.rssi))
    n += 1
    for (adtype, desc, value) in dev.getScanData():
        if (value == DEVICE_NAME):
            print('/////////////////////////////////////////////////////////////////')
            print('HERE IS THE NAME OF YOUR DEVICE:')
            print ("%s, %s" % (desc, value))
            print('/////////////////////////////////////////////////////////////////')
        else:
            print ("%s, %s" % (desc, value))

number = input('Enter your device number: ')
print('Device', number)
print(list(devices)[int(number)].addr)

print ("Connecting...")
dev = Peripheral(list(devices)[int(number)].addr, 'random')

# mode = input('Choose the mode: (1: default; else: custom)')
try:
    # if mode=="1":
    #     print ("Services...")
    #     for service in dev.services:
    #         print (str(service))
    #     print (str(dev.getServiceByUUID(UUID(SERVICE_UUID))))
    #     ch = dev.getCharacteristics(uuid=UUID(CHAR_UUID))[0]    # writeHandle = ch.valHandle or ch.getHandle()
    # else:
    #     print ("Services...")
    #     n = 0
    #     services = dev.services
    #     for service in services:
    #         print (str(n)+": "+str(service))
    #         n+=1
    #     number = input('Enter your service number: ')
    #     print('Service', number)
    #     service_uuid = list(services)[int(number)].uuid
    #     print(service_uuid)
    #     service = dev.getServiceByUUID(UUID(service_uuid))
    #     print (str(service))

    #     print ("Characteristics...")
    #     n = 0
    #     characteristics = service.getCharacteristics()
    #     for ch in characteristics:
    #         print (str(n)+": "+str(ch))
    #         n+=1
    #     number = input('Enter your characteristic number: ')
    #     print('Characteristic', number)
    #     ch = list(characteristics)[int(number)]
    #     print (str(ch))
        
    # #dev.writeCharacteristic(ch.valHandle, b"\x65\x66")     # Test writeCharacteristic
    # desc = ch.getDescriptors(AssignedNumbers.client_characteristic_configuration)
    # print('Do you want to set the CCCD to notification or indication mode?')
    # notify_or_indicate = int(input('Please type 1 for notification, or type 2 for indication: '))
    # one = 1
    # two = 2
    # if (notify_or_indicate == one): #notification
    #     dev.writeCharacteristic(desc[0].handle, NOTIF_ON)
    #     print('Successfully changed the CCCD to notification. Please check the Server Log page in the BLE-tool app.')
    # elif (notify_or_indicate == two): #indication
    #     dev.writeCharacteristic(desc[0].handle, INDIC_ON)
    #     print('Successfully changed the CCCD to indication. Please chekc the Server Log page in the BLE-tool app.')
    # else:
    #     print('Wrong format, sorry. The CCCD remains unchanged.')
    # #dev.writeCharacteristic(desc[0].handle, INDIC_ON)  # To Modify CCCD

    # custom_service_handle_cccd = ch.valHandle + 1
    # dev = dev.withDelegate(PeripheralDelegate(custom_service_handle_cccd))

    print ("Services...")
    for service in dev.services:
        print (str(service))
    # print (str(dev.getServiceByUUID(UUID(SERVICE_UUID))))
    heart_service = dev.getServiceByUUID(HEART_SERVICE_UUID)
    ch_heartrate = dev.getCharacteristics(uuid=UUID(HEART_RATE_UUID))[0]
    ch_heartlocation = dev.getCharacteristics(uuid=UUID(HEART_LOCATION_UUID))[0]

    mag_service = dev.getServiceByUUID(MAG_SERVICE_UUID)
    ch_mag_x_rate = dev.getCharacteristics(uuid=MAG_X_RATE_UUID)[0]
    ch_mag_y_rate = dev.getCharacteristics(uuid=MAG_Y_RATE_UUID)[0]
    ch_mag_z_rate = dev.getCharacteristics(uuid=MAG_Z_RATE_UUID)[0]

    # ch = dev.getCharacteristics(uuid=UUID(CHAR_UUID))[0]    # writeHandle = ch.valHandle or ch.getHandle()

    #dev.writeCharacteristic(ch.valHandle, b"\x65\x66")     # Test writeCharacteristic
    # print('Do you want to set the CCCD to notification or indication mode?')
    # dev.writeCharacteristic(desc[0].handle, NOTIF_ON)

    # custom_service_handle_cccd = ch.valHandle + 1
    # dev = dev.withDelegate(PeripheralDelegate(custom_service_handle_cccd))
    ch_heartrate_handle_cccd = ch_heartrate.valHandle + 1
    dev = dev.withDelegate(HeartRateDelegate(ch_heartrate_handle_cccd))

    # ch_heartlocation_handle_cccd = ch_heartlocation.valHandle + 1
    # dev = dev.withDelegate(PeripheralDelegate(ch_heartlocation_handle_cccd))

    ch_mag_x_handle_cccd = ch_mag_x_rate.valHandle + 1
    dev = dev.withDelegate(MagXRateDelegate(ch_mag_x_handle_cccd))

    ch_mag_y_handle_cccd = ch_mag_y_rate.valHandle + 1
    dev = dev.withDelegate(MagYRateDelegate(ch_mag_y_handle_cccd))

    ch_mag_z_handle_cccd = ch_mag_z_rate.valHandle + 1
    dev = dev.withDelegate(MagZRateDelegate(ch_mag_z_handle_cccd))

    while True:
        if dev.waitForNotifications(3.0):
            #print("Notification, now:")
            continue
        print ("Waiting...")
    
finally:
    dev.disconnect()