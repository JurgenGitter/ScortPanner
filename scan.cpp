#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <regex>
#include <sstream>
#include <bitset>

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
std::vector<std::string> grabInfo() {
	std::string ipInput;
    std::vector<std::string> ips;

	std::cout << "Please enter the IP address (xxx.xxx.xxx.xxx) or range (xxx.xxx.xxx.xxx - xxx.xxx.xxx.xxx, xxx.xxx.xxx.xxx/xx): ";
	std::cin >> ipInput;

	// Check for CIDR notation
	if (ipInput.find('/') != std::string::npos) {
		std::string baseIP = ipInput.substr(0, ipInput.find('/'));
		int prefix = std::stoi(ipInput.substr(ipInput.find('/') + 1));


		in_addr addr;
		inet_pton(AF_INET, baseIP.c_str(), &addr);
		uint32_t ipAdd = ntohl(addr.s_addr);
		int hostBits = 32 - prefix;
		int numHosts = 1 << hostBits;

		uint32_t network = ipAdd & (~((1 << hostBits) - 1));
		for (int i = 1; i < numHosts - 1; i++) {
			in_addr tmp{};
			tmp.s_addr = htonl(network + i);
			char buffer[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &tmp, buffer, INET_ADDRSTRLEN);
			ips.push_back(buffer);
		}
	}

	// Check for -
	else if (ipInput.find('-') != std::string::npos) {
		auto dashPos = ipInput.find('-');
		std::string startIP = ipInput.substr(0, dashPos);
		std::string endIP = ipInput.substr(dashPos + 1);

		in_addr start{}, end{};
		inet_pton(AF_INET, startIP.c_str(), &start);
		inet_pton(AF_INET, endIP.c_str(), &end);

		uint32_t startInt = ntohl(start.s_addr);
		uint32_t endInt = ntohl(end.s_addr);

		for (uint32_t i = startInt; i <= endInt; i++) {
			in_addr addr{};
			addr.s_addr = htonl(i);
			char buf[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &addr, buf, INET_ADDRSTRLEN);
			ips.push_back(buf);
		}
	}

	else {
		ips.push_back(ipInput);
	}

	return ips;
}

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
		
#ifdef _WIN32
		inet_pton(AF_INET, ip.c_str(), &server.sin_addr);
#else
		inet_pton(AF_INET, ip.c_str(), &server.sin_addr);
#endif

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
#ifdef _WIN32
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		std::cerr << "WSAStartup failed." << std::endl;
		return 1;
	}
#endif

	std::vector<int> ports;
	for (int i = 0; i <= 65535; i++){
		ports.push_back(i);
	}

	std::vector<std::string> ips = grabInfo();

	std::vector<std::thread> threads;
	for (const auto& ip : ips) {
		for (int port : ports) {
			threads.emplace_back(scanAndPrint, ip, port);
		}
	}

	for (auto& t : threads) {
		if (t.joinable()) t.join();
	}

#ifdef _WIN32
	WSACleanup();
#endif

	return 0;
}
//////////////////////////////////////MAIN/////////////////////////////////////
