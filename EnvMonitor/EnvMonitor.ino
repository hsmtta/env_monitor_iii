#include "QMP6988.h"        // Pressure sensor
#include "SHT3X.h"          // Temperature and Humidity sensor
#include <Adafruit_SGP30.h> // CO2 sensor
#include <Ambient.h>
#include <M5Atom.h>

// WiFi
#define WIFI_SSID "ssid"
#define WIFI_PASSWD "passwd"

// Measurement interval in second
#define INTERVAL 9
#define WINDOW 20

// Ambient
#define IS_USE_AMBIENT true
#define CHANNELID -1
#define WRITEKEY "writekey"

Adafruit_SGP30 sgp;
SHT3X sht3x;
QMP6988 qmp6988;

WiFiClient client;
Ambient ambient;

uint32_t getAbsoluteHumidity(float temperature, float humidity)
{
    // approximation formula from Sensirion SGP30 Driver Integration chapter 3.15
    const float absoluteHumidity = 216.7f * ((humidity / 100.0f) * 6.112f * exp((17.62f * temperature) / (243.12f + temperature)) / (273.15f + temperature)); // [g/m^3]
    const uint32_t absoluteHumidityScaled = static_cast<uint32_t>(1000.0f * absoluteHumidity);                                                                // [mg/m^3]
    return absoluteHumidityScaled;
}

bool has_temp_sensor = false;
bool has_pressure_sensor = false;
bool has_co2_sensor = false;

void setup()
{
    M5.begin(true, true, true);

    while (!Serial) // Wait for serial console to open.
    {
        delay(10);
    }

    has_temp_sensor = sht3x.get() == 0;
    has_pressure_sensor = qmp6988.init();
    has_co2_sensor = sgp.begin();

    // Start sensors
    if (has_temp_sensor)
        Serial.println("Temp / Hum sensor were found :)");
    if (has_pressure_sensor)
        Serial.println("Pressure sensor was found :)");
    if (has_co2_sensor)
        Serial.println("CO2 sensor was found :)");

    WiFi.begin(WIFI_SSID, WIFI_PASSWD);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(100);
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.print("Connected to WiFi: ");
        Serial.println(WiFi.localIP());
    }

    ambient.begin(CHANNELID, WRITEKEY, &client);

    Serial.println("Set up finished!!");
}

float temperature_tot = 0;
float humidity_tot = 0;
float pressure_tot = 0;
float eco2_tot = 0;
float tvoc_tot = 0;
int count = 0;

void loop()
{
    count++;

    // Measure temp and humidity
    has_temp_sensor = sht3x.get() == 0;
    if (has_temp_sensor)
    {
        float temperature = sht3x.cTemp;
        float humidity = sht3x.humidity;
        Serial.print("Temp: ");
        Serial.print(temperature);
        Serial.print(", Hum: ");
        Serial.print(humidity);
        Serial.print("\n");

        temperature_tot += temperature;
        humidity_tot += humidity;

        // Set the absolute humidity to enable the humditiy compensation for the air quality signals
        if (has_co2_sensor)
            sgp.setHumidity(getAbsoluteHumidity(temperature, humidity));
    }

    if (has_pressure_sensor)
    {
        float pressure = qmp6988.calcPressure();
        Serial.print("Pressure: ");
        Serial.print(pressure / 100);
        Serial.print("\n");

        pressure_tot += pressure / 100;
    }

    // Measure air quality
    if (sgp.IAQmeasure())
    {
        Serial.print("TVOC: ");
        Serial.print(sgp.TVOC);
        Serial.print(" ppb, ");
        Serial.print("eCO2: ");
        Serial.print(sgp.eCO2);
        Serial.print(" ppm\n");

        tvoc_tot += sgp.TVOC;
        eco2_tot += sgp.eCO2;
    }

    if (IS_USE_AMBIENT && count == WINDOW)
    {
        ambient.set(1, temperature_tot / count);
        ambient.set(2, humidity_tot / count);
        ambient.set(3, pressure_tot / count);
        ambient.set(4, tvoc_tot / count);
        ambient.set(5, eco2_tot / count);
        
        count = 0;
        temperature_tot = 0;
        humidity_tot = 0;
        pressure_tot = 0;
        tvoc_tot = 0;
        eco2_tot = 0;

        if (!ambient.send()){
            Serial.println("Failed to send the measurements to the ambient server");
        }
        else {
            Serial.println("Push the measurements to the ambient server"); 
        }
    }
    delay(INTERVAL * 1000);
}
