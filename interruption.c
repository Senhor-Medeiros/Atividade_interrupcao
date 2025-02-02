
#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"

// --------------------------------------------------
// Implementação de ws2812_put
// --------------------------------------------------
// O arquivo ws2812.pio.h não fornece uma implementação para ws2812_put.
// Defini esta função para enviar os dados do pixel para o PIO.
void ws2812_put(PIO pio, uint sm, uint32_t color) {
    pio_sm_put_blocking(pio, sm, color << 8u);
}

// --------------------------------------------------
// Definições de Pinos e Constantes
// --------------------------------------------------
#define WS2812_PIN      7      // Pino conectado à matriz WS2812
#define NUM_LEDS        25     // Matriz 5x5
#define BUTTON_A_PIN    5      // Botão A: incrementa número
#define BUTTON_B_PIN    6      // Botão B: decrementa número

// Uso do pino 13 para o canal vermelho do LED RGB
#define LED_RED_PIN     13     // LED RGB – canal vermelho (para piscar)

#define DEBOUNCE_MS     200    // Tempo de debounce (200 ms)

// Macro para converter (r, g, b) para o formato GRB do WS2812
#define GRB(r, g, b) (((g & 0xFF) << 16) | ((r & 0xFF) << 8) | (b & 0xFF))

// --------------------------------------------------
// Variáveis Globais
// --------------------------------------------------
uint32_t led_matrix[NUM_LEDS];  // Buffer de cores para os LEDs da matriz

volatile bool button_a_flag = false;
volatile bool button_b_flag = false;

uint32_t last_time_button_a = 0;
uint32_t last_time_button_b = 0;

int current_number = 0;  // Representa o dígito atual (0 a 9)

// --------------------------------------------------
// Mapeamento físico (serpentino) da matriz 5x5
// --------------------------------------------------
const int led_index_map[5][5] = {
    {24, 23, 22, 21, 20},  // Linha 0 (topo): LEDs 25,24,23,22,21
    {15, 16, 17, 18, 19},  // Linha 1: LEDs 16,17,18,19,20
    {14, 13, 12, 11, 10},  // Linha 2: LEDs 15,14,13,12,11
    {5,   6,  7,   8,   9},  // Linha 3: LEDs 6,7,8,9,10
    {4,   3,  2,   1,   0}   // Linha 4 (base): LEDs 5,4,3,2,1
};

// --------------------------------------------------
// Padrões dos dígitos (0 a 9) em formato 5x5
// Cada padrão é uma matriz 5×5 de floats (1.0 = LED aceso e intensidade 100%; 0.0 = LED apagado)
// --------------------------------------------------
const float digit_patterns[10][5][5] = {
    // Dígito 0
    {
        {1.0, 1.0, 1.0, 1.0, 1.0},
        {1.0, 0.0, 0.0, 0.0, 1.0},
        {1.0, 0.0, 0.0, 0.0, 1.0},
        {1.0, 0.0, 0.0, 0.0, 1.0},
        {1.0, 1.0, 1.0, 1.0, 1.0}
    },
    // Dígito 1
    {
        {0.0, 0.0, 1.0, 0.0, 0.0},
        {0.0, 1.0, 1.0, 0.0, 0.0},
        {0.0, 0.0, 1.0, 0.0, 0.0},
        {0.0, 0.0, 1.0, 0.0, 0.0},
        {0.0, 1.0, 1.0, 1.0, 0.0}
    },
    // Dígito 2
    {
        {1.0, 1.0, 1.0, 1.0, 1.0},
        {0.0, 0.0, 0.0, 0.0, 1.0},
        {1.0, 1.0, 1.0, 1.0, 1.0},
        {1.0, 0.0, 0.0, 0.0, 0.0},
        {1.0, 1.0, 1.0, 1.0, 1.0}
    },
    // Dígito 3
    {
        {1.0, 1.0, 1.0, 1.0, 1.0},
        {0.0, 0.0, 0.0, 0.0, 1.0},
        {1.0, 1.0, 1.0, 1.0, 1.0},
        {0.0, 0.0, 0.0, 0.0, 1.0},
        {1.0, 1.0, 1.0, 1.0, 1.0}
    },
    // Dígito 4
    {
        {1.0, 0.0, 0.0, 0.0, 1.0},
        {1.0, 0.0, 0.0, 0.0, 1.0},
        {1.0, 1.0, 1.0, 1.0, 1.0},
        {0.0, 0.0, 0.0, 0.0, 1.0},
        {0.0, 0.0, 0.0, 0.0, 1.0}
    },
    // Dígito 5
    {
        {1.0, 1.0, 1.0, 1.0, 1.0},
        {1.0, 0.0, 0.0, 0.0, 0.0},
        {1.0, 1.0, 1.0, 1.0, 1.0},
        {0.0, 0.0, 0.0, 0.0, 1.0},
        {1.0, 1.0, 1.0, 1.0, 1.0}
    },
    // Dígito 6
    {
        {1.0, 1.0, 1.0, 1.0, 1.0},
        {1.0, 0.0, 0.0, 0.0, 0.0},
        {1.0, 1.0, 1.0, 1.0, 1.0},
        {1.0, 0.0, 0.0, 0.0, 1.0},
        {1.0, 1.0, 1.0, 1.0, 1.0}
    },
    // Dígito 7
    {
        {1.0, 1.0, 1.0, 1.0, 1.0},
        {0.0, 0.0, 0.0, 0.0, 1.0},
        {0.0, 0.0, 0.0, 1.0, 0.0},
        {0.0, 0.0, 1.0, 0.0, 0.0},
        {0.0, 1.0, 0.0, 0.0, 0.0}
    },
    // Dígito 8
    {
        {1.0, 1.0, 1.0, 1.0, 1.0},
        {1.0, 0.0, 0.0, 0.0, 1.0},
        {1.0, 1.0, 1.0, 1.0, 1.0},
        {1.0, 0.0, 0.0, 0.0, 1.0},
        {1.0, 1.0, 1.0, 1.0, 1.0}
    },
    // Dígito 9
    {
        {1.0, 1.0, 1.0, 1.0, 1.0},
        {1.0, 0.0, 0.0, 0.0, 1.0},
        {1.0, 1.0, 1.0, 1.0, 1.0},
        {0.0, 0.0, 0.0, 0.0, 1.0},
        {1.0, 1.0, 1.0, 1.0, 1.0}
    }
};

