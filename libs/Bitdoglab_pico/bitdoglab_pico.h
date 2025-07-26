#ifndef BITDOGLAB_PICO_H
#define BITDOGLAB_PICO_H

#include "pico/stdlib.h"

// --- Definição dos Pinos para a Placa BitDogLab ---

// LED RGB (assumindo anodo comum, lógica invertida)
#define LED_R_PIN 13
#define LED_G_PIN 11
#define LED_B_PIN 12

// Buzzer
#define BUZZER_PIN 21

// Botões
#define BUTTON_1_PIN 5
#define BUTTON_2_PIN 6

// Enum para as cores do LED
typedef enum {
    LED_COLOR_OFF,
    LED_COLOR_RED,
    LED_COLOR_GREEN,
    LED_COLOR_BLUE,
    LED_COLOR_YELLOW,
    LED_COLOR_PURPLE,
    LED_COLOR_CYAN,
    LED_COLOR_WHITE
} led_color_t;

/**
 * @brief Inicializa todos os periféricos da BitDogLab (LED, Buzzer, Botões).
 * @param callback A função de callback a ser chamada quando um botão for pressionado.
 */
void bitdog_init(gpio_irq_callback_t callback);

/**
 * @brief Define a cor do LED RGB.
 * @param color A cor desejada a partir do enum led_color_t.
 */
void bitdog_led_set(led_color_t color);

/**
 * @brief Emite um beep no buzzer.
 * @param count O número de beeps.
 * @param duration_ms A duração de cada beep em milissegundos.
 */
void bitdog_buzzer_beep(uint8_t count, uint16_t duration_ms);

#endif // BITDOGLAB_PICO_H
