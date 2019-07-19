#ifndef UNIT_TEST

#include <Arduino.h>
#include <BSONPP.h>

uint8_t buffer[128];
BSONPP bson(buffer, sizeof(buffer));

void printBuffer(uint8_t *buffer, int32_t length) {
    Serial.println();
    for (int32_t i = 0; i < length; i++) {
        Serial.print(buffer[i], HEX);
        Serial.print(", ");
    }
    Serial.println();
}

void setup() {
    Serial.begin(115200);
    Serial.println("Startup");
}

void loop() {
    bson.clear();

    bson.append("runtime", (int64_t) millis());
    bson.append("floaty", 0.34534);
    printBuffer(bson.getBuffer(), bson.getSize());
}

#endif // UNIT_TEST
