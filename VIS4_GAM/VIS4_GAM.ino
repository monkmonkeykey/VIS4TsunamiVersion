
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

//Wifi
char ssid[] = "AXTEL XTREMO-4E67";  // your network SSID (name)
char pass[] = "03874E67";           // your network password

// A UDP instance to let us send and receive packets over UDP
WiFiUDP Udp;
const IPAddress outIp(192, 168, 15, 19);  // remote IP (not needed for receive)
const unsigned int outPort = 8001;        // remote port (not needed for receive)
const unsigned int localPort = 8000;      // local port to listen for UDP packets (here's where we send the packets)
IPAddress ip;

OSCErrorCode error;
// Variables

int layer1;
int layer2;
int layer3;
int momentaneo;
int contadorA = 0;
int contadorB = 0;
int contadorC = 0;

int statusPlaying1;
int statusPlaying2;
int statusPlaying3;

int layerSS1 = 0;
int layerSS2 = 0;
int layerSS3 = 0;

float estado1 = 0.0;
float estado2 = 0.0;
float estado3 = 0.0;

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

  delay(5000);
}


void layerS1(OSCMessage &msg4) {
  layerSS1 = msg4.getFloat(0);
  estado1 = layerSS1;
  display.clearDisplay();
  display.setCursor(0, 32);

  display.println(layerSS1);
}

void layerS2(OSCMessage &msg5) {
  layerSS2 = msg5.getFloat(0);
  estado2 = layerSS2;
  display.clearDisplay();
  display.setCursor(0, 32);

  display.println(layerSS2);
}

void layerS3(OSCMessage &msg6) {
  layerSS3 = msg6.getFloat(0);
  estado3 = layerSS3;
  display.clearDisplay();
  display.setCursor(0, 32);

  display.println(layerSS3);
}

