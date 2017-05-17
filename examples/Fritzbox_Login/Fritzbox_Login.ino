#include <SPI.h>
#include <Ethernet.h>
//https://github.com/tzikis/ArduinoMD5/
#include <MD5.h>

/////////////////////////////////////////////
//Konfigurieren                            //
/////////////////////////////////////////////
String chid = "";
// Enter a MAC address for your controller below.
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
//ÄNDERNÄNDERNÄNDERNÄNDERNÄNDERNÄNDERNÄNDERNÄNDERNÄNDERNÄNDERNÄNDERN
char server[] = "192.168.2.120";    // name address for Google (using DNS)
char fritzbox[] = "Host: 192.168.2.120";
char* meinpasswort = "-Passwort"; // '-' muss davor
String benutzername = "";
// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192, 168, 2, 121);
//ÄNDERNÄNDERNÄNDERNÄNDERNÄNDERNÄNDERNÄNDERNÄNDERNÄNDERNÄNDERNÄNDERN
EthernetClient client;
byte mynewbytes[100];
void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  }
  // give the Ethernet shield a second to initialize:
  delay(1000);
  Serial.println("connecting...");
  /////////////////////////////////////////////
  //ChallengeID aufrufen                     //
  /////////////////////////////////////////////
  // if you get a connection, report back via serial:
  if (client.connect(server, 80)) {
    Serial.println("connected");
    // Make a HTTP request:
    client.println("GET /login_sid.lua HTTP/1.1");
    client.println(fritzbox);
    client.println("Connection: close");
    client.println();
  } else {
    // if you didn't get a connection to the server:
    Serial.println("connection failed");
  }
  /////////////////////////////////////////////
  //ChallengeID extrahieren                  //
  /////////////////////////////////////////////

  while (!client.available()) {} //warten bis LAN-Chip Daten sendet
  if (client.find("<Challenge>")) { //verwerfe alle Zeichen im Stream bis "<Challenge>"
    char c;
    //Lese alle Zeichen bis </Challenge in chid ein
    while (1) {
      c = client.read();
      if (c == '<')break; //Verlasse While bei <
      chid += c;
    }
    Serial.println(chid); //
  } else Serial.println("Nichts gefunden");
  //Restliche Zeichen der Seite einlesen, damit Buffer für neue Seite geleert wird
  while (client.available()) char dummy = client.read();
  client.stop(); //diesen Aufruf schliessen
  /////////////////////////////////////////////
  //MD5 berechnen und String zusammenbauen   //
  //CHID-(MD5 von chid-passwort)             //
  /////////////////////////////////////////////
  String temp = chid + meinpasswort;
  char meinMD5String[temp.length() + 1] = {} ;
  temp.toCharArray(meinMD5String, temp.length() + 1);
  //Nach 16Bit Konvertieren
  int i = 0;
  int x = 0;
  while ( x < strlen(meinMD5String))
  {
    mynewbytes[i] = meinMD5String[x];
    i++;
    mynewbytes[i] = 0x00;
    i++;
    x++;
  }
  //MD5 Verarbeitung mit chid+passwort MD5
  //https://github.com/tzikis/ArduinoMD5/blob/master/examples/MD5_Hash/MD5_Hash.ino
  //https://github.com/tzikis/ArduinoMD5/
  unsigned char* hash = MD5::make_hash(mynewbytes, (strlen(meinMD5String)) * 2);
  char *md5str = MD5::make_digest(hash, 16);
  free(hash);
  Serial.println(md5str);

  String tempz = md5str;
  tempz.toUpperCase();
  String meineSid = "";
  // String body = "login:command/response=" + chid + "-" + tempz + "&getpage=../html/login_sid.xml";
  String test = "GET /login_sid.lua?username=" + benutzername + "&response=" + chid + "-" + tempz  +  " HTTP/1.1";
  Serial.println(test);
  /////////////////////////////////////////////
  //Loginpasswort übergeben und sid auslesen //
  /////////////////////////////////////////////
  if (client.connect(server, 80)) {
    Serial.println("connected");
    // Make a HTTP request:
    client.println(test);
    client.println(fritzbox);
    client.println("Connection: close");
    client.println();
  } else {
    // if you didn't get a connection to the server:
    Serial.println("connection failed");
  }

  while (!client.available()) {} //warten bis LAN-Chip Daten sendet
  if (client.find("<SID>")) { //verwerfe alle Zeichen im Stream bis "<SID>"
    char d;
    //Lese alle Zeichen bis </SID in meineSid ein
    while (1) {
      d = client.read();
      if (d == '<')break; //Verlasse While bei <
      meineSid += d;
    }
    Serial.println("meineSid:"); //
    Serial.println(meineSid); //
  } else Serial.println("Nichts gefunden");
    while (client.available()) char dummy = client.read();
      client.stop(); //diesen Aufruf schliessen
    }
void loop() {
}
