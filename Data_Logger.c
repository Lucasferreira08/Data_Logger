// #include <stdio.h>
// #include <string.h>

// #include "pico/stdlib.h"
// #include "hardware/i2c.h"

// // Bibliotecas do projeto
// #include "bitdoglab_pico.h"
// #include "mpu6050.h"
// #include "ssd1306.h"
// #include "f_util.h"
// #include "ff.h"
// #include "diskio.h"
// #include "hw_config.h"
// #include "my_debug.h"
// #include "rtc.h"
// #include "sd_card.h"

// // --- Definições de Hardware ---
// #define I2C_PORT_MPU    i2c0
// #define I2C_SDA_MPU     0
// #define I2C_SCL_MPU     1

// #define I2C_PORT_DISP   i2c1
// #define I2C_SDA_DISP    14
// #define I2C_SCL_DISP    15
// #define DISP_ADDR       0x3C

// // --- Estados da Máquina de Estados ---
// typedef enum {
//     STATE_INITIALIZING,
//     STATE_NO_SD,
//     STATE_IDLE,
//     STATE_RECORDING,
//     STATE_SAVING,
//     STATE_ERROR
// } system_state_t;

// // Variáveis globais voláteis para comunicação com a ISR
// volatile bool button1_pressed = false;
// volatile bool button2_pressed = false;

// // --- Protótipos de Funções ---
// void update_display(ssd1306_t *p, system_state_t state, uint32_t sample_count);
// bool mount_sd_card();
// void unmount_sd_card();

// // --- Lógica de Debounce na Interrupção ---
// void gpio_callback(uint gpio, uint32_t events) {
//     static uint64_t last_press_time = 0;
//     uint64_t now = time_us_64();
//     if (now - last_press_time > 200 * 1000) { // Debounce de 200ms
//         last_press_time = now;
//         if (gpio == BUTTON_1_PIN) {
//             button1_pressed = true;
//         } else if (gpio == BUTTON_2_PIN) {
//             button2_pressed = true;
//         }
//     }
// }

// int main() {
//     stdio_init_all();
//     sleep_ms(2000); // Aguarda a serial

//     system_state_t current_state = STATE_INITIALIZING;
    
//     // Inicializa periféricos da BitDogLab
//     bitdog_init(gpio_callback);
//     bitdog_led_set(LED_COLOR_YELLOW);

//     // Inicializa Display
//     ssd1306_t disp;
//     // ssd1306_init(&disp, 128, 64, false, DISP_ADDR, I2C_PORT_DISP);
//     // gpio_set_function(I2C_SDA_DISP, GPIO_FUNC_I2C);
//     // gpio_set_function(I2C_SCL_DISP, GPIO_FUNC_I2C);
//     // gpio_pull_up(I2C_SDA_DISP);
//     // gpio_pull_up(I2C_SCL_DISP);
//     // ssd1306_config(&disp);
//     // update_display(&disp, current_state, 0);
//     display_init(&disp);
//     ssd1306_draw_string(&disp, "Iniciando", 0, 0);
//     ssd1306_send_data(&disp);

//     sleep_ms(5000);

//     // Inicializa MPU6050
//     mpu6050_init(I2C_PORT_MPU, I2C_SDA_MPU, I2C_SCL_MPU);
//     mpu6050_reset();

//     // Tenta montar o cartão SD
//     if (!mount_sd_card()) {
//         current_state = STATE_NO_SD;
//     } else {
//         current_state = STATE_IDLE;
//     }

//     FIL fil;
//     uint32_t sample_counter = 0;
//     int16_t accel[3], gyro[3], temp;
//     char filename[20];

//     while (1) {
//         update_display(&disp, current_state, sample_counter);

//         switch (current_state) {
//             case STATE_IDLE:
//                 bitdog_led_set(LED_COLOR_GREEN);
//                 if (button1_pressed) {
//                     button1_pressed = false;
                    
