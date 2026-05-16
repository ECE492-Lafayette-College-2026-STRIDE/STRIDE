/*
 * Battery_status.cpp
 *
 *  Created on: Feb 12, 2026
 *      Author: otienom
 */



extern "C" {
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_i2c.h"
#include "i2c.h"
}
#include "Battery_status.hpp"
namespace battery {

// ----------------- Ctor -----------------

BatteryStatusLTC2944::BatteryStatusLTC2944(I2C_HandleTypeDef* hi2c1, const float rsense_ohms)
    : m_hi2c1(hi2c1),
      m_rsense_ohms(rsense_ohms)
{
}

//Run an initializing routine to determine bus errors or to configure the chip
bool BatteryStatusLTC2944::init(AdcMode adc, Prescaler presc, AlccMode alcc, bool shutdown)
{
    // Optional: touch status register to verify comms
    uint8_t status = 0;
    if (!read8(REG_STATUS_A, status)) return false;
    return configure(adc, presc, alcc, shutdown);
}

//read the status of the LTC2944
bool BatteryStatusLTC2944::readStatus(uint8_t& raw_status_a, StatusBits& decoded)
{
    if (!read8(REG_STATUS_A, raw_status_a)) return false;
    decoded = decodeStatus(raw_status_a);
    return true;
}

bool BatteryStatusLTC2944::readAll(Measurements& out)
{
    uint16_t acr  = 0;
    uint16_t vraw = 0;
    uint16_t iraw = 0;
    uint16_t traw = 0;

    // NOTE: read16() takes the MSB register address
    if (!read16(REG_ACR_MSB_C,  acr))  return false;
    if (!read16(REG_VOLT_MSB_I, vraw)) return false;
    if (!read16(REG_CURR_O,     iraw)) return false; // current is two regs O/P; O is first
    if (!read16(REG_TEMP_U,     traw)) return false; // temp is two regs U/V; U is first

    out.raw_acr_cd     = acr;
    out.raw_voltage_ij = vraw;
    out.raw_current_op = iraw;
    out.raw_temp_uv    = traw;

    out.v_batt_V = convertVoltage_V(vraw);
    out.i_batt_A = convertCurrent_A(iraw);
    out.temp_C   = convertTemp_C(traw);

  //Accumulated charge
    out.acr_mAh = convertAcrTo_mAh(acr, prescalerToM(Prescaler::M256));

    const float battery_capacity_mAh = 30000.0f; // 30Ah battery

    out.capacity_percent = (out.acr_mAh / battery_capacity_mAh) * 100.0f;

    // Limit percentage between 0 and 100
    if (out.capacity_percent > 100.0f)
    {
        out.capacity_percent = 100.0f;
    }
    else if (out.capacity_percent < 0.0f)
    {
        out.capacity_percent = 0.0f;
    }
  //Total Power
    out.power = out.v_batt_V * out.i_batt_A;
    return true;
}

bool BatteryStatusLTC2944::readControl(uint8_t& ctrl)
{
    return read8(REG_CONTROL_B, ctrl);
}

bool BatteryStatusLTC2944::writeControl(uint8_t ctrl)
{
    return write8(REG_CONTROL_B, ctrl);
}
bool BatteryStatusLTC2944::write16(uint8_t reg_msb, uint16_t val)
{
    uint8_t buf[2];

    buf[0] = static_cast<uint8_t>((val >> 8) & 0xFF);
    buf[1] = static_cast<uint8_t>(val & 0xFF);

    const uint16_t dev = static_cast<uint16_t>(I2C_ADDR_7BIT << 1);

    return (HAL_I2C_Mem_Write(m_hi2c1, dev,reg_msb,I2C_MEMADD_SIZE_8BIT,buf,2,100) == HAL_OK);
}
bool BatteryStatusLTC2944::setAccumulatedCharge_mAh(float charge_mAh)
{
    const uint16_t M_value = prescalerToM(Prescaler::M256);
    const float rsense_mOhm = m_rsense_ohms * 1000.0f;
    if (rsense_mOhm <= 0.0f) return false;
    const float qlsb_mAh = 0.340f *(50.0f / rsense_mOhm) *(static_cast<float>(M_value) / 4096.0f);
    uint16_t acr_counts = static_cast<uint16_t>(charge_mAh / qlsb_mAh);
    return write16(REG_ACR_MSB_C, acr_counts);
}
bool BatteryStatusLTC2944::configure(AdcMode adc, Prescaler presc, AlccMode alcc, bool shutdown)
{
    uint8_t ctrl = 0;

    ctrl |= (static_cast<uint8_t>(adc)   & 0x03u) << 6; // B[7:6]
    ctrl |= (static_cast<uint8_t>(presc) & 0x07u) << 3; // B[5:3]
    ctrl |= (static_cast<uint8_t>(alcc)  & 0x03u) << 1; // B[2:1]
    ctrl |= (shutdown ? 1u : 0u);                       // B[0]

    // 0b11 for ALCC is not allowed per datasheet
    if (((ctrl >> 1) & 0x03u) == 0x03u) return false;

    return writeControl(ctrl);
}

// ----------------- Conversions -----------------

uint16_t BatteryStatusLTC2944::prescalerToM(Prescaler p)
{
    switch (p) {
        case Prescaler::M1:        return 1;
        case Prescaler::M4:        return 4;
        case Prescaler::M16:       return 16;
        case Prescaler::M64:       return 64;
        case Prescaler::M256:      return 256;
        case Prescaler::M1024:     return 1024;
        case Prescaler::M4096:
        case Prescaler::M4096_ALT: return 4096;
        default:                   return 4096;
    }
}

float BatteryStatusLTC2944::convertVoltage_V(uint16_t raw_ij) const
{
    // LTC2944: full-scale 70.8V across 16-bit code
    return 70.8f * (static_cast<float>(raw_ij) / 65535.0f);
}

float BatteryStatusLTC2944::convertCurrent_A(uint16_t raw_op) const
{
    // LTC2944 current: excess-32767 format
    const float code  = static_cast<float>(raw_op);
    const float norm  = (code - 32767.0f) / 32767.0f; // -1..+1 approx
    const float vsense = 0.064f * norm;               // +/-64mV full-scale across RSENSE

    if (m_rsense_ohms <= 0.0f) return 0.0f;
    return vsense / m_rsense_ohms;
}

float BatteryStatusLTC2944::convertTemp_C(uint16_t raw_uv) const
{
    // LTC2944 temp: 510K full-scale over 16-bit code
    const float tempK = 510.0f * (static_cast<float>(raw_uv) / 65535.0f);
    return tempK - 273.15f;
}

float BatteryStatusLTC2944::convertAcrTo_mAh(uint16_t acr_counts, uint16_t M_value) const
{
    // qLSB = 0.340mAh * (50mΩ/RSENSE) * (M/prescalar)
    if (m_rsense_ohms <= 0.0f) return 0.0f;

    const float rsense_mOhm = m_rsense_ohms * 1000.0f; // 0.001Ω -> 1mΩ
    const float qlsb_mAh = 0.340f * (50.0f / rsense_mOhm) * (static_cast<float>(M_value) / 4096.0f);

    return static_cast<float>(acr_counts) * qlsb_mAh;
}

// ----------------- I2C primitives -----------------

bool BatteryStatusLTC2944::read8(uint8_t reg, uint8_t& val)
{
    const uint16_t dev = static_cast<uint16_t>(I2C_ADDR_7BIT << 1);
    return (HAL_I2C_Mem_Read(m_hi2c1, dev, reg, I2C_MEMADD_SIZE_8BIT, &val, 1, 100) == HAL_OK);
}

bool BatteryStatusLTC2944::read16(uint8_t reg_msb, uint16_t& val)
{
    uint8_t buf[2] = {0, 0};
    const uint16_t dev = static_cast<uint16_t>(I2C_ADDR_7BIT << 1);

    if (HAL_I2C_Mem_Read(m_hi2c1, dev, reg_msb, I2C_MEMADD_SIZE_8BIT, buf, 2, 100) != HAL_OK)
        return false;

    val = (static_cast<uint16_t>(buf[0]) << 8) | static_cast<uint16_t>(buf[1]);
    return true;
}

bool BatteryStatusLTC2944::write8(uint8_t reg, uint8_t val)
{
    const uint16_t dev = static_cast<uint16_t>(I2C_ADDR_7BIT << 1);
    return (HAL_I2C_Mem_Write(m_hi2c1, dev, reg, I2C_MEMADD_SIZE_8BIT, &val, 1, 100) == HAL_OK);
}

// ----------------- Helpers -----------------

BatteryStatusLTC2944::StatusBits BatteryStatusLTC2944::decodeStatus(uint8_t a)
{
    StatusBits s{};
    s.uvlo_alert    = (a & (1u << 0)) != 0;
    s.voltage_alert = (a & (1u << 1)) != 0;
    s.charge_low    = (a & (1u << 2)) != 0;
    s.charge_high   = (a & (1u << 3)) != 0;
    s.temp_alert    = (a & (1u << 4)) != 0;
    s.acr_overflow  = (a & (1u << 5)) != 0;
    s.current_alert = (a & (1u << 6)) != 0;
    return s;
}

bool battery::BatteryStatusLTC2944::setVoltageThresholds(float v_low, float v_high)
{
    uint16_t raw_low  = static_cast<uint16_t>((v_low  / 70.8f) * 65535.0f);
    uint16_t raw_high = static_cast<uint16_t>((v_high / 70.8f) * 65535.0f);

    // Write HIGH threshold
    if (!write8(REG_VOLT_THRH_MSB_K, (raw_high >> 8) & 0xFF)) return false;
    if (!write8(REG_VOLT_THRH_LSB_L,  raw_high       & 0xFF)) return false;

    // Write LOW threshold
    if (!write8(REG_VOLT_THRL_MSB_M, (raw_low >> 8) & 0xFF)) return false;
    if (!write8(REG_VOLT_THRL_LSB_N,  raw_low       & 0xFF)) return false;

    return true;
}

bool battery::BatteryStatusLTC2944::setCurrentThresholds(float i_low, float i_high)
{
    float norm_low  = (i_low  * m_rsense_ohms) / 0.064f;
    float norm_high = (i_high * m_rsense_ohms) / 0.064f;

    uint16_t raw_low  = static_cast<uint16_t>((norm_low  * 32767.0f) + 32767.0f);
    uint16_t raw_high = static_cast<uint16_t>((norm_high * 32767.0f) + 32767.0f);

    if (!write8(REG_CURR_THRH_MSB_Q, (raw_high >> 8) & 0xFF)) return false;
    if (!write8(REG_CURR_THRH_LSB_R,  raw_high       & 0xFF)) return false;

    if (!write8(REG_CURR_THRL_MSB_S, (raw_low >> 8) & 0xFF)) return false;
    if (!write8(REG_CURR_THRL_LSB_T,  raw_low       & 0xFF)) return false;

    return true;
}

} // namespace battery

