#include "avroutils.h"

AvroSchema::AvroSchema(std::string schema_json) : iface(nullptr) {
    avro_schema_error_t error;
    if (avro_schema_from_json(schema_json.c_str(), strlen(schema_json.c_str()), &schema, &error) != 0) {
        throw AvroException("Unable to parse schema: " + schema_json);
    }
    iface = avro_generic_class_from_schema(schema);
}

AvroSchema::~AvroSchema() {
    avro_value_iface_decref(iface);
    avro_schema_decref(schema);
}

AvroMemoryWriter::AvroMemoryWriter(AvroValue& value) : value(value) {
    writer = avro_writer_memory(buffer, sizeof(buffer));

    if (avro_value_write(writer, &value.value) != 0) {
        throw AvroException("Failed to serialize value");
    }
}

AvroMemoryWriter::~AvroMemoryWriter() {
    avro_writer_free(writer);
}


AvroValueFromSchema::AvroValueFromSchema(AvroSchema& schema) : schema(schema) {
    avro_generic_value_new(schema.iface, &value);
}

AvroValueFromSchema::AvroValueFromSchema(AvroSchema& schema, Socket& socket) : schema(schema) {
    avro_generic_value_new(schema.iface, &value);
    while (!socket.TryCancel()) {
        const std::vector<char>& raw_buffer = socket.GetBuffer();
        if (raw_buffer.size() > 0) {
            avro_reader_t reader = avro_reader_memory(raw_buffer.data(), raw_buffer.size());
            if (avro_value_read(reader, &value) == 0) {
                // How reliable is this? Is this guaranteed to be the same size as what was read?
                size_t used_bytes;
                avro_value_sizeof(&value, &used_bytes);
                avro_reader_free(reader);
                socket.ConsumeBytes(used_bytes);
                return;
            }
            avro_reader_free(reader);
        }
        socket.ReadEnoughData(raw_buffer.size() + 1);
        avro_value_reset(&value);
    }
    throw AvroException("Interrupted by thread termination");
}

AvroValueFromSchema::~AvroValueFromSchema() {
    avro_value_decref(&value);
}


AvroValue::AvroValue() {
}

AvroValue::~AvroValue() {
}

void AvroValue::Failure(std::string attempt) {
    throw AvroException(attempt + " (in " + GetTypeName() + ")");
}

std::string AvroValue::Serialize() {
    AvroMemoryWriter writer(*this);
    return std::string(writer.buffer, avro_writer_tell(writer.writer));
}

avro_type_t AvroValue::GetType() { return avro_value_get_type(&value); }

std::string AvroValue::GetTypeName() {
    avro_type_t type = GetType();
    switch (type) {
        case AVRO_STRING: return "AVRO_STRING";
        case AVRO_BYTES: return "AVRO_BYTES";
        case AVRO_INT32: return "AVRO_INT32";
        case AVRO_INT64: return "AVRO_INT64";
        case AVRO_FLOAT: return "AVRO_FLOAT";
        case AVRO_DOUBLE: return "AVRO_DOUBLE";
        case AVRO_BOOLEAN: return "AVRO_BOOLEAN";
        case AVRO_NULL: return "AVRO_NULL";
        case AVRO_RECORD: return "AVRO_RECORD";
        case AVRO_ENUM: return "AVRO_ENUM";
        case AVRO_FIXED: return "AVRO_FIXED";
        case AVRO_MAP: return "AVRO_MAP";
        case AVRO_ARRAY: return "AVRO_ARRAY";
        case AVRO_UNION: return "AVRO_UNION";
        case AVRO_LINK: return "AVRO_LINK";
        default: return "UNKNOWN_TYPE";
    }
}

avro_schema_t AvroValue::GetSchema() { return avro_value_get_schema(&value); }
bool AvroValue::Equal(AvroValue other) { return avro_value_equal(&value, &other.value); }
uint32_t AvroValue::Hash() { return avro_value_hash(&value); }
void AvroValue::Reset() { if (avro_value_reset(&value) != 0) Failure("Failed to reset value"); }

bool AvroValue::GetBoolean() {
    int dest;
    if (avro_value_get_boolean(&value, &dest) != 0)
        Failure("Failed to get bool value");
    return dest;
}
std::string AvroValue::GetBytes() {
    const void *dest;
    size_t size;
    if (avro_value_get_bytes(&value, &dest, &size) != 0)
        Failure("Failed to get bytes value");
    return std::string((char *) dest, size);
}
double AvroValue::GetDouble() {
    double dest;
    if (avro_value_get_double(&value, &dest) != 0)
        Failure("Failed to get bytes value");
    return dest;
}
float AvroValue::GetFloat() {
    float dest;
    if (avro_value_get_float(&value, &dest) != 0)
        Failure("Failed to get float value");
    return dest;
}
int32_t AvroValue::GetInt() {
    int32_t dest;
    if (avro_value_get_int(&value, &dest) != 0)
        Failure("Failed to get int value");
    return dest;
}
int64_t AvroValue::GetLong() {
    int64_t dest;
    if (avro_value_get_long(&value, &dest) != 0)
        Failure("Failed to get long value");
    return dest;
}
void AvroValue::GetNull() {
    if (avro_value_get_null(&value) != 0)
        Failure("Failed to get null value");
}
std::string AvroValue::GetString() {
    const char *dest;
    size_t size;
    if (avro_value_get_string(&value, &dest, &size) != 0)
        Failure("Failed to get string value");
    return std::string(dest, size);
}

