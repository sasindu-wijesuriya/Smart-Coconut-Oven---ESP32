#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <WiFi.h>
#include <PubSubClient.h>

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

// Wi-Fi and MQTT configurations
const char *ssid = "AndroidAP";
const char *password = "blng354212";
const char *mqtt_server = "test.mosquitto.org"; // Test MQTT broker

WiFiClient espClient;
PubSubClient client(espClient);

const char *fan1SpeedTopic = "smart_coconut_oven/fan_1_speed";
const char *fan2SpeedTopic = "smart_coconut_oven/fan_2_speed"; // New topic for Fan 2 speed

// Topics for temperature and humidity
const char *tempTopic = "smart_coconut_oven/sensor1/temp";
const char *humidityTopic = "smart_coconut_oven/sensor1/humidity";

void setFanSpeed(int fan, int speed);
void setFanDirection(int fan, bool forward);

void callback(char *topic, byte *payload, unsigned int length)
{
  String message = "";
  for (unsigned int i = 0; i < length; i++)
  {
    message += (char)payload[i];
  }

  if (String(topic) == fan1SpeedTopic)
  {
    int newSpeed = message.toInt();
    if (newSpeed >= 0 && newSpeed <= 255)
    {
      fan1Speed = newSpeed;
      setFanSpeed(1, fan1Speed);
      Serial.print("Fan 1 speed set to: ");
      Serial.println(fan1Speed);
    }
  }
  else if (String(topic) == fan2SpeedTopic)
  {
    int newSpeed = message.toInt();
    if (newSpeed >= 0 && newSpeed <= 255)
    {
      fan2Speed = newSpeed;
      setFanSpeed(2, fan2Speed);
      Serial.print("Fan 2 speed set to: ");
      Serial.println(fan2Speed);
    }
  }
}

void reconnect()
{
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("")) // Empty client ID
    {
      Serial.println("connected");
      client.subscribe(fan1SpeedTopic); // Subscribe to Fan 1 speed topic
      client.subscribe(fan2SpeedTopic); // Subscribe to Fan 2 speed topic
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      delay(5000); // Wait for 5 seconds before retrying
    }
  }
}

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

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected to Wi-Fi");

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop()
{
  if (!client.connected())
  {
    reconnect();
  }

  client.loop();

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

    // Publish the temperature and humidity readings to MQTT topics
    char tempStr[8];
    char humidityStr[8];
    dtostrf(temperature, 1, 2, tempStr);  // Convert temperature to string
    dtostrf(humidity, 1, 2, humidityStr); // Convert humidity to string

    client.publish(tempTopic, tempStr);         // Publish temperature
    client.publish(humidityTopic, humidityStr); // Publish humidity
  }

  showLEDPulse();
  delay(2000); // Wait for 2 seconds before reading again
}
