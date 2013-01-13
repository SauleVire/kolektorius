// Tutorial:
// http://www.hacktronics.com/Tutorials/arduino-1-wire-tutorial.html
//define input pin for 1-wire bus
#define ONE_WIRE_BUS 2 //ds18b20 davikliu pajungimo isvadas 
// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);
// Assign the addresses of your 1-Wire temp sensors.
// See the tutorial on how to obtain these addresses:
// http://www.hacktronics.com/Tutorials/arduino-1-wire-address-finder.html


//- kolektoriaus davikliu geri adresai
DeviceAddress KolektoriausDaviklioAdresas = {0x28, 0x1F, 0x34, 0xDD, 0x03, 0x00, 0x00, 0x82};
DeviceAddress KolektoriausOroDaviklioAdresas =  {0x28, 0xD7, 0x3D, 0xDD, 0x03, 0x00, 0x00, 0x4F};
DeviceAddress BoilerioDaviklioAdresas = {0x28, 0xAA, 0x48, 0x97, 0x03, 0x00, 0x00, 0xD9};
DeviceAddress BoilerioPadavimoVamzdzioDaviklioAdresas = {0x28, 0xA7, 0x2A, 0xDD, 0x03, 0x00, 0x00, 0xFA};
//{0x28, 0xD3, 0x72, 0x97, 0x03, 0x00, 0x00, 0xEE};//daviklis testams

DeviceAddress putplastis_5cmAdresas = {0x28, 0x37, 0x68, 0x97, 0x03, 0x00, 0x00, 0xDA};
DeviceAddress putplastis_10cmAdresas =  {0x28, 0xE2, 0x1F, 0xDD, 0x03, 0x00, 0x00, 0x32};
DeviceAddress neoporas_5cmAdresas = {0x28, 0xF2, 0x5B, 0xDD, 0x03, 0x00, 0x00, 0x9F};
DeviceAddress neoporas_10cmAdresas = {0x28, 0x25, 0x39, 0xDD, 0x03, 0x00, 0x00, 0x4C};
DeviceAddress lauko_temperaturaAdresas = {0x28, 0x3C, 0x5A, 0x97, 0x03, 0x00, 0x00, 0x25};
DeviceAddress kambario_temperaturaAdresas = {0x28, 0x36, 0x1D, 0xDD, 0x03, 0x00, 0x00, 0xA9}; 
//temperaturu kintamieji
   //assign value to some variables
// paprograme davikliu adresams spausdinti
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}
//paprograme temperaturoms spausdinti
void printTemperature(DeviceAddress deviceAddress)
{
  float tempC = sensors.getTempC(deviceAddress);
  if (tempC == -127.00) {
    Serial.print("Temperaturos matavimo klaida");
  } else {
    Serial.print("C: ");
    Serial.print(tempC);
//    Serial.print(" F: ");
//    Serial.print(DallasTemperature::toFahrenheit(tempC));
  }
}