//                     // Encontra um nome de arquivo não utilizado
//                     int file_num = 0;
//                     FRESULT fr;
//                     do {
//                         sprintf(filename, "datalog_%02d.csv", file_num++);
//                         fr = f_stat(filename, NULL);
//                     } while (fr != FR_NO_FILE && file_num < 100);

//                     if (f_open(&fil, filename, FA_WRITE | FA_CREATE_ALWAYS) == FR_OK) {
//                         current_state = STATE_RECORDING;
//                         sample_counter = 0;
//                         f_printf(&fil, "numero_amostra,accel_x,accel_y,accel_z,giro_x,giro_y,giro_z\n");
//                         bitdog_buzzer_beep(1, 100);
//                     } else {
//                         current_state = STATE_ERROR;
//                     }
//                 }
//                 if (button2_pressed) {
//                     button2_pressed = false;
//                     unmount_sd_card();
//                     current_state = STATE_NO_SD;
//                 }
//                 break;

//             case STATE_RECORDING:
//                 bitdog_led_set(LED_COLOR_RED);
                
//                 // Pisca LED azul para indicar acesso ao SD
//                 bitdog_led_set(LED_COLOR_BLUE);
//                 mpu6050_read_raw(accel, gyro, &temp);
//                 f_printf(&fil, "%lu,%d,%d,%d,%d,%d,%d\n", sample_counter, accel[0], accel[1], accel[2], gyro[0], gyro[1], gyro[2]);
//                 sample_counter++;
//                 bitdog_led_set(LED_COLOR_RED);

//                 if (button1_pressed) {
//                     button1_pressed = false;
//                     current_state = STATE_SAVING;
//                 }
//                 sleep_ms(10); // Controla a taxa de amostragem (~100Hz)
//                 break;

//             case STATE_SAVING:
//                 bitdog_led_set(LED_COLOR_BLUE); // Azul piscando (simulado)
//                 update_display(&disp, current_state, sample_counter);
//                 f_close(&fil); // Ação mais importante: salvar os dados!
//                 bitdog_buzzer_beep(2, 100);
//                 current_state = STATE_IDLE;
//                 break;

//             case STATE_NO_SD:
//                 bitdog_led_set(LED_COLOR_PURPLE);
//                 sleep_ms(250);
//                 bitdog_led_set(LED_COLOR_OFF);
//                 sleep_ms(250);
//                 if (button2_pressed) {
//                     button2_pressed = false;
//                     if (mount_sd_card()) {
//                         current_state = STATE_IDLE;
//                     }
//                 }
//                 break;
            
//             case STATE_ERROR: // Estado de erro genérico
//                 bitdog_led_set(LED_COLOR_PURPLE); // Roxo sólido
//                 // Fica aqui até um reset
//                 break;

//             case STATE_INITIALIZING:
//                 // Apenas um estado de passagem
//                 break;
//         }
//     }
//     return 0;
// }

// void update_display(ssd1306_t *p, system_state_t state, uint32_t sample_count) {
//     char line1[20], line2[20];
//     ssd1306_fill(p, false);

//     switch (state) {
//         case STATE_INITIALIZING: strcpy(line1, "Datalogger IMU"); strcpy(line2, "Inicializando..."); break;
//         case STATE_NO_SD: strcpy(line1, "Erro no Cartao"); strcpy(line2, "Pressione B2"); break;
//         case STATE_IDLE: strcpy(line1, "Pronto!"); sprintf(line2, "B1 p/ Gravar"); break;
//         case STATE_RECORDING: strcpy(line1, "Gravando..."); sprintf(line2, "Amostras: %lu", sample_count); break;
//         case STATE_SAVING: strcpy(line1, "Salvando dados"); sprintf(line2, "%lu amostras", sample_count); break;
//         case STATE_ERROR: strcpy(line1, "ERRO FATAL"); strcpy(line2, "Reinicie o Pico"); break;
//     }
//     ssd1306_draw_string(p, line1, 0, 10);
//     ssd1306_draw_string(p, line2, 0, 30);
//     ssd1306_send_data(p);
// }