void loop() {

  reproduccion();

  receivedMessage();

}
void reproduccion() {
  statusPlaying1 = tsunami.isTrackPlaying(1);
  statusPlaying2 = tsunami.isTrackPlaying(5);
  statusPlaying3 = tsunami.isTrackPlaying(9);

  if (estado1 == 1.) {
    contadorA++;
    if (contadorA == 1) {
      display.clearDisplay();
      display.setCursor(0, 32);
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
      display.println("Se reproduce la capa 1 mayor a 1");
      display.setCursor(0, 48);
      display.display();
      contadorA++;
    } else if (estado1 == 0. || statusPlaying1 == 0) {
      tsunami.stopAllTracks();
      digitalWrite(BUILTIN_LED, 0);
      OSCMessage msg("/layer1");
      msg.add(0.);
      Udp.beginPacket(outIp, outPort);
      msg.send(Udp);
      Udp.endPacket();
      msg.empty();
      display.clearDisplay();
      display.setCursor(0, 32);
      display.println("Se ha terminado la capa 1");
      display.display();
      contadorA = 0;
      estado1 = 0.;
    }
  }
  else if (estado1 == 0.) {
    tsunami.stopAllTracks();
    contadorA = 0;
  } else if (statusPlaying1 == 1) {
    display.clearDisplay();
    display.setCursor(0, 32);
    display.println("No pasa nada oiga");
    display.display();
    Serial.println("Esta reproduciendose la capa 1");
  }
  // CAPA 2
  else if (estado2 == 1.) {
    contadorB++;
    if (contadorB == 1) {
      display.clearDisplay();
      display.setCursor(0, 32);
      display.println("Se reproduce la capa 2");
      display.display();
      tsunami.trackLoad(1, 0, false);  // track 1 on output 0 (aka "1L"), Lock = false (voice stealing active)
      tsunami.trackLoad(2, 1, false);  // track 2 on output 1 (aka "1R"), Lock = false (voice stealing active)
      tsunami.trackLoad(3, 2, false);  // track 3 on output 2 (aka "2L"), Lock = false (voice stealing active)
      tsunami.trackLoad(4, 3, false);  // track 4 on output 3 (aka "2R"), Lock = false (voice stealing active)
      tsunami.resumeAllInSync();
      digitalWrite(BUILTIN_LED, 1);
      display.clearDisplay();
      display.setCursor(0, 32);
      display.println("Se reproduce la capa 2 mayor a 2");
      display.setCursor(0, 48);
      display.display();
      contadorB++;
    } else if (estado2 == 0. || statusPlaying2 == 0) {
      //receivedMessage();
      tsunami.stopAllTracks();
      digitalWrite(BUILTIN_LED, 0);
      //Serial.println("Reproduciendo capa 2");
      OSCMessage msg("/layer2");
      msg.add(0.);
      Udp.beginPacket(outIp, outPort);
      msg.send(Udp);
      Udp.endPacket();
      msg.empty();
      display.clearDisplay();
      display.setCursor(0, 32);

      display.println("Se ha terminado la capa 2");
      display.display();
      contadorB = 0;
      estado2 = 0.;
    }
  }
  else if (estado2 == 0.) {
    tsunami.stopAllTracks();
    contadorB = 0;
  }
  // CAPA 3
  else if (statusPlaying3 == 1) {
    display.clearDisplay();
    display.setCursor(0, 32);

    display.println("No pasa nada oiga");
    display.display();
    Serial.println("Esta reproduciendose la capa 3");
  }
  else if (estado3 == 1.) {
    contadorC++;
    //delay(100);
    if (contadorC == 1) {
      display.clearDisplay();
      display.setCursor(0, 32);

      display.println("Se reproduce la capa 3");
      display.display();
      tsunami.trackLoad(1, 0, false);  // track 1 on output 0 (aka "1L"), Lock = false (voice stealing active)
      tsunami.trackLoad(2, 1, false);  // track 2 on output 1 (aka "1R"), Lock = false (voice stealing active)
      tsunami.trackLoad(3, 2, false);  // track 3 on output 2 (aka "2L"), Lock = false (voice stealing active)
      tsunami.trackLoad(4, 3, false);  // track 4 on output 3 (aka "2R"), Lock = false (voice stealing active)
      tsunami.resumeAllInSync();
      digitalWrite(BUILTIN_LED, 1);
      display.clearDisplay();
      display.setCursor(0, 32);

      display.println("Se reproduce la capa 3 mayor a 3");
      display.setCursor(0, 48);

      display.display();
      contadorC++;
    } else if (estado3 == 0. || statusPlaying3 == 0) {
      tsunami.stopAllTracks();
      digitalWrite(BUILTIN_LED, 0);
      //Serial.println("Reproduciendo capa 3");
      OSCMessage msg("/layer3");
      msg.add(0.);
      Udp.beginPacket(outIp, outPort);
      msg.send(Udp);
      Udp.endPacket();
      msg.empty();
      display.clearDisplay();
      display.setCursor(0, 32);

      display.println("Se ha terminado la capa 3");
      display.display();
      contadorC = 0;
      estado3 = 0.;
      //delay(100);
    }
  }
  else if (estado3 == 0.) {
    tsunami.stopAllTracks();
    contadorC = 0;
  }
  else if (statusPlaying3 == 1) {
    display.clearDisplay();
    display.setCursor(0, 32);

    display.println("No pasa nada oiga");
    display.display();
    Serial.println("Esta reproduciendose la capa 2");
  }
  else {
    //Serial.println("No pasa nada");
  }
}
// REVISAR LA FUNCIÓN
void receivedMessage() {
  OSCMessage msg4;
  OSCMessage msg5;
  OSCMessage msg6;
  int size = Udp.parsePacket();

  if (size > 0) {
    while (size--) {
      msg4.fill(Udp.read());
      msg5.fill(Udp.read());
      msg6.fill(Udp.read());
    }
    if (!msg4.hasError()) {
      msg4.dispatch("/layer1", layerS1);
    }
    else if (!msg5.hasError()) {
      msg5.dispatch("/layer2", layerS2);
    }
    else if (!msg6.hasError()) {
      msg6.dispatch("/layer3", layerS3);
    }

    else {
      error = msg4.getError();
      error = msg5.getError();
      error = msg6.getError();
      Serial.print("error: ");
      Serial.println(error);
    }
  }
}
