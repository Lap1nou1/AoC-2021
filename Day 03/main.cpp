#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>
#include <utility>
#include <array>
#include <vector>
#include <chrono>
#include <algorithm>

#define SIZE 12
#define UINT12_MAX 0xFFF

void searchCommonBits(std::vector<std::string>* lines, std::array<std::int16_t, SIZE>* commonBits) {
	commonBits->fill(0);

	for (auto line = lines->begin(); line != lines->end(); ++line) {
		for (std::size_t i = 0; i < SIZE; i++) {
			(*commonBits)[i] += 1 - 2 * ((*line)[i] != '1');
		}
	}
}

int searchCommonBit(std::vector<std::string>* lines, std::size_t ind) {
	int d0 = 0;

	for (auto line = lines->begin(); line != lines->end(); ++line) {
		 d0 += 1 - 2 * ((*line)[ind] != '1');
	}

	return d0;
}

std::size_t dichotomie(std::vector<std::string>* lines, std::size_t chr, std::size_t start, std::size_t end) {
	while (start != end){
		std::size_t mid = (start + end) / 2;

		if ((*lines)[mid][chr] == '0') {
			start = mid+1;
		}
		else {
			end = mid;
		}

	}

	return start;
}

int main() {
	//On commence le temps
	auto start = std::chrono::high_resolution_clock::now();

	//On charge le fichier
	std::ifstream inputFile("input.txt");

	//commonBits[n] va avoir comme résultat la différence entre le nombre de 1 à la n-ième position avec le nombre de 0
	std::array<std::int16_t, SIZE> commonBits;

	//On enregistre les lignes une par une
	std::vector<std::string> lines;

	//On lit le fichier
	for (std::string line; std::getline(inputFile, line);) {
		lines.push_back(line);
	}

	//On mesure le nombre de 0 et de 1
	searchCommonBits(&lines, &commonBits);

	//Part 1: On trouve les deux nombres gamma et epsilon
	std::uint16_t epsilon = 0;
	
	for (std::size_t i = 0; i < SIZE; i++) {
		//On fait un shift pour les deux
		epsilon <<= 1;

		//Si commonBits[i] est négatif alors c'est epsilon qui prend le 1
		if (commonBits[i] < 0) {
			epsilon |= 1;
		}
	}

	std::uint16_t gamma = UINT12_MAX ^ epsilon;

	//for (std::size_t i = 0; i < SIZE; i++) {
	//	std::cout << commonBits[i] << " ";
	//}

	//std::cout << std::endl;

	//Part 2: letzgong
	/*
	std::vector<std::string> CO2rating = lines;
	std::vector<std::string> oxygenGenerator = lines;

	//On fait d'abord pour CO2rating (most common bit)
	
	for (std::size_t i = 0; CO2rating.size() > 1 || oxygenGenerator.size() > 1; i++) {
		if (CO2rating.size() > 1) {
			//On cherche le bit le plus commun pour avoir le critère
			searchCommonBits(&CO2rating, &commonBits);

			//On regarde chaque élément, et le supprime si besoin
			//On le fait dans l'ordre pour ne pas avoir de problème de taille

			int commonBit = searchCommonBit(&CO2rating, i);
			char mostCommon = (commonBit >= 0) * '1' | (commonBit < 0) * '0';
			for (std::int16_t j = CO2rating.size() - 1; j >= 0; j--) {

				if (CO2rating[j][i] != mostCommon) {
					CO2rating.erase(CO2rating.begin() + j);
				}
			}
		}

		if (oxygenGenerator.size() > 1) {
			int commonBit = searchCommonBit(&oxygenGenerator, i);
			char leastCommon = (commonBit < 0) * '1' | (commonBit >= 0) * '0';

			for (std::int16_t j = oxygenGenerator.size() - 1; j >= 0; j--) {

				if (oxygenGenerator[j][i] != leastCommon) {
					oxygenGenerator.erase(oxygenGenerator.begin() + j);
				}
			}
		}
	}

	////Puis pour oxygenGenerator (least common bit)
	//for (std::size_t i = 0; oxygenGenerator.size() != 1; i++) {
	//	//On cherche le bit le plus commun pour avoir le critère
	//	searchCommonBits(&oxygenGenerator, &commonBits);

	//	//On regarde chaque élément, et le supprime si besoin
	//	//On le fait dans l'ordre pour ne pas avoir de problème de taille
	//	for (std::int16_t j = oxygenGenerator.size() - 1; j >= 0; j--) {
	//		char leastCommon = (commonBits[i] < 0) * '1' | (commonBits[i] >= 0) * '0';

	//		if (oxygenGenerator[j][i] != leastCommon) {
	//			oxygenGenerator.erase(oxygenGenerator.begin() + j);
	//		}
	//	}
	//}

	std::uint16_t CO2rate = 0, oxygenRate = 0;

	for (std::size_t i = 0; i < SIZE; i++) {
		CO2rate <<= 1;
		oxygenRate <<= 1;

		CO2rate += (CO2rating[0][i] == '1');
		oxygenRate += (oxygenGenerator[0][i] == '1');
	}
	*/

	//On trie la liste
	std::sort(lines.begin(), lines.end());

	//Vive la dichotomie
	std::size_t startDicho = 0, endDicho = lines.size()-1;

	for (std::size_t i = 0; endDicho != startDicho; i++) {
		auto firstOne = dichotomie(&lines, i, startDicho, endDicho);
		std::size_t mid = (startDicho + endDicho + 1) / 2;

		//Si le premier 1 est <= au milieu, alors il y a plus de 1
		if (firstOne <= mid) {
			startDicho = firstOne;
		}
		else {
			endDicho = firstOne -1;
		}
	}

	std::string oxygenRateString = lines[endDicho];

	startDicho = 0, endDicho = lines.size() - 1;
	for (std::size_t i = 0; endDicho != startDicho; i++) {
		auto firstOne = dichotomie(&lines, i, startDicho, endDicho);
		std::size_t mid = (startDicho + endDicho + 1) / 2;

		//Cas où il y a moins de 0
		//firstOne = lastZero + 1 donc lastZero <= mid <=> firstOne - 1 <= mid <=> firstOne <= mid + 1 <=> firstOne < mid
		if (firstOne <= mid) {
			endDicho = firstOne-1;
		}
		else {
			startDicho = firstOne;
		}
	}

	std::string CO2rateString = lines[endDicho];

	//On converti les valeurs
	std::uint16_t CO2rate = 0, oxygenRate = 0;

	for (std::size_t i = 0; i < SIZE; i++) {
		CO2rate <<= 1;
		oxygenRate <<= 1;

		CO2rate += (CO2rateString[i] == '1');
		oxygenRate += (oxygenRateString[i] == '1');
	}

	//On commence le temps
	auto end = std::chrono::high_resolution_clock::now();

	std::cout << epsilon << " " << gamma << std::endl;
	std::cout << "Part 1: " << epsilon * gamma << std::endl;
	std::cout << CO2rate << " " << oxygenRate << std::endl;
	std::cout << "Part 2: " << CO2rate * oxygenRate << std::endl;
	std::cout << "Time: " << std::chrono::duration_cast<std::chrono::microseconds>(end-start).count() << " microseconds" << std::endl;

	return 0;
}
