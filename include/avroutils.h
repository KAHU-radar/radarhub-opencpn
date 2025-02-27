#ifndef _AVROUTILS_H_
#define _AVROUTILS_H_

#include <avro.h>
#include <string>
#include "socketutils.h"

class AvroValue;

class AvroSchema {
public:
    avro_schema_t schema;
    avro_value_iface_t *iface;
    AvroSchema(std::string schema_json);
    ~AvroSchema();
};

class AvroValue {
public:
    avro_value_t value;

    AvroValue();
    ~AvroValue();
    std::string Serialize();
    avro_type_t GetType();
    std::string GetTypeName();
    avro_schema_t GetSchema();
    bool Equal(AvroValue other);
    uint32_t Hash();
    void Reset();
    bool GetBoolean();
    std::string GetBytes();
    double GetDouble();
    float GetFloat();
    int32_t GetInt();
    int64_t GetLong();
    void GetNull();
    std::string GetString();
    int GetEnum();
    std::string GetFixed();
    size_t GetSize();
    AvroValue Get(std::string name, size_t *index=NULL);
    AvroValue Get(ssize_t index);
    AvroValue Get(ssize_t index, std::string& key);
    int GetDiscriminant();
    AvroValue Get();
    
    void Set(bool src);
    void Set(double src);
    void Set(float src);
    void Set(int32_t src);
    void Set(int64_t src);
    void Set();
    void Set(std::string src);
    void SetEnum(int src);
    void SetBytes(std::string src);
    void SetFixed(std::string src);
    
    AvroValue Append(size_t *new_index=NULL);
    AvroValue Add(std::string name, size_t *index=NULL, int *is_new=NULL);
    AvroValue SetCurrentBranch(int discriminant);
    void Debug(std::string path = "");

private:
    void Failure(std::string attempt);
};

class AvroValueFromSchema : public AvroValue {
public:
    AvroSchema& schema;

    AvroValueFromSchema(AvroSchema& schema);
    AvroValueFromSchema(AvroSchema& schema, Socket& socket);
    ~AvroValueFromSchema();
};

class AvroMemoryWriter {
public:
    AvroValue& value;
    avro_writer_t writer;
    char buffer[4096];

     AvroMemoryWriter(AvroValue& value);
     ~AvroMemoryWriter();
};




class AvroException : public std::exception {
public:
    explicit AvroException(std::string context) {
        full_error = context + ": " + avro_strerror();
    }

    std::string full_error;
 
    const char* what() const noexcept override {
        return full_error.c_str();
    }
};


#endif
