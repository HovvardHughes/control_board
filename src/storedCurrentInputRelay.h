#include <constants.h>
#include <EEPROM.h>
#include <Relay.h>

#define CURRENT_INPUT_RELAY_IO_NUMBER_EEPROM_ADDRESS 0
#define EEPROM_SIZE_IN_BYTES 512

byte readCurrentInputRelayIONumberFromEEPROM()
{
  return -1;
  // return EEPROM.read(CURRENT_INPUT_RELAY_IO_NUMBER_EEPROM_ADDRESS);
}

void writeCurrentInputRelayIONumberToEEPROM(byte iONumber)
{
  // EEPROM.write(CURRENT_INPUT_RELAY_IO_NUMBER_EEPROM_ADDRESS, iONumber);
  // EEPROM.commit();
}

void InitEEPROM()
{
  // EEPROM.begin(EEPROM_SIZE_IN_BYTES);
}
