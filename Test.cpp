#ifdef __LINUX_BUILD

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <gtest/gtest.h>
#include <BSONPP.h>

constexpr int32_t kBufferSize = 256;

class Test : public ::testing::Test {
public:
    Test() {}

    void SetUp() override {
        bson.clear();
    }

    void compare(uint8_t *expected, uint16_t length) {
        ASSERT_EQ(0, memcmp(expected, bson.getBuffer(), length));
    }

    void printBuffer() {
        for (int32_t i = 0; i < bson.getSize(); i++) {
            printf("0x%02hhx, ", bson.getBuffer()[i]);
        }
        printf("\n");
    }

    BSONPP bson = BSONPP(new uint8_t[kBufferSize], kBufferSize);
};

TEST_F(Test, DefaultLengthAfterClear) {
    ASSERT_EQ(5, bson.getSize());
}

TEST_F(Test, AppendInt32) {
    ASSERT_EQ(BSONPP_SUCCESS, bson.append("fish", 10));
    uint8_t expected[] = { 0xf, 0x0, 0x0, 0x0, 0x10, 0x66, 0x69, 0x73, 0x68, 0x0, 0xa, 0x0, 0x0, 0x0, 0x0 };
    compare(expected, sizeof(expected));
}

TEST_F(Test, AppendTwoInt32) {
    ASSERT_EQ(BSONPP_SUCCESS, bson.append("fishA", 10));
    ASSERT_EQ(BSONPP_SUCCESS, bson.append("fishB", 20));
    uint8_t expected[] = { 0x1b, 0x0, 0x0, 0x0, 0x10, 0x66, 0x69, 0x73, 0x68, 0x41, 0x0, 0xa, 0x0, 0x0, 0x0, 0x10, 0x66, 0x69, 0x73, 0x68, 0x42, 0x0, 0x14, 0x0, 0x0, 0x0, 0x0 };
    compare(expected, sizeof(expected));
}

TEST_F(Test, GetInt32) {
    ASSERT_EQ(BSONPP_SUCCESS, bson.append("fish", 10));
    int64_t ret = 0;
    ASSERT_EQ(BSONPP_SUCCESS, bson.get("fish", &ret));
    ASSERT_EQ(10, ret);

    int32_t ret32 = 0;
    ASSERT_EQ(BSONPP_SUCCESS, bson.get("fish", &ret32));
    ASSERT_EQ(10, ret);
}

TEST_F(Test, GetInt32AndInt64) {
    ASSERT_EQ(BSONPP_SUCCESS, bson.append("fishA", 20));
    ASSERT_EQ(BSONPP_SUCCESS, bson.append("fishB", int64_t{500}));

    int64_t ret = 0;

    ASSERT_EQ(BSONPP_SUCCESS, bson.get("fishA", &ret));
    ASSERT_EQ(20, ret);

    ASSERT_EQ(BSONPP_SUCCESS, bson.get("fishB", &ret));
    ASSERT_EQ(500, ret);
}

TEST_F(Test, AppendDouble) {
    ASSERT_EQ(BSONPP_SUCCESS, bson.append("fl", 0.2342));
    uint8_t expected[] = { 0x11, 0x0, 0x0, 0x0, 0x1, 0x66, 0x6c, 0x0, 0xd1, 0x91, 0x5c, 0xfe, 0x43, 0xfa, 0xcd, 0x3f, 0x0 };
    compare(expected, sizeof(expected));
}

TEST_F(Test, GetDouble) {
    ASSERT_EQ(BSONPP_SUCCESS, bson.append("fl", 0.2342));
    double val = 0;
    ASSERT_EQ(BSONPP_SUCCESS, bson.get("fl", &val));
    ASSERT_EQ(0.2342, val);
}

TEST_F(Test, AppendString) {
    ASSERT_EQ(BSONPP_SUCCESS, bson.append("str", "stringy mc stringyson"));
    uint8_t expected[] = { 0x24, 0x0, 0x0, 0x0, 0x2, 0x73, 0x74, 0x72, 0x0, 0x16, 0x0, 0x0, 0x0, 0x73, 0x74, 0x72, 0x69, 0x6e, 0x67, 0x79, 0x20, 0x6d, 0x63, 0x20, 0x73, 0x74, 0x72, 0x69, 0x6e, 0x67, 0x79, 0x73, 0x6f, 0x6e, 0x0, 0x0 };
    compare(expected, sizeof(expected));
}

