#include <WiFi.h>
#include <PubSubClient.h>

#define WIFI_STA_NAME "_WIFI_SSID_"
#define WIFI_STA_PASS "_WIFI_PASSWORD_"

#define MQTT_SERVER   "_MQTT_BROKER_"
#define MQTT_PORT     10803
#define MQTT_USERNAME "_MQTT_USER_"
#define MQTT_PASSWORD "_MQTT_PASSWORD_"
#define MQTT_NAME     "_MQTT_NAME_"

WiFiClient client;
PubSubClient mqtt(client);
uint8_t tmp_state = 0;
double temp = 0.0;
char mqtt_payload[100];

// MQTT Callback
void callback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0';
  String topic_str = topic, payload_str = (char*)payload;
  Serial.println("[" + topic_str + "]: " + payload_str);
}

void setup() {

  // Initial Serial 
  Serial.begin(115200);
  Serial2.begin(9600);

  // WIFI Connecting
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_STA_NAME);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_STA_NAME, WIFI_STA_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  // Init MQTT
  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.setCallback(callback);
}

void loop() {
 
  // Read Serial from GP-100 and process with State Machine
  while (Serial2.available()){
    uint8_t recv = Serial2.read();
    Serial.println(recv);
    
    switch (tmp_state){
      case 0: 
        if (recv == 254)
          tmp_state = 1;
      break;
      
      case 1: 
      if (recv == 1)
          tmp_state = 2;
        else
          tmp_state = 0;
      break;
      
      case 2: 
      if (recv == 1)
          tmp_state = 3;
        else
          tmp_state = 0;
      break;
      
      case 3: 
        temp = (recv + 256.0)/10.0;
        char tempString[20];
        dtostrf(temp, 1, 2, tempString);

        strcpy(mqtt_payload, "{\"name\":\""); 
        strcat(mqtt_payload,MQTT_NAME);
        strcpy(mqtt_payload, "\",\"temp\":"); 
        strcat(mqtt_payload,tempString);
        strcat(mqtt_payload,"}");

        Serial.print("Temp : ");
        Serial.println(mqtt_payload);

        mqtt.publish("GP100/TEMP",mqtt_payload);
        tmp_state = 0;
      
      break;
    }
  }

  // MQTT Process
  if (mqtt.connected() == false) {
    Serial.print("MQTT connection... ");
    if (mqtt.connect(MQTT_NAME, MQTT_USERNAME, MQTT_PASSWORD)) {
      Serial.println("Connected");
      mqtt.publish("GP100", "Hello");
    } else {
      Serial.println("failed");
      delay(5000);
    }
  } else {
    mqtt.loop();
  }
}
