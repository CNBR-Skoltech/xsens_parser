#define PACK_TIMEOUT_MS 5
#define UART_BUF_LEN 256

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsServer.h>
#include <Hash.h>



// MPU Inits
//#include <Adafruit_MPU6050.h>
//#include <Adafruit_Sensor.h>
//#include <Wire.h>
//
//int MPU_TOGGLE = 0;
//Adafruit_MPU6050 mpu;
//Adafruit_MPU6050 mpu2;
//
//int MPU_AVAILABLE = 0;
//int MPU2_AVAILABLE = 0;
//
//sensors_event_t a, g, temp;
//sensors_event_t a2, g2, temp2;

// End of MPU Inits


enum PACK_STATUS {
  PACK_IDLE = 0,
  PACK_DEVICE,
  PACK_REGISTER,
  PACK_LEN,
  PACK_DONE
};


const char * SIDE = "L";


uint8_t uart_pack[UART_BUF_LEN];
PACK_STATUS pack_status = PACK_IDLE;
uint64_t pack_start;
uint64_t prev_pack_start;
uint16_t adc_pack[10];

static const uint8_t crc_table[256] = {
    0, 94,188,226, 97, 63,221,131,194,156,126, 32,163,253, 31, 65,
  157,195, 33,127,252,162, 64, 30, 95,  1,227,189, 62, 96,130,220,
   35,125,159,193, 66, 28,254,160,225,191, 93,  3,128,222, 60, 98,
  190,224,  2, 92,223,129, 99, 61,124, 34,192,158, 29, 67,161,255,
   70, 24,250,164, 39,121,155,197,132,218, 56,102,229,187, 89,  7,
  219,133,103, 57,186,228,  6, 88, 25, 71,165,251,120, 38,196,154,
  101, 59,217,135,  4, 90,184,230,167,249, 27, 69,198,152,122, 36,
  248,166, 68, 26,153,199, 37,123, 58,100,134,216, 91,  5,231,185,
  140,210, 48,110,237,179, 81, 15, 78, 16,242,172, 47,113,147,205,
   17, 79,173,243,112, 46,204,146,211,141,111, 49,178,236, 14, 80,
  175,241, 19, 77,206,144,114, 44,109, 51,209,143, 12, 82,176,238,
   50,108,142,208, 83, 13,239,177,240,174, 76, 18,145,207, 45,115,
  202,148,118, 40,171,245, 23, 73,  8, 86,180,234,105, 55,213,139,
   87,  9,235,181, 54,104,138,212,149,203, 41,119,244,170, 72, 22,
  233,183, 85, 11,136,214, 52,106, 43,117,151,201, 74, 20,246,168,
  116, 42,200,150, 21, 75,169,247,182,232, 10, 84,215,137,107, 53
};

uint8_t get_crc(const uint8_t *data, uint8_t count){
    uint8_t result = 0;
    while(count--) {  result = crc_table[result ^ *data++]; }
    return result;
}


#define USE_SERIAL Serial

ESP8266WiFiMulti WiFiMulti;
int WIFI_AVAILABLE = 0;

WebSocketsServer webSocket = WebSocketsServer(81);

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t len) {

    switch(type) {
        case WStype_DISCONNECTED:
            USE_SERIAL.printf("[%u] Disconnected!\n", num);
            break;
            
        case WStype_CONNECTED:
            {
                IPAddress ip = webSocket.remoteIP(num);
//                USE_SERIAL.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        
        // send message to client
//        webSocket.sendTXT(num, "Connected");
            }
            break;
        case WStype_TEXT:
            USE_SERIAL.printf("[%u] get Text: %s\n", num, payload);

            // send message to client
//             webSocket.sendTXT(num, payload);

            // send data to all connected clients
            // webSocket.broadcastTXT("message here");
            break;
        case WStype_BIN:
            hexdump(payload, len);
            
            if (len < 4) break;
            if (payload[0] != 0x01) break;
            if (payload[1] != 0x01) break;
            if (payload[2] != 0) break;
            if (payload[3] != 0x01) break;
                      

            char strout[256];
            sprintf(strout, "%d %d %d %d %d %d %d %d %d %d %d %lu", 
                    adc_pack[0], 
                    adc_pack[1], 
                    adc_pack[2], 
                    adc_pack[3], 
                    adc_pack[4], 
                    adc_pack[5], 
                    adc_pack[6], 
                    adc_pack[7], 
                    adc_pack[8], 
                    adc_pack[9], 
                    WiFi.RSSI(),
                    millis());

            webSocket.sendTXT(num, strout);
            
            break;
    }
}