int AvroValue::GetEnum() {
    int dest;
    if (avro_value_get_enum(&value, &dest) != 0)
        Failure("Failed to get enum value");
    return dest;
}
std::string AvroValue::GetFixed() {
    const char *dest;
    size_t size;
    if (avro_value_get_fixed(&value, (const void **) &dest, &size) != 0)
        Failure("Failed to get bytes value");
    return std::string(dest, size);
}

void AvroValue::Set(bool src)         { if (avro_value_set_boolean(&value, (int) src) != 0) Failure("Failed to set bool value"); }
void AvroValue::SetBytes(std::string src) { if (avro_value_set_bytes(&value, (void *) src.c_str(), src.size()) != 0) Failure("Failed to set bytes value"); }
void AvroValue::Set(double src)      { if (avro_value_set_double(&value, src) != 0) Failure("Failed to set double value"); }
void AvroValue::Set(float src)       { if (avro_value_set_float(&value, src) != 0) Failure("Failed to set float value"); }
void AvroValue::Set(int32_t src)     { if (avro_value_set_int(&value, src) != 0) Failure("Failed to set int32 value"); }
void AvroValue::Set(int64_t src)     { if (avro_value_set_long(&value, src) != 0) Failure("Failed to set int64 value"); }
void AvroValue::Set()                { if (avro_value_set_null(&value) != 0) Failure("Failed to set null value"); }
void AvroValue::Set(std::string src) { if (avro_value_set_string(&value, src.c_str()) != 0) Failure("Failed to set string value"); }
void AvroValue::SetEnum(int src)     { if (avro_value_set_enum(&value, src) != 0) Failure("Failed to set enum value"); }
void AvroValue::SetFixed(std::string src) { if (avro_value_set_fixed(&value, (void *) src.c_str(), src.size()) != 0) Failure("Failed to set fixed value"); }


size_t AvroValue::GetSize() { size_t dest; if (avro_value_get_size(&value, &dest) != 0) Failure("Failed to get size"); return dest; }
AvroValue AvroValue::Append(size_t *new_index) {
    AvroValue res;
    if (avro_value_append(&value, &res.value, new_index) != 0)
        Failure("Failed to append value");
    return res;
}
AvroValue AvroValue::Get(std::string name, size_t *index) {
    AvroValue res;
    if (avro_value_get_by_name(&value, name.c_str(), &res.value, index) != 0)
        Failure("Failed to get by name");
    return res;
}
AvroValue AvroValue::Get(ssize_t index) {
    AvroValue res;
    if (index < 0) { index = GetSize() + index; }
    if (avro_value_get_by_index(&value, (size_t) index, &res.value, NULL) != 0)
        Failure("Failed to get by index");
    return res;
}
AvroValue AvroValue::Get(ssize_t index, std::string& key) {
    AvroValue res;
    const char *keyres;
    if (index < 0) { index = GetSize() + index; }
    if (avro_value_get_by_index(&value, (size_t) index, &res.value, &keyres) != 0)
        Failure("Failed to get by index");
    key.assign(keyres);
    return res;
}
AvroValue AvroValue::Add(std::string name, size_t *index, int *is_new) {
    AvroValue res;
    if (avro_value_add(&value, name.c_str(), &res.value, index, is_new) != 0)
        Failure("Failed to get by name (" + name + ")");
    return res;
}

int AvroValue::GetDiscriminant() {
    int dest;
    if(avro_value_get_discriminant(&value, &dest) != 0)
        Failure("Failed to get discriminant");
    return dest;
}
AvroValue AvroValue::Get() {
    AvroValue res;

    if (avro_value_get_current_branch(&value, &res.value) != 0)
        Failure("Failed to get current branch");
    return res;
}
AvroValue AvroValue::SetCurrentBranch(int discriminant) {
    AvroValue res;

    if (avro_value_set_branch(&value, discriminant, &res.value) != 0)
        Failure("Failed to set current branch");
    return res;
}


void AvroValue::Debug(std::string path) {
    avro_type_t type = GetType();
    
    if (type == AVRO_UNION) {
        if (GetDiscriminant() == -1)
           Failure("Current branch not set: " + path);
        Get().Debug(path + "{" + std::to_string(GetDiscriminant()) + "}");
    }
    else if ((type == AVRO_RECORD) || (type == AVRO_MAP)) {
        int size = GetSize();
        for (int i = 0; i < size; i++) {
            std::string key;
            Get(i, key).Debug(path + "." + key);
        }
    }
    else if (type == AVRO_ARRAY) {
        int size = GetSize();
        for (int i = 0; i < size; i++) {
            Get(i).Debug(path + "[" + std::to_string(i) + "]");
        }
    }
}
