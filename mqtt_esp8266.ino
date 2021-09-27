
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <MFRC522.h>
#include <SPI.h>

#define RST_PIN         D3         // Configurable, see typical pin layout above
#define SS_PIN          D8        // Configurable, see typical pin layout above

MFRC522 rfid(SS_PIN, RST_PIN);  
// Update these with values suitable for your network.
byte nuidPICC[4];
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];

const char* ssid = "iPhone";
const char* password = "shayshay1";
const char* mqtt_server = "172.20.10.2";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;

int value = 0;
String userid;

int LEDWATER = D0;
int LEDTEMP = D1;
int LEDWAIT = 4;


void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  String inmsg;
  int temp=0;
  int waterlevel=0;
  int newt,neww=0;
  String tempstr;
  String waterlevelstr;
  for (int i = 0; i < length; i++) {
    inmsg += (char)payload[i];
  }
  tempstr = inmsg.substring(0,2);
  waterlevelstr = inmsg.substring(2,4);
  newt = tempstr.toInt();
  neww = waterlevelstr.toInt();
  Serial.printf("\nwater lvl is :%d and temp is :%d\n",neww,newt);
  temp = map(newt, 0,100, 0,255);
  waterlevel = map(neww, 0,100, 0,255);
  
  int sensorvalue=analogRead(A0);
  int value = map(sensorvalue, 0,1023, 0,255);
  analogWrite(LEDTEMP,temp);
  while(value<waterlevel){
    sensorvalue=analogRead(A0);
    value = map(sensorvalue, 0,1023, 0,255);
    analogWrite(LEDWATER,value);
    Serial.printf("\nwater lvl is :%d and we want :%d\n",value,waterlevel);
    digitalWrite(LEDWAIT,HIGH);
    delay(200);
    digitalWrite(LEDWAIT,LOW);
    delay(200);
  }
  delay(2000);
  digitalWrite(LEDTEMP,LOW);
  digitalWrite(LEDWATER,LOW);
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP";
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      client.subscribe("userdata");
    } else {
      delay(5000);
    }
  }
}

void setup() {
  
  Serial.begin(9600);
  SPI.begin();      // Init SPI bus
  rfid.PCD_Init();   // Init MFRC522
  rfid.PCD_DumpVersionToSerial();  // Show details of PCD - MFRC522 Card Reader details
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  pinMode(LEDWATER,OUTPUT);
  pinMode(LEDTEMP,OUTPUT);
  pinMode(LEDWAIT,OUTPUT);
}

void loop() {
  
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  if ( ! rfid.PICC_IsNewCardPresent()) {
    return;
  }
  if ( ! rfid.PICC_ReadCardSerial()) {
    return;
  }
  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
    piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
    piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }
  for (byte i = 0; i < 4; i++) {
      nuidPICC[i] = rfid.uid.uidByte[i];
  }
 
for (byte i = 0; i < rfid.uid.size; i++) {
  userid += String(rfid.uid.uidByte[i], HEX);
}
Serial.printf("sending %s\n",userid.c_str());
client.publish("userid", userid.c_str());
  userid="";
  // Halt PICC
  rfid.PICC_HaltA();
  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
}
