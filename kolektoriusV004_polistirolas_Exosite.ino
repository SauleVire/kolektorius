#include <SPI.h>
#include <Ethernet.h>
#include <WebServer.h>
#include <EthernetUdp.h>
#include <avr/eeprom.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "davikliai.h"
#include "kolektorius.h"
#include <Exosite.h>

template<class T>
inline Print &operator <<(Print &obj, T arg)
{ obj.print(arg); return obj; }

struct Config {
 // ethernet interface mac address - must be unique on your network
byte usedhcp;
byte mac[6] ;
byte ip[4];
byte mask[4];
byte gateway[4];
byte dns[4];
byte ntpServer[4];
char domain[22];
char publicKey[21];
char secretKey[21];
int  timeZone;
byte valid; // keep this as last byte
} config
= {
  // The default values
  1,
  { 0x66,0x04,0xA3,0x2C,0x12,0x35 },
  {192,168,1,135},
  {255,255,255,0},
  {192,168,1,254},
  {192,168,1,254},
  {193,219,52,41},
  "data.domain.prefix",
  "NX564654-DASDADA",
  "NX564654-DASDADA",
  2,
  253
};

// <-- Fill in your MAC here! (You can find it on the bottom of Ethernet Shield)
byte macData[] = { 0x66,0x04,0xA3,0x2C,0x12,0x36}; 
// <-- Fill in your CIK here! (https://portals.exosite.com -> Add Device)
String cikData = "exosite kodas";      
Exosite exosite(&Ethernet, macData, cikData);

// initialize the library instance:
EthernetClient client;
//data logging timer, ntp timer, exosite
static long Temp_laikmatis, data_timer, ntp_timer, data_timer_polistirolas, exosite_timer_kolektorius, exosite_timer_polistirolas; 
int tm_hour, dayclock;

EthernetUDP Udp;
//webo serveris arduine, index.html formavimas
WebServer webserver(PREFIX, 80);

