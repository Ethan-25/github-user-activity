#include <iostream>
#include <string>
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

std::string getUserActivity(const std::string& username) {
    std::string host = "api.github.com";
    std::string path = "/users/" + username + "/events";
    std::string request = "GET " + path + " HTTP/1.1\r\nHost: " + host + "\r\nUser-Agent: C++ CLI\r\nConnection: close\r\n\r\n";

    WSADATA wsaData;
    SOCKET sockfd;
    struct addrinfo hints, *serverInfo, *p;
    int rv;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return "";
    }

    // Setup hints
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    // Get server info
    if ((rv = getaddrinfo(host.c_str(), "80", &hints, &serverInfo)) != 0) {
        std::cerr << "getaddrinfo: " << gai_strerror(rv) << "\n";
        WSACleanup();
        return "";
    }

    // Loop through all the results and connect to the first we can
    for (p = serverInfo; p != nullptr; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == INVALID_SOCKET) {
            continue;
        }

        if (connect(sockfd, p->ai_addr, static_cast<int>(p->ai_addrlen)) == SOCKET_ERROR) {
            closesocket(sockfd);
            continue;
        }

        break;
    }

    if (p == nullptr) {
        std::cerr << "Failed to connect\n";
        freeaddrinfo(serverInfo);
        WSACleanup();
        return "";
    }

    freeaddrinfo(serverInfo);

    // Send the request
    if (send(sockfd, request.c_str(), static_cast<int>(request.length()), 0) == SOCKET_ERROR) {
        std::cerr << "Error sending request\n";
        closesocket(sockfd);
        WSACleanup();
        return "";
    }

    // Read the response
    std::string response;
    char buffer[1024];
    int bytes;
    while ((bytes = recv(sockfd, buffer, sizeof(buffer), 0)) > 0) {
        response.append(buffer, bytes);
    }

    if (bytes < 0) {
        std::cerr << "Error reading response\n";
    }

    closesocket(sockfd);
    WSACleanup();

    return response;
}

std::vector<std::string> parseActivity(const std::string& jsonResponse) {
    std::vector<std::string> activities;

    // Simple and naive parsing (assuming certain formatting)
    size_t pos = 0;
    while ((pos = jsonResponse.find("\"type\":", pos)) != std::string::npos) {
        size_t start = jsonResponse.find("\"", pos + 7) + 1;
        size_t end = jsonResponse.find("\"", start);
        std::string type = jsonResponse.substr(start, end - start);

        start = jsonResponse.find("\"repo\":", end);
        start = jsonResponse.find("\"name\":", start) + 8;
        end = jsonResponse.find("\"", start);
        std::string repo = jsonResponse.substr(start, end - start);

        activities.push_back(type + " in " + repo);
        pos = end;
    }

    return activities;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <github_username>\n";
        return 1;
    }

    std::string username = argv[1];
    std::string response = getUserActivity(username);

    if (response.empty()) {
        std::cerr << "Failed to fetch activity for user: " << username << "\n";
        return 1;
    }

    // Skip the HTTP headers
    size_t jsonStart = response.find("\r\n\r\n");
    if (jsonStart != std::string::npos) {
        response = response.substr(jsonStart + 4);
    }

    std::vector<std::string> activities = parseActivity(response);

    if (activities.empty()) {
        std::cout << "No recent activity found for user: " << username << "\n";
    } else {
        for (const auto& activity : activities) {
            std::cout << "- " << activity << "\n";
        }
    }

    return 0;
}
