#include <Wire.h>
#include <INA219_WE.h>
#include <string>
#include <iostream>

#define INA_COUNT 4
#define CORRECTION_FACTOR 1.025
#define CORRECTION_ADS 1.01

class CurrentMeasurer
{
private:
    INA219_WE inas[INA_COUNT] = {INA219_WE(INA_PIN_1), INA219_WE(INA_PIN_2), INA219_WE(INA_PIN_3), INA219_WE(INA_PIN_4)};

    float getCathodeCurrent(INA219_WE ina)
    {
        float busVoltage_V = ina.getBusVoltage_V();
        float correctedVoltage_V = busVoltage_V * CORRECTION_FACTOR;
        float cathodeShuntCurrent_mA = correctedVoltage_V * 100;
        return cathodeShuntCurrent_mA;
    }

public:
    void setup()
    {
        Wire.begin(SDA, SCL);

        for (size_t i = 0; i < INA_COUNT; i++)
        {
            INA219_WE ina = inas[i];
            ina.setADCMode(SAMPLE_MODE_128);
            ina.setMeasureMode(CONTINUOUS);
            ina.setPGain(PG_40);
            ina.setBusRange(BRNG_16);
            if (!ina.init())
                Serial.println("Failed to init INA");
        }
    }

    void printDebugInfo()
    {
        Serial.println("");
        Serial.print("Voltage [V]: ");
        Serial.print(getVoltageString());

        for (size_t i = 0; i < INA_COUNT; i++)
        {
            INA219_WE ina = inas[i];

            float shuntVoltage_mV = ina.getShuntVoltage_mV();              // not used, for debug info only
            float busVoltage_V = ina.getBusVoltage_V();                    // used, primary reading
            float loadVoltage_V = busVoltage_V + (shuntVoltage_mV / 1000); // not used, for debug info only
            float current_mA = ina.getCurrent_mA();                        // not used, for debug info only
            float power_mW = ina.getBusPower();                            // not used, for debug info only
            bool ina219_overflow = ina.getOverflow();                      // for debug, migt be useful, might be changed to simple "o\f" in user intrface
            float correctedVoltage_V = busVoltage_V * CORRECTION_FACTOR;   // used, corrected measurements
            float cathodeShuntCurrent_mA = correctedVoltage_V * 100;       // used, "cheat" conversion V to mA, cause we measure V instead of mA
            float busVoltage_magnifier = busVoltage_V * 1000;              // Workaround used for debug + precise voltage("mA") tuning. The proposal to display in the terminal mode or do it getting out of a commentary for the possibility of adjusting
            Serial.println("");
            Serial.print("INA: ");
            Serial.print(i);
            Serial.print(" :: Shunt Voltage [mV]: ");
            Serial.print(shuntVoltage_mV);
            Serial.print(" :: Bus Voltage [V]: ");
            Serial.print(busVoltage_V);
            Serial.print(" :: Load Voltage [V]: ");
            Serial.print(loadVoltage_V);
            Serial.print(" :: Current [mA]: ");
            Serial.print(current_mA);
            Serial.print(" :: Bus Power [mW]: ");
            Serial.print(power_mW);

            Serial.print(" :: Bus Voltage Magnifier [V*100]: ");
            Serial.print(busVoltage_magnifier);

            Serial.print(" :: Corrected Voltage [V]: ");
            Serial.print(correctedVoltage_V);
            Serial.print(" ::Cathode current [mA]: ");
            Serial.print(cathodeShuntCurrent_mA);

            Serial.print(" :: Overflow: ");
            Serial.print(ina219_overflow);
        }

        Serial.println("");
    }

    String getCathodeCurrentsString()
    {
        return String(getCathodeCurrent(inas[0])) + ":" +
               String(getCathodeCurrent(inas[1])) + ":" +
               String(getCathodeCurrent(inas[2])) + ":" +
               String(getCathodeCurrent(inas[3]));
    }

    String getVoltageString()
    {
        return String(analogRead(ADC_PIN) * 3.3 / 4095 * CORRECTION_ADS);
    }
};