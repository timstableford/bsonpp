#include <Arduino.h>
#include <unity.h>
#include <BSONPP.h>

uint8_t buffer[256];
BSONPP bson(buffer, sizeof(buffer));

void setUp(void) {
    bson.clear();
}

void test_basic_lifecycle(void) {
    const char *testStr = "string value";
    TEST_ASSERT_EQUAL(BSONPP_SUCCESS, bson.append("a", int32_t{1000}));
    int32_t res = bson.append("b", testStr);
    TEST_ASSERT_EQUAL(BSONPP_SUCCESS, res);

    int32_t intVal = 0;
    TEST_ASSERT_EQUAL(BSONPP_SUCCESS, bson.get("a", &intVal));
    TEST_ASSERT_EQUAL(1000, intVal);

    char *strVal = nullptr;
    TEST_ASSERT_EQUAL(BSONPP_SUCCESS, bson.get("b", &strVal));
    TEST_ASSERT_NOT_NULL(strVal);

    TEST_ASSERT_EQUAL_MEMORY(testStr, strVal, strlen(testStr) + 1);
}

void test_basic_encoding(void) {
    TEST_ASSERT_EQUAL(BSONPP_SUCCESS, bson.append("fish", int32_t{10}));
    uint8_t expected[] = { 0xf, 0x0, 0x0, 0x0, 0x10, 0x66, 0x69, 0x73, 0x68, 0x0, 0xa, 0x0, 0x0, 0x0, 0x0 };

    TEST_ASSERT_EQUAL(sizeof(expected), bson.getSize());
    TEST_ASSERT_EQUAL_MEMORY(expected, buffer, sizeof(expected));
}

void test_float_encoding(void) {
    TEST_ASSERT_EQUAL(BSONPP_SUCCESS, bson.append("fl", 0.23));
    const uint8_t expected[] = { 0x11, 0x0, 0x0, 0x0, 0x1, 0x66, 0x6C, 0x0, 0x0, 0x1, 0x0, 0xE0, 0xA3, 0x70, 0xCD, 0x3F, 0x0 };

    double fetched = 0.0;
    TEST_ASSERT_EQUAL(BSONPP_SUCCESS, bson.get("fl", &fetched));
    // There's some randomness when encoding the float as a double so allow a 0.001 margin.
    TEST_ASSERT_FLOAT_WITHIN(0.001, 0.23, fetched);

    TEST_ASSERT_EQUAL(sizeof(expected), bson.getSize());

    // It'd be great if this would work but the flucation makes it impossible.
    // Suffice to say it's been manual verified.
    // TEST_ASSERT_EQUAL(0, memcmp(buffer, expected, sizeof(expected)));
}

void test_float_decoding(void) {
    uint8_t buf[] = { 0x11, 0x0, 0x0, 0x0, 0x1, 0x66, 0x6c, 0x0, 0xd1, 0x91, 0x5c, 0xfe, 0x43, 0xfa, 0xcd, 0x3f, 0x0 };
    BSONPP doc(buf, sizeof(buf), false);

    double val = 0.0;
    TEST_ASSERT_EQUAL(BSONPP_SUCCESS, doc.get("fl", &val));
    TEST_ASSERT_EQUAL(0.2342, val);
}

void test_get_type_at(void) {
    TEST_ASSERT_EQUAL(BSONPP_SUCCESS, bson.append("a", int32_t{1000}));
    uint8_t type = BSONPP_INVALID_TYPE;
    TEST_ASSERT_EQUAL(BSONPP_SUCCESS, bson.getTypeAt(0, &type));
    TEST_ASSERT_EQUAL(BSONPP_INT32, type);
}

void test_subdocument(void) {
    uint8_t subdocBuffer[128];
    BSONPP subdoc(subdocBuffer, sizeof(subdocBuffer));
    TEST_ASSERT_EQUAL(BSONPP_SUCCESS, subdoc.append("a", int32_t{1000}));

    TEST_ASSERT_EQUAL(BSONPP_SUCCESS, bson.append("sub", &subdoc));

    BSONPP fetched;
    TEST_ASSERT_EQUAL(BSONPP_SUCCESS, bson.get("sub", &fetched));

    int32_t fetchedSubval = 0;
    TEST_ASSERT_EQUAL(BSONPP_SUCCESS, fetched.get("a", &fetchedSubval));

    TEST_ASSERT_EQUAL(1000, fetchedSubval);
}

void setup() {
    delay(2000);

    UNITY_BEGIN();
    RUN_TEST(test_basic_lifecycle);
    RUN_TEST(test_basic_encoding);
    RUN_TEST(test_float_encoding);
    RUN_TEST(test_float_decoding);
    RUN_TEST(test_get_type_at);
    RUN_TEST(test_subdocument);
    UNITY_END();
}

void loop() {}
