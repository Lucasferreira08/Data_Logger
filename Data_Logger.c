/**
 * @file Data_Logger.c
 * @author Seu Nome
 * @date 25 de Julho de 2025
 * @brief Datalogger de movimento com IMU MPU6050, SD Card e feedback ao usuário.
 * * Este projeto implementa um datalogger que captura dados de aceleração e giroscópio
 * de um sensor MPU6050, armazena em um cartão MicroSD em formato .csv e utiliza
 * os periféricos da placa BitDogLab (Display OLED, LED RGB, Buzzer, Botões) para
 * fornecer uma interface de usuário clara e interativa.
 */

#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/i2c.h"

// Bibliotecas do projeto
#include "bitdoglab_pico.h"
#include "mpu6050.h"
#include "ssd1306.h"
#include "f_util.h"
#include "ff.h"
#include "hw_config.h"

// --- Definições de Hardware ---
#define I2C_PORT_MPU    i2c0
#define I2C_SDA_MPU     0
#define I2C_SCL_MPU     1

#define I2C_PORT_DISP   i2c1
#define I2C_SDA_DISP    14
#define I2C_SCL_DISP    15
#define DISP_ADDR       0x3C

/**
 * @brief Enumeração dos estados do sistema.
 * * Define todos os possíveis estados operacionais do datalogger,
 * controlando o fluxo do programa através de uma máquina de estados.
 */
typedef enum {
    STATE_INITIALIZING, // Estado inicial, configuração de periféricos
    STATE_IDLE,         // Sistema pronto, aguardando comando
    STATE_RECORDING,    // Gravando dados do sensor para o SD
    STATE_SAVING,       // Salvando e fechando o arquivo no SD
    STATE_NO_SD,        // Erro: Cartão SD não detectado ou falha ao montar
    STATE_UNMOUNTED,    // Cartão SD desmontado com segurança pelo usuário
    STATE_UNMOUNTING,   // Estado de transição para desmontar o SD
    STATE_MOUNTING,     // Estado de transição para montar o SD
    STATE_ERROR         // Estado de erro genérico e fatal
} system_state_t;

// --- Variáveis Globais ---
// Flags voláteis para comunicação segura entre a ISR e o loop principal
volatile bool button1_pressed = false;
volatile bool button2_pressed = false;

// --- Protótipos de Funções ---
void init_peripherals(ssd1306_t *disp);
void gpio_callback(uint gpio, uint32_t events);
void update_display(ssd1306_t *p, system_state_t state, uint32_t sample_count);
bool mount_sd_card();
void unmount_sd_card();
void handle_led_feedback(system_state_t state);

/**
 * @brief Função principal (main)
 * * Ponto de entrada do programa. Contém a máquina de estados principal
 * que gerencia o comportamento do datalogger.
 */
int main() {
    // Inicializa a comunicação serial para depuração
    stdio_init_all();
    sleep_ms(2000); // Aguarda a serial estabilizar

    // Declaração das variáveis principais
    system_state_t current_state = STATE_INITIALIZING;
    ssd1306_t disp;
    FIL fil;
    uint32_t sample_counter = 0;
    char filename[20];

    // Variáveis para dados brutos e processados
    int16_t raw_accel[3], raw_gyro[3], raw_temp;
    float proc_accel[3], proc_gyro[3];

    // Configuração inicial dos periféricos
    init_peripherals(&disp);

    while (1) {
        // Atualiza o display e o LED RGB com base no estado atual
        update_display(&disp, current_state, sample_counter);
        handle_led_feedback(current_state);

        // Máquina de estados principal
        switch (current_state) {
            
            case STATE_INITIALIZING:
                if (mount_sd_card()) {
                    current_state = STATE_IDLE;
                } else {
                    current_state = STATE_NO_SD;
                }
                break;

            case STATE_IDLE:
                // Se o botão 1 for pressionado, inicia a gravação
                if (button1_pressed) {
                    button1_pressed = false;
                    
                    int file_num = 0;
                    FRESULT fr;
                    do {
                        sprintf(filename, "datalog_%02d.csv", file_num++);
                        fr = f_stat(filename, NULL);
                    } while (fr == FR_OK && file_num < 100);

                    if (f_open(&fil, filename, FA_WRITE | FA_CREATE_ALWAYS) == FR_OK) {
                        // --- ALTERADO: Mantido o cabeçalho original do CSV ---
                        f_printf(&fil, "numero_amostra,accel_x,accel_y,accel_z,giro_x,giro_y,giro_z\n");
                        sample_counter = 0;
                        bitdog_buzzer_beep(1, 100);
                        current_state = STATE_RECORDING;
                    } else {
                        current_state = STATE_ERROR;
                    }
                }
                if (button2_pressed) {
                    button2_pressed = false;
                    current_state = STATE_UNMOUNTING;
                }
                break;

            case STATE_RECORDING:
                // --- ALTERADO: Removida a linha que definia o LED como azul. ---
                // Agora, a cor do LED é controlada apenas pela função handle_led_feedback.
                
                // Lê os dados brutos do MPU6050
                mpu6050_read_raw(raw_accel, raw_gyro, &raw_temp);

                // Converte os dados brutos para unidades físicas
                for (int i = 0; i < 3; i++) {
                    proc_accel[i] = (float)raw_accel[i] / ACCEL_SENSITIVITY;
                    proc_gyro[i]  = (float)raw_gyro[i]  / GYRO_SENSITIVITY;
                }
                
                // Escreve a linha de dados processados no arquivo CSV
                f_printf(&fil, "%lu,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f\n", 
                         sample_counter, 
                         proc_accel[0], proc_accel[1], proc_accel[2], 
                         proc_gyro[0], proc_gyro[1], proc_gyro[2]);
                sample_counter++;
                
                sleep_ms(10);
                
                if (button1_pressed) {
                    button1_pressed = false;
                    current_state = STATE_SAVING;
                }
                break;

            case STATE_SAVING:
                f_close(&fil);
                bitdog_buzzer_beep(2, 100);
                current_state = STATE_IDLE;
                break;

            case STATE_UNMOUNTING:
                unmount_sd_card();
                current_state = STATE_UNMOUNTED;
                break;

            case STATE_UNMOUNTED:
                if (button2_pressed) {
                    button2_pressed = false;
                    current_state = STATE_MOUNTING;
                }
                break;

            case STATE_MOUNTING:
                if (mount_sd_card()) {
                    current_state = STATE_IDLE;
                } else {
                    current_state = STATE_NO_SD;
                }
                break;

            case STATE_NO_SD:
                if (button2_pressed) {
                    button2_pressed = false;
                    current_state = STATE_MOUNTING;
                }
                break;
            
            case STATE_ERROR:
                sleep_ms(1000);
                break;
        }
    }
    return 0;
}

