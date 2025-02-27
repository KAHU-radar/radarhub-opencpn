#include "socketutils.h"
#include <thread>

Socket::Socket(const std::string& ip, int port, int min_reconnect_time, int max_reconnect_time, CancelFunction cancel_function, ConnectFunction connect_function) :
  sock(nullptr),
  reconnect_time(0),
  ip(ip),
  port(port),
  min_reconnect_time(min_reconnect_time),
  max_reconnect_time(max_reconnect_time),
  cancel_function(cancel_function),
  connect_function(connect_function),
  status("Not yet connected") { }

Socket::~Socket() {
    Close();
}

void Socket::Close() {
    if (sock) {
        std::cerr << "Deleting wxSocket " << sock << " in " << std::this_thread::get_id() << "\n";
        delete sock;
        sock = nullptr;
    }
}

void Socket::ConnectionFailure(wxSocketError error, const std::string& attempt) {
    Close();
    if (reconnect_time == 0) {
        reconnect_time = min_reconnect_time;
    } else if (reconnect_time < max_reconnect_time) {
        reconnect_time = reconnect_time * 2;
    }
    SocketException exc(error, attempt);
    status.assign(exc.what());
    throw exc;
}

bool Socket::TryCancel() {
    if (!cancel_function) return false;
    return cancel_function();
}

void Socket::Connect() {
    int slept;
    int tosleep;
    // Sleep reconnect_time ms, but check TryCancel regularly
    if (reconnect_time > 0) {
        slept = 0;
        while (!TryCancel() && (slept < reconnect_time)) {
            tosleep = reconnect_time - slept > 200 ? 200 : reconnect_time - slept;
            wxMilliSleep(tosleep);
            slept += tosleep;
        }
    }

    // If any exception is thrown from inside Connect that isn't from
    // a ConnectionFailure(), we might have an old socket here that
    // needs cleanup.
    Close();
    
    wxIPV4address serv_addr;
    if (!serv_addr.Hostname(ip)) ConnectionFailure(wxSOCKET_INVADDR, "Invalid hostname/IP");
    if (!serv_addr.Service(port)) ConnectionFailure(wxSOCKET_INVADDR, "Invalid port number");

    sock = new wxSocketClient(wxSOCKET_BLOCK);
    std::cerr << "Created wxSocket " << sock << " in " << std::this_thread::get_id() << "\n";
    sock->SetTimeout(0);
    sock->Connect(serv_addr, false);

    slept = 0;
    while (!TryCancel() && (slept < 10000)) {
        tosleep = 10000 - slept > 200 ? 200 : 10000 - slept;
        if (sock->WaitOnConnect(0, tosleep)) break;
        slept += tosleep;
    }
    
    if (!sock->IsConnected()) {
        ConnectionFailure(sock->LastError(), "Connection Failed");
    }

    status.assign("Connected");
    
    if (connect_function) connect_function();

    status.assign("Logged in");

    reconnect_time = min_reconnect_time;
}

void Socket::EnsureConnection() {
    while (!TryCancel() && !sock) {
        try {
            Connect();
        } catch (const std::exception& e) {
            std::cerr << e.what() << "\n";
            if (TryCancel()) throw e;
        }
    }
}

void Socket::Send(const std::string& data, int flags) {
    if (!sock) {
        ConnectionFailure(wxSOCKET_INVSOCK, "Socket disconnected when trying to send");
    }
    if (!sock->IsConnected()) {
        ConnectionFailure(sock->LastError(), "Socket disconnected when trying to send");
    }

    sock->Write(data.c_str(), data.size());

    if (sock->Error()) {
        ConnectionFailure(sock->LastError(), "Send failed");
    }
}

void Socket::WaitAndSendInitial(const std::string& data, int flags) {
    while (!TryCancel()) {
        EnsureConnection();
        try {
            Send(data, flags);
            return;
        } catch (const std::exception& e) {
            std::cerr << e.what() << "\n";
        }
    }
}


void Socket::ReadEnoughData(size_t size) {
    int slept = 0;
    int tosleep = 0;
    sock->SetTimeout(0);
    while ((buffer.size() < size) && (slept < 10000)) {
        if (TryCancel()) {
            ConnectionFailure(wxSOCKET_NOERROR, "Interrupted by thread termination");
        }
        char temp[4096];
        sock->Read(temp, size - buffer.size());
        slept += tosleep;
        bool isError = sock->Error();
        wxSocketError error = sock->LastError();
        size_t bytes_read = sock->LastCount();

        if (isError) {
            if (error == wxSOCKET_TIMEDOUT) {
                tosleep = 10000 - slept > 200 ? 200 : 10000 - slept;
                continue;
            } else {
                ConnectionFailure(sock->LastError(), "Socket error while reading");
            }
        }

        if (bytes_read == 0) {
            ConnectionFailure(wxSOCKET_IOERR, "read 0 bytes from socket");
        }

        slept = 0;
        buffer.insert(buffer.end(), temp, temp + bytes_read);
    }
    if (buffer.size() < size) {
        ConnectionFailure(wxSOCKET_TIMEDOUT, "Did not read enough bytes from socket before timeout");
    }
}

std::vector<char> Socket::RetrieveBytes(size_t size) {
    if (buffer.size() < size) {
        throw std::runtime_error("Not enough data in buffer");
    }

    std::vector<char> result(buffer.begin(), buffer.begin() + size);
    buffer.erase(buffer.begin(), buffer.begin() + size);
    return result;
}

const std::vector<char>& Socket::GetBuffer() const { return buffer; }

void Socket::ConsumeBytes(size_t size) {
    if (size > buffer.size()) {
        throw std::runtime_error("Cannot consume more bytes than available");
    }
    buffer.erase(buffer.begin(), buffer.begin() + size);
}
