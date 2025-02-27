#ifndef _SOCKETUTILS_H_
#define _SOCKETUTILS_H_

#include <wx/wx.h>
#include <wx/socket.h>
#include <wx/utils.h> 
#include <stdexcept>
#include <string.h>
#include <functional>
#include <vector>

typedef std::function<bool()> CancelFunction;
typedef std::function<void()> ConnectFunction;

class Socket {
private:
    wxSocketClient* sock;
    int reconnect_time;
    std::vector<char> buffer;
public:
    std::string ip;
    int port;
    int min_reconnect_time;
    int max_reconnect_time;
    CancelFunction cancel_function;
    ConnectFunction connect_function;
    std::string status;
    Socket(const std::string& ip, int port, int min_reconnect_time, int max_reconnect_time, CancelFunction cancel_function = nullptr, ConnectFunction connect_function = nullptr);
    ~Socket();
    bool TryCancel();
    void Close();
    void Connect();
    void EnsureConnection();
    void ConnectionFailure(wxSocketError error, const std::string&);
    void Send(const std::string& data, int flags = 0);
    void WaitAndSendInitial(const std::string& data, int flags = 0);
    void ReadEnoughData(size_t size);
    std::vector<char> RetrieveBytes(size_t size);
    const std::vector<char>& GetBuffer() const;
    void ConsumeBytes(size_t size);
};


class SocketException : public std::exception {
public:
    explicit SocketException(wxSocketError error, std::string attempt) {
        this->attempt = attempt;
        this->error = error;
        
        std::string errstr;
        switch (error)
         {
         case wxSOCKET_NOERROR:    errstr = "No error."; break;
         case wxSOCKET_INVOP:      errstr = "Invalid operation."; break;
         case wxSOCKET_IOERR:      errstr = "Input/output error."; break;
         case wxSOCKET_INVADDR:    errstr = "Invalid address."; break;
         case wxSOCKET_INVSOCK:    errstr = "Invalid socket."; break;
         case wxSOCKET_NOHOST:     errstr = "Host not found."; break;
         case wxSOCKET_INVPORT:    errstr = "Invalid port."; break;
         case wxSOCKET_WOULDBLOCK: errstr = "Operation would block."; break;
         case wxSOCKET_TIMEDOUT:   errstr = "Operation timed out."; break;
         case wxSOCKET_MEMERR:     errstr = "Memory error."; break;
         default:                  errstr = "Unknown error."; break;
        }
        
        full_error = attempt + ": " + errstr;
    };
    
    wxSocketError error;
    std::string attempt;
    std::string full_error;
 
    const char* what() const noexcept override {
        return full_error.c_str();
    }
};

#endif