// ... (init_peripherals, gpio_callback, update_display permanecem iguais) ...

void init_peripherals(ssd1306_t *disp) {
    bitdog_init(gpio_callback);
    i2c_init(I2C_PORT_DISP, 100 * 1000);
    gpio_set_function(I2C_SDA_DISP, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_DISP, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_DISP);
    gpio_pull_up(I2C_SCL_DISP);
    ssd1306_init(disp, 128, 64, false, DISP_ADDR, I2C_PORT_DISP);
    ssd1306_config(disp);
    mpu6050_init(I2C_PORT_MPU, I2C_SDA_MPU, I2C_SCL_MPU);
    mpu6050_reset();
}

void gpio_callback(uint gpio, uint32_t events) {
    static uint64_t last_press_time = 0;
    uint64_t now = time_us_64();
    if (now - last_press_time > 200 * 1000) {
        last_press_time = now;
        if (gpio == BUTTON_1_PIN) {
            button1_pressed = true;
        } else if (gpio == BUTTON_2_PIN) {
            button2_pressed = true;
        }
    }
}

void update_display(ssd1306_t *p, system_state_t state, uint32_t sample_count) {
    char line1[20], line2[20];
    ssd1306_fill(p, false);
    switch (state) {
        case STATE_INITIALIZING:
        case STATE_MOUNTING:
            strcpy(line1, "Datalogger IMU");
            strcpy(line2, "Inicializando...");
            break;
        case STATE_IDLE:
            strcpy(line1, "Pronto!");
            strcpy(line2, "A=Grava B=M/D");
            break;
        case STATE_RECORDING:
            strcpy(line1, "Gravando...");
            sprintf(line2, "Amostras: %lu", sample_count);
            break;
        case STATE_SAVING:
            strcpy(line1, "Salvando dados");
            sprintf(line2, "%lu amostras", sample_count);
            break;
        case STATE_NO_SD:
            strcpy(line1, "SD Nao Detectado");
            strcpy(line2, "Pressione B");
            break;
        case STATE_UNMOUNTED:
            strcpy(line1, "SD Desmontado");
            strcpy(line2, "Pressione B");
            break;
        case STATE_UNMOUNTING:
            strcpy(line1, "Aguarde...");
            strcpy(line2, "Desmontando SD");
            break;
        case STATE_ERROR:
            strcpy(line1, "ERRO FATAL");
            strcpy(line2, "Reinicie o Pico");
            break;
    }
    ssd1306_draw_string(p, line1, 0, 10);
    ssd1306_draw_string(p, line2, 0, 30);
    ssd1306_send_data(p);
}


/**
 * @brief Controla o LED RGB com base no estado do sistema.
 * @param state O estado atual do sistema.
 */
void handle_led_feedback(system_state_t state) {
    static bool purple_led_on = false;
    static uint64_t last_blink_time = 0;
    uint64_t now = time_us_64();

    switch (state) {
        case STATE_INITIALIZING:
        case STATE_MOUNTING:
            bitdog_led_set(LED_COLOR_YELLOW);
            break;
        case STATE_IDLE:
        case STATE_SAVING: 
            bitdog_led_set(LED_COLOR_GREEN);
            break;
        case STATE_RECORDING:
            // --- ALTERADO: Cor do LED definida como vermelho durante a gravação. ---
            bitdog_led_set(LED_COLOR_RED);
            break;
        case STATE_NO_SD:
        case STATE_UNMOUNTED:
            if (now - last_blink_time > 500 * 1000) {
                last_blink_time = now;
                purple_led_on = !purple_led_on;
                bitdog_led_set(purple_led_on ? LED_COLOR_PURPLE : LED_COLOR_OFF);
            }
            break;
        case STATE_ERROR:
            bitdog_led_set(LED_COLOR_PURPLE);
            break;
        default:
            bitdog_led_set(LED_COLOR_OFF);
            break;
    }
}

/**
 * @brief Monta o sistema de arquivos do cartão SD.
 * @return true se a montagem for bem-sucedida, false caso contrário.
 */
bool mount_sd_card() {
    sd_card_t *pSD = sd_get_by_num(0);
    FRESULT fr = f_mount(&pSD->fatfs, pSD->pcName, 1);
    return fr == FR_OK;
}

/**
 * @brief Desmonta o sistema de arquivos do cartão SD.
 */
void unmount_sd_card() {
    sd_card_t *pSD = sd_get_by_num(0);
    f_unmount(pSD->pcName);
}