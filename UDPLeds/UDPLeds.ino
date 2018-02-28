#include <Ethernet.h>
#include <EthernetUdp.h>
#include <TimerOne.h>
#include "LPD6803.h"
#include <math.h>
#include <SPI.h>

byte mac[] = { 
  0x90, 0xA2, 0xDA, 0x0D, 0x78, 0x62 };
EthernetUDP udp;

typedef uint8_t command_t;
const command_t ADDSTRIP = 0;
const command_t DELSTRIP = 1;
const command_t SETLED = 2;
const command_t CLEAR = 3; 

struct ledStrip_t {
  LPD6803 strip;
  struct ledStrip_t *next;
};

struct _addStripData {
  uint8_t stripId;
  uint16_t numLeds;
  uint8_t dpin;
  uint8_t cpin;
};

struct _delStripData {
  uint8_t stripId;
};

struct _setLedData {
  uint8_t stripId;
  uint16_t ledNum;
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

struct _clearData {
  uint8_t stripId;
};

union command_data {
  struct _addStripData addStripData;
  struct _delStripData delStripData;
  struct _setLedData setLedData;
  struct _clearData clearData;
};

typedef struct _command_s {
  command_t command;
  union command_data data;
} 
command_s;

#define loop_forever() for(;;);

#define DEBUG_PRINTING

#ifdef DEBUG_PRINTING
#define DEBUG_PRINT_INIT() Serial.begin(9600)
#define DEBUG_PRINT(m) Serial.print(m)
#define DEBUG_PRINTLN(m) Serial.println(m)
#else
#define DEBUG_PRINT_INIT()
#define DEBUG_PRINT(M)
#define DEBUG_PRINTLN(m)
#endif

void setup() {
  DEBUG_PRINT_INIT();
  DEBUG_PRINTLN("Setup");
  if (Ethernet.begin(mac) == 0) {
    DEBUG_PRINTLN("Failed to configure Ethernet using DHCP, looping forever");
    loop_forever();
  }
  udp.begin(12345);
}

ledStrip_t *strips;

/**
 * If an UDP-packet has been received, reads it into buffer. Buffer must be at least UDP_TX_PACKET_MAX_SIZE large.
 * Returns the number of bytes read, or 0 if no packet was received.
 */
int readUDP(uint8_t * buffer) {
  char packetBuffer[UDP_TX_PACKET_MAX_SIZE];
  int packetSize = udp.parsePacket();
  if (packetSize > 0) {
    DEBUG_PRINT("Received packet of size ");
    DEBUG_PRINT( packetSize );
    DEBUG_PRINT(" from ");
    IPAddress remote = udp.remoteIP();
    for (int i = 0 ; i < 4 ; i++) {
      DEBUG_PRINT(remote[i]);
      if (i < 3)
        DEBUG_PRINT("."); 
    }
    DEBUG_PRINT(":");
    DEBUG_PRINTLN(udp.remotePort());
    udp.read(packetBuffer,UDP_TX_PACKET_MAX_SIZE);
    DEBUG_PRINTLN(packetBuffer);
  }
  return packetSize;  
}

void perform_command(struct _command_s * command) {
  switch(command->command) {
  case ADDSTRIP:
    break;
  case DELSTRIP:
    break;
  case SETLED:
    break;
  case CLEAR:
    break;
  default:
    DEBUG_PRINTLN("Something weird happened, received incorrect command.");
  }
  return;
}

void readCommands() {
  uint8_t buffer[UDP_TX_PACKET_MAX_SIZE];
  while (1) {
    int len = readUDP(buffer);
    if (len == 0)
      break;
    for (int i = 0 ; i < len ; i++) {
      command_s command;
      command.command = buffer[i++];
      switch (command.command) {
      case ADDSTRIP:
        if (i + sizeof(struct _addStripData) >= len) goto someError;
        memcpy(&(command.data),&buffer[i],sizeof(struct _addStripData));
        i += sizeof(struct _addStripData);
        break;
      case DELSTRIP:
        if (i + sizeof(struct _delStripData) >= len) goto someError;
        memcpy(&(command.data),&buffer[i],sizeof(struct _delStripData));
        i += sizeof(struct _delStripData);
        break;
      case SETLED:
        if (i + sizeof(struct _setLedData) >= len) goto someError;
        memcpy(&(command.data),&buffer[i],sizeof(struct _setLedData));
        i += sizeof(struct _setLedData);
        break;
      case CLEAR:
        if (i + sizeof(struct _clearData) >= len) goto someError;
        memcpy(&(command.data),&buffer[i],sizeof(struct _clearData));
        i += sizeof(struct _clearData);
        break;
      default:
        DEBUG_PRINT("Received unknown command (");
        DEBUG_PRINT(buffer[i]);
        DEBUG_PRINTLN("), ignoring the rest of this packet!");
        i = len;
        continue;
someError:
        DEBUG_PRINTLN("Received incomplete command, ignoring the rest of this packet.");
        i = len;
        continue;
      }
      perform_command(&command);
    }
  }
}

void loop() {
  delay(100);
}




