HW4_Mbed_BLE_Lab
===

# 目標
透過Mbed os提供的BLE API撰寫程式，控制STM32控制板作為GATT Server，發送模擬的Heart Rate信號、Magneto Sensor偵測的信號;RaspberryPi作為GATT Client，接收並完成如同BLE Scanner的功能，將資料顯示出來。

# How to Run Code

## On Mbed Program

* `git clone https://github.com/ARMmbed/mbed-os-example-ble`
* `cd mbed-os-example-ble`
* New an mbed studio project, delete the `main.cpp`
* Copy the files under the directory `BLE_GattServer_AddService` to the new project
* Modify `mbed_app.json`
```javascript
{
    "target_overrides": {
        ...
        // 新增以下程式碼
        "DISCO_L475VG_IOT01A": {
            "target.features_add": ["BLE"],
            "target.extra_labels_add": ["CORDIO", "CORDIO_BLUENRG"]
        }
    }
}
```

## On RaspberryPi
待補

# 程式架構
* `HeartRateService.h`
    * HeartRateService  0x180D
        * hrmRate   0x2A37
        * hrmLocation   0x2A38
* `MagnetoService.h`
    * MagnetoSensorService  0xA000
        * magXRate  0xA001
        * magYRate  0xA002
        * magZRate  0xA003
* `main.cpp`

# 運行結果
在RPI上，會看到如下圖的結果。
![report](./report.pdf)

# 問題紀錄

## 問題一：資料型別的轉換
透過程式原本的架構編寫的話，需要為偵測器讀取的int16_t資料建立一個轉換型別的`struct`，如：`HeartRateService.h`中的`HeartRateValueBytes`，但其實也可以透過以下程式碼進行資料的更新
```javascript
void update{NAME}State({TYPE} newState) {
    ble.gattServer().write({NAME}State.getValueHandle(), (uint8_t *)&newState, sizeof({TYPE}));
}
```
透過`(uint8_t *)`與對應`sizeof({TYPE})`直接進行更新。

## 問題二：RPI端仍需要writeCharacteristic
```javascript
dev.writeCharacteristic({SERVICE}_handle_cccd, NOTIF_ON)
```
由於`bluepy`中的函式設定`Delegate`的位置，所以仍須加上
```javascript
def writeCharacteristic(self, handle, val, withResponse=False, timeout=None):
    # Without response, a value too long for one packet will be truncated,
    # but with response, it will be sent as a queued write
    cmd = "wrr" if withResponse else "wr"
    self._writeCmd("%s %X %s\n" % (cmd, handle, binascii.b2a_hex(val).decode('utf-8')))
    return self._getResp('wr', timeout)
```
```javascript
def _getResp(self, wantType, timeout=None):
    ...
        respType = resp['rsp'][0]
        if respType == 'ntfy' or respType == 'ind':
            hnd = resp['hnd'][0]
            data = resp['d'][0]
            if self.delegate is not None:
                self.delegate.handleNotification(hnd, data)
    ...
```
