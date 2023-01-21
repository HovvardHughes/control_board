#include <Wire.h>
#include <INA219_WE.h>
#include <string>
#include <iostream>

#define INA_COUNT 4
#define CORRECTION_FACTOR 1.025
#define CORRECTION_ADS 1.01

#define MAX_NORMAL_CURRENT_MA 85.00
#define HIGH_CURRENT_MAX_DURATION 5000

struct INA219_WE_STATE
{
    INA219_WE ina;
    float lastCurrentMeasurement;
    unsigned long highCurrentStaredMillis;
};

class CurrentMeasurer
{
private:
    INA219_WE_STATE inas[INA_COUNT] = {
        {INA219_WE(INA_PIN_1), 0.0, 0},
        {INA219_WE(INA_PIN_2), 0.0, 0},
        {INA219_WE(INA_PIN_3), 0.0, 0},
        {INA219_WE(INA_PIN_4), 0.0, 0}};

    TaskController *_taskController;

    void (*_powerOffEmergency)();
    bool (*_isPowerOn)();

    void updateCathodeCurrentMa(INA219_WE_STATE *inaState)
    {
        float busVoltage_V = inaState->ina.getBusVoltage_V();
        float correctedVoltage_V = busVoltage_V * CORRECTION_FACTOR;
        float cathodeShuntCurrent_mA = correctedVoltage_V * 100;
        inaState->lastCurrentMeasurement = cathodeShuntCurrent_mA;
    }

    bool checkInaState(INA219_WE_STATE *inaState)
    {
        if (inaState->lastCurrentMeasurement > MAX_NORMAL_CURRENT_MA)
        {
            Serial.println("Токи больше");
            if (inaState->highCurrentStaredMillis > 0 && millis() - inaState->highCurrentStaredMillis > HIGH_CURRENT_MAX_DURATION)
            {
                Serial.println("Выключаю питание");
                _powerOffEmergency();
                inaState->highCurrentStaredMillis = 0;
                return false;
            }
            else
            {
                if (inaState->highCurrentStaredMillis == 0)
                {
                    Serial.println("Устанавливаю секундомер");
                    inaState->highCurrentStaredMillis = millis();
                }
            }
        }
        else
            inaState->highCurrentStaredMillis = 0;

        return true;
    }

public:
    CurrentMeasurer(TaskController *taskController, void (*powerOffEmergency)(), bool (*isPowerOn)())
    {
        _taskController = taskController;
        _powerOffEmergency = powerOffEmergency;
        _isPowerOn = isPowerOn;
    }

    void setup()
    {
        Wire.begin(SDA, SCL);

        for (size_t i = 0; i < INA_COUNT; i++)
        {
            INA219_WE ina = inas[i].ina;
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
            INA219_WE ina = inas[i].ina;

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

    void process()
    {
        for (size_t i = 0; i < INA_COUNT; i++)
            updateCathodeCurrentMa(&inas[i]);

        if (!_isPowerOn() || _taskController->isRunningTask())
            return;

        for (size_t i = 0; i < INA_COUNT; i++)
            if (!checkInaState(&inas[i]))
                break;
    }

    String getCathodeCurrentsString()
    {
        return String(inas[0].lastCurrentMeasurement) + ":" +
               String(inas[1].lastCurrentMeasurement) + ":" +
               String(inas[2].lastCurrentMeasurement) + ":" +
               String(inas[3].lastCurrentMeasurement);
    }

    String getVoltageString()
    {
        return String(analogRead(ADC_PIN) * 3.3 / 4095 * CORRECTION_ADS);
    }
};