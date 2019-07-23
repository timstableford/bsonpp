#ifndef __BSONPP_H__
#define __BSONPP_H__

#include <stdint.h>

#define BSONPP_SUCCESS (0)
#define BSONPP_KEY_NOT_FOUND (-1)
#define BSONPP_INCORRECT_TYPE (-2)
#define BSONPP_OUT_OF_SPACE (-3)
#define BSONPP_NO_BUFFER (-4)
#define BSONPP_DUPLICATE_KEY (-5)
#define BSONPP_NULL_VALUE (-6)

#define BSONPP_INVALID_TYPE (0x00)
#define BSONPP_DOUBLE (0x01)
#define BSONPP_STRING (0x02)
#define BSONPP_DOCUMENT (0x03)
#define BSONPP_ARRAY (0x04)
#define BSONPP_BINARY (0x05)
#define BSONPP_BOOLEAN (0x08)
#define BSONPP_DATETIME (0x09)
#define BSONPP_NULL (0x0A)
#define BSONPP_INT32 (0x10)
#define BSONPP_INT64 (0x12)

#define BSONPP_BINARY_SUBTYPE_GENERIC (0x00)
#define BSONPP_BOOLEAN_FALSE (0x00)
#define BSONPP_BOOLEAN_TRUE (0x01)

class BSONPP {
public:
    BSONPP(uint8_t *buffer, int32_t length, bool clear = true);
    BSONPP();

    int32_t getSize();
    uint8_t *getBuffer();
    int32_t getBufferSize();
    void clear();
    bool exists(const char *key);
    // Various functions for easy iteration.
    int32_t getKeyCount(int32_t *count);
    int32_t getKeyAt(int32_t index, char **key);
    int32_t getTypeAt(int32_t index, uint8_t *type);

    int32_t append(const char *key, int32_t val);
    int32_t append(const char *key, int64_t val, bool dateTime = false);
    int32_t append(const char *key, double val);
    int32_t append(const char *key, const char *val);
    int32_t append(const char *key, BSONPP *val, bool isArray = false);
    int32_t append(const char *key, const uint8_t *data, const int32_t length);
    int32_t append(const char *key, bool val);

    int32_t get(const char *key, int32_t *val);
    int32_t get(const char *key, int64_t *val);
    int32_t get(const char *key, double *val);
    int32_t get(const char *key, BSONPP *val);
    int32_t get(const char *key, char **val);
    int32_t get(const char *key, uint8_t **val, int32_t *length = nullptr);
    int32_t get(const char *key, bool *val);

private:
    int32_t appendInternal(const char *key, uint8_t type, const uint8_t *data, int32_t length);
    int32_t getOffset(const char *key, uint8_t type = BSONPP_INVALID_TYPE);
    int32_t getOffset(int32_t index);
    void setSize(int32_t size);
    // Type size is inclusive of the length field for variable length values.
    static int32_t getTypeSize(uint8_t type, uint8_t *data);
    static uint8_t getType(uint8_t *data);
    static uint8_t *getData(uint8_t *data);

    uint8_t *m_buffer;
    int32_t m_length;
};

#endif // __BSONPP_H__