TEST_F(Test, GetString) {
    const char *val = "stringy mc stringyson";
    ASSERT_EQ(BSONPP_SUCCESS, bson.append("str", val));
    char *fetched = nullptr;
    ASSERT_EQ(BSONPP_SUCCESS, bson.get("str", &fetched));
    ASSERT_NE(nullptr, fetched);
    ASSERT_EQ(0, strncmp(val, fetched, strlen(val) + 1));
}

TEST_F(Test, AppendStringAndNumber) {
    // It's a possibility variable length strings clobber crap.
    // This test makes sure that doesn't happen.
    const char *strVal = "stringy";
    ASSERT_EQ(BSONPP_SUCCESS, bson.append("numA", 20));
    ASSERT_EQ(BSONPP_SUCCESS, bson.append("str", strVal));
    ASSERT_EQ(BSONPP_SUCCESS, bson.append("numB", 50));

    int64_t numVal = 0;
    ASSERT_EQ(BSONPP_SUCCESS, bson.get("numA", &numVal));
    ASSERT_EQ(20, numVal);

    ASSERT_EQ(BSONPP_SUCCESS, bson.get("numB", &numVal));
    ASSERT_EQ(50, numVal);

    char *fetched = nullptr;
    ASSERT_EQ(BSONPP_SUCCESS, bson.get("str", &fetched));
    ASSERT_NE(nullptr, fetched);
    ASSERT_EQ(0, strncmp(strVal, fetched, strlen(strVal) + 1));
}

TEST_F(Test, AppendSubdocument) {
    uint8_t buffer[kBufferSize];
    BSONPP subdoc(buffer, kBufferSize);
    subdoc.append("num", 10);

    ASSERT_EQ(BSONPP_SUCCESS, bson.append("doc", &subdoc));

    uint8_t expected[] = { 0x18, 0x0, 0x0, 0x0, 0x3, 0x64, 0x6f, 0x63, 0x0, 0xe, 0x0, 0x0, 0x0, 0x10, 0x6e, 0x75, 0x6d, 0x0, 0xa, 0x0, 0x0, 0x0, 0x0, 0x0 };
    compare(expected, sizeof(expected));
}

TEST_F(Test, GetSubdocument) {
    uint8_t buffer[kBufferSize];
    BSONPP subdoc(buffer, kBufferSize);
    ASSERT_EQ(BSONPP_SUCCESS, subdoc.append("num", 10));
    ASSERT_EQ(BSONPP_SUCCESS, bson.append("doc", &subdoc));

    BSONPP fetchedSubdoc;
    ASSERT_EQ(BSONPP_SUCCESS, bson.get("doc", &fetchedSubdoc));
    int64_t val = 0;
    ASSERT_EQ(BSONPP_SUCCESS, fetchedSubdoc.get("num", &val));
    ASSERT_EQ(10, val);
}

TEST_F(Test, GetIncorrectType) {
    ASSERT_EQ(BSONPP_SUCCESS, bson.append("num", 10));
    double val = 0;
    ASSERT_EQ(BSONPP_INCORRECT_TYPE, bson.get("num", &val));
    ASSERT_EQ(0, val);
}

TEST_F(Test, GetNotFound) {
    double val = 0;
    ASSERT_EQ(BSONPP_KEY_NOT_FOUND, bson.get("num", &val));
    ASSERT_EQ(0, val);
}

TEST_F(Test, AppendOutOfSpace) {
    // Arbitrary minus, leave space for key names though.
    char *longStr = new char[kBufferSize - 20];
    memset(longStr, 's', kBufferSize - 20);
    longStr[kBufferSize - 21] = 0;

    ASSERT_EQ(BSONPP_SUCCESS, bson.append("longStr", longStr));
    ASSERT_EQ(BSONPP_OUT_OF_SPACE, bson.append("longStr2", longStr));
}

