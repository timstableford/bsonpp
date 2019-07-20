#include <string.h>
#include "BSONPP.h"
#include "NetworkUtil.h"
#include "IEEE754tools.h"

BSONPP::BSONPP(uint8_t *buffer, int32_t length, bool clear): m_buffer(buffer), m_length(length) {
    if (clear) {
        this->clear();
    }
}

BSONPP::BSONPP(): m_buffer(nullptr), m_length(0) {}

void BSONPP::clear() {
    memset(m_buffer, 0x00, m_length);
    // Default size, 4 length bytes and a 0x00 suffix.
    this->setSize(5);
}

int32_t BSONPP::getSize() {
    int32_t size = 0;
    memcpy(&size, m_buffer, sizeof(int32_t));
    return letoh32(size);
}

void BSONPP::setSize(int32_t size) {
    int32_t swapped = htole32(size);
    memcpy(m_buffer, &swapped, sizeof(int32_t));
}

uint8_t *BSONPP::getBuffer() {
    return m_buffer;
}

int32_t BSONPP::getBufferSize() {
    return m_length;
}

bool BSONPP::exists(const char *key) {
    return this->getOffset(key) > 0;
}

int32_t BSONPP::getKeyCount(int32_t *countOut) {
    int32_t count = 0;
    // Start at the end of the header.
    int32_t offset = sizeof(int32_t);
    // Minus 1 for the object null terminator
    int32_t size = this->getSize() - 1;

    while (offset < size) {
        uint8_t type = m_buffer[offset++];
        // +1 null terminator
        offset += strlen(reinterpret_cast<char *>(m_buffer + offset)) + 1;
        int32_t size = BSONPP::getTypeSize(type, m_buffer + offset);
        if (size < 0) {
            return BSONPP_INCORRECT_TYPE;
        }
        offset += size;
        count++;
    }

    *countOut = count;
    return BSONPP_SUCCESS;
}

int32_t BSONPP::getKeyAt(int32_t index, char **key) {
    int32_t offset = this->getOffset(index);

    if (BSONPP_INVALID_TYPE == BSONPP::getType(m_buffer + offset)) {
        return BSONPP_KEY_NOT_FOUND;
    }

    *key = reinterpret_cast<char *>(m_buffer + offset + 1);

    return BSONPP_SUCCESS;
}

int32_t BSONPP::getTypeAt(int32_t index, uint8_t *type) {
    int32_t offset = this->getOffset(index);

    if (BSONPP_INVALID_TYPE == BSONPP::getType(m_buffer + offset)) {
        return BSONPP_KEY_NOT_FOUND;
    }

    *type = BSONPP::getType(m_buffer + offset);

    return BSONPP_SUCCESS;
}

int32_t BSONPP::append(const char* key, double val) {
    // To cope with systems that don't support doubles properly.
    if (sizeof(double) == 4) {
        uint8_t doubleData[8];
        float2DoublePacked(val, doubleData);
        return this->appendInternal(key, BSONPP_DOUBLE, doubleData, 8);
    } else {
        return this->appendInternal(key, BSONPP_DOUBLE, reinterpret_cast<uint8_t *>(&val), 8);
    }
}

int32_t BSONPP::append(const char* key, int32_t val) {
    int32_t swapped = htole32(val);
    return this->appendInternal(key, BSONPP_INT32, reinterpret_cast<uint8_t *>(&swapped), sizeof(int32_t));
}

int32_t BSONPP::append(const char* key, int64_t val, bool dateTime) {
    int64_t swapped = htole64(val);
    uint8_t type = dateTime ? BSONPP_DATETIME : BSONPP_INT64;
    return this->appendInternal(key, type, reinterpret_cast<uint8_t *>(&swapped), sizeof(int64_t));
}

int32_t BSONPP::append(const char *key, const char *val) {
    return this->appendInternal(key, BSONPP_STRING, reinterpret_cast<const uint8_t *>(val), strlen(val) + 1);
}

int32_t BSONPP::append(const char *key, BSONPP *val, bool isArray) {
    return this->appendInternal(key, isArray ? BSONPP_ARRAY : BSONPP_DOCUMENT, val->getBuffer(), val->getSize());
}

int32_t BSONPP::append(const char *key, const uint8_t *data, const int32_t length) {
    return this->appendInternal(key, BSONPP_BINARY, data, length);
}

