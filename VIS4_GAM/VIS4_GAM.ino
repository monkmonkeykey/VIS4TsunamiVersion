/*---------------------------------------------------------------------------------------------

  Open Sound Control (OSC) library for the ESP8266/ESP32

  Example for receiving open sound control (OSC) messages on the ESP8266/ESP32
  Send integers '0' or '1' to the address "/led" to turn on/off the built-in LED of the esp8266.

  This example code is in the public domain.

  --------------------------------------------------------------------------------------------- */
//Pantalla OLED
#define __DEBUG__

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define ANCHO_PANTALLA 128  // ancho pantalla OLED
#define ALTO_PANTALLA 64    // alto pantalla OLED

Adafruit_SSD1306 display(ANCHO_PANTALLA, ALTO_PANTALLA, &Wire, -1);


//Tsunami
#include <SparkFun_Tsunami_Qwiic.h>  //http://librarymanager/All#SparkFun_Tsunami_Super_WAV_Trigger

TsunamiQwiic tsunami;


//ESP8266
#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <OSCBundle.h>
#include <OSCData.h>

char ssid[] = "AXTEL XTREMO-4E67";  // your network SSID (name)
char pass[] = "03874E67";           // your network password

// A UDP instance to let us send and receive packets over UDP
WiFiUDP Udp;
const IPAddress outIp(192, 168, 15, 19);  // remote IP (not needed for receive)
const unsigned int outPort = 8001;        // remote port (not needed for receive)
const unsigned int localPort = 8000;      // local port to listen for UDP packets (here's where we send the packets)
IPAddress ip;

OSCErrorCode error;

int layer1;
int layer2;
int layer3;
int momentaneo;
int contador1 = 0;
int statusPlaying1;
int statusPlaying2;
int layerSS1 = 0;
float estado1 = 0.0;
int memoria1 = 0;
void setup() {
  Serial.begin(115200);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
#ifdef __DEBUG__
    Serial.println("No se encuentra la pantalla OLED");
#endif
    while (true)
      ;
  }
  Wire.begin();
  display.clearDisplay();

  // Tamaño del texto
  display.setTextSize(1);
  // Color del texto
  display.setTextColor(SSD1306_WHITE);
  // Posición del texto
  display.setCursor(0, 32);
  // Escribir texto
  display.println("VIS4 version de 4 CH");
  display.display();
  delay(3000);

  //Comprobacion de conexión entre ESP8266 y Tsunami
  if (tsunami.begin() == false) {
    display.clearDisplay();
    display.println("Conexion fallida con Tsunami");
    display.display();
    Serial.println("Tsunami Qwiic failed to respond. Please check wiring and possibly the I2C address. Freezing...");
    while (1)
      ;
  };
  display.clearDisplay();

  // Conexion a WiFi
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  display.setCursor(0, 8);
  display.println("Conectando a");
  display.display();
  display.setCursor(0, 16);
  display.println(ssid);
  display.display();
  Serial.println(ssid);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  display.setCursor(0, 24);
  display.println(localPort);
  display.display();
  Udp.begin(localPort);
  Serial.println("Starting UDP");
  Serial.print("Local port: ");
#ifdef ESP32
  Serial.println(localPort);
#else
  Serial.println(Udp.localPort());
#endif
  layer1 = 0;
  layer2 = 0;
  layer3 = 0;
  ip = WiFi.localIP();
  Serial.println(ip);
  display.setCursor(0, 32);
  display.println(ip);
  display.display();
  //tsunami.trackPlaySolo(1, 0);  // track = 1 (aka "1.WAV"), output = 0 (aka "1L")
  delay(5000);
}


void layerS1(OSCMessage &msg4) {
  layerSS1 = msg4.getFloat(0);
  estado1 = layerSS1;
  display.clearDisplay();
  display.setCursor(0, 32);
  // Escribir texto
  display.println(layerSS1);
}