TEST_F(Test, GetKeyAt) {
    char *key = nullptr;

    ASSERT_EQ(BSONPP_KEY_NOT_FOUND, bson.getKeyAt(0, &key));

    ASSERT_EQ(BSONPP_SUCCESS, bson.append("a", 1));
    ASSERT_EQ(BSONPP_SUCCESS, bson.getKeyAt(0, &key));
    ASSERT_NE(nullptr, key);
    ASSERT_EQ(0, strncmp("a", key, 2));

    ASSERT_EQ(BSONPP_SUCCESS, bson.append("b", 2));
    ASSERT_EQ(BSONPP_SUCCESS, bson.getKeyAt(0, &key));
    ASSERT_EQ(0, strncmp("a", key, 2));
    ASSERT_EQ(BSONPP_SUCCESS, bson.getKeyAt(1, &key));
    ASSERT_EQ(0, strncmp("b", key, 2));

    ASSERT_EQ(BSONPP_SUCCESS, bson.append("c", "just a string"));
    ASSERT_EQ(BSONPP_SUCCESS, bson.getKeyAt(0, &key));
    ASSERT_EQ(0, strncmp("a", key, 2));
    ASSERT_EQ(BSONPP_SUCCESS, bson.getKeyAt(1, &key));
    ASSERT_EQ(0, strncmp("b", key, 2));
    ASSERT_EQ(BSONPP_SUCCESS, bson.getKeyAt(2, &key));
    ASSERT_EQ(0, strncmp("c", key, 2));

    ASSERT_EQ(BSONPP_SUCCESS, bson.append("d", "just another string"));
    ASSERT_EQ(BSONPP_SUCCESS, bson.getKeyAt(0, &key));
    ASSERT_EQ(0, strncmp("a", key, 2));
    ASSERT_EQ(BSONPP_SUCCESS, bson.getKeyAt(1, &key));
    ASSERT_EQ(0, strncmp("b", key, 2));
    ASSERT_EQ(BSONPP_SUCCESS, bson.getKeyAt(2, &key));
    ASSERT_EQ(0, strncmp("c", key, 2));
    ASSERT_EQ(BSONPP_SUCCESS, bson.getKeyAt(3, &key));
    ASSERT_EQ(0, strncmp("d", key, 2));
}

TEST_F(Test, GetKeyCount) {
    int32_t count = -1;
    ASSERT_EQ(BSONPP_SUCCESS, bson.getKeyCount(&count));
    ASSERT_EQ(0, count);

    ASSERT_EQ(BSONPP_SUCCESS, bson.append("a", 1));
    ASSERT_EQ(BSONPP_SUCCESS, bson.getKeyCount(&count));
    ASSERT_EQ(1, count);

    ASSERT_EQ(BSONPP_SUCCESS, bson.append("b", 2));
    ASSERT_EQ(BSONPP_SUCCESS, bson.getKeyCount(&count));
    ASSERT_EQ(2, count);

    ASSERT_EQ(BSONPP_SUCCESS, bson.append("c", 3));
    ASSERT_EQ(BSONPP_SUCCESS, bson.getKeyCount(&count));
    ASSERT_EQ(3, count);

    ASSERT_EQ(BSONPP_SUCCESS, bson.append("d", 5));
    ASSERT_EQ(BSONPP_SUCCESS, bson.getKeyCount(&count));
    ASSERT_EQ(4, count);
}

TEST_F(Test, GetTypeAt) {
    uint8_t type = BSONPP_INVALID_TYPE;
    ASSERT_EQ(BSONPP_KEY_NOT_FOUND, bson.getTypeAt(0, &type));
    ASSERT_EQ(BSONPP_INVALID_TYPE, type);

    ASSERT_EQ(BSONPP_SUCCESS, bson.append("a", 1));
    ASSERT_EQ(BSONPP_SUCCESS, bson.getTypeAt(0, &type));
    ASSERT_EQ(BSONPP_INT32, type);

    ASSERT_EQ(BSONPP_SUCCESS, bson.append("b", "sdfsdf"));
    ASSERT_EQ(BSONPP_SUCCESS, bson.getTypeAt(1, &type));
    ASSERT_EQ(BSONPP_STRING, type);
}

TEST_F(Test, KeyExists) {
    ASSERT_EQ(false, bson.exists("a"));
    ASSERT_EQ(BSONPP_SUCCESS, bson.append("a", 1));
    ASSERT_EQ(true, bson.exists("a"));
}

TEST_F(Test, AppendArray) {
    uint8_t buffer[kBufferSize];
    BSONPP subdoc(buffer, kBufferSize);
    ASSERT_EQ(BSONPP_SUCCESS, subdoc.append("0", "potato"));
    ASSERT_EQ(BSONPP_SUCCESS, subdoc.append("1", 800));
    ASSERT_EQ(BSONPP_SUCCESS, subdoc.append("2", "swimmily"));
    ASSERT_EQ(BSONPP_SUCCESS, subdoc.append("3", 0.5));

    ASSERT_EQ(BSONPP_SUCCESS, bson.append("arr", &subdoc, true));

    uint8_t expected[] = { 0x3f, 0x0, 0x0, 0x0, 0x4, 0x61, 0x72, 0x72, 0x0, 0x35, 0x0, 0x0, 0x0, 0x2, 0x30, 0x0, 0x7, 0x0, 0x0, 0x0, 0x70, 0x6f, 0x74, 0x61, 0x74, 0x6f, 0x0, 0x10, 0x31, 0x0, 0x20, 0x3, 0x0, 0x0, 0x2, 0x32, 0x0, 0x9, 0x0, 0x0, 0x0, 0x73, 0x77, 0x69, 0x6d, 0x6d, 0x69, 0x6c, 0x79, 0x0, 0x1, 0x33, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xe0, 0x3f, 0x0, 0x0 };
    compare(expected, sizeof(expected));
}

