// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#define  _CRT_SECURE_NO_WARNINGS
#define  _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <winsock2.h>
#include <thread>
#include <mutex>

#pragma comment(lib,"ws2_32.lib")

using namespace std;

mutex outmutex;

atomic<bool> received[2] = { false, false };
atomic<bool> result[2]   = { false, false };


void getValue(int x, unsigned short int port, int index)
{
    // Создание сокета
    SOCKET s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET) throw runtime_error("socket creation");

    // Вызов
    sockaddr_in name;
    memset(&name, 0, sizeof(name));
    name.sin_addr.s_addr = inet_addr("127.0.0.1");
    name.sin_family = AF_INET;
    name.sin_port = htons(port);
    if (SOCKET_ERROR == connect(s, (SOCKADDR*)&name, sizeof(name)))
        throw runtime_error("connect");

    {
        lock_guard<mutex> lock(outmutex);
        cout << "Send " << x << " to port " << port << "\n";
    }

    if (send(s,(char*)&x,sizeof(x),0) != sizeof(x)) throw runtime_error("send");

    // Получение ответа
    if (recv(s,(char*)&x,sizeof(x),0) != sizeof(x)) throw runtime_error("recv");

    {
        lock_guard<mutex> lock(outmutex);
        cout << "Recv " << x << " from port " << port << "\n";
    }
    result[index]   = bool(x);
    received[index] = true;
    closesocket(s);
}

int main(int argc, char * argv[])
{
    int x = 1, y = 2;
    if (argc > 1) x = atoi(argv[1]);
    if (argc > 2) y = atoi(argv[2]);

    unsigned short int portF = 20120, portG = 20121;
    char portS[20];
    sprintf(portS,"%hd",portF);
    _spawnl(_P_NOWAIT,"func.exe","func.exe",portS,"f",0);
    sprintf(portS,"%hd",portG);
    _spawnl(_P_NOWAIT,"func.exe","func.exe",portS,"g",0);

    WORD wVersionRequested = MAKEWORD(1, 1);
    WSADATA wsaData;
    try {

        if (WSAStartup(wVersionRequested, &wsaData) != 0)
            throw runtime_error("WSAStartup");

        thread(getValue,x,portF,0).detach();
        thread(getValue,y,portG,1).detach();

        int Result = -1;
        bool quest = true;

        for(int i = 1;;++i)
        {
            if (received[0] && received[1])
            {
                Result = result[0] || result[1];
            }
            else if (received[0] && result[0])
            {
                Result = 1;
            }
            else if (received[1] && result[1])
            {
                Result = 1;
            }

            if (Result != -1)
            {
                lock_guard<mutex> lock(outmutex);
                cout << "f(" << x << ") || g(" << y << ") == "
                    << (Result ? "true\n" : "false\n");
                break;
            }

            this_thread::sleep_for(1s);
            if (i%10 == 0 && quest)
            {
                lock_guard<mutex> lock(outmutex);
                cout << "Too long wait. 1 - abort, 2 - retry, 3 - no more questions: ";
                int n;
                cin >> n;
                if (n == 3) quest = false;
                else if (n == 2) cout << "retry\n";
                else if (n == 1) throw runtime_error("user choose abort");
                else cout << "wrong choice, retry\n";
            }
        }

    }
    catch (exception& e)
    {
        cerr << "Error: " << e.what() << endl;
    }
    cout << "Bye!\n";
    WSACleanup();
    system("taskkill /IM func.exe /F >nul 2>&1");
}