int32_t BSONPP::append(const char *key, bool val) {
    uint8_t converted = val ? BSONPP_BOOLEAN_TRUE : BSONPP_BOOLEAN_FALSE;
    return this->appendInternal(key, BSONPP_BOOLEAN, &converted, 1);
}

int32_t BSONPP::get(const char *key, int32_t *val) {
    int32_t offset = this->getOffset(key, BSONPP_INT32);

    if (offset < 0) {
        return offset;
    }

    memcpy(val, BSONPP::getData(m_buffer + offset), sizeof(int32_t));
    *val = letoh32(*val);

    return BSONPP_SUCCESS;
}

int32_t BSONPP::get(const char *key, int64_t *val) {
    int32_t offset = this->getOffset(key);
    if (offset < 0) {
        return offset;
    }
    uint8_t *data = BSONPP::getData(m_buffer + offset);

    switch (BSONPP::getType(m_buffer + offset)) {
        case BSONPP_INT32:
            int32_t val32;
            memcpy(&val32, data, sizeof(int32_t));
            *val = letoh32(val32);
            return BSONPP_SUCCESS;
        case BSONPP_INT64: // Fallthrough
        case BSONPP_DATETIME:
            memcpy(val, data, sizeof(int64_t));
            *val = letoh64(*val);
            return BSONPP_SUCCESS;
        default:
            return BSONPP_INCORRECT_TYPE;
    }
}

int32_t BSONPP::get(const char *key, double *val) {
    int32_t offset = this->getOffset(key, BSONPP_DOUBLE);
    if (offset < 0) {
        return offset;
    }

    uint8_t *data = BSONPP::getData(m_buffer + offset);
    if (sizeof(double) == 4) {
        *val = doublePacked2Float(data);
    } else {
        memcpy(val, data, sizeof(double));
    }
    return BSONPP_SUCCESS;
}

int32_t BSONPP::get(const char *key, BSONPP *val) {
    int32_t offset = this->getOffset(key);
    if (offset < 0) {
        return offset;
    }
    uint8_t type = BSONPP::getType(m_buffer + offset);
    if (BSONPP_DOCUMENT != type && BSONPP_ARRAY != type) {
        return BSONPP_INCORRECT_TYPE;
    }
    uint8_t *data = BSONPP::getData(m_buffer + offset);
    val->m_buffer = data;
    val->m_length = BSONPP::getTypeSize(BSONPP_DOCUMENT, data);

    return BSONPP_SUCCESS;
}

int32_t BSONPP::get(const char *key, char **val) {
    int32_t offset = this->getOffset(key, BSONPP_STRING);
    if (offset < 0) {
        return offset;
    }

    // +sizeof(int32_t) to skip length
    *val = reinterpret_cast<char *>(BSONPP::getData(m_buffer + offset) + sizeof(int32_t));

    return BSONPP_SUCCESS;
}

int32_t BSONPP::get(const char *key, uint8_t **val, int32_t *length) {
    int32_t offset = this->getOffset(key, BSONPP_BINARY);
    if (offset < 0) {
        return offset;
    }

    uint8_t *data = BSONPP::getData(m_buffer + offset);

    if (length != nullptr) {
        memcpy(length, data, sizeof(int32_t));
        *length = letoh32(*length);
    }

    // +sizeof(int32_t) to skip length, +1 to skip subtype
    *val = reinterpret_cast<uint8_t *>(data + sizeof(int32_t)) + 1;

    return BSONPP_SUCCESS;
}

int32_t BSONPP::get(const char *key, bool *val) {
    int32_t offset = this->getOffset(key, BSONPP_BOOLEAN);
    if (offset < 0) {
        return offset;
    }

    *val = BSONPP::getData(m_buffer + offset)[0] == BSONPP_BOOLEAN_TRUE;

    return BSONPP_SUCCESS;
}