TEST_F(Test, DuplicateKey) {
    ASSERT_EQ(BSONPP_SUCCESS, bson.append("a", "asdfsdf"));
    ASSERT_EQ(BSONPP_DUPLICATE_KEY, bson.append("a", "s"));
}

TEST_F(Test, AppendBinary) {
    uint8_t binary[] = { 0x00, 0x01, 0x02, 0x04, 0x05, 0xA0, 0xFF, 0x44 };
    ASSERT_EQ(BSONPP_SUCCESS, bson.append("bin", binary, sizeof(binary)));

    uint8_t expected[] = { 0x17, 0x0, 0x0, 0x0, 0x5, 0x62, 0x69, 0x6e, 0x0, 0x8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x2, 0x4, 0x5, 0xa0, 0xff, 0x44, 0x0 };
    compare(expected, sizeof(expected));
}

TEST_F(Test, GetBinary) {
    uint8_t binary[] = { 0x00, 0x01, 0x02, 0x04, 0x05, 0xA0, 0xFF, 0x44 };
    ASSERT_EQ(BSONPP_SUCCESS, bson.append("bin", binary, sizeof(binary)));

    uint8_t *fetched = nullptr;
    int32_t length = 0;
    ASSERT_EQ(BSONPP_SUCCESS, bson.get("bin", &fetched, &length));

    ASSERT_NE(nullptr, fetched);
    ASSERT_EQ(sizeof(binary), length);
    ASSERT_EQ(0, memcmp(binary, fetched, sizeof(binary)));
}

TEST_F(Test, AppendBoolean) {
    ASSERT_EQ(BSONPP_SUCCESS, bson.append("truthy", true));
    ASSERT_EQ(BSONPP_SUCCESS, bson.append("falsey", false));

    uint8_t expected[] = { 0x17, 0x0, 0x0, 0x0, 0x8, 0x74, 0x72, 0x75, 0x74, 0x68, 0x79, 0x0, 0x1, 0x8, 0x66, 0x61, 0x6c, 0x73, 0x65, 0x79, 0x0, 0x0, 0x0 };
    compare(expected, sizeof(expected));
}

TEST_F(Test, GetBoolean) {
    ASSERT_EQ(BSONPP_SUCCESS, bson.append("truthy", true));
    ASSERT_EQ(BSONPP_SUCCESS, bson.append("falsey", false));

    bool val;
    ASSERT_EQ(BSONPP_SUCCESS, bson.get("truthy", &val));
    ASSERT_TRUE(val);

    ASSERT_EQ(BSONPP_SUCCESS, bson.get("falsey", &val));
    ASSERT_FALSE(val);
}

TEST_F(Test, AppendDateTime) {
    // Arbitrary timestamp
    ASSERT_EQ(BSONPP_SUCCESS, bson.append("time", 1563464196213, true));

    uint8_t expected[] = { 0x13, 0x0, 0x0, 0x0, 0x9, 0x74, 0x69, 0x6d, 0x65, 0x0, 0x75, 0x60, 0xba, 0x5, 0x6c, 0x1, 0x0, 0x0, 0x0 };
    compare(expected, sizeof(expected));
}

TEST_F(Test, GetDateTime) {
    // Arbitrary timestamp
    ASSERT_EQ(BSONPP_SUCCESS, bson.append("time", 1563464196213, true));

    int64_t val = 0;
    ASSERT_EQ(BSONPP_SUCCESS, bson.get("time", &val));
    ASSERT_EQ(1563464196213, val);
}

TEST_F(Test, CopeWithNull) {
    uint8_t data[] = { 0x15, 0x0, 0x0, 0x0, 0xa, 0x76, 0x61, 0x6c, 0x0, 0x10, 0x74, 0x68, 0x69, 0x6e, 0x67, 0x0, 0xa, 0x0, 0x0, 0x0, 0x0 };
    BSONPP doc(data, sizeof(data), false);
    int64_t val;
    ASSERT_EQ(BSONPP_NULL_VALUE, doc.get("val", &val));
    ASSERT_EQ(BSONPP_SUCCESS, doc.get("thing", &val));
    ASSERT_EQ(10, val);
}

#endif // __LINUX_BUILD
