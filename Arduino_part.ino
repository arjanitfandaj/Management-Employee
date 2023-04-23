

#include "header.h"
// #include <HTTPClient.h>



// #define HOST "example.com"  // Enter HOST URL without "http:// "  and "/" at the end of URL


data obj_t;
//pfp

//pfp

byte nuidPICC[4];
MFRC522 rfid(SS_PIN, RST_PIN);  // Instance of the class
MFRC522::MIFARE_Key key;

void setup() {
  memset(obj_t.last_data, 0, sizeof(obj_t.last_data));
  Serial.begin(115200);
  SPI.begin();      // Init SPI bus
  rfid.PCD_Init();  // Init MFRC522
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  pinMode(LED_P1,OUTPUT);
  pinMode(LED_P2,OUTPUT);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);  //try to connect with wifi
  Serial.print("Connecting to ");
  Serial.print(WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }


  // Test MySQL connection
  Serial.print("Connecting to SQL Server @ ");
  Serial.print(server);
  Serial.println(String(", Port = ") + server_port);
  Serial.println(String("User = ") + user + String(", PW = ") + password + String(", DB = ") + database);
  // check_Mysql();

  Serial.println();
  Serial.print("Connected to ");
  Serial.println(WIFI_SSID);
  Serial.print("IP Address is : ");
  Serial.println(WiFi.localIP());
  Serial.println("\n\n\n");
  timeClient.begin();
  timeClient.setTimeOffset(7200);
  // check_Mysql();
}
void loop() {

  while (!timeClient.update()) {
    timeClient.forceUpdate();
  }

  formattedDate = timeClient.getFormattedDate();

  int splitT = formattedDate.indexOf("T");
  dayStamp = formattedDate.substring(0, splitT);
  timeStamp = formattedDate.substring(splitT + 1, formattedDate.length() - 1);

  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("NO WIFI\n BREAKING NOW");
    Serial.println("");
    digitalWrite(LED_P2,HIGH); //1
    // delay(30);
    // digitalWrite(LED_P2,LOW); //1
    return;

  } else {
    digitalWrite(LED_P2,LOW);
    // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
    if (!rfid.PICC_IsNewCardPresent())
      return;
    // Verify if the NUID has been readed
    if (!rfid.PICC_ReadCardSerial())
      return;
    MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
    if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI && piccType != MFRC522::PICC_TYPE_MIFARE_1K && piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
      return;
    }


    main_t(rfid.uid.uidByte, rfid.uid.size, &obj_t);

    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
    obj_t.status_data &= ~(obj_t.status_data);
    delay(2000);
    // reset_buffer();
  }
}

void printHex(const byte *buffer, const byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(*(buffer + i) < 0x10 ? " 0" : " ");
    Serial.print(*(buffer + i), HEX);
  }
}






void main_t(byte *buffer, size_t buffer_size, data *obj) {

  obj->status_data |= (0x01 << 7);
  data_check(buffer, buffer_size, obj);
}




