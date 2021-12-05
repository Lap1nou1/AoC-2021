#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>
#include <utility>
#include <chrono>

int main() {
	//On prend le temps du début
	auto start = std::chrono::high_resolution_clock().now();

	//On charge le fichier
	std::ifstream inputFile("input.txt");

	//On crée la variable de la position (x, depth) (pour p1 & 2) et aim
	std::pair<std::uint16_t, std::uint16_t> position(0, 0);
	std::pair<std::uint32_t, std::uint32_t> position2(0, 0);
	std::uint16_t aim = 0;

	//On regarde ligne par ligne les instructions
	for (std::string line; std::getline(inputFile, line);) {
		//D'abord on prend le "changement" (de combien on avance/monte/descend)
		std::uint16_t change = std::stoi(line.substr(line.find(' ')));

		//On regarde la première lettre pour avoir l'instruction
		switch (line[0]) {
		//Forward
		case 'f':
			position.first += change;
			position2.first += change;
			position2.second += change*aim;
			break;
		//Up
		case 'u':
			position.second -= change;
			aim -= change;
			break;
		//Down
		case 'd':
			position.second += change;
			aim += change;
			break;
		}
	}

	//On arrête le temps
	auto end = std::chrono::high_resolution_clock().now();

	std::cout << position.first << " " << position.second << std::endl;
	std::cout << "Part 1: " << position.first * position.second << std::endl;
	std::cout << position2.first << " " << position2.second << std::endl;
	std::cout << "Part 2: " << position2.first * position2.second << std::endl;
	std::cout << "time: " << std::chrono::duration_cast<std::chrono::microseconds>(end-start).count() << " microseconds" << std::endl;

	return 0;
}
