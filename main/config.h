#ifndef CONFIG_H
#define CONFIG_H

#include "driver/gpio.h"
#include "driver/i2c.h"
#include "soc/gpio_num.h"

// I2C конфигурация
#define I2C_MASTER_SCL_IO GPIO_NUM_22      /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO GPIO_NUM_23      /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM I2C_NUM_0  /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ 100000 /*!< I2C master clock frequency */

// GPIO пины
#define HUMIDER_PIN GPIO_NUM_5
#define LIGHT_PIN GPIO_NUM_18
#define HEAT_PIN GPIO_NUM_19
#define FOUNTAIN_PIN GPIO_NUM_21

// Состояния GPIO
#define ON 1
#define OFF 0

// Температурные пороги
#define TEMP_DAY_MIN 26.0f
#define TEMP_DAY_MAX 28.0f
#define TEMP_NIGHT_MIN 24.0f
#define TEMP_NIGHT_MAX 26.0f

// Пороги влажности
#define HUMIDITY_MIN 40.0f
#define HUMIDITY_MAX 50.0f

// Временные интервалы
#define DAY_START_HOUR 7
#define DAY_END_HOUR 21
#define RESTART_HOUR 12
#define RESTART_MIN 0
#define RESTART_SEC_MIN 0
#define RESTART_SEC_MAX 30

#endif // CONFIG_H

