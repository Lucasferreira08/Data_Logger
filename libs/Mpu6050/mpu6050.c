#include "mpu6050.h"
#include <math.h>
#include <stdio.h>
#include "hardware/i2c.h"
#include "pico/stdlib.h"

// Variáveis estáticas (privadas) para a biblioteca
static i2c_inst_t *i2c_instance;
static uint8_t device_address = MPU6050_I2C_ADDR;

// Função de inicialização da comunicação I2C
void mpu6050_init(i2c_inst_t *i2c, uint sda_pin, uint scl_pin) {
    i2c_instance = i2c;
    
    // Inicializa o I2C com uma frequência de 400kHz
    i2c_init(i2c_instance, 400 * 1000);
    
    // Configura os pinos SDA e SCL para a função I2C
    gpio_set_function(sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(scl_pin, GPIO_FUNC_I2C);

    // Habilita os resistores de pull-up internos
    gpio_pull_up(sda_pin);
    gpio_pull_up(scl_pin);
}

// Função para resetar o sensor e acordá-lo
void mpu6050_reset() {
    // Duas escritas de bytes: registrador 0x6B (PWR_MGMT_1), valor 0x80 (reset)
    uint8_t buf[] = {0x6B, 0x80};
    i2c_write_blocking(i2c_instance, device_address, buf, 2, false);
    sleep_ms(100); // Aguarda o reset


    // Tira o dispositivo do modo de suspensão
    buf[1] = 0x00;
    i2c_write_blocking(i2c_instance, device_address, buf, 2, false);
    sleep_ms(10);
}

// Função para ler os dados brutos
void mpu6050_read_raw(int16_t accel[3], int16_t gyro[3], int16_t *temp) {
    uint8_t buffer[6];

    // Começa a leitura a partir do registrador 0x3B (ACCEL_XOUT_H)
    uint8_t val = 0x3B;
    i2c_write_blocking(i2c_instance, device_address, &val, 1, true); // true para manter o controle do barramento
    i2c_read_blocking(i2c_instance, device_address, buffer, 6, false);

    // Combina os bytes alto e baixo para cada eixo do acelerômetro
    for (int i = 0; i < 3; i++) {
        accel[i] = (buffer[i * 2] << 8) | buffer[(i * 2) + 1];
    }

    // Começa a leitura a partir do registrador 0x43 (GYRO_XOUT_H)
    val = 0x43;
    i2c_write_blocking(i2c_instance, device_address, &val, 1, true);
    i2c_read_blocking(i2c_instance, device_address, buffer, 6, false);

    // Combina os bytes alto e baixo para cada eixo do giroscópio
    for (int i = 0; i < 3; i++) {
        gyro[i] = (buffer[i * 2] << 8) | buffer[(i * 2) + 1];
    }

    // Começa a leitura a partir do registrador 0x41 (TEMP_OUT_H)
    val = 0x41;
    i2c_write_blocking(i2c_instance, device_address, &val, 1, true);
    i2c_read_blocking(i2c_instance, device_address, buffer, 2, false);
    
    // Combina os bytes da temperatura
    *temp = (buffer[0] << 8) | buffer[1];
}

// Função para calcular Roll e Pitch
void mpu6050_get_roll_pitch(float *roll, float *pitch) {
    int16_t accel[3], gyro[3], temp;
    mpu6050_read_raw(accel, gyro, &temp);

    // Converte os valores brutos do acelerômetro para 'g'
    float ax = accel[0] / ACCEL_SENSITIVITY;
    float ay = accel[1] / ACCEL_SENSITIVITY;
    float az = accel[2] / ACCEL_SENSITIVITY;

    // Calcula Roll e Pitch usando a trigonometria
    *roll  = atan2(ay, az) * 180.0f / M_PI;
    *pitch = atan2(-ax, sqrt(ay * ay + az * az)) * 180.0f / M_PI;
}