void Send_data(String Date, String Time, char *Name) {
  // String INSERT_SQL = String("INSERT INTO ") + database + "." + table + " (RFID,Date,Time,Name) VALUES ('" + RFID + "," + Date + "," + Time + "," + Name + "')";


 


  String INSERT_SQL = String("INSERT INTO ") + database + "." + table + " (Name,Time,Date) VALUES ('" + Name + "','" + Time + "','" + Date + "')";
  if (conn.connectNonBlocking(server, server_port, user, password) != RESULT_FAIL) {
    delay(500);
    // Initiate the query class instance
    MySQL_Query query_mem = MySQL_Query(&conn);

    if (conn.connected()) {
      // Serial.println(INSERT_SQL);
      Serial.println("Connected Successfully");

      // Execute the query
      // KH, check if valid before fetching
      if (!query_mem.execute(INSERT_SQL.c_str()))
      {
        Serial.println("Insert error");
        digitalWrite(LED_P1,HIGH); //1
        delay(200);
        digitalWrite(LED_P1,LOW); //0
        delay(200);
        digitalWrite(LED_P1,HIGH); //1
        delay(200);
        digitalWrite(LED_P1,LOW); // 0 Error Database
      }
      else
      {
        Serial.println("Data Inserted.");
        digitalWrite(LED_P1,HIGH);
        delay(1000);
        digitalWrite(LED_P1,LOW);
      }
    } else {
      Serial.println("Disconnected from Server. Can't insert.");
    }
    conn.close();  // close the connection
  } else {
    Serial.println("\nConnect failed. Trying again on next iteration.");
    digitalWrite(LED_P1,HIGH); //1
        delay(200);
        digitalWrite(LED_P1,LOW); //0
        delay(200);
        digitalWrite(LED_P1,HIGH); //1
        delay(200);
        digitalWrite(LED_P1,LOW); // 0 Error Database
    // conn.
  }
}



void data_check(const byte *buffer, size_t buffer_size, data *obj) {
  char msg[30];
  uint8_t count = 0;
  // char * name = (char *)malloc(30*sizeof(char));
  char name[30];
  // sprintf(msg,"Date: %s\nTime: %s",(dayStamp),(timeStamp));
  sprintf(msg, "%s; %s; ", (dayStamp), (timeStamp));
  if (((obj->status_data >> 7) & 0x01)) {

    for (uint8_t i = 0; i < 100; i++) {
      for (uint8_t j = 0; j < 4; j++) {
        if (obj->last_data[i][j] == 0) {
          if (count != 4) {
            obj->last_data[i][j] = *(buffer + j);
            count++;

          } else if (count == 4) {
            break;
          }
        }
      }
    }
  }


  uint8_t data_c[30];
  char data_s[30];



  for (uint8_t i = 0; i < buffer_size; i++) {
    data_c[i] = *(buffer + i);
  }

  strcpy(data_s, (char *)data_c);

  printHex(buffer,buffer_size);
  Serial.print("; ");
  Serial.print(msg);
  if(check_ID(obj, name) == 1)
  {
 Send_data(dayStamp, timeStamp, obj->data_in);
  }
  else{
    Serial.println("DATA INVALID"); 
    digitalWrite(LED_P2,HIGH);
    delay(1000);
    digitalWrite(LED_P2,LOW);
  }

 
  // Serial.println(obj->data_in);
  // memset(obj->data_in,"",sizeof(obj->data_in));
}





uint8_t check_ID(data *obj, char *name) {
  uint8_t ret = -1;
  uint8_t count = 0;
  uint8_t addr;
  uint8_t check = 0;
  char data[30];


  for (uint8_t i = 0; i < MAX_ROW; i++) {
    for (uint8_t j = 0; j < MAX_COL; j++) {
      if (obj->last_data[i][j] == 0) {
        check++;
      }
    }
    if (check == MAX_COL) {
      count++;
      check = 0;
    }
  }
  addr = (MAX_ROW - count) - 1;
  if (addr == 99) {
    memset(obj->last_data, 0, sizeof(obj->last_data));
    Serial.print("Data was resetted\n");
    addr = 0;
  }
  for (uint8_t i = 0; i < 2; i++) {
    for (uint8_t j = 0; j < 4; j++) {
      if ((obj->last_data[addr][j] == obj->Data_C[i][j]) && (obj->last_data[addr][j] == obj->Data_C[i][j]) && (obj->last_data[addr][j] == obj->Data_C[i][j]) && (obj->last_data[addr][j] == obj->Data_C[i][j])) {
        Serial.println(obj->names_[i]);
        // data = obj->names_[i];
        // obj->data_in = str(obj->names_[i]);
        strcpy(obj->data_in,obj->names_[i]);

        ret = 1;

        break;
      }
    }
  }
  // name = data;
  // Send_data(obj->)
  return ret;
}




void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}