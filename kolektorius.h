#ifndef kolektorius_h
#define kolektorius_h


static const int Siurblys = 39; //siurblio reles valdymo isvadas
//-------------------------------------------------
int n = 0; //skaitiklis kolektoriaus vidurkiu ciklui suskaiciuoti
int k = 0; //skaitiklis polistirolo vidurkiu ciklui suskaiciuoti
//kintamieji vidurkiams, kurie siunciami i DuomenuBaze, isskaiciuoti
float KolektoriusVid, KolektoriausOrasVid, BoilerisVid, BoilerioPadavimoVamzdisVid=0;
float Putplascio_5cmTVid, Putplascio_10cmTVid, Neoporo_5cmTVid, Neoporo_10cmTVid, Lauko_TVid, Kambario_TVid = 0;
//-------------------------------------------------
static const int skirtumasON =5; //temp. skirtumas siurbliui INjungti
static const int skirtumasOFF =2; //temp. skirtumas siurbliui ISjungti
static const int vid_Kolektorius =20; //vidurkis kolektoriaus duomenu siuntimui
static const int vid_Polistirolas =20; //vidurkis polistirolo duomenu siuntimui
//data logging settings
#define DATA_REQUEST_RATE 30000 // data logging frequency
#define DATA_REQUEST_RATE_polistirolas 30000 //30 sekundziu, laikas tarpams tarp matavimu 
#define EXOSITE_REQUEST_RATE 30000
#define Temp_REQUEST_RATE 30000

//#define NTP_REQUEST_RATE 30000 // 30 sekundziu
//NTP settings
#define NTP_REQUEST_RATE 30000 // time server request frequency // 30 sekundziu
#define	SECS_DAY 86400UL  //(24L * 60L * 60L)
unsigned int localPort = 8888; // local port to listen for UDP packets
const int NTP_PACKET_SIZE= 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets 
IPAddress timeServer(64, 90, 182, 55); // time.nist.gov NTP server

//webserver settings
#define PREFIX ""

//EEPROM settings
#define CONFIG_EEPROM_ADDR ((byte*) 0x10)// configuration, as stored in EEPROM

#endif
