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

// BUTTONS:
#define POWER_BUTTON_PIN 19
#define INPUT_SELECTOR_BUTTON_PIN 18
#define MAIN_POWER_ON_PIN 21
// LEDS:
#define POWER_BUTTON_LED_PIN 15
#define INPUT_SELECTOR_BUTTON_LED_PIN 2

#define POWER_BUTTON_LED_CHANNEL 0
#define INPUT_SELECTOR_BUTTON_LED_CHANNEL 1

#define FREQUENCY 25000

#define LED_RESOLUTION 8

#define MIN_LED__DUTY 0
#define MAX_LED__DUTY 255

#define LED_BLINK_DELAY 150 // blink delay in millis

// BUZZER:
#define BUZZER_PIN 4
// I2C:

// 1WIRE:

#define COM_PORT_SPEED 115200 // COM Port Baud Rate

#define DELAY_IN_MILLIS 1000 // For Debugging
//#define DELAY_IN_MILLIS 25000    //For Release - Pre-programmed Heating Filaments Delay

#define DELAY_COMMAND -1

#define POWER_RELAY_COUNT 5 // Number of power-relays in array
#define INPUT_RELAY_COUNT 2 // Number of power-relays in array

#define SHORT_BUZZ_TIME 50 // short buzz delay in millis
#define LONG_BUZZ_TIME 100 // long buzz delay in millis

#define MAIN_INPUT_RELAY_IO_NUMBER 32
#define SECONDARY_INPUT_RELAY_IO_NUMBER 33
