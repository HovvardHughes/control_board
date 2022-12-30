#include <EEPROM.h>

#define VU_TURNED_OFF_MANUALLY_ADDRESS 0
#define SIZE_IN_BYTES 512

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
};