void indexCmd(WebServer &server, WebServer::ConnectionType type, char *, bool)
{
  /* this line sends the standard "we're all OK" headers back to the browser */
  server.httpSuccess();
  /* if we're handling a GET or POST, we can output our data here.
     For a HEAD request, we just stop after outputting headers. */
  if (type != WebServer::HEAD)
  {
    
    /* this defines some HTML text in read-only memory aka PROGMEM.
     * This is needed to avoid having the string copied to our limited
     * amount of RAM. */
    P(header) = 
    
    "<a href=\"/index.html\">Pagrindinis</a> | "
    "<a href=\"/net.html\">Tinklo nustatymai</a> | "
    "<a href=\"/time.html\">Laiko nustatymai</a> | "
    "<a href=\"/other.html\">Kiti nustatymai</a>"
    "<br><br><br>" 
    ;
    server.printP(header);
    
    server << "Kolektorius: " << sensors.getTempC(KolektoriausDaviklioAdresas);
    server << "<br>Kolektoriaus oras: " << sensors.getTempC(KolektoriausOroDaviklioAdresas);
    server << "<br>Boileris: " << sensors.getTempC(BoilerioDaviklioAdresas);
    server << "<br>Boilerio padavimo vamzdis: " << sensors.getTempC(BoilerioPadavimoVamzdzioDaviklioAdresas);
    server << "<br>_____________<br>Laisva valdiklio atmintis: " << (freeRam());
    server << "<br>_____________<br>Putplastis 5cm: " << sensors.getTempC(putplastis_5cmAdresas);
    server << "<br>Putplastis 10cm: " << sensors.getTempC(putplastis_10cmAdresas);
    server << "<br>Neoporas 5cm: " << sensors.getTempC(neoporas_5cmAdresas);
    server << "<br>Neoporas 10cm: " << sensors.getTempC(neoporas_10cmAdresas);
    server << "<br>Lauko temperatura: " << sensors.getTempC(lauko_temperaturaAdresas);
    server << "<br>Kambario temperatura: " << sensors.getTempC(kambario_temperaturaAdresas);
  }
}
//slaptazodis arduino webo serveryje, kitu webo puslapiu formavimas
void netCmd(WebServer &server, WebServer::ConnectionType type, char *, bool)
{
  /* if the user has requested this page using the following credentials
   * username = admin
   * password = admin
   * display a page saying "Hello Admin"
   *
   * in other words: "YWRtaW46YWRtaW4=" is the Base64 representation of "admin:admin" */
  if (server.checkCredentials("YWRtaW46YWRtaW4="))
  {
    
      if (type == WebServer::POST)
      {
        bool repeat;
        char name[16], value[16];
        do
        {
          /* readPOSTparam returns false when there are no more parameters
           * to read from the input.  We pass in buffers for it to store
           * the name and value strings along with the length of those
           * buffers. */
          repeat = server.readPOSTparam(name, 16, value, 16);
    
          /* this is a standard string comparison function.  It returns 0
           * when there's an exact match.  We're looking for a parameter
           * named "buzz" here. */
          if (strcmp(name, "buzz") == 0)
          {
            /* use the STRing TO Unsigned Long function to turn the string
             	 * version of the delay number into our integer buzzDelay
             	 * variable */
            //buzzDelay = strtoul(value, NULL, 10);
          }
        } 
        while (repeat);
    
        // after procesing the POST data, tell the web browser to reload
        // the page using a GET method. 
        server.httpSeeOther(PREFIX);
        return;
      }
    
    server.httpSuccess();
    if (type != WebServer::HEAD)
    {
      P(header) = 
      "<a href=\"/index.html\">Pagrindinis</a> | "
      "<a href=\"/net.html\">Tinklo nustatymai</a> | "
      "<a href=\"/time.html\">Laiko nustatymai</a> | "
      "<a href=\"/other.html\">Kiti nustatymai</a>"
      "<br><br><br>" 
      ;
      server.printP(header);
      server << "<form method=\"POST\">";
      server << "IP: <input type=\"text\" name=\"ip\" value=\"";
      for (byte i = 0; i < sizeof config.ip; ++i){
        server << config.ip[i];
        if(i < 3)
          server << ".";
      }
      server << "\" /><br />";
      server << "DNS: <input type=\"text\" name=\"dns\" value=\"";
      for (byte i = 0; i < sizeof config.ip; ++i){
        server << config.dns[i];
        if(i < 3)
          server << ".";
      }
      server << "\" /><br />";
      server << "Gateway: <input type=\"text\" name=\"gtw\" value=\"";
      for (byte i = 0; i < sizeof config.ip; ++i){
        server << config.gateway[i];
        if(i < 3)
          server << ".";
      }
      server << "\" /><br />";
      server << "Subnet mask: <input type=\"text\" name=\"mask\" value=\"";
      for (byte i = 0; i < sizeof config.mask; ++i){
        server << config.ip[i];
        if(i < 3)
          server << ".";
      }
      server << "\" /><br />";
      server << "<input type=\"hidden\" value=\"go\" />";
      server << "<input type=\"submit\" value=\"Submit\" />";
      server << "</form>";
    }
  }
  else
  {
    /* send a 401 error back causing the web browser to prompt the user for credentials */
    server.httpUnauthorized();
  }
}
// programos inicializavimas
void setup() {
  // start serial port:
    exosite.init();
  Serial.begin(9600);
  sensors.begin();
  // set the resolution to 12 bit (good enough?)
  sensors.setResolution(KolektoriausDaviklioAdresas, 12);
  sensors.setResolution(KolektoriausOroDaviklioAdresas, 12);
  sensors.setResolution(BoilerioDaviklioAdresas, 12);
  sensors.setResolution(BoilerioPadavimoVamzdzioDaviklioAdresas, 12);
  
  sensors.setResolution(putplastis_5cmAdresas, 12);
  sensors.setResolution(putplastis_10cmAdresas, 12);
  sensors.setResolution(neoporas_5cmAdresas, 12);
  sensors.setResolution(neoporas_10cmAdresas, 12);
  sensors.setResolution(lauko_temperaturaAdresas, 12);
  sensors.setResolution(kambario_temperaturaAdresas, 12);  
  
  // start the Ethernet connection:
  Ethernet.begin(config.mac, config.ip, config.dns, config.gateway, config.mask);

  // print your local IP address:
  Serial.print("My IP address: ");
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    if(thisByte < 3)
      Serial.print("."); 
  }
  Serial.println();
  
  
  //web server setup
  webserver.setDefaultCommand(&indexCmd);
  webserver.addCommand("index.html", &indexCmd);
  webserver.addCommand("net.html", &netCmd);
  /* start the webserver */
  webserver.begin();
  
  data_timer = millis();
  
  //Start UDP lib
  Udp.begin(localPort);
  
  //NTP setup
  ntp_timer = millis();
  sendNTPpacket(timeServer); // send an NTP packet to a time server
  
  //begin sensors lib
  /*
Serial.print("Kolektorius: "); printTemperature(KolektoriausDaviklioAdresas);Serial.print("\n\r");
  Serial.print("Kolektoriaus Oras: "); printTemperature(KolektoriausOroDaviklioAdresas);Serial.print("\n\r");
Serial.print("Boileris: "); printTemperature(BoilerioDaviklioAdresas);Serial.print("\n\r");
  Serial.print("Boilerio Padavimo Vamzdis: "); printTemperature(BoilerioPadavimoVamzdzioDaviklioAdresas);Serial.print("\n\r");
Serial.print("\n\r");
Serial.print("Polistirolas 5cm: "); printTemperature(putplastis_5cmAdresas);Serial.print("\n\r");
  Serial.print("Polistirolas 10cm: "); printTemperature(putplastis_10cmAdresas);Serial.print("\n\r");
Serial.print("Neoporas 5cm: "); printTemperature(neoporas_5cmAdresas);Serial.print("\n\r");
  Serial.print("Neoporas 10cm "); printTemperature(neoporas_10cmAdresas);Serial.print("\n\r");
Serial.print("Lauko T: "); printTemperature(lauko_temperaturaAdresas);Serial.print("\n\r"); 
*/
Serial.println("Setup finished.");
  
  Serial.print("Free RAM: "); Serial.println(freeRam());
  Serial.print("Reiksmiu skaicius KOLEKTORIAUS temperaturu vidurkiui: "); Serial.println(vid_Kolektorius);
  Serial.print("Reiksmiu skaicius PUTPLASCIO temperaturu vidurkiui: "); Serial.println(vid_Polistirolas);
  Serial.print("\n\r");
  pinMode(Siurblys, OUTPUT); digitalWrite(Siurblys, HIGH);
  
}
// pagrindinis programos ciklas,
//startas void loop()
void loop() {
  if (millis() > Temp_laikmatis + Temp_REQUEST_RATE) {
    Temp_laikmatis = millis();
  Serial.print("Requesting temperatures...");
  sensors.setWaitForConversion(true);
  unsigned long start = millis();   
  sensors.requestTemperatures();
  unsigned long stop = millis();
  Serial.print("Time used: ");  Serial.println(stop - start);
 
  Serial.println("DONE");
Serial.println("_____KOLEKTORIAUS_MATAVIMAI______");
float Kolektorius = sensors.getTempC(KolektoriausDaviklioAdresas);
Serial.print("Kolektorius: "); Serial.print(Kolektorius); 
//Serial.print(" Adresas- "),printAddress(KolektoriausDaviklioAdresas); 
Serial.println();
     float KolektoriausOras = sensors.getTempC(KolektoriausOroDaviklioAdresas);
     Serial.print("Kolektoriaus Oras: "); Serial.print(KolektoriausOras);
//     Serial.print(" Adresas- "),printAddress(KolektoriausOroDaviklioAdresas);
     Serial.println();
float Boileris = sensors.getTempC(BoilerioDaviklioAdresas);
Serial.print("Boileris: "); Serial.print(Boileris);
//Serial.print(" Adresas- "),printAddress(BoilerioDaviklioAdresas); 
Serial.println();
     float BoilerioPadavimoVamzdis = sensors.getTempC(BoilerioPadavimoVamzdzioDaviklioAdresas);
     Serial.print("Boilerio padavimo vamzdis: "); Serial.print(BoilerioPadavimoVamzdis);
//Serial.print(" Adresas- "),printAddress(BoilerioPadavimoVamzdzioDaviklioAdresas); 
Serial.println();
    Serial.println("______PUTPLASCIO_MATAVIMAI______________");
float Putplascio_5cmT = sensors.getTempC(putplastis_5cmAdresas);
Serial.print("Putplascio 5cm T: "); Serial.print(Putplascio_5cmT); 
//Serial.print(" Adresas- "),printAddress(putplastis_5cmAdresas); 
Serial.println();

        float Putplascio_10cmT = sensors.getTempC(putplastis_10cmAdresas);
	Serial.print("Putplascio 10cm T: "); Serial.print(Putplascio_10cmT); 
       // Serial.print(" Adresas- "),printAddress(putplastis_10cmAdresas); 
        Serial.println();

float Neoporo_5cmT = sensors.getTempC(neoporas_5cmAdresas);
Serial.print("Neoporo 5cm T: "); Serial.print(Neoporo_5cmT); 
//Serial.print(" Adresas- "),printAddress(neoporas_5cmAdresas); 
Serial.println();

        float Neoporo_10cmT = sensors.getTempC(neoporas_10cmAdresas);
	Serial.print("Neoporo 10cm T: "); Serial.print(Neoporo_10cmT); 
       // Serial.print(" Adresas- "),printAddress(neoporas_10cmAdresas); 
        Serial.println();

float Lauko_T = sensors.getTempC(lauko_temperaturaAdresas);
Serial.print("Lauko T: "); Serial.print(Lauko_T); 
//Serial.print(" Adresas- "),printAddress(lauko_temperaturaAdresas); 
Serial.println();

        float Kambario_T = sensors.getTempC(kambario_temperaturaAdresas);
	Serial.print("Kambario T: "); Serial.print(Kambario_T); 
       // Serial.print(" Adresas- "),printAddress(kambario_temperaturaAdresas); 
        Serial.println();
//Startas-kolektoriaus ir boilerio temperaturu skirtumas, siurblio ijungimas
if (Kolektorius-Boileris>=skirtumasON)
      {digitalWrite(Siurblys, LOW);
      Serial.println("Siurblio busena- ON");
      }
if (Kolektorius-Boileris<=skirtumasOFF) 
      {digitalWrite(Siurblys, HIGH);
      Serial.println("Siurblio busena- OFF");
} 
//Finisas-kolektoriaus ir boilerio temperaturu skirtumas, siurblio ijungimas


  
//startas- temperaturos matavimas kolektoriuje
  if (millis() > data_timer + DATA_REQUEST_RATE) {
    data_timer = millis();
    char duomenys[60];
    

//Startas - Duomenu siuntimas
if (n>vid_Kolektorius) { 
            KolektoriusVid=KolektoriusVid / n;
            KolektoriausOrasVid=KolektoriausOrasVid / n;
            BoilerisVid=BoilerisVid / n;
            BoilerioPadavimoVamzdisVid=BoilerioPadavimoVamzdisVid / n;

     char temptemp1[10];
     char temptemp2[10];
     char temptemp3[10];
     char temptemp4[10];
     dtostrf(KolektoriusVid, 2, 2, temptemp1);
     dtostrf(KolektoriausOrasVid, 2, 2, temptemp2);
     dtostrf(BoilerisVid, 2, 2, temptemp3);
     dtostrf(BoilerioPadavimoVamzdisVid, 2, 2, temptemp4);
    sprintf(duomenys, "kolektorius.php?p1=%s&p2=%s&p3=%s&p4=%s", temptemp1, temptemp2, temptemp3, temptemp4);
    if (client.connect("www.domenas.lt", 80)) {
      Serial.println("Connected!");
      client.print("GET http://www.domenas.lt/");
      client.println(duomenys);
      client.println("HTTP/1.1");
      client.println("Host: www.domenas.lt");
      client.println();
      client.stop();
      client.flush();
    } else {
      Serial.println("Failed to send data to remote server.");
      client.stop();
      client.flush();
    }

  KolektoriusVid=0.0;
  Serial.print("Po duomenu issiuntimo KolektoriusVid: "); Serial.println(KolektoriusVid);
  KolektoriausOrasVid=0.0;
  Serial.print("Po duomenu issiuntimo KolektoriausOrasVid: "); Serial.println(KolektoriausOrasVid);
  BoilerisVid=0.0;
  Serial.print("Po duomenu issiuntimo BoilerisVid: "); Serial.println(BoilerisVid);
  BoilerioPadavimoVamzdisVid=0.0;
  Serial.print("Po duomenu issiuntimo BoilerioPadavimoVamzdisVid: "); Serial.println(BoilerioPadavimoVamzdisVid);
  n=0;
Serial.print("Po duomenu issiuntimo n: "); Serial.println(n);  
  } else {Serial.print(n); Serial.println(" --------------- dar ne laikas siusti kolektoriaus duomenis");}
  

// PRADZIA Duomenu siuntimas i EXOSITE 
if(millis() > exosite_timer_kolektorius + EXOSITE_REQUEST_RATE){
    exosite_timer_kolektorius = millis();
exosite.sendToCloud("Kolektorius", Kolektorius);

exosite.sendToCloud("KolektoriausOras", KolektoriausOras);

exosite.sendToCloud("Boileris", Boileris);

exosite.sendToCloud("BoilerioPadavimoVamzdis", BoilerioPadavimoVamzdis);

  }// Duomenu siuntimas i EXOSITE PABAIGA


  KolektoriusVid=KolektoriusVid + Kolektorius;
  Serial.print("kol neissiusta KolektoriusVid: "); Serial.println(KolektoriusVid);
  KolektoriausOrasVid=KolektoriausOrasVid + KolektoriausOras;
  Serial.print("kol neissiusta KolektoriausOrasVid: "); Serial.println(KolektoriausOrasVid);
  BoilerisVid=BoilerisVid + Boileris;
  Serial.print("kol neissiusta BoilerisVid: "); Serial.println(BoilerisVid);
  BoilerioPadavimoVamzdisVid=BoilerioPadavimoVamzdisVid + BoilerioPadavimoVamzdis;
  Serial.print("kol neissiusta BoilerioPadavimoVamzdisVid: "); Serial.println(BoilerioPadavimoVamzdisVid);
  n++;
  Serial.print("kol neissiusta n: "); Serial.println(n);
}  
//Finisas- Duomenu siuntimas
//Finisas- Temperaturos matavimas kolektoriuje
//--------------------------------------------




//startas- temperaturos matavimas polistirolo
  if (millis() > data_timer_polistirolas + DATA_REQUEST_RATE_polistirolas) {
    data_timer_polistirolas = millis();
    char duomenys1[60];

if (k>vid_Polistirolas) { 
          Putplascio_5cmTVid = Putplascio_5cmTVid  / k;
         Putplascio_10cmTVid = Putplascio_10cmTVid / k;
             Neoporo_5cmTVid = Neoporo_5cmTVid  / k;
            Neoporo_10cmTVid = Neoporo_10cmTVid / k;
                  Lauko_TVid = Lauko_TVid / k;
               Kambario_TVid = Kambario_TVid /k;
     char temptemp5[10];
     char temptemp6[10];
     char temptemp7[10];
     char temptemp8[10];
     char temptemp9[10];
     char temptemp10[10];
     dtostrf(Putplascio_5cmT, 2, 2, temptemp5);
     dtostrf(Putplascio_10cmT, 2, 2, temptemp6);
     dtostrf(Neoporo_5cmT, 2, 2, temptemp7);
     dtostrf(Neoporo_10cmT, 2, 2, temptemp8);
     dtostrf(Lauko_T, 2, 2, temptemp9);
     dtostrf(Kambario_T, 2, 2, temptemp10);
    sprintf(duomenys1, "polistirolas.php?p5=%s&p6=%s&p7=%s&p8=%s&p9=%s&p10=%s", temptemp5, temptemp6, temptemp7, temptemp8, temptemp9, temptemp10);
    if (client.connect("www.domenas.lt", 80)) {
      Serial.println("Connected!");
      client.print("GET http://www.domenas.lt/");
      client.println(duomenys1);
      client.println("HTTP/1.1");
      client.println("Host: www.domenas.lt");
      client.println();
      client.stop();
      client.flush();
    } else {
      Serial.println("Failed to send data to remote server.");
      client.stop();
      client.flush();
    }
  Putplascio_5cmTVid=0;
  Serial.print("Po duomenu issiuntimo Putplascio_5cmTVid: "); Serial.println(Putplascio_5cmTVid);
  Putplascio_10cmTVid=0;
  Serial.print("Po duomenu issiuntimo Putplascio_10cmTVid: "); Serial.println(Putplascio_10cmTVid);
  Neoporo_5cmTVid=0;
  Serial.print("Po duomenu issiuntimo Neoporo_5cmTVid: "); Serial.println(Neoporo_5cmTVid);
  Neoporo_10cmTVid=0;
  Serial.print("Po duomenu issiuntimo Neoporo_10cmTVid: "); Serial.println(Neoporo_10cmTVid);
  Lauko_TVid=0;
  Serial.print("Po duomenu issiuntimo Lauko_TVid: "); Serial.println(Lauko_TVid);
  Kambario_TVid=0;
  Serial.print("Po duomenu issiuntimo Kambario_TVid: "); Serial.println(Kambario_TVid);
  k=0;
Serial.print("Po duomenu issiuntimo k: "); Serial.println(k);  
  } else {Serial.print(k); Serial.println(" --------------- dar ne laikas siusti putplascio duomenis");}
  
      Serial.println(); Serial.print("Requesting temperatures...");
  sensors.requestTemperatures();
  Serial.println("DONE");

// PRADZIA Duomenu siuntimas i EXOSITE 
if(millis() > exosite_timer_polistirolas + EXOSITE_REQUEST_RATE){
    exosite_timer_polistirolas = millis();
exosite.sendToCloud("Putplastis_5cm", Putplascio_5cmT);

exosite.sendToCloud("Putplastis_10cm", Putplascio_10cmT);

exosite.sendToCloud("Neoporas_5cm", Neoporo_5cmT);

exosite.sendToCloud("Neoporas_10cm", Neoporo_10cmT);

exosite.sendToCloud("Lauko_T", Lauko_T);

exosite.sendToCloud("Kambario_T", Kambario_T);


  }// Duomenu siuntimas i EXOSITE PABAIGA  
  
 Putplascio_5cmTVid = Putplascio_5cmTVid + Putplascio_5cmT;
 Serial.print("kol neissiusta Putplascio_5cmTVid: "); Serial.println(Putplascio_5cmTVid);
  Putplascio_10cmTVid = Putplascio_10cmTVid + Putplascio_10cmT;
  Serial.print("kol neissiusta Putplascio_10cmTVid: "); Serial.println(Putplascio_10cmTVid);
Neoporo_5cmTVid = Neoporo_5cmTVid + Neoporo_5cmT;
Serial.print("kol neissiusta Neoporo_5cmTVid: "); Serial.println(Neoporo_5cmTVid);
  Neoporo_10cmTVid = Neoporo_10cmTVid + Neoporo_10cmT;
  Serial.print("kol neissiusta Neoporo_10cmTVid: "); Serial.println(Neoporo_10cmTVid);
Lauko_TVid=Lauko_TVid+Lauko_T;
Serial.print("kol neissiusta Lauko_TVid: "); Serial.println(Lauko_TVid);
  Kambario_TVid = Kambario_TVid + Kambario_T;
  Serial.print("kol neissiusta Kambario_TVid: "); Serial.println(Kambario_TVid);

  k++;
  Serial.print("kol neissiusta k: "); Serial.println(k);
}  

//Finisas- Polistirolo Duomenu siuntimas
//Finisas- Temperaturos matavimas Polistirolo
//--------------------------------------------

  if(millis() > ntp_timer + NTP_REQUEST_RATE){
    ntp_timer = millis();
    sendNTPpacket(timeServer); // send an NTP packet to a time server
  }
/*  
  if ( Udp.parsePacket() ) {  
    // We've received a packet, read the data from it
    Udp.read(packetBuffer,NTP_PACKET_SIZE);  // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);  
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;  
    Serial.print("Seconds since Jan 1 1900 = " );
    Serial.println(secsSince1900);               

    // now convert NTP time into everyday time:
    Serial.print("Unix time = ");
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;     
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears;  
    // print Unix time:
    Serial.println(epoch);                               


    // print the hour, minute and second:
    Serial.print("The UTC time is ");       // UTC is the time at Greenwich Meridian (GMT)
    Serial.print((epoch  % 86400L) / 3600+3); // print the hour (86400 equals secs per day)
    Serial.print(':');  
    if ( ((epoch % 3600) / 60) < 10 ) {
      // In the first 10 minutes of each hour, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.print((epoch  % 3600) / 60); // print the minute (3600 equals secs per minute)
    Serial.print(':'); 
    if ( (epoch % 60) < 10 ) {
      // In the first 10 seconds of each minute, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.println(epoch %60); // print the second
  }
  */ 
  if ( Udp.parsePacket() ) {  
    // We've received a packet, read the data from it
    Udp.read(packetBuffer,NTP_PACKET_SIZE);  // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);  
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;  
    Serial.print("Unix time: ");
    Serial.println(secsSince1900-2208988800UL);
    unsigned long epoch = secsSince1900 - 2208988800UL;
    gmtime(secsSince1900);
    Serial.print("Hour ATM: ");
    Serial.print(tm_hour);Serial.print(':');  
    if ( ((epoch % 3600) / 60) < 10 ) {
      // In the first 10 minutes of each hour, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.println((epoch  % 3600) / 60); // print the minute (3600 equals secs per minute)
        if ( (epoch % 60) < 10 ) {
      // In the first 10 seconds of each minute, we'll want a leading '0'
      Serial.println('0');
    }
  }
 
  webserver.processConnection();
  
}
}
// finisas void loop()
// send an NTP request to the time server at the given address 
unsigned long sendNTPpacket(IPAddress& address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE); 
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49; 
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp: 		   
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer,NTP_PACKET_SIZE);
  Udp.endPacket(); 
}
 
void gmtime(uint32_t time)
{
  uint32_t dayclock;
  time -= 2208988800UL; // 1900 to 1970
  time += 3600L * 2; //time offset
  time += 3600L; // summer offset
  
  dayclock = time % SECS_DAY;
  
  tm_hour = dayclock / 3600UL;
}

static int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

static void loadConfig() {
    for (byte i = 0; i < sizeof config; ++i)
        ((byte*) &config)[i] = eeprom_read_byte(CONFIG_EEPROM_ADDR + i);
    if (config.valid != 253) {
        config.valid = 253;
        config.mac[1] = 0x74; //{ 0x74,0x69,0x69,0x2D,0x30,0x31 };
        config.mac[2] = 0x69;
    }
}

static void saveConfig() {
    for (byte i = 0; i < sizeof config; ++i)
        eeprom_write_byte(CONFIG_EEPROM_ADDR + i, ((byte*) &config)[i]);
}
