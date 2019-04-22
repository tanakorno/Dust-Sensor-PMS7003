#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <PMS.h>
#include <DHT.h>

const char* ssid      = "ssid";
const char* password  = "password";
const char* host      = "dweet.io";

#define DHTPIN  D5
#define RXPIN   D6
#define TXPIN   D7

SoftwareSerial pms_serial(RXPIN, TXPIN);
PMS pms(pms_serial);
PMS::DATA data;
DHT dht(DHTPIN, DHT11);

const char* thing  = "thing_name";

float h         = 0;
float t         = 0;
float pm1_0     = 0;
float pm2_5     = 0;
float pm10_0    = 0;

void setup()
{
    Serial.begin(115200);
    delay(10);

    dht.begin();

    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    // Connecting to WiFi network
    connectWiFi();
}


void connectWiFi()
{
    if (WiFi.status() == WL_CONNECTED) return;

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}


void loop()
{
    connectWiFi();

    if (pms.read(data)) {
        pm1_0   = data.PM_AE_UG_1_0;
        pm2_5   = data.PM_AE_UG_2_5;
        pm10_0  = data.PM_AE_UG_10_0;

        h       = dht.readHumidity();
        t       = dht.readTemperature();

        if (isnan(h) || isnan(t)) {
            Serial.println("Failed to read from DHT sensor!");
            return;
        }

        Serial.println("");
        Serial.print("connecting to ");
        Serial.println(host);

        // Use WiFiClient class to create TCP connections
        WiFiClient client;
        const int httpPort = 80;
        if (!client.connect(host, httpPort)) {
            Serial.println("connection failed");
            return;
        }

        String url = "/dweet/for/";
        url += thing;
        url += "?";
        url += "temp=";
        url += t;
        url += "&humidity=";
        url += h;
        url += "&pm1_0=";
        url += pm1_0;
        url += "&pm2_5=";
        url += pm2_5;
        url += "&pm10_0=";
        url += pm10_0;

        Serial.print("Requesting URL: ");
        Serial.println(url);

        // This will send the request to the server
        client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                     "Host: " + host + "\r\n" +
                     "Connection: close\r\n\r\n");

        unsigned long timeout = millis();
        while (client.available() == 0) {
            if (millis() - timeout > 5000) {
                Serial.println(">>> Client Timeout !");
                client.stop();
                return;
            }
        }

        // Read all the lines of the reply from server and print them to Serial
        while (client.available()) {
            String line = client.readStringUntil('\r');
            Serial.print(line);
        }

        Serial.println();
        Serial.println("closing connection");

    }

}
