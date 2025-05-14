#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <thread>
#include <mutex>
#include <regex>
#include <sstream>
#include <bitset>
#include <algorithm>
#include <utility>

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
//////////////////////////////////////STRUCTS////////////////////////////////
struct info {
	std::string ip;
	int port;

	info(std::string ip, int p) : ip(ip), port(p) {}
};
//////////////////////////////////////STRUCTS////////////////////////////////
//////////////////////////////////////FUNCTIONS////////////////////////////////
bool isIP(const std::string& ip) {
	std::stringstream ss(ip);
	std::string segment;
	int count = 0;

	while (std::getline(ss, segment, '.')) {
		count++;

		try {
			int num = std::stoi(segment);
			if (num < 0 || num > 255) return false;
		}
		catch (...) {
			return false;
		}
	}
	return count == 4;
}

std::vector<std::string> grabIp(int argc, char* argv[]) {
	std::string ipInput;
    std::vector<std::string> ips;
	int needThree = 0;

	
	for (int i = 1; i < argc; i++) {
		std::string arg = argv[i];

		if (arg.find('.') != std::string::npos && isIP(arg)) {
			ipInput = arg;
		}
	}

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

	// Check for hyphen
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


std::vector<int> options(int argc, char* argv[]) {
	std::vector<int> ports;

	for (int i = 1; i < argc; i++) {
		std::string arg = argv[i];
		if (arg == "-A") {
			for (int i = 0; i <= 65535; i++) {
				ports.emplace_back(i);
			};
		}
		else if (arg == "-N") {
			ports = { 7, 19, 20, 21, 22, 23, 25, 42, 43, 49, 53, 67, 68, 69, 70, 79, 80, 88, 102, 110, 113, 119, 123, 135, 137, 138, 139, 143, 161, 162, 177, 179, 194, 201, 264, 318, 381, 383, 389, 411, 412, 427, 443, 445, 464, 465, 497, 500, 512, 513, 514, 515, 520, 521, 540, 548, 554, 546, 547, 560, 563, 587, 591, 593, 596, 631, 636, 639, 646, 691, 860, 873, 902, 989, 990, 993, 995, 1025, 1026, 1027, 1028, 1029, 1080, 1194, 1214, 1241, 1311, 1337, 1589, 1701, 1720, 1723, 1725, 1741, 1755, 1812, 1813, 1863, 1900, 1985, 2000, 2002, 2049, 2082, 2100, 2222, 2302, 2483, 2484, 2745, 2967, 3050, 3074, 3127, 3128, 3222, 3260, 3306, 3389, 3689, 3690, 3724, 3784, 3785, 4333, 4444, 4500, 4664, 4672, 4899, 5000, 5001, 5004, 5005, 5050, 5060, 5061, 5190, 5222, 5223, 5353, 5432, 5554, 5631, 5632, 5800, 5900, 6000, 6001, 6112, 6129, 6257, 6346, 6347, 6379, 6500, 6566, 6588, 6665, 6666, 6667, 6668, 6669, 6679, 6697, 6699, 6891, 6982, 6983, 6984, 6985, 6986, 6987, 6988, 6989, 6990, 6901, 6970, 7000, 7001, 7199, 7648, 7649, 8000, 8080, 8086, 8087, 8118, 8200, 8222, 8500, 8767, 8866, 9042, 9100, 9101, 9102, 9103, 9119, 9800, 9898, 9999, 10000, 10161, 10162, 10113, 10114, 10115, 10115, 11371, 12345, 13720, 13721, 14567, 15118, 19226, 19638, 20000, 24800, 25999, 27015, 27017, 27374, 38960, 31337, 33434, 33435, 33436, 33437, 33438, 33439, 33440 };

			for (int x = 5901; x <= 5999; x++) {
				ports.emplace_back(x);
			};
			for (int x = 6881; x <= 6999; x++) {
				ports.emplace_back(x);
			};
		}
		else if (arg == "-help" || arg == "-Help" || arg == "-h" || arg == "-H") {
			std::cout << "ScortPanner is a work in progress port/vulnerability scanner." << std::endl;
			std::cout << "Usage: ./ScortPanner [argument] [argument]" << std::endl;
			// Gonna have to come back here and fix every fucking thing
		}
		else {
			ports = { 7, 19, 20, 21, 22, 23, 25, 42, 43, 49, 53, 67, 68, 69, 70, 79, 80, 88, 102, 110, 113, 119, 123, 135, 137, 138, 139, 143, 161, 162, 177, 179, 194, 201, 264, 318, 381, 383, 389, 411, 412, 427, 443, 445, 464, 465, 497, 500, 512, 513, 514, 515, 520, 521, 540, 548, 554, 546, 547, 560, 563, 587, 591, 593, 596, 631, 636, 639, 646, 691, 860, 873, 902, 989, 990, 993, 995, 1025, 1026, 1027, 1028, 1029, 1080, 1194, 1214, 1241, 1311, 1337, 1589, 1701, 1720, 1723, 1725, 1741, 1755, 1812, 1813, 1863, 1900, 1985, 2000, 2002, 2049, 2082, 2100, 2222, 2302, 2483, 2484, 2745, 2967, 3050, 3074, 3127, 3128, 3222, 3260, 3306, 3389, 3689, 3690, 3724, 3784, 3785, 4333, 4444, 4500, 4664, 4672, 4899, 5000, 5001, 5004, 5005, 5050, 5060, 5061, 5190, 5222, 5223, 5353, 5432, 5554, 5631, 5632, 5800, 5900, 6000, 6001, 6112, 6129, 6257, 6346, 6347, 6379, 6500, 6566, 6588, 6665, 6666, 6667, 6668, 6669, 6679, 6697, 6699, 6891, 6982, 6983, 6984, 6985, 6986, 6987, 6988, 6989, 6990, 6901, 6970, 7000, 7001, 7199, 7648, 7649, 8000, 8080, 8086, 8087, 8118, 8200, 8222, 8500, 8767, 8866, 9042, 9100, 9101, 9102, 9103, 9119, 9800, 9898, 9999, 10000, 10161, 10162, 10113, 10114, 10115, 10115, 11371, 12345, 13720, 13721, 14567, 15118, 19226, 19638, 20000, 24800, 25999, 27015, 27017, 27374, 38960, 31337, 33434, 33435, 33436, 33437, 33438, 33439, 33440 };

			for (int x = 5901; x <= 5999; x++) {
				ports.emplace_back(x);
			};
			for (int x = 6881; x <= 6999; x++) {
				ports.emplace_back(x);
			};
		};
	}

	return ports;
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
	std::vector<std::pair<std::string, int>> goodPorts;
	std::vector<std::pair<std::string, int>> badPorts;

	if (scanPort(ip, port)) {
		std::lock_guard<std::mutex> lock(outputMutex);
		std::cout << "[+] Port " << port << " is OPEN" << std::endl;
		goodPorts.emplace_back(info(ip, port));
	}
	else {
		std::lock_guard<std::mutex> lock(outputMutex);
		std::cout << "[-] Port " << port << " is closed" << std::endl;
		badPorts.emplace_back(info(ip, port));
	}

	print(goodPorts, badPorts);
};

void print(const std::vector<std::string, int>& good, const std::vector<std::string, int>& bad) {
	std::sort(good.begin(), good.end(), cmp);
	for (int i = 0; i < good.size(); i++) {
		std::cout << "[+] Port " << good[i].second << " is OPEN" << std::endl;
	}
}

bool cmp(const info& a, const info& b) {
	return a.ip < b.ip || (a.ip == b.ip && a.port < b.port);
}
//////////////////////////////////////FUNCTIONS////////////////////////////////
//////////////////////////////////////MAIN/////////////////////////////////////
int main(int argc, char* argv[]) {
#ifdef _WIN32
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		std::cerr << "WSAStartup failed." << std::endl;
		return 1;
	}
#endif

	std::vector<int> ports;
	ports = options(argc, argv);
	

	std::vector<std::string> ips = grabIp(argc, argv);

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

	return 0;
}
//////////////////////////////////////MAIN/////////////////////////////////////