void setup() {
  
  Serial.begin(115200);
  Serial.setTimeout(PACK_TIMEOUT_MS);

  // Try to initialize MPU
//  
//  if (mpu.begin(0x68)) {
//    Serial.println("0x68 Found!");
//    MPU_AVAILABLE = 1;
//    mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
//    mpu.setGyroRange(MPU6050_RANGE_2000_DEG);
//    mpu.setFilterBandwidth(MPU6050_BAND_184_HZ);
//  }
//  else Serial.println("Failed to find 68 chip");    
//  
//  if (mpu2.begin(0x69)) {
//    Serial.println("0x69 Found!");
//    MPU2_AVAILABLE = 1;
//    mpu2.setAccelerometerRange(MPU6050_RANGE_16_G);
//    mpu2.setGyroRange(MPU6050_RANGE_2000_DEG);
//    mpu2.setFilterBandwidth(MPU6050_BAND_184_HZ);
//  }
//  else Serial.println("Failed to find 69 chip");
//
//  Serial.println("");
//  delay(100);  
  
  
  Serial.println("\n");
  Serial.println("\n");
  Serial.println("\n");

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 0);
  
  pack_start = millis();
  prev_pack_start = pack_start;
  
  WiFiMulti.addAP("Redmimimi", "y82yuisL");

  int iters = 0;
  WIFI_AVAILABLE = 1;
  
  while(WiFiMulti.run() != WL_CONNECTED) {
      if (iters > 10) {
        WIFI_AVAILABLE = 0;
        break;
      }
      
      iters++;
      delay(100);
  }

  if (WIFI_AVAILABLE){
    Serial.println("Connected!");
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
  }
  
  else Serial.println("Wifi not available");
  
}

void loop() {
  uint64_t loop_start = millis();

  if (Serial.available()){    
    uint8_t c = Serial.read();

    if ((loop_start - pack_start) > PACK_TIMEOUT_MS) {
      pack_status = PACK_IDLE;
      digitalWrite(LED_BUILTIN, 1);
    }

    switch (pack_status) {

      case PACK_IDLE:
        if (c == 0xFE) {
          uart_pack[0] = c;
          pack_status = PACK_DEVICE;  
          pack_start = millis();
          digitalWrite(LED_BUILTIN, 0);
        }
        
        else {
          pack_status = PACK_IDLE;
          digitalWrite(LED_BUILTIN, 1);
        }

        break;

      case PACK_DEVICE:
        if (c == 0x01){
          uart_pack[1] = c;
          pack_status = PACK_REGISTER;
        }

        else {
          pack_status = PACK_IDLE;
          digitalWrite(LED_BUILTIN, 1);
        }
      
        break;

      case PACK_REGISTER:
        uart_pack[2] = c;
        Serial.readBytes(&(uart_pack[3]), uart_pack[2]+1);
        pack_status = PACK_DONE;     

      case PACK_DONE:
        if (get_crc(uart_pack, 3 + uart_pack[2]) == uart_pack[1 + 1 + 1 + uart_pack[2]]){
          // Update adc
          
          for (int i = 0; i < uart_pack[2]; i+=2){
            adc_pack[i/2] = word( uart_pack[3+i], uart_pack[3+i+1]);
            Serial.print(adc_pack[i/2]); Serial.print(" ");
          }

//           //Update MPU
//
//          if (MPU_AVAILABLE){
//            mpu.getEvent(&a, &g, &temp);
//            for (int i = 0; i < 3; i++){
//              Serial.print(a.acceleration.v[i]); Serial.print(" ");
//              Serial.print(g.acceleration.v[i]); Serial.print(" ");
//            }
//          }
//          else {
//            for (int i = 0; i < 3; i++){
//              Serial.print(-1); Serial.print(" ");
//              Serial.print(-1); Serial.print(" ");
//            } 
//          }
//          
//          if (MPU2_AVAILABLE){
//            mpu2.getEvent(&a2, &g2, &temp2);
//  
//            for (int i = 0; i < 3; i++){
//              Serial.print(a2.acceleration.v[i]); Serial.print(" ");
//              Serial.print(g2.acceleration.v[i]); Serial.print(" ");
//            }
//          }
//          else {
//            for (int i = 0; i < 3; i++){
//              Serial.print(-1); Serial.print(" ");
//              Serial.print(-1); Serial.print(" ");
//            }
//          }
//          // END of Update MPU

          Serial.print(SIDE); Serial.print(" ");
          
//          Serial.print(pack_start - prev_pack_start); Serial.print(" "); // Sanity check

          Serial.println(pack_start); // current millis
          
          prev_pack_start = pack_start;
        }
        
        pack_status = PACK_IDLE;
        digitalWrite(LED_BUILTIN, 1);

        break;
    }
  }
  if (WIFI_AVAILABLE){
    webSocket.loop();  
  }
}
