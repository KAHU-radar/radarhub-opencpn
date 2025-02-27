#include "sqlutils.h"
#include <iostream>

Query::Query(sqlite3* db, std::string query_string) {
    this->db = db;
    this->query_string = query_string;
    this->orig_query_string = this->query_string.c_str();
    this->remaining_query_string = this->orig_query_string;
    this->current_query_string = this->orig_query_string;
    stmt = nullptr;
    next();
}
Query::~Query() {
    sqlite3_finalize(stmt); // Can't throw in a destructor if != SQLITE_OK
    stmt = nullptr;
}

bool Query::next(void) {
    while (isspace(remaining_query_string[0])) remaining_query_string++;
    if (!remaining_query_string[0]) return false;
    current_query_string = remaining_query_string;
    const char *leftover;
    if (stmt != nullptr) sqlite3_finalize(stmt);
    stmt = nullptr;
    int res = sqlite3_prepare_v2(db, remaining_query_string, -1, &stmt, &leftover);
    if (   (stmt != nullptr)
        && (res != SQLITE_OK)
        && (res != SQLITE_ROW)
        && (res != SQLITE_DONE)) {
        throw QueryException(db, std::string(current_query_string));
    }
    remaining_query_string = leftover;
    return true;
}

Query& Query::bind(int param, const void* value, int len, void(*destructor)(void*)) {
    int res = sqlite3_bind_blob(stmt, param, value, len, destructor);
    if (res != SQLITE_OK) throw QueryException(db, current_query_string);
    return *this;
}

Query& Query::bind(int param, const void* value, sqlite3_uint64 len, void(*destructor)(void*)) {
    int res = sqlite3_bind_blob64(stmt, param, value, len, destructor);
    if (res != SQLITE_OK) throw QueryException(db, current_query_string);
    return *this;
}

Query& Query::bind(int param, double value) {
    int res = sqlite3_bind_double(stmt, param, value);
    if (res != SQLITE_OK) throw QueryException(db, current_query_string);
    return *this;
}

Query& Query::bind(int param, int value) {
 int res = sqlite3_bind_int(stmt, param, value);
    if (res != SQLITE_OK) throw QueryException(db, current_query_string);
    return *this;
}

Query& Query::bind(int param, sqlite3_int64 value) {
    int res = sqlite3_bind_int64(stmt, param, value);
    if (res != SQLITE_OK) throw QueryException(db, current_query_string);
    return *this;
}

Query& Query::bind(int param) {
    int res = sqlite3_bind_null(stmt, param);
    if (res != SQLITE_OK) throw QueryException(db, current_query_string);
    return *this;
}

Query& Query::bind(int param, const char* value, int len, void(*destructor)(void*)) {
    int res = sqlite3_bind_text(stmt, param, value, len, destructor);
    if (res != SQLITE_OK) throw QueryException(db, current_query_string);
    return *this;
}

static void std_string_ref_destructor(void*) {}
Query& Query::bind(int param, std::string& value) {
    int res = sqlite3_bind_text(stmt, param, value.c_str(), -1, &std_string_ref_destructor);
    if (res != SQLITE_OK) throw QueryException(db, current_query_string);
    return *this;
}
Query& Query::bind(int param, std::string&& value) {
    return bind(param, value);
}

Query& Query::bindWChar(int param, const void* value, int len, void(*destructor)(void*)) {
    int res = sqlite3_bind_text16(stmt, param, value, len, destructor);
    if (res != SQLITE_OK) throw QueryException(db, current_query_string);
    return *this;
}

Query& Query::bind(int param, const char*value, sqlite3_uint64 len, void(*destructor)(void*), unsigned char encoding) {
    int res = sqlite3_bind_text64(stmt, param, value, len, destructor, encoding);
    if (res != SQLITE_OK) throw QueryException(db, current_query_string);
    return *this;
}

Query& Query::bind(int param, const sqlite3_value* value) {
    int res = sqlite3_bind_value(stmt, param, value);
    if (res != SQLITE_OK) throw QueryException(db, current_query_string);
    return *this;
}

Query& Query::bind(int param, void* pointer, const char*ptype,void(*destructor)(void*)) {
    int res = sqlite3_bind_pointer(stmt, param, pointer, ptype, destructor);
    if (res != SQLITE_OK) throw QueryException(db, current_query_string);
    return *this;
}

Query& Query::bindZeroblob(int param, int len) {
    int res = sqlite3_bind_zeroblob(stmt, param, len);
    if (res != SQLITE_OK) throw QueryException(db, current_query_string);
    return *this;
}

Query& Query::bindZeroblob(int param, sqlite3_uint64 len) {
    int res = sqlite3_bind_zeroblob64(stmt, param, len);
    if (res != SQLITE_OK) throw QueryException(db, current_query_string);
    return *this;
}

bool Query::step() {
    int res = sqlite3_step(stmt);

    if (debug) {
        const char *expanded_sql = sqlite3_expanded_sql(stmt);
        if (expanded_sql) {
           std::cerr << "Expanded SQL: " << expanded_sql << "\n";
           sqlite3_free((void *)expanded_sql);
        }
    }
    
    if (res == SQLITE_ROW) return true;
    if (res == SQLITE_DONE) return false;
    if (res == SQLITE_OK) return false;
    throw QueryException(db, current_query_string);
}

const void *Query::get_blob(int iCol) {
    return sqlite3_column_blob(stmt, iCol);
}

double Query::get_double(int iCol) {
    return sqlite3_column_double(stmt, iCol);
}

int Query::get_int(int iCol) {
    return sqlite3_column_int(stmt, iCol);
}

sqlite3_int64 Query::get_int64(int iCol) {
    return sqlite3_column_int64(stmt, iCol);
}

const unsigned char *Query::get_text(int iCol) {
    return sqlite3_column_text(stmt, iCol);
}

const std::string Query::get_string(int iCol) {
    return std::string(reinterpret_cast<const char*>(get_text(iCol)));
}

const void *Query::get_text16(int iCol) {
    return sqlite3_column_text16(stmt, iCol);
}

sqlite3_value *Query::get_value(int iCol) {
    return sqlite3_column_value(stmt, iCol);
}

int Query::get_bytes(int iCol) {
    return sqlite3_column_bytes(stmt, iCol);
}

int Query::get_bytes16(int iCol) {
    return sqlite3_column_bytes16(stmt, iCol);
}

int Query::get_type(int iCol) {
    return sqlite3_column_type(stmt, iCol);}


int Query::changes() {
    return sqlite3_changes(db);
}
