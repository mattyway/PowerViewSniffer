// sketch to sniff PowerView packets from a pebble remote or hub

#include <Arduino.h>
#include <RFPowerView.h>
#include <PrintHelpers.h>

#define SER_BAUDRATE                   (115200)

// Copied from Powerview Hub userdata API 
// eg: http://POWERVIEW_HUB_IP_ADDRESS/api/userdata/ and find the field labeled "rfID"
#define RF_ID                          (0x2EC8)

RFPowerView powerView(RF_CE_PIN, RF_CS_PIN, RF_IRQ_PIN, RF_ID);

void processBuffer(const uint8_t*);
void processPacket(const Packet*);
void printField(Field& field);


void setup() {
  Serial.begin(SER_BAUDRATE);

  Serial.println("Starting up");

  powerView.setPacketReceivedCallback(processPacket);
  powerView.setValidBufferReceivedCallback(processBuffer);
  if (!powerView.begin()) {
    Serial.println("Failed to start RFPowerView");
    return;
  }

  Serial.println("Ready");
}

void loop() {
  powerView.loop();
}

void processBuffer(const uint8_t *buffer) {
  printBuffer(buffer);
  Serial.println();
}

void processPacket(const Packet *packet) {
  if (std::holds_alternative<BroadcastHeader>(packet->header)) {
    auto header = std::get<BroadcastHeader>(packet->header);
    Serial.print("Source: ");
    Serial.print(header.source, HEX);
  } else if (std::holds_alternative<UnicastHeader>(packet->header)) {
    auto header = std::get<UnicastHeader>(packet->header);
    Serial.print("Source: ");
    Serial.print(header.source, HEX);
    Serial.print(" Destination: ");
    Serial.print(header.destination, HEX);
  } else if (std::holds_alternative<GroupsHeader>(packet->header)) {
    auto header = std::get<GroupsHeader>(packet->header);
    Serial.print("Source: ");
    Serial.print(header.source, HEX);
    Serial.print(" Groups: ");
    for (size_t i = 0; i < header.groups.size(); i++) {
      Serial.print(header.groups[i]);
      if (i != header.groups.size() - 1) {
        Serial.print(" ");
      }
    }
  }
  Serial.print(" ");
  Serial.print("Rolling codes: ");
  Serial.print(packet->rollingCode1, HEX);
  Serial.print(" ");
  Serial.print(packet->rollingCode2, HEX);
  Serial.print(" ");
  switch(packet->type) {
    case PacketType::STOP:
      Serial.print("Stop");
      break;
    case PacketType::OPEN:
      Serial.print("Open");
      break;
    case PacketType::CLOSE:
      Serial.print("Close");
      break;
    case PacketType::OPEN_SLOW:
      Serial.print("Open Slow");
      break;
    case PacketType::CLOSE_SLOW:
      Serial.print("Close Slow");
      break;
    case PacketType::MOVE_TO_SAVED_POSITION:
      Serial.print("Move To Saved Position");
      break;
    case PacketType::FIELDS: {
      FieldsParameters parameters = std::get<FieldsParameters>(packet->parameters);
      for (size_t i = 0; i < parameters.fields.size(); i++) {
        Field field = parameters.fields[i];
        printField(field);
      }
      break;
    }
    case PacketType::FIELD_COMMAND: {
      FieldsParameters parameters = std::get<FieldsParameters>(packet->parameters);
      for (size_t i = 0; i < parameters.fields.size(); i++) {
        Field field = parameters.fields[i];
        printField(field);
      }
      break;
    }
    case PacketType::UNKNOWN:
      Serial.print("???");
      break;
  }
  Serial.println();
}

void printField(Field& field) {
  if (field.type == FieldType::SET) {
    Serial.print("Set ");
  }
  if (field.type == FieldType::FETCH) {
    Serial.print("Fetch ");
  }
  switch (field.identifier) {
    case 0x50: {
      // position
      Serial.print("Position");
      if (field.hasValue) {
        uint16_t value = std::get<uint16_t>(field.value);
        uint8_t position = (uint8_t)std::round(((float)value / 0xFFFF) * 100);
        Serial.print("=");
        Serial.print(position);
      }
      break;
    }
    case 0x42: {
      // battery
      Serial.print("Battery");
      if (field.hasValue) {
        uint8_t value = std::get<uint8_t>(field.value);
        uint8_t battery = uint8_t(((float)value / 200) * 100);
        Serial.print("=");
        Serial.print(battery);
      }
      break;
    }
    default: {
      printUint8(field.identifier);
      if (field.hasValue) {
        if (std::holds_alternative<uint8_t>(field.value)) {
          Serial.print("=");
          printUint8(std::get<uint8_t>(field.value));
        } else if (std::holds_alternative<uint16_t>(field.value)) {
          Serial.print("=");
          printUint16(std::get<uint16_t>(field.value));
        }
      }
    }
  }
  Serial.print(" ");
}
