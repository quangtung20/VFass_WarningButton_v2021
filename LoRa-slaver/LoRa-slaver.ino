/*
  LoRa Simple Gateway/Node Exemple

  This code uses InvertIQ function to create a simple Gateway/Node logic.

  Gateway - Sends messages with enableInvertIQ()
          - Receives messages with disableInvertIQ()

  Node    - Sends messages with disableInvertIQ()
          - Receives messages with enableInvertIQ()

  With this arrangement a Gateway never receive messages from another Gateway
  and a Node never receive message from another Node.
  Only Gateway to Node and vice versa.

  This code receives messages and sends a message every second.

  InvertIQ function basically invert the LoRa I and Q signals.

  See the Semtech datasheet, http://www.semtech.com/images/datasheet/sx1276.pdf
  for more on InvertIQ register 0x33.

  created 05 August 2018
  by Luiz H. Cassettari
*/

#include <SPI.h>              // include libraries
#include <LoRa.h>
#include <SoftwareSerial.h>


SoftwareSerial softSerial(3, 4);

#define DEBUG softSerial
//#define DEBUG Serial
const long frequency = 9214E5;  // LoRa Frequency

const int csPin = 10;          // LoRa radio chip select
const int resetPin = 8;        // LoRa radio reset
const int irqPin = 2;          // change for your board; must be a hardware interrupt pin
int spreadingFactor = 12; // from 7 to 12, default is 12
long signalBandwidth = 125E3; // 7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3,41.7E3,62.5E3,125E3,250E3,500e3, default is 125E3

String masterData = "";
String response_data = "";
enum cmd_type {STOP_WARNING, START_WARNING, UNIDENTIFY_CMD};
#define WARNING_START_CMD "WNLV5"
#define WARNING_START_RESPONSE "WNLV5_OK"
#define WARNING_STOP_CMD "WSTOP"
#define WARNING_STOP_RESPONSE "WSTOP_OK"

uint8_t sendCommand(const char* ATcommand, const char* expected_answer, unsigned int timeout);
bool receive_lora_response (String expected_answer, unsigned int timeout);
String config_data_before_send_loraCMD(String config_data );
int masterDataConfig();
bool checkCMD(String rxBuffer, String desiredStr );
bool checkResponse(String rxBuffer, String desiredStr );
void LoRa_rxMode();
void LoRa_txMode();
void LoRa_ReceivedResponseMode();
void LoRa_sendMessage(String message);
void onReceive();
void onTxDone();
boolean runEvery(unsigned long interval);


void(* resetFunc) (void) = 0;//cài đặt hàm reset

uint8_t sendCommand(const char* ATcommand, const char* expected_answer, unsigned int timeout) {
  uint8_t x = 0,  answer = 0;
  char response[50];
  unsigned long previous;
  memset(response, 0 , 50);    // Initialize the string
  DEBUG.print("\nSending command to Master:  ") ;
  DEBUG.println(ATcommand) ;

  while ( Serial.available() > 0) Serial.read();   // Clean the input buffer
  Serial.println(ATcommand);    // Send the AT command
  x = 0;
  previous = millis();
  // this loop waits for the answer
  do {
    if (Serial.available() != 0) {
      // if there are data in the UART input buffer, reads it and checks for the asnwer
      response[x] = Serial.read();
      //            Serial.print(response[x]);
      x++;
      // check if the desired answer  is in the response of the module
      if (strstr(response, expected_answer) != NULL) {
        answer = 1;
      }
    }
    // Waits for the asnwer with time out
  } while ((answer == 0) && ((millis() - previous) < timeout));

  return answer;
}

bool receive_lora_response (String expected_answer, unsigned int timeout) {
  bool answer = false;
  unsigned long previous;
  response_data = "";
  previous = millis();
  LoRa_ReceivedResponseMode();
  DEBUG.println("receiving ...");
  do {
    onReceive();
    if (checkResponse(response_data, expected_answer)) {
      answer = true;
    }
  } while ((answer == false) && ((millis() - previous) < timeout));
  return answer;
}

String config_data_before_send_loraCMD(String config_data ) {
  // VFASS001 00000001 12345
  String loraData = "";
  for ( int i = 0 ; i < config_data.length() - 3 ; i++ ) {
    loraData += config_data[ i + 3 ];
  }
  return loraData;
}



