//导入BLE功能库
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
/*
   -----------------------------------
  |         以下定义与BLE相关           |
   -----------------------------------
*/
//以下网站可以生成UUID：
//https://www.uuidgenerator.net/
#define SERVICE_UUID        "0c548795-49ae-4a30-b03d-1ed905668672"  //服务UUID
#define CHARACTERISTIC_UUID "aae8e1b6-d2f6-460f-811b-361f35bfcc0c"  //特性UUID
const char *server_name = "BLE_server";           //定义服务器名称
BLEServer *pServer = NULL;  //定义为全局空变量，方便之后调用
//定义布尔变量
bool deviceConnected = false;    //设备连接状态
bool oldDeviceConnected = false; //之前设备连接状态

//定义类，蓝牙事件返回调用
class MyServerCallbacks : public BLEServerCallbacks  //蓝牙服务器返回调用类
{
    void onConnect(BLEServer *pServer)
    {
      deviceConnected = true;
    };
    void onDisconnect(BLEServer *pServer)
    {
      deviceConnected = false;
    }
};

class MyCallbacks : public BLECharacteristicCallbacks  ////蓝牙特征返回调用类
{
    //以下仅为示例，实际应用中为回调函数。
    void onWrite(BLECharacteristic *pCharacteristic) {
      String command = pCharacteristic->getValue().c_str();
      bool new_command = true;
      Serial.print("收到指令：");
      Serial.println(command);
      if (command == "TO") {
        Serial.print("TO收到");
      }
    }
};

void setup() {
  /*
       -----------------------------------
      |         以下为BLE初始化             |
       -----------------------------------
  */
  Serial.begin(9600);  //初始化串口通讯
  Serial.println("开始初始化蓝牙低功耗！");
  BLEDevice::init(server_name);  //初始化一个蓝牙设备
  BLEServer *pServer = BLEDevice::createServer();  //创建一个蓝牙服务器
  pServer->setCallbacks(new MyServerCallbacks());  //服务器回调函数设置为MyServerCallbacks
  BLEService *pService = pServer->createService(SERVICE_UUID);  //创建一个BLE服务
  BLECharacteristic *pCharacteristic = pService->createCharacteristic( //创建一个特征值，类型是读写
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  pCharacteristic->setValue("Hello World says Neil");
  pCharacteristic->setCallbacks(new MyCallbacks());  //为特征添加一个回调
  pService->start();  //开启服务
  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();  //服务器开始广播
  Serial.println("已定义特征，开始广播！");

}

void loop() {
  checkToReconnect();
  //每循环周期检查连接状态

}

void checkToReconnect() {  //检查设备是否断开连接，如果断开，则准备重新广播
  if (!deviceConnected && oldDeviceConnected) {//新发生的断连事件，重新广播
    delay(1000);   //让蓝牙堆栈缓冲
    pServer->startAdvertising();  //重新广播
    Serial.println("已断连，准备重新广播...");
    oldDeviceConnected = deviceConnected;
  }
  if (deviceConnected && !oldDeviceConnected) {//已重新连接
    Serial.println("已连接设备！");
    oldDeviceConnected = deviceConnected;
  }
}
