#ifndef MPU6050_H
#define MPU6050_H

#include "pico/stdlib.h"
#include "hardware/i2c.h"

// Endereço I2C padrão do MPU6050
#define MPU6050_I2C_ADDR 0x68

// Fator de conversão para o acelerômetro (datasheet)
// O sensor está configurado para +/- 2g, e o valor é de 16 bits.
// LSB/g = 16384.0
#define ACCEL_SENSITIVITY 16384.0

// Fator de conversão para o giroscópio (datasheet)
// O sensor está configurado para +/- 250 graus/s, e o valor é de 16 bits.
// LSB/(graus/s) = 131.0
#define GYRO_SENSITIVITY 131.0

/**
 * @brief Inicializa a comunicação I2C para o MPU6050.
 * * @param i2c Ponteiro para a instância I2C a ser usada (i2c0 ou i2c1).
 * @param sda_pin O pino GPIO para o SDA.
 * @param scl_pin O pino GPIO para o SCL.
 */
void mpu6050_init(i2c_inst_t *i2c, uint sda_pin, uint scl_pin);

/**
 * @brief Reseta o MPU6050 e o tira do modo de suspensão.
 * É necessário chamar esta função após a inicialização.
 */
void mpu6050_reset();

/**
 * @brief Lê os valores brutos (raw) do acelerômetro, giroscópio e temperatura.
 * * @param accel Array de 3 posições para armazenar os dados do acelerômetro (x, y, z).
 * @param gyro Array de 3 posições para armazenar os dados do giroscópio (x, y, z).
 * @param temp Ponteiro para armazenar o valor bruto da temperatura.
 */
void mpu6050_read_raw(int16_t accel[3], int16_t gyro[3], int16_t *temp);

/**
 * @brief Calcula os ângulos de Roll e Pitch com base nos dados do acelerômetro.
 * * @param roll Ponteiro para armazenar o ângulo de Roll em graus.
 * @param pitch Ponteiro para armazenar o ângulo de Pitch em graus.
 */
void mpu6050_get_roll_pitch(float *roll, float *pitch);

#endif // MPU6050_H
