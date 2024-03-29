/*
Setup:
* Connect a btn0 to pin 19 (ButtonPin) and ground.
* Connect a btn1 to pin 18 (ButtonPin) and ground.
*
* The Serial interface is used for output the detected events.
*/

/*
GPIO Pins Corresponding:

GPIO      Connected to...
2         BTNLED1             'INPUT_SELECTOR_BUTTON_LED'
4         SPK                 'BUZZER'
12        PRLY4               'PowerRelay(12)'  (360VDCOUT)
13        1WIRE               'TEMP_SENS'
14        PRLY3               'PowerRelay(14)'  (6.3ACOUT)
15        BTNLED0             'POWER_BUTTON_LED'
16        VOLUME MOTOR REV
17        VOLUME MOTOR FWD
18        BTN1                'INPUT_SELECTOR_BUTTON'
19        BTN0                'POWER_BUTTON'
21        MPWRON              'MAINPOWERON'
22        SDA                 'I2C_SDA'
23        SCL                 'I2C_SCL'
25        PRLY0               'PowerRelay(25)'  (230VACIN)
26        PRLY1               'PowerRelay(26)'  (12ACOUT)
27        PRLY2               'PowerRelay(27)'  (250VACOUT)
32        INPTRLY0            'InputRelay(32)'  (IN1)
33        INPTRLY1            'InputRelay(33)'  (IN2)
36        ADC1                'SELFVIN'
*/

// POWER:
#define PRLY0_PIN 25
#define PRLY1_PIN 26
#define PRLY2_PIN 27
#define PRLY3_PIN 14
#define PRLY4_PIN 12

#define MAIN_POWER_SOURCE_PIN 21

// BUTTONS:
#define POWER_BUTTON_PIN 19
#define INPUT_SELECTOR_BUTTON_PIN 18

// VOLUME:
#define REVERSE_VOLUME_PIN 16
#define REVERSE_VOLUME_CHANNEL 2

#define FORWARD_VOLUME_PIN 17
#define FORWARD_VOLUME_CHANNEL 3

// LEDS:
#define POWER_BUTTON_LED_PIN 15
#define POWER_BUTTON_LED_CHANNEL 0

#define INPUT_SELECTOR_BUTTON_LED_PIN 2
#define INPUT_SELECTOR_BUTTON_LED_CHANNEL 1

#define SHORT_LED_BLINK_INTERVAL 75
#define LONG_LED_BLINK_INTERVAL 150

#define SHORT_LED_PWM_INTERVAL 5
#define LONG_LED_PWM_INTERVAL 15

#define PWM_FREQUENCY 25000
#define PWM_RESOLUTION 8
#define MIN_PWM_DUTY 0
#define MAX_PWM_DUTY 255

// BUZZER:
#define BUZZER_PIN 4
#define SHORT_BUZZ_INTERVAL 50
#define LONG_BUZZ_INTERVAL 100

// INPUT RELAYS
#define MAIN_INPUT_RELAY_PIN 32
#define SECONDARY_INPUT_RELAY_PIN 33

// Tasks:
// #define LONG_TASK_DELAY 1000 // For Debugging
#define LONG_TASK_DELAY 25000 // For Release

#define LONG_TASK_RUNTIME LONG_TASK_DELAY + 500
#define FAST_TASK_RUNTIME 500

// 1WIRE:
#define ONE_WIRE_BUS 13

// I2C
#define SDA 22
#define SCL 23

#define INA_PIN_1 0x40
#define INA_PIN_2 0x41
#define INA_PIN_3 0x42
#define INA_PIN_4 0x43

#define ADC_PIN 36
// Other:
#define COM_PORT_SPEED 115200