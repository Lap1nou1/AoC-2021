#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <string>
#include <cstdint>
#include <chrono>

int main() {
	//On prend le temps du début
	auto start = std::chrono::high_resolution_clock().now();

	//On charge le fichier
	std::ifstream inputFile("input.txt");

	if (!inputFile.is_open()) {
		return -1;
	}

	//On lit le fichier en mettant tout dans un vector et en faisant la première question
	std::int16_t part1 = 0; //Part 1
	std::array<std::int16_t, 3> lastValues = {-1,-1,-1}; //Dernière valeurs prises par depthsMeasurements
	std::int16_t part2 = 0; //Part 2
	std::int16_t sumOfDepths = 0;

	for (std::string line; std::getline(inputFile, line);) {
		if (line[0] < '0' || line[0] > '9') {
			continue;
		}

		std::int16_t depth = std::stoi(line);

		//On compte le nombre d'augmentation, que si le premier élement des trois derniers est différent de -1, donc défini
		if (lastValues[0] != -1) {
			if (lastValues[0] < depth) {
				part1++;
			}
		}

		//On compare la somme que si on a le 3ème dernier élément différend de -1, donc si on en a passé 3
		if (lastValues[2] != -1) {
			//Comparé A + B + C à B + C + D revient à comparé A et D
			if (depth > lastValues[2]) {
				part2++;
			}
		}

		//On décale tous les nombres puis ajoute notre nouvelle valeur
		lastValues[2] = lastValues[1];
		lastValues[1] = lastValues[0];
		lastValues[0] = depth;
	}

	//On arrête le temps
	auto end = std::chrono::high_resolution_clock().now();

	std::cout << "Part 1: " << part1 << std::endl;
	std::cout << "Part 2: " << part2 << std::endl;

	std::cout << "time: " << std::chrono::duration_cast<std::chrono::microseconds>(end-start).count() << " microseconds" << std::endl;

	return 0;
}