// --------------------------------------------------
// Função: ws2812_show()
// Envia os dados do buffer led_matrix para os LEDs WS2812
// usando a função ws2812_put definida acima.
// --------------------------------------------------
void ws2812_show() {
    for (int i = 0; i < NUM_LEDS; i++) {
        ws2812_put(pio0, 0, led_matrix[i]);
    }
    // Aguarda um curto período para que os dados "trave" nos LEDs (mínimo 50 us)
    sleep_us(50);
}

// --------------------------------------------------
// Função: display_number()
// Atualiza o buffer led_matrix para exibir o dígito (0 a 9)
// de acordo com o padrão definido e o mapeamento serpentino.
// Decidi que os LEDs acesos serão exibidos na cor azul.
// --------------------------------------------------
void display_number(int num) {
    if(num < 0 || num > 9) return;
    
    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 5; col++) {
            int index = led_index_map[row][col];
            if (digit_patterns[num][row][col] == 1.0) {
                // LED aceso: cor azul (somente canal azul ativo)
                led_matrix[index] = GRB(0, 0, 255);
            } else {
                // LED apagado
                led_matrix[index] = GRB(0, 0, 0);
            }
        }
    }
    ws2812_show();
}

// --------------------------------------------------
// Função: gpio_callback()
// Callback de interrupção para os botões A e B.
// Implementa debounce via software e seta flags para o loop principal.
// --------------------------------------------------
void gpio_callback(uint gpio, uint32_t events) {
    uint32_t now = to_ms_since_boot(get_absolute_time());
    if (gpio == BUTTON_A_PIN) {
        if (now - last_time_button_a > DEBOUNCE_MS) {
            last_time_button_a = now;
            button_a_flag = true;
        }
    } else if (gpio == BUTTON_B_PIN) {
        if (now - last_time_button_b > DEBOUNCE_MS) {
            last_time_button_b = now;
            button_b_flag = true;
        }
    }
}

// --------------------------------------------------
// Função: led_timer_callback()
// Callback do timer repetitivo para piscar o LED vermelho a 5 Hz.
// --------------------------------------------------
bool led_timer_callback(struct repeating_timer *t) {
    static bool led_state = false;
    led_state = !led_state;
    gpio_put(LED_RED_PIN, led_state);
    return true;  // Retorna true para manter o timer
}

// --------------------------------------------------
// Função Principal
// --------------------------------------------------
int main() {
    stdio_init_all();

    // --------------------------------------------------
    // Inicialização do WS2812 via PIO
    // --------------------------------------------------
    PIO pio = pio0;
    uint sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, false);
    
    // --------------------------------------------------
    // Configuração do LED Vermelho (saída)
    // --------------------------------------------------
    gpio_init(LED_RED_PIN);
    gpio_set_dir(LED_RED_PIN, GPIO_OUT);
    gpio_put(LED_RED_PIN, 0);

    // --------------------------------------------------
    // Configuração dos Botões (entradas com pull-up)
    // --------------------------------------------------
    gpio_init(BUTTON_A_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN);
    
    gpio_init(BUTTON_B_PIN);
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_B_PIN);

    // Configura interrupções para os botões (borda de descida)
    gpio_set_irq_enabled_with_callback(BUTTON_A_PIN, GPIO_IRQ_EDGE_FALL, true, gpio_callback);
    gpio_set_irq_enabled(BUTTON_B_PIN, GPIO_IRQ_EDGE_FALL, true);

    // Exibe o dígito inicial (0)
    display_number(current_number);

    // --------------------------------------------------
    // Configuração do Timer Repetitivo para piscar o LED vermelho
    // --------------------------------------------------
    struct repeating_timer led_timer;
    add_repeating_timer_ms(100, led_timer_callback, NULL, &led_timer);

    // --------------------------------------------------
    // Loop Principal: verifica flags dos botões para atualizar o dígito exibido
    // --------------------------------------------------
    while (true) {
        if (button_a_flag) {
            button_a_flag = false;
            current_number = (current_number + 1) % 10;
            display_number(current_number);
        }
        if (button_b_flag) {
            button_b_flag = false;
            current_number = (current_number + 9) % 10;  // Decremento circular
            display_number(current_number);
        }
        sleep_ms(10);
    }
    
    return 0;
}