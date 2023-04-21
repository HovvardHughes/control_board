#include <EEPROM.h>

#define VU_TURNED_OFF_MANUALLY_ADDRESS 0
#define INPUT_SELECTOR_STATE_ADDRESS 1

#define MAIN_INPUT_SELECTOR_RELAY_BIT 0
#define SECONDARY_INPUT_SELECTOR_RELAY_BIT 1

#define SIZE_IN_BYTES 512

struct InputSelectorState
{
    bool isMainRelayPoweredOn;
    bool isSecondaryRelayPowered;
    uint8_t selectedRelayPin;
};

class ControlBoardEEPROM
{
public:
    void setup()
    {
        EEPROM.begin(SIZE_IN_BYTES);
    }

    bool readVUTurnedOffManually()
    {
        return EEPROM.read(VU_TURNED_OFF_MANUALLY_ADDRESS);
    }

    void writeVUTurnedOffManually(bool state)
    {
        EEPROM.write(VU_TURNED_OFF_MANUALLY_ADDRESS, state);
        EEPROM.commit();
    }

    void writeInputSelectorState(bool mainRelayState, bool secondaryRelayState, uint8_t selectedRelayPin)
    {
        uint8_t state = 0;

        state |= mainRelayState;
        state |= secondaryRelayState << 1;
        state |= (selectedRelayPin == MAIN_INPUT_RELAY_PIN ? MAIN_INPUT_SELECTOR_RELAY_BIT : SECONDARY_INPUT_SELECTOR_RELAY_BIT) << 2;

        Serial.println(state);
        EEPROM.write(INPUT_SELECTOR_STATE_ADDRESS, state);
        EEPROM.commit();
    }

    InputSelectorState readInputSelectorState()
    {
        uint8_t readResult = EEPROM.read(INPUT_SELECTOR_STATE_ADDRESS);
        return {
            (readResult & 1) == 1,
            (readResult & 2) == 2,
            (readResult & 4) == 4 ? (uint8_t)SECONDARY_INPUT_RELAY_PIN : (uint8_t)MAIN_INPUT_RELAY_PIN};
    }
};