#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <tchar.h>

#ifdef _WIN32
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#pragma comment(lib, "Ws2_32.lib")
#else
	#include <unistd.h>
	#include <arpa/inet.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
#endif

std::mutex outputMutex;

//////////////////////////////////////FUNCTIONS////////////////////////////////

bool scanPort(const std::string& ip, int port) {
#ifdef _WIN32
	SOCKET sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#else
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
#endif

	if (sockfd < 0) return false;

	sockaddr_in server{};
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	InetPton(AF_INET, _T("127.0.0.1"), &server.sin_addr.s_addr);

#ifdef _WIN32
	DWORD timeout = 1000;
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
#else
	struct timeval timeout {};
	timeout.tv_sec = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
#endif

	bool isOpen = connect(sockfd, (sockaddr*)&server, sizeof(server)) == 0;

#ifdef _WIN32
	closesocket(sockfd);
#else
	close(sockfd);
#endif

	return isOpen;
};

void scanAndPrint(const std::string& ip, int port) {
	if (scanPort(ip, port)) {
		std::lock_guard<std::mutex> lock(outputMutex);
		std::cout << "[+] Port " << port << " is OPEN" << std::endl;
	}
	else {
		std::lock_guard<std::mutex> lock(outputMutex);
		std::cout << "[-] Port " << port << " is closed" << std::endl;
	}
};

//////////////////////////////////////FUNCTIONS////////////////////////////////
//////////////////////////////////////MAIN/////////////////////////////////////
int main() {
	WSADATA wsaData;
	std::string ip = "127.0.0.1";
	std::vector<int> ports;

	for (int i = 20; i <= 100; i++){
		ports.push_back(i);
	}

#ifdef _WIN32
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
			std::cerr << "WSAStartup failed." << std::endl;
			return 1;
		}
#endif

	std::vector<std::thread> threads;

	for (int port : ports) {
		threads.emplace_back(scanAndPrint, ip, port);
	}

	for (auto& t : threads) {
		if (t.joinable()) t.join();
	}

#ifdef _WIN32
	int WSACleanup();
#endif

	return 0;
}
//////////////////////////////////////MAIN/////////////////////////////////////
