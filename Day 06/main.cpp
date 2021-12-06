#include <iostream>
#include <fstream>
#include <string>
#include <array>
#include <utility>
#include <numeric>
#include <chrono>

int main() {
	//On commence le temps
	auto start = std::chrono::steady_clock::now();

	//On charge le fichier
	std::ifstream inputFile("input.txt");

	//On lit le fichier
	std::array<std::uint64_t, 9> fishes; //Array uint16_t où fishes[0] contient le nombre de poissons ayant de temps restant 0
	fishes.fill(0);

	std::string line;
	std::getline(inputFile, line);
	std::size_t pos = -1;
	do {
		//On incrémente la position (pour passer de la virgule au nombre
		pos++;

		//On incrémente d'un le poisson de la ligne
		fishes[std::stoi(line.substr(pos))]++;

		//On trouve la nouvelle virgule
		pos = line.find(',', pos);
	} while (pos != std::string::npos);

	//On fait les 80 jours
	for (std::size_t day = 0; day < 80; day++) {
		//Chaque jour on baisse de 1 chaque poisson, pour le 0, on remet chaque poisson à 6 et met autant de poisson sur le 8
		std::uint64_t fishes0 = fishes[0];
		
		for (std::size_t i = 0; i < fishes.size()-1; i++) {
			fishes[i] = fishes[i + 1];
		}

		fishes[8] = fishes0;
		fishes[6] += fishes0;
	}

	std::uint64_t p1 = std::accumulate(fishes.begin(), fishes.end(), std::uint64_t(0));

	//On fait les jours restant pour la p2
	for (std::size_t day = 80; day < 256; day++) {
		//Chaque jour on baisse de 1 chaque poisson, pour le 0, on remet chaque poisson à 6 et met autant de poisson sur le 8
		std::uint64_t fishes0 = fishes[0];

		for (std::size_t i = 0; i < fishes.size() - 1; i++) {
			fishes[i] = fishes[i + 1];
		}

		fishes[8] = fishes0;
		fishes[6] += fishes0;
	}

	std::uint64_t p2 = std::accumulate(fishes.begin(), fishes.end(), std::uint64_t(0));

	//On finit le temps
	auto end = std::chrono::steady_clock::now();

	std::cout << "Part 1: " << p1 << std::endl;
	std::cout << "Part 2: " << p2 << std::endl;
	std::cout << "Time: " << std::chrono::duration_cast<std::chrono::microseconds>(end-start).count() << " microseconds" << std::endl;

	return 0;
}
