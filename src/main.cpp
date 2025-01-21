#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

// Pin Definitions
#define DHTPIN 4      // GPIO pin where the DHT11 is connected
#define DHTTYPE DHT11 // DHT11 sensor type

#define LED_BUILTIN 2 // Built-in LED

const int ENA = 32; // PWM for Fan 1
const int ENB = 33; // PWM for Fan 2
const int IN1 = 25; // Direction for Fan 1
const int IN2 = 26;
const int IN3 = 27; // Direction for Fan 2
const int IN4 = 14;

int fan1Speed = 150;
int fan2Speed = 150;

DHT dht(DHTPIN, DHTTYPE);

void setFanSpeed(int fan, int speed)
{
  if (fan == 1)
  {
    analogWrite(ENA, speed);
  }
  else if (fan == 2)
  {
    analogWrite(ENB, speed);
  }
}

void setFanDirection(int fan, bool forward)
{
  if (fan == 1)
  {
    digitalWrite(IN1, forward ? HIGH : LOW);
    digitalWrite(IN2, forward ? LOW : HIGH);
  }
  else if (fan == 2)
  {
    digitalWrite(IN3, forward ? HIGH : LOW);
    digitalWrite(IN4, forward ? LOW : HIGH);
  }
}

void showLEDPulse(int duration = 100)
{
  digitalWrite(LED_BUILTIN, HIGH);
  delay(duration);
  digitalWrite(LED_BUILTIN, LOW);
  delay(duration);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(duration);
  digitalWrite(LED_BUILTIN, LOW);
  delay(duration);
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting ESP...");

  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  setFanDirection(1, true);
  setFanDirection(2, true);
  setFanSpeed(1, fan1Speed);
  setFanSpeed(2, fan2Speed);

  dht.begin();
  Serial.println("DHT11 sensor started");
}

void loop()
{

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity))
  {
    Serial.println("Failed to read from DHT sensor!");
  }

  else
  {
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");

    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");
  }

  showLEDPulse();

  delay(2000);
}
