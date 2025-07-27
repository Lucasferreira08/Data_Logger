#include "bitdoglab_pico.h"
#include "hardware/gpio.h"

void bitdog_init(gpio_irq_callback_t callback) {
    // Inicializa LED RGB
    gpio_init(LED_R_PIN); gpio_set_dir(LED_R_PIN, GPIO_OUT);
    gpio_init(LED_G_PIN); gpio_set_dir(LED_G_PIN, GPIO_OUT);
    gpio_init(LED_B_PIN); gpio_set_dir(LED_B_PIN, GPIO_OUT);
    bitdog_led_set(LED_COLOR_OFF); // Desliga o LED inicialmente

    // Inicializa Buzzer
    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);
    gpio_put(BUZZER_PIN, 0);

    // Inicializa Botões com interrupção
    gpio_init(BUTTON_1_PIN);
    gpio_set_dir(BUTTON_1_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_1_PIN);

    gpio_init(BUTTON_2_PIN);
    gpio_set_dir(BUTTON_2_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_2_PIN);
    
    // Configura a interrupção para ambos os botões na borda de descida
    gpio_set_irq_enabled_with_callback(BUTTON_1_PIN, GPIO_IRQ_EDGE_FALL, true, callback);
    gpio_set_irq_enabled(BUTTON_2_PIN, GPIO_IRQ_EDGE_FALL, true);
}

void bitdog_led_set(led_color_t color) {
    bool r = false, g = false, b = false;

    switch (color) {
        case LED_COLOR_RED:    r = true; break;
        case LED_COLOR_GREEN:  g = true; break;
        case LED_COLOR_BLUE:   b = true; break;
        case LED_COLOR_YELLOW: r = true; g = true; break;
        case LED_COLOR_PURPLE: r = true; b = true; break;
        case LED_COLOR_CYAN:   g = true; b = true; break;
        case LED_COLOR_WHITE:  r = true; g = true; b = true; break;
        case LED_COLOR_OFF:
        default:
            break;
    }
    gpio_put(LED_R_PIN, r);
    gpio_put(LED_G_PIN, g);
    gpio_put(LED_B_PIN, b);
}

void bitdog_buzzer_beep(uint8_t count, uint16_t duration_ms) {
    for (uint8_t i = 0; i < count; i++) {
        gpio_put(BUZZER_PIN, 1);
        sleep_ms(duration_ms);
        gpio_put(BUZZER_PIN, 0);
        if (i < count - 1) {
            sleep_ms(duration_ms);
        }
    }
}
