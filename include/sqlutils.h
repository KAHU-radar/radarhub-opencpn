#ifndef _SQLUTILS_H_
#define _SQLUTILS_H_

#include <sqlite3.h>
#include <string>
#include <exception>

class Query {
public:
    Query(sqlite3* db, std::string query_string);
    ~Query();

    bool next();
    
    Query& bind(int param, const void* value, int len, void(*destructor)(void*));
    Query& bind(int param, const void* value, sqlite3_uint64 len, void(*destructor)(void*));
    Query& bind(int param, double value);
    Query& bind(int param, int value);
    Query& bind(int param, sqlite3_int64 value);
    Query& bind(int param);
    Query& bind(int param, const char* value, int len, void(*destructor)(void*));
    Query& bind(int param, std::string& value);
    Query& bind(int param, std::string&& value);
    Query& bindWChar(int param, const void* value, int len, void(*destructor)(void*));
    Query& bind(int param, const char*value, sqlite3_uint64 len, void(*destructor)(void*), unsigned char encoding);
    Query& bind(int param, const sqlite3_value* value);
    Query& bind(int param, void* pointer, const char*ptype,void(*destructor)(void*));
    Query& bindZeroblob(int param, int len);
    Query& bindZeroblob(int param, sqlite3_uint64 len);

    bool step();

    const void *get_blob(int iCol);
    double get_double(int iCol);
    int get_int(int iCol);
    sqlite3_int64 get_int64(int iCol);
    const unsigned char *get_text(int iCol);
    const std::string get_string(int iCol);
    const void *get_text16(int iCol);
    sqlite3_value *get_value(int iCol);
    int get_bytes(int iCol);
    int get_bytes16(int iCol);
    int get_type(int iCol);
    int changes();

    std::string query_string;
    const char *orig_query_string;
    const char *current_query_string;
    const char *remaining_query_string;
    sqlite3_stmt* stmt;
    sqlite3* db;
    bool debug = false;
};


class QueryException : public std::exception {
public:
    explicit QueryException(sqlite3* db, std::string query) {
        this->query = query;
        errcode = sqlite3_errcode(db);
        extended_errcode = sqlite3_extended_errcode(db);
        errmsg.assign(sqlite3_errmsg(db));
        if (errcode == SQLITE_MISUSE) {
            full_error = "Assertion failure: sqlite API misuse: " + errmsg + " while executing " + query;
        } else {
            full_error = errmsg + " while executing " + query;
        }
    }

    std::string query;
    int errcode;
    int extended_errcode;
    std::string errmsg;
    std::string full_error;
 
    const char* what() const noexcept override {
        return full_error.c_str();
    }
};


#endif