int masterDataConfig() {

  String rxBuffer = "" ;
  masterData = "";
  while (Serial.available()) {
    rxBuffer = Serial.readString();
  }

  DEBUG.println(rxBuffer);

  if ( checkCMD(rxBuffer, WARNING_START_CMD) ) {
    masterData = config_data_before_send_loraCMD(rxBuffer);
    return START_WARNING;
  }

  else if (checkCMD(rxBuffer, WARNING_STOP_CMD)) {
    masterData = config_data_before_send_loraCMD(rxBuffer);
    return STOP_WARNING;
  }

  else {
    return UNIDENTIFY_CMD;
  }

}

bool checkCMD(String rxBuffer, String desiredStr ) {

  bool check  = true ;
  for ( int i = 0; i < desiredStr.length(); i++) {
    if ( rxBuffer[i + 19] != desiredStr[i] ) {
      check = false;
      break;
    }
  }
  return check;

}

bool checkResponse(String rxBuffer, String desiredStr ) {

  bool check  = true ;
  for ( int i = 0; i < desiredStr.length(); i++) {
    if ( rxBuffer[i + 16] != desiredStr[i] ) {
      check = false;
      break;
    }
  }
  return check;

}



void LoRa_rxMode() {
  LoRa.enableInvertIQ();                // active invert I and Q signals
  LoRa.receive();                       // set receive mode
}

void LoRa_txMode() {
  LoRa.idle();                          // set standby mode
  LoRa.disableInvertIQ();               // normal mode
}

void LoRa_ReceivedResponseMode() {
  LoRa.disableInvertIQ();               // normal mode
  LoRa.receive();                       // set receive mode
}

void LoRa_sendMessage(String message) {
  LoRa_txMode();                        // set tx mode
  LoRa.beginPacket();                   // start packet
  LoRa.print(message);                  // add payload
  LoRa.endPacket(true);                 // finish packet and send it
  DEBUG.println("Send LoRa Message: " + message);
}

void onReceive() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String message = "";
    response_data = "";
    while (LoRa.available()) {
      message += (char)LoRa.read();
    }
    response_data = message;
    DEBUG.print("Gateway Receive: ");
    DEBUG.println(response_data);


  }
}


void onTxDone() {
  Serial.println("TxDone");
  LoRa_rxMode();
}

boolean runEvery(unsigned long interval)
{
  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    return true;
  }
  return false;
}


void setup() {
  Serial.begin(9600);                   // initialize serial
  while (!Serial);
  DEBUG.begin(9600);
  LoRa.setPins(csPin, resetPin, irqPin);

  if (!LoRa.begin(frequency)) {
    DEBUG.println("LoRa init failed. Check your connections.");
    while (true);                       // if failed, do nothing
  }
  LoRa.setSpreadingFactor(spreadingFactor);
  LoRa.setSignalBandwidth(signalBandwidth);

  DEBUG.println("LoRa init succeeded.");
  DEBUG.println();
  DEBUG.println("LoRa Simple Node");
  DEBUG.println("Only receive messages from gateways");
  DEBUG.println("Tx: invertIQ disable");
  DEBUG.println("Rx: invertIQ enable");
  DEBUG.println();
  //LoRa.setSyncWord(0xAA);
  //  LoRa.onReceive(onReceive);
  // LoRa.onTxDone(onTxDone);
  LoRa_txMode();
}

void loop() {
  if (Serial.available()) {
    int type_of_cmd = masterDataConfig();
    String uart_tx_data = "";
    String lora_command = "";

    if (type_of_cmd == START_WARNING) {
      lora_command = masterData;
      LoRa_sendMessage(lora_command);
      if (receive_lora_response(WARNING_START_RESPONSE, 15000)) {
        uart_tx_data = "SM+";
        uart_tx_data += response_data;
        Serial.println(uart_tx_data);
        LoRa_txMode();
      }
    } else if (type_of_cmd == STOP_WARNING) {
      lora_command = masterData;
      LoRa_sendMessage(lora_command);
      if (receive_lora_response(WARNING_STOP_RESPONSE, 15000)) {
        uart_tx_data = "SM+";
        uart_tx_data += response_data;
        Serial.println(uart_tx_data);
        LoRa_txMode();
      }
    } else {
      Serial.println("ERROR");
      LoRa_txMode();
    }
    masterData = "";
    response_data = "";
  }
}