// bool mount_sd_card() {
//     sd_card_t *pSD = sd_get_by_num(0);
//     FRESULT fr = f_mount(&pSD->fatfs, pSD->pcName, 1);
//     return fr == FR_OK;
// }

// void unmount_sd_card() {
//     sd_card_t *pSD = sd_get_by_num(0);
//     f_unmount(pSD->pcName);
// }

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
#include "diskio.h"
#include "hw_config.h"
#include "my_debug.h"
#include "rtc.h"
#include "sd_card.h"

// --- Definições de Hardware ---
#define I2C_PORT_MPU    i2c0
#define I2C_SDA_MPU     0
#define I2C_SCL_MPU     1

#define I2C_PORT_DISP   i2c1
#define I2C_SDA_DISP    14
#define I2C_SCL_DISP    15
#define DISP_ADDR       0x3C

// --- Estados da Máquina de Estados ---
typedef enum {
    STATE_INITIALIZING,
    STATE_NO_SD,
    STATE_IDLE,
    STATE_RECORDING,
    STATE_SAVING,
    STATE_ERROR
} system_state_t;

// Variáveis globais voláteis para comunicação com a ISR
volatile bool button1_pressed = false;
volatile bool button2_pressed = false;

// --- Protótipos de Funções ---
void update_display(ssd1306_t *p, system_state_t state, uint32_t sample_count);
bool mount_sd_card();
void unmount_sd_card();
// Declaração da função display_init se não estiver em um cabeçalho
void display_init(ssd1306_t *p); 

// --- Lógica de Debounce na Interrupção ---
void gpio_callback(uint gpio, uint32_t events) {
    static uint64_t last_press_time = 0;
    uint64_t now = time_us_64();
    printf("[DEBUG_ISR] Interrupt on GPIO %d\n", gpio);
    if (now - last_press_time > 200 * 1000) { // Debounce de 200ms
        last_press_time = now;
        if (gpio == BUTTON_1_PIN) {
            printf("[DEBUG_ISR] Button 1 press accepted.\n");
            button1_pressed = true;
        } else if (gpio == BUTTON_2_PIN) {
            printf("[DEBUG_ISR] Button 2 press accepted.\n");
            button2_pressed = true;
        }
    } else {
        printf("[DEBUG_ISR] Debounce: press ignored.\n");
    }
}

int main() {
    stdio_init_all();
    sleep_ms(2000); // Aguarda a serial
    printf("\n\n[DEBUG] --- PROGRAMA INICIADO ---\n");

    system_state_t current_state = STATE_INITIALIZING;
    system_state_t last_state = -1; // Para logar a mudança de estado apenas uma vez
    
    printf("[DEBUG] main: Entrando no estado: STATE_INITIALIZING\n");
    
    // Inicializa periféricos da BitDogLab
    printf("[DEBUG] main: Inicializando periféricos BitDogLab...\n");
    bitdog_init(gpio_callback);
    bitdog_led_set(LED_COLOR_YELLOW);

    // Inicializa Display
    ssd1306_t disp;
    printf("[DEBUG] main: Inicializando Display (usando display_init)...\n");
    // ssd1306_init(&disp, 128, 64, false, DISP_ADDR, I2C_PORT_DISP);
    // gpio_set_function(I2C_SDA_DISP, GPIO_FUNC_I2C);
    // gpio_set_function(I2C_SCL_DISP, GPIO_FUNC_I2C);
    // gpio_pull_up(I2C_SDA_DISP);
    // gpio_pull_up(I2C_SCL_DISP);
    // ssd1306_config(&disp);
    // update_display(&disp, current_state, 0);
    display_init(&disp);
    // ssd1306_draw_string(&disp, "Iniciando", 0, 0);
    // ssd1306_send_data(&disp);
    printf("[DEBUG] main: Display inicializado.\n");

    printf("[DEBUG] main: Pausa de 5 segundos...\n");
    sleep_ms(5000);

    // Inicializa MPU6050
    printf("[DEBUG] main: Inicializando MPU6050...\n");
    mpu6050_init(I2C_PORT_MPU, I2C_SDA_MPU, I2C_SCL_MPU);
    // Declara os pinos como I2C na Binary Info para depuração
    // bi_decl(bi_2pins_with_func(I2C_SDA_MPU, I2C_SCL_MPU, GPIO_FUNC_I2C));
    mpu6050_reset();
    printf("[DEBUG] main: MPU6050 inicializado.\n");

    // Tenta montar o cartão SD
    printf("[DEBUG] main: Tentando montar o cartão SD...\n");
    if (!mount_sd_card()) {
        current_state = STATE_NO_SD;
    } else {
        current_state = STATE_IDLE;
    }

    FIL fil;
    uint32_t sample_counter = 0;
    int16_t accel[3], gyro[3], temp;
    char filename[20];

    printf("[DEBUG] main: Entrando no loop principal (while(1)).\n");
    while (1) {
        if (current_state != last_state) {
            printf("[DEBUG_STATE] Transicao de estado: %d -> %d\n", last_state, current_state);
            last_state = current_state;
        }

        update_display(&disp, current_state, sample_counter);

        switch (current_state) {
            case STATE_IDLE:
                bitdog_led_set(LED_COLOR_GREEN);
                if (button1_pressed) {
                    button1_pressed = false;
                    printf("[DEBUG_STATE_IDLE] Botao 1 pressionado. Iniciando processo de gravacao.\n");
                    
                    // Encontra um nome de arquivo não utilizado
                    printf("[DEBUG_STATE_IDLE] Procurando por nome de arquivo disponivel...\n");
                    int file_num = 0;
                    FRESULT fr;
                    do {
                        sprintf(filename, "datalog_%02d.csv", file_num++);
                        fr = f_stat(filename, NULL);
                    } while (fr != FR_NO_FILE && file_num < 100);
                    printf("[DEBUG_STATE_IDLE] Usando nome de arquivo: '%s'\n", filename);

                    FRESULT open_res = f_open(&fil, filename, FA_WRITE | FA_CREATE_ALWAYS);
                    if (open_res == FR_OK) {
                        printf("[DEBUG_STATE_IDLE] Arquivo aberto com sucesso. Transicao para STATE_RECORDING.\n");
                        current_state = STATE_RECORDING;
                        sample_counter = 0;
                        f_printf(&fil, "numero_amostra,accel_x,accel_y,accel_z,giro_x,giro_y,giro_z\n");
                        bitdog_buzzer_beep(1, 100);
                    } else {
                        printf("[ERROR] STATE_IDLE: Falha ao abrir o arquivo. Erro: %s\n", FRESULT_str(open_res));
                        current_state = STATE_ERROR;
                    }
                }
                if (button2_pressed) {
                    button2_pressed = false;
                    printf("[DEBUG_STATE_IDLE] Botao 2 pressionado. Desmontando o cartao SD.\n");
                    unmount_sd_card();
                    current_state = STATE_NO_SD;
                }
                break;

            case STATE_RECORDING:
                bitdog_led_set(LED_COLOR_RED);
                
                // Pisca LED azul para indicar acesso ao SD
                bitdog_led_set(LED_COLOR_BLUE);
                mpu6050_read_raw(accel, gyro, &temp);
                if (f_printf(&fil, "%lu,%d,%d,%d,%d,%d,%d\n", sample_counter, accel[0], accel[1], accel[2], gyro[0], gyro[1], gyro[2]) < 0) {
                     printf("[ERROR] STATE_RECORDING: falha no f_printf na amostra %lu\n", sample_counter);
                }
                sample_counter++;
                bitdog_led_set(LED_COLOR_RED);

                if (button1_pressed) {
                    button1_pressed = false;
                    printf("[DEBUG_STATE_RECORDING] Botao 1 pressionado. Transicao para STATE_SAVING.\n");
                    current_state = STATE_SAVING;
                }
                sleep_ms(10); // Controla a taxa de amostragem (~100Hz)
                break;

            case STATE_SAVING:
                bitdog_led_set(LED_COLOR_BLUE); // Azul piscando (simulado)
                update_display(&disp, current_state, sample_counter);
                printf("[DEBUG_STATE_SAVING] Fechando o arquivo para salvar os dados...\n");
                FRESULT close_res = f_close(&fil); // Ação mais importante: salvar os dados!
                if (close_res != FR_OK) {
                    printf("[ERROR] STATE_SAVING: Falha ao fechar o arquivo! Erro: %s\n", FRESULT_str(close_res));
                } else {
                    printf("[DEBUG_STATE_SAVING] Arquivo fechado com sucesso.\n");
                }
                bitdog_buzzer_beep(2, 100);
                printf("[DEBUG_STATE_SAVING] Transicao para STATE_IDLE.\n");
                current_state = STATE_IDLE;
                break;

            case STATE_NO_SD:
                bitdog_led_set(LED_COLOR_PURPLE);
                sleep_ms(250);
                bitdog_led_set(LED_COLOR_OFF);
                sleep_ms(250);
                if (button2_pressed) {
                    button2_pressed = false;
                    printf("[DEBUG_STATE_NO_SD] Botao 2 pressionado. Tentando montar o cartao novamente.\n");
                    if (mount_sd_card()) {
                        printf("[DEBUG_STATE_NO_SD] Sucesso ao montar. Transicao para STATE_IDLE.\n");
                        current_state = STATE_IDLE;
                    } else {
                        printf("[DEBUG_STATE_NO_SD] Nova falha ao montar o cartao.\n");
                    }
                }
                break;
            
            case STATE_ERROR: // Estado de erro genérico
                bitdog_led_set(LED_COLOR_PURPLE); // Roxo sólido
                // O erro já foi impresso quando ocorreu. O sistema para aqui.
                sleep_ms(1000);
                break;

            case STATE_INITIALIZING:
                // Apenas um estado de passagem
                break;
        }
    }
    return 0;
}

void update_display(ssd1306_t *p, system_state_t state, uint32_t sample_count) {
    char line1[20], line2[20];
    ssd1306_fill(p, false);

    switch (state) {
        case STATE_INITIALIZING: strcpy(line1, "Datalogger IMU"); strcpy(line2, "Inicializando..."); break;
        case STATE_NO_SD: strcpy(line1, "Erro no Cartao"); strcpy(line2, "Pressione B2"); break;
        case STATE_IDLE: strcpy(line1, "Pronto!"); sprintf(line2, "B1 p/ Gravar"); break;
        case STATE_RECORDING: strcpy(line1, "Gravando..."); sprintf(line2, "Amostras: %lu", sample_count); break;
        case STATE_SAVING: strcpy(line1, "Salvando dados"); sprintf(line2, "%lu amostras", sample_count); break;
        case STATE_ERROR: strcpy(line1, "ERRO FATAL"); strcpy(line2, "Reinicie o Pico"); break;
    }
    ssd1306_draw_string(p, line1, 0, 10);
    ssd1306_draw_string(p, line2, 0, 30);
    ssd1306_send_data(p);
}

bool mount_sd_card() {
    printf("[DEBUG_SD] Tentando obter o handle do cartao SD...\n");
    sd_card_t *pSD = sd_get_by_num(0);
    printf("[DEBUG_SD] Chamando f_mount...\n");
    FRESULT fr = f_mount(&pSD->fatfs, pSD->pcName, 1);
    if (fr != FR_OK) {
        printf("[ERROR_SD] f_mount falhou com o codigo %d: %s\n", fr, FRESULT_str(fr));
    } else {
        printf("[DEBUG_SD] f_mount executado com sucesso.\n");
    }
    return fr == FR_OK;
}

void unmount_sd_card() {
    printf("[DEBUG_SD] Chamando f_unmount...\n");
    sd_card_t *pSD = sd_get_by_num(0);
    f_unmount(pSD->pcName);
    printf("[DEBUG_SD] Cartao SD desmontado.\n");
}
