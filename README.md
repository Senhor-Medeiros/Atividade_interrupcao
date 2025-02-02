# Projeto: Interrupções no RP2040 com Matriz de LEDs WS2812

## Descrição
Este projeto explora o uso de interrupções no microcontrolador RP2040, implementado na placa BitDogLab, para controlar uma matriz 5x5 de LEDs WS2812 e um LED RGB, bem como a leitura de botões com debouncing via software. O projeto segue os requisitos da atividade descritos no documento de referência.

## Funcionalidades Implementadas
- O LED vermelho do LED RGB pisca continuamente 5 vezes por segundo.
- O botão A incrementa o número exibido na matriz de LEDs a cada pressão.
- O botão B decrementa o número exibido na matriz de LEDs a cada pressão.
- A matriz WS2812 exibe números de 0 a 9 em um formato fixo (estilo digital).
- Interrupções (IRQ) são utilizadas para capturar os eventos dos botões.
- Debouncing via software para evitar leituras incorretas dos botões.

## Componentes Utilizados
- **Microcontrolador RP2040** (BitDogLab)
- **Matriz de LEDs WS2812** (5x5) conectada à GPIO 7
- **LED RGB** com pinos nas GPIOs 11, 12 e 13
- **Botão A** conectado à GPIO 5
- **Botão B** conectado à GPIO 6
- **Resistores de Pull-Up** internos ativados para os botões

## Estrutura do Projeto
```
/
├── CMakeLists.txt              # Arquivo de configuração do CMake
├── pico_sdk_import.cmake       # Importação do SDK do RP2040
├── interruption.c              # Código principal do projeto
├── ws2812.pio.h                # Arquivo gerado pelo PIO para controle dos LEDs WS2812
├── blink.pio                   # Código PIO para o controle dos LEDs (se aplicável)
├── diagram.json                # Diagrama de conexão do projeto
├── wokwi.toml                  # Arquivo de configuração para simulação no Wokwi
└── README.txt                  # Este arquivo
```

## Configuração e Compilação
1. **Instale o Raspberry Pi Pico SDK**
   ```bash
   git clone -b master https://github.com/raspberrypi/pico-sdk.git
   cd pico-sdk
   git submodule update --init
   ```
2. **Clone este repositório**
   ```bash
   git clone <URL_DO_REPOSITORIO>
   cd <NOME_DO_PROJETO>
   ```
3. **Crie um diretório de build**
   ```bash
   mkdir build && cd build
   ```
4. **Configure o CMake**
   ```bash
   cmake ..
   ```
5. **Compile o projeto**
   ```bash
   make
   ```
6. **Grave o firmware na placa**
   - Conecte a BitDogLab ao PC segurando o botão BOOTSEL.
   - Copie o arquivo `interruption.uf2` para a unidade que aparece no sistema.

## Simulação no Wokwi
1. Abra o [Wokwi](https://wokwi.com/)
2. Importe o arquivo `diagram.json`
3. Execute a simulação

## Demonstração do Projeto
Confira o funcionamento do projeto no seguinte vídeo: [YouTube](https://youtu.be/mHMIAnvlU1A)

## Licença
Este projeto é distribuído sob a licença MIT. Veja o arquivo LICENSE para mais detalhes.

---

Autor: **Gabriel**

