#include "iic.h"

/**
 * @brief 构造函数
 * @param scl_pin SCL引脚号，默认为D1 (GPIO5)
 * @param sda_pin SDA引脚号，默认为D2 (GPIO4)
 */
US22310S_I2C::US22310S_I2C(uint8_t scl_pin, uint8_t sda_pin) {
    _scl_pin = scl_pin;
    _sda_pin = sda_pin;
}

/**
 * @brief 初始化I2C通信
 */
void US22310S_I2C::begin() {
    Wire.begin(_sda_pin, _scl_pin);
    Wire.setClock(100000); // 设置I2C时钟频率为100kHz
}

/**
 * @brief 写数据到I2C设备
 * @param device_addr 设备地址（8位地址，例如0x72）
 * @param data 数据缓冲区
 * @param length 数据长度
 * @return 写入成功返回true，失败返回false
 * 
 * 注意：US22310S使用8位I2C地址格式，所以需要右移1位
 */
bool US22310S_I2C::write(uint8_t device_addr, uint8_t *data, uint8_t length) {
    // 将8位地址转换为7位地址（Arduino Wire库使用7位地址）
    Wire.beginTransmission(device_addr >> 1);
    for (uint8_t i = 0; i < length; i++) {
        Wire.write(data[i]);
    }
    uint8_t result = Wire.endTransmission();
    return (result == 0);
}

/**
 * @brief 从I2C设备读取数据
 * @param device_addr 设备地址（8位读地址，例如0x73）
 * @param data 数据缓冲区
 * @param length 数据长度
 * @return 读取成功返回true，失败返回false
 * 
 * 注意：根据US22310S文档，读取时直接使用读地址0x73
 */
bool US22310S_I2C::read(uint8_t device_addr, uint8_t *data, uint8_t length) {
    // 将8位读地址转换为7位地址（Arduino Wire库使用7位地址）
    // 0x73 >> 1 = 0x39
    uint8_t bytes_read = Wire.requestFrom((uint8_t)(device_addr >> 1), length);
    
    if (bytes_read != length) {
        return false;
    }
    
    uint8_t i = 0;
    while (Wire.available() && i < length) {
        data[i++] = Wire.read();
    }
    
    return (i == length);
}

/**
 * @brief 启动单次测量
 * @return 测量启动成功返回true，失败返回false
 * 
 * 根据文档：
 * WriteCurrent(0x72);    // 写模块地址 0x72
 * WriteCurrent(0x50);    // 写单次测量模式  
 * WriteCurrent(0x10);    // 写启动测量命令
 * 
 * Arduino Wire库自动处理地址，所以只发送：0x50, 0x10
 */
bool US22310S_I2C::startSingleMeasurement() {
    uint8_t cmd_data[2] = {CMD_SINGLE_MEASURE, 0x10};
    return write(US22310S_WRITE_ADDR, cmd_data, 2);
}

/**
 * @brief 设置阈值中断模式
 * @param threshold_mm 阈值距离，单位mm
 * @return 设置成功返回true，失败返回false
 * 
 * 根据文档：
 * WriteCurrent(0x72);    // 写模块地址 0x72
 * WriteCurrent(0x51);    // 写中断测量模式
 * WriteCurrent(0x64);    // 设定距离值100mm (0x64 = 100)
 */
bool US22310S_I2C::setThresholdMode(uint8_t threshold_mm) {
    uint8_t cmd_data[2] = {CMD_THRESHOLD_MODE, threshold_mm};
    return write(US22310S_WRITE_ADDR, cmd_data, 2);
}

/**
 * @brief 读取测量距离（单字节模式）
 * @param distance_mm 存储距离值的指针，单位mm
 * @return 读取成功返回true，失败返回false
 * 
 * 根据文档：
 * WriteCurrent(0x73);    // 写地址+读命令  0x72+0x01
 * m1=ReadData();         // 读出数据
 */
bool US22310S_I2C::readDistance(uint8_t *distance_mm) {
    return read(US22310S_READ_ADDR, distance_mm, 1);
}

/**
 * @brief 读取测量距离（双字节模式）
 * @param distance_mm 存储距离值的指针，单位mm
 * @return 读取成功返回true，失败返回false
 * 
 * 某些模块可能返回2字节数据：高字节+低字节
 */
bool US22310S_I2C::readDistance16(uint16_t *distance_mm) {
    uint8_t data[2];
    if (read(US22310S_READ_ADDR, data, 2)) {
        // 高字节在前，低字节在后
        *distance_mm = (data[0] << 8) | data[1];
        return true;
    }
    return false;
}

/**
 * @brief 检查设备是否在线
 * @return 设备在线返回true，离线返回false
 */
bool US22310S_I2C::checkDevice() {
    // US22310S的I2C地址是0x72（8位），转换为7位是0x39
    Wire.beginTransmission(US22310S_WRITE_ADDR >> 1);
    uint8_t error = Wire.endTransmission();
    return (error == 0);
}
