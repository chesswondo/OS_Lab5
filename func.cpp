#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <thread>
#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")


using namespace std;

// Function f
int f(int x)
{
    this_thread::sleep_for(28s);
    return 421;
}
// Function g
int g(int x)
{
    this_thread::sleep_for(3s);
    return 0;
}

int main(int argc, char * argv[])
{
    if (argc < 3)
    {
        cerr << "Usage: funt port f|g\n";
        return 1;
    }
    // Command-line options
    unsigned short int port = (unsigned short int)atoi(argv[1]);
    int (*func)(int) = (argv[2][0] == 'f') ? f : g;

    SOCKET ms = INVALID_SOCKET, cs = INVALID_SOCKET;
    try {
        WORD wVersionRequested = MAKEWORD(1, 1);
        WSADATA wsaData;
        if (WSAStartup(wVersionRequested, &wsaData) != 0)
            throw runtime_error("WSAStartup");

        // Socket creation
        ms = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (ms == INVALID_SOCKET) throw runtime_error("socket creation");

        // bind to port 
        sockaddr_in name;
        memset(&name, 0, sizeof(name));
        name.sin_addr.s_addr = INADDR_ANY;
        name.sin_family = AF_INET;
        name.sin_port = htons(port);
        if (SOCKET_ERROR == bind(ms, (SOCKADDR*)&name, sizeof(name)))
            throw runtime_error("bind");

        if (listen(ms, 1) != 0) throw runtime_error("listen");

        // Wait for connect
        cs = accept(ms, NULL, NULL);
        if (cs == INVALID_SOCKET) throw runtime_error("accept");

        // Receive x and ...
        int x;
        int rc = recv(cs, (char*)&x, sizeof(x), 0);
        if (rc < 0) throw runtime_error("recv data");
        if (rc == 0) throw runtime_error("connection closed");;

        // ...caclulation...
        rc = func(x);

        // ...and send answer
        rc = send(cs, (char*)&rc, sizeof(rc), 0);
        if (rc != sizeof(rc)) throw("send data");

        closesocket(cs);
    }
    catch (exception& e)
    {
        cerr << "Error: " << e.what() << endl;
    }
    closesocket(ms);
    closesocket(cs);
    WSACleanup();
}