// Private methods
int32_t BSONPP::appendInternal(const char *key, uint8_t type, const uint8_t *data, const int32_t length) {
    if (m_buffer == nullptr) {
        return BSONPP_NO_BUFFER;
    }

    bool includeLength = false;
    switch (type) {
        case BSONPP_STRING:
        case BSONPP_BINARY:
            includeLength = true;
            break;
        default:
            break;
    }

    // +1 for key null terminator
    int32_t sizeAfter = this->getSize() + strlen(key) + 1 + sizeof(type) + length;
    if (includeLength) {
        sizeAfter += sizeof(int32_t);
    }

    if (sizeAfter > m_length) {
        return BSONPP_OUT_OF_SPACE;
    }

    if (this->exists(key)) {
        return BSONPP_DUPLICATE_KEY;
    }

    // Minus one for the null terminator of the BSON object
    int32_t offset = this->getSize() - 1;
    // Set the type.
    m_buffer[offset++] = type;

    // Copy the key
    memcpy(m_buffer + offset, key, strlen(key) + 1);
    offset += strlen(key) + 1;

    if (includeLength) {
        int32_t swapped = htole32(length);
        memcpy(m_buffer + offset, &swapped, sizeof(int32_t));
        offset += sizeof(int32_t);
    }

    if (type == BSONPP_BINARY) {
        m_buffer[offset++] = BSONPP_BINARY_SUBTYPE_GENERIC;
    }

    memcpy(m_buffer + offset, data, length);
    offset += length;

    // Plus one for the null terminator of the BSON object
    this->setSize(offset + 1);

    return BSONPP_SUCCESS;
}

int32_t BSONPP::getTypeSize(uint8_t type, uint8_t *data) {
    int32_t cache = 0;
    switch (type) {
        case BSONPP_DOUBLE:
            return 8;
        case BSONPP_STRING: // Fallthrough
        case BSONPP_DOCUMENT: // Fallthrough
        case BSONPP_ARRAY:
            memcpy(&cache, data, sizeof(int32_t));
            return letoh32(cache) + sizeof(int32_t);
        case BSONPP_BINARY:
            // +1 for subtype
            memcpy(&cache, data, sizeof(int32_t));
            return letoh32(cache) + sizeof(int32_t) + 1;
        case BSONPP_INT32:
            return sizeof(int32_t);
        case BSONPP_INT64: // Fallthrough
        case BSONPP_DATETIME:
            return sizeof(int64_t);
        case BSONPP_BOOLEAN:
            return 1;
        case BSONPP_NULL:
            return 0;
        default:
            // For unhandled types return a 0 length.
            return -1;
    }
}

uint8_t BSONPP::getType(uint8_t *data) {
    return data[0];
}

uint8_t *BSONPP::getData(uint8_t *data) {
    // +1 for type, +the key length, +the key null terminator
    return data + 1 + strlen(reinterpret_cast<char *>(data + 1)) + 1;
}

int32_t BSONPP::getOffset(const char *key, uint8_t type) {
    // Start at the end of the header.
    int32_t offset = sizeof(int32_t);
    // Minus 1 for the object null terminator
    int32_t size = this->getSize() - 1;

    while (offset < size) {
        // +1 to skip the type
        if (strncmp(key, reinterpret_cast<char *>(m_buffer + offset + 1), strlen(key)) == 0) {
            if (m_buffer[offset] == BSONPP_NULL) {
                return BSONPP_NULL_VALUE;
            }
            if (type != BSONPP_INVALID_TYPE && type != m_buffer[offset]) {
                return BSONPP_INCORRECT_TYPE;
            }
            return offset;
        }
        // Extract the type and move the offset on
        uint8_t type = m_buffer[offset++];
        // +1 null terminator
        offset += strlen(reinterpret_cast<char *>(m_buffer + offset)) + 1;
        int32_t dataSize = BSONPP::getTypeSize(type, m_buffer + offset);
        if (dataSize < 0) {
            return 0;
        }
        offset += dataSize;
    }

    return BSONPP_KEY_NOT_FOUND;
}

int32_t BSONPP::getOffset(int32_t index) {
    // Start at the end of the header.
    int32_t offset = sizeof(int32_t);
    // Minus 1 for the object null terminator
    int32_t size = this->getSize() - 1;

    for (int32_t i = 0; i < index && offset < size; i++) {
        uint8_t type = m_buffer[offset++];
        // +1 null terminator
        offset += strlen(reinterpret_cast<char *>(m_buffer + offset)) + 1;
        int32_t size = BSONPP::getTypeSize(type, m_buffer + offset);
        if (size < 0) {
            return BSONPP_INCORRECT_TYPE;
        }
        offset += size;
    }

    return offset;
}
