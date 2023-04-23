#include <NTPClient.h>

#include <SPI.h>
#include <MFRC522.h>
#include <Streaming.h>
#include <TimeLib.h>
#include <ESP8266WiFi.h>
// #include <ESP8266HTTPClient.h>
#include <WiFiUdp.h>
#include <string.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <MySQL_Generic.h>


#define MAX_ROW 100
#define MAX_COL 4
#define PORT 3306


WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);


#define WIFI_SSID "Blerta"         // WIFI SSID here
#define WIFI_PASSWORD "8888888888"  // WIFI password here

#define SS_PIN D8
#define RST_PIN D0

#define LED_P1 D4 
#define LED_P2 D3

// Init array that will store new NUID


char server[] = "192.168.0.106";
uint16_t server_port = PORT;
char user[] = "arjanitfandaj";
char password[] = "arjanitfanda";

char database[] = "employeemanagement";
char table[] = "employeemgm";

MySQL_Connection conn((Client *)&client);
MySQL_Query *query_mem;

String sendval, sendval2, postData;
String formattedDate;
String dayStamp;
String timeStamp;





typedef struct
{

  byte Data_C[2][4] = {
    { 138, 02, 110, 190 },  // AF enum 00
    { 12, 32, 203, 51 }     // FA enum 01

  };

  char names_[2][30]{
    "Arjanit Fandaj",
    "Armelind Fandaj"
  };


  byte last_data[MAX_ROW][MAX_COL];
  uint8_t status_data;  //0000 0000
                        //status bits 1111 0000
                        //r
  char data[2][30]
  {
    "8A 02 6E BE",
    "0C 20 CB 33",
  };

  char data_in[30];

} data;



void data_check(const byte *buffer, const size_t buffer_size, data *obj);
void main_t(byte *buffer, size_t buffer_size, data *obj);
// void check_ID(data *obj);
uint8_t check_ID(data *obj, char *name);
// void check_Mysql();
void Send_data(char *RFID, char *Date, char *Time, char *Name);
// void Send_data(uint8_t *RFID, String Date, String Time, char *Name);
void printHex(const byte *buffer, const byte bufferSize);
