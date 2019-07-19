# C++ BSON Serialization Library
BSONPP is a BSON serialization and parser library designed to be used on devices with limited memory.
At the core of this is that it does no dynamic memory allocation. This means that documents can only be appended to and not modified after creation.

## Supported Types
* Double (devices which only support floats are auto-converted).
* String.
* Sub-documents.
* Arrays (sort of). Arrays are standard BSON documents with string numbers are keys.
* Binary.
* Boolean.
* Datetime.
* Int32.
* Int64.
* Null (partial). The library copes with parsing null values but doesn't support serializing them.

## Limitations
* Appending to a sub-document after it's been added to a parent object will not add to the parent copy.
* Arrays are a little awkward to work with.
* The following types aren't supported: ObjectID, Regular Expression, DBPointer, JavaScript code, JavaScript code w/ scope, uint64 timestamps, 128-bit decimal floating point, min key, and max key. Timestamps are MongoDB specific.
* Only generic binary sub-types are supported.

## Usage
Please see the [header file](src/BSONPP.h) and [test file](Test.cpp) in this repository.

A basic example for creating and using a document:
```
// This is the buffer to be used by the library.
uint8_t buffer[256];
BSONPP doc(buffer, sizeof(buffer));
doc.append("stringKey", "Just a medium length string");
doc.append("num", 10234234);

uint8_t subBuffer[256];
BSONPP subDoc(subBuffer, sizeof(subBuffer));
subDoc.append("subVal", 0.2343);

// When the sub-doc is appended to a document it's buffer is copied.
doc.append("subDoc", subDoc);

// You can do this call to get the amount of the buffer used
doc.getSize();

// To read BSON from a buffer pass in the buffer, length and false. By default when passed
// a buffer it clears the buffer and initialises a new BSON object. Set false to avoid that.
BSONPP parsed(buffer, sizeof(buffer), false);

char *val;
if (BSONPP_SUCCESS == parsed.get("stringKey", val)) {
    printd("%\n", val);
}

// To fetch the sub-document do the following. It will tell the sub-doc to use the same buffer
// used by the parent object.
BSONPP parsedSubDoc;
parsed.get("subDoc", &parsedSubDoc);
```

Arrays work like this:
```
uint8_t buffer[256];
BSONPP doc(buffer, sizeof(buffer));

uint8_t arrBuffer[256];
BSONPP arr(arrBuffer, sizeof(arrBuffer));
arr.append("0", "first");
arr.append("1", "second");
arr.append("2", "third");

// Set true to append a document that's an array.
doc.append("arrList", arr, true);
```

### Return Values
All of the append and get functions return either:
* BSONPP_SUCCESS
* BSONPP_KEY_NOT_FOUND
* BSONPP_INCORRECT_TYPE (If you try to get an int when the data's a string)
* BSONPP_OUT_OF_SPACE (If the documents buffer will not fit the element)
* BSONPP_NO_BUFFER (If a document doesn't have a buffer)
* BSONPP_DUPLICATE_KEY (If you try to set the same key twice)
* BSONPP_NULL_VALUE (If you try to get a value which is null)

### Getting Datetime
To fetch the Datetime type use the getter for int64_t.

### Iteration/Introspection
To see how many keys, or get the names of keys from an object you can use the following example. Be warned though that it's designed for simplicity rather than speed so foregoes an iterator.
```
uint8_t buffer[256];
BSONPP doc(buffer, sizeof(buffer));
doc.append("stringKey", "Just a medium length string");
doc.append("num", 10234234);

int32_t count;
// You should check the return value of these functions
getKeyCount(&count);

for (int32_t i = 0; i < count; i++) {
	char *key;
    uint8_t type;
    doc.getKeyAt(0, &key);
    doc.getTypeAt(0, &type);
	printf("Key: %s, Type: %d\n", key, type);
}

// Additionally there's also an exists function, note that this will return false if the value is null.
if (doc.exists("num")) {
  ...Do something
}
```

### Clearing/Resetting an Object
An empty BSON object looks like this as a byte array [0x05, 0x00, 0x00, 0x00, 0x00]. What this means is that if you pass in a zeroed array bad things will happen. To minimise the number of bad things happening the default constructor for BSONPP initialises the object. This means that when parsing a buffer you must be sure to pass `false` as the last argument of the constructor.
An object can also manually be reset by calling `.clear()`.
