/*
 * Battery_status.h
 *
 *  Created on: Feb 12, 2026
 *      Author: otienom
 *
 */
/*      QBat = 30Ah
 *      Rsense: 1mohm ;
        Prescalar = 256; 111 from calculation but has to be multiple of 4 and > 111 so we chose
        QLSB = 0.0010625 Ah

 */
//
#pragma once

#include <cstdint>
#include "main.h"

extern float totalCharge;
extern float totalPower;
namespace battery {

class BatteryStatusLTC2944 {

public:
	//define the device I2C address
  static constexpr uint8_t I2C_ADDR_7BIT = 0x64; // 1100100b

  //the complete list of registers
  enum Reg : uint8_t {
    REG_STATUS_A        = 0x00,
    REG_CONTROL_B       = 0x01,

    REG_ACR_MSB_C       = 0x02,
    REG_ACR_LSB_D       = 0x03,

	REG_CHRG_THRH_MSB_E  = 0x04,
    REG_CHRG_THRH_LSB_F  = 0x05,

	REG_CHRG_THRL_MSB_G  = 0x06,
	REG_CHRG_THRL_LSB_H  = 0x07,

    REG_VOLT_MSB_I      = 0x08,
    REG_VOLT_LSB_J      = 0x09,

    REG_VOLT_THRH_MSB_K  = 0x0A,
    REG_VOLT_THRH_LSB_L  = 0x0B,

    REG_VOLT_THRL_MSB_M  = 0x0C,
	REG_VOLT_THRL_LSB_N  = 0x0D,

    REG_CURR_O          = 0x0E,
    REG_CURR_P          = 0x0F,

	REG_CURR_THRH_MSB_Q  = 0x10,
	REG_CURR_THRH_LSB_R  = 0x11,

	REG_CURR_THRL_MSB_S  = 0x12,
	REG_CURR_THRL_LSB_T  = 0x13,

    REG_TEMP_U          = 0x14,
    REG_TEMP_V          = 0x15,

	REG_TEMP_THR_MSB_W  = 0x16,
	REG_TEMP_THR_LSB_X  = 0x17,

  };

// set the control mode. set control register bits[7:6]
  enum class AdcMode : uint8_t {
    Sleep     = 0b00,
    Manual1x  = 0b01,
    Scan10s   = 0b10,
    Auto      = 0b11
  };

//Prescalar
// The prescalar is programmed by setting the control register bits B[5:3]
  enum class Prescaler : uint8_t {
    M1    = 0b000,
    M4    = 0b001,
    M16   = 0b010,
    M64   = 0b011,
    M256  = 0b100,//Our chosesn prescalar
    M1024 = 0b101,
    M4096 = 0b110,
    M4096_ALT = 0b111
  };

  enum class AlccMode : uint8_t {
    Disabled       = 0b00,
    ChargeComplete = 0b01,
    Alert          = 0b10
  };

//allows for reads from status register B
  struct StatusBits {
    bool uvlo_alert;
    bool voltage_alert;
    bool charge_low;
    bool charge_high;
    bool temp_alert;
    bool acr_overflow;
    bool current_alert;
  };
//16 bit voltage, current and voltage measurements
  typedef struct Measurements {
    uint16_t raw_voltage_ij;
    uint16_t raw_current_op;
    uint16_t raw_temp_uv;
    uint16_t raw_acr_cd;

    float v_batt_V;
    float i_batt_A;
    float temp_C;

    float acr_mAh;
    float capacity_percent;
    float power;
  } Measurements;

  // RSENSE = 1mΩ => pass 0.001f
  explicit BatteryStatusLTC2944(I2C_HandleTypeDef* hi2c1, float rsense_ohms);

  bool init(AdcMode adc = AdcMode::Scan10s,
		  Prescaler presc = Prescaler::M256,
		  AlccMode alcc = AlccMode::Alert,
		  bool shutdown = false);

  bool readStatus(uint8_t& raw_status_a, StatusBits& decoded);
  bool readAll(Measurements& out);
  bool setAccumulatedCharge_mAh(float charge_mAh);
  bool readControl(uint8_t& ctrl);
  bool writeControl(uint8_t ctrl);
  bool configure(AdcMode adc,
		  Prescaler presc,
		  AlccMode alcc,
		  bool shutdown);

  // Threshold programming (writes LTC2944 threshold registers)
  bool setVoltageThresholds(float v_low, float v_high);
  bool setCurrentThresholds(float i_low, float i_high);
  float convertVoltage_V(uint16_t raw_ij) const;
  float convertCurrent_A(uint16_t raw_op) const;
  float convertTemp_C(uint16_t raw_uv) const;
  uint16_t prescalerToM(Prescaler p);

  float convertAcrTo_mAh(uint16_t acr_counts, uint16_t M_value) const;
  static void batteryInit();
  static bool tick_read(battery::BatteryStatusLTC2944::Measurements& battery_measurements);
private:
  static constexpr float v_high = 25.0f;
  static constexpr float v_low  = 23.0f;
  static constexpr float i_high = 30.0f;// 30 Amps
  static constexpr float i_low  = 1.0f;
  I2C_HandleTypeDef* m_hi2c1 ;
  float m_rsense_ohms ;
  //define read and write routines
  bool read8(uint8_t reg, uint8_t& val);
  bool read16(uint8_t reg_msb, uint16_t& val);
  bool write8(uint8_t reg, uint8_t val);
  bool write16(uint8_t reg_msb, uint16_t val);
  static StatusBits decodeStatus(uint8_t a);
};

} // namespace battery