void loop() {

  reproduccion();

  receivedMessage();
  // Start playing them all at the same time.
  // Note, they will remain in "sample-sync" as they play back - sweet!
}
void reproduccion() {
  statusPlaying1 = tsunami.isTrackPlaying(1);
  statusPlaying2 = tsunami.isTrackPlaying(5);
  // Serial.println(contador1);
  //tsunami.update();

  if (estado1 == 1.) {


    contador1++;
    //delay(100);
    if (contador1 == 1) {
      display.clearDisplay();
      display.setCursor(0, 32);
      // Escribir texto
      display.println("Se reproduce la capa 1");
      display.display();
      tsunami.trackLoad(1, 0, false);  // track 1 on output 0 (aka "1L"), Lock = false (voice stealing active)
      tsunami.trackLoad(2, 1, false);  // track 2 on output 1 (aka "1R"), Lock = false (voice stealing active)
      tsunami.trackLoad(3, 2, false);  // track 3 on output 2 (aka "2L"), Lock = false (voice stealing active)
      tsunami.trackLoad(4, 3, false);  // track 4 on output 3 (aka "2R"), Lock = false (voice stealing active)
      tsunami.resumeAllInSync();
      digitalWrite(BUILTIN_LED, 1);
      display.clearDisplay();
      display.setCursor(0, 32);
      // Escribir texto
      display.println("Se reproduce la capa 1 mayor a 1");
      display.setCursor(0, 48);

      display.display();
      contador1++;
    } else if (estado1 == 0. || statusPlaying1 == 0) {

      //receivedMessage();
      tsunami.stopAllTracks();
      digitalWrite(BUILTIN_LED, 0);


      //Serial.println("Reproduciendo capa 1");
      OSCMessage msg("/layer1");
      msg.add(0.);
      Udp.beginPacket(outIp, outPort);
      msg.send(Udp);
      Udp.endPacket();
      msg.empty();
      display.clearDisplay();
      display.setCursor(0, 32);
      // Escribir texto
      display.println("Se ha terminado la capa 1");
      display.display();
      contador1 = 0;
      estado1 = 0.;
      //delay(100);
    }
  }
  /*
    //Serial.println("Reproduciendo capa 1");
    OSCMessage msg("/layer1");
    msg.add(0);
    Udp.beginPacket(outIp, outPort);
    msg.send(Udp);
    Udp.endPacket();
    msg.empty();
    delay(500);
*/

  else if (estado1 == 0.) {
    tsunami.stopAllTracks();
  } else if (layer2 == 1) {
    tsunami.trackLoad(1, 0, false);  // track 1 on output 0 (aka "1L"), Lock = false (voice stealing active)
    tsunami.trackLoad(2, 1, false);  // track 2 on output 1 (aka "1R"), Lock = false (voice stealing active)
    tsunami.trackLoad(3, 2, false);  // track 3 on output 2 (aka "2L"), Lock = false (voice stealing active)
    tsunami.trackLoad(4, 3, false);  // track 4 on output 3 (aka "2R"), Lock = false (voice stealing active)
    tsunami.resumeAllInSync();

    Serial.println("Reproduciendo capa 2");
  } else if (layer3 == 1) {
    tsunami.trackLoad(1, 0, false);  // track 1 on output 0 (aka "1L"), Lock = false (voice stealing active)
    tsunami.trackLoad(2, 1, false);  // track 2 on output 1 (aka "1R"), Lock = false (voice stealing active)
    tsunami.trackLoad(3, 2, false);  // track 3 on output 2 (aka "2L"), Lock = false (voice stealing active)
    tsunami.trackLoad(4, 3, false);  // track 4 on output 3 (aka "2R"), Lock = false (voice stealing active)
    tsunami.resumeAllInSync();
    Serial.println("Reproduciendo capa 3");
  } else if (statusPlaying1 == 1) {
    display.clearDisplay();
    display.setCursor(0, 32);
    // Escribir texto
    display.println("No pasa nada oiga");
    display.display();
    Serial.println("Esta reproduciendose la capa 1");
  }
  /*
  
  else if (statusPlaying1 == 0) {
    OSCMessage msg1("/layer1");
    OSCMessage msg2("/layer2");
    OSCMessage msg3("/layer3");
    msg1.add(0);
    Udp.beginPacket(outIp, outPort);
    msg1.send(Udp);
    Udp.endPacket();
    msg1.empty();
    delay(500);
  } else if (layerSS1 = 0) {
    tsunami.stopAllTracks();
  } 
  */
  else {
    //Serial.println("No pasa nada");
  }
}

void receivedMessage() {
  OSCMessage msg4;
  int size = Udp.parsePacket();

  if (size > 0) {
    while (size--) {
      msg4.fill(Udp.read());
    }
    if (!msg4.hasError()) {
      msg4.dispatch("/layer1", layerS1);
    } else {
      error = msg4.getError();
      Serial.print("error: ");
      Serial.println(error);
    }
  }
}
/*
  void sendMessage() {
  OSCMessage msg("/test");
  msg.add("hello, osc.");
  Udp.beginPacket(outIp, outPort);
  msg.send(Udp);
  Udp.endPacket();
  msg.empty();
  delay(500);
  }
*/