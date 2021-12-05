#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>
#include <utility>
#include <array>
#include <vector>
#include <chrono>

#define PART 2

using bingoNumber = std::pair<std::uint32_t, bool>;

//Lis une ligne de bingo
void readLineForBingo(std::array<bingoNumber, 5>* arr, std::string* line) {
	std::size_t pos = line->find_first_of("0123456789");
	
	for (auto el = arr->begin(); el != arr->end(); ++el) {
		*el = bingoNumber(std::stoi(line->substr(pos)), false);

		pos = line->find_first_of("0123456789", line->find(" ", pos));
	}
}

std::uint16_t countPoint(std::array<std::array<bingoNumber, 5>,5>* grid) {
	std::uint16_t sum = 0;

	for (auto bingoLine = grid->begin(); bingoLine != grid->end(); ++bingoLine) {
		for (auto el : *bingoLine) {
			if (!el.second) {
				sum += el.first;
			}
		}
	}

	return sum;
}

int main() {
	//On commence le chrono
	auto start = std::chrono::high_resolution_clock::now();

	//On charge le fichier
	std::ifstream inputFile("input.txt");

	//On vérifie au cas où
	if (!inputFile.is_open()) {
		return -1;
	}

	//On lit la ligne des nombres tirés au sort
	std::string line;
	std::vector<std::uint16_t> numbers;


	std::getline(inputFile, line);
	
	std::size_t pos = -1;
	do {
		pos++;

		numbers.push_back(std::stoi(line.substr(pos)));

		pos = line.find_first_of(",", pos + 1);
	} while (pos < line.size());

	//On crée le vecteur contenant les grilles de bingo
	std::vector<std::array<std::array<bingoNumber, 5>, 5>> bingoGrids;

	for (std::size_t i = 0; std::getline(inputFile, line); i++) {
		//Si on est à la fin d'une grille, on crée une nouvelle grille vide
		if (line.size() == 0) {
			std::array<std::array<bingoNumber, 5>, 5> newGrid;
			bingoGrids.push_back(newGrid);

			//On reset i
			i = -1;

			continue;
		}

		//Sinon on ajoute ligne par ligne notre grille
		std::array<bingoNumber, 5> bingoLine;

		readLineForBingo(&bingoLine, &line);
	
		(*bingoGrids.rbegin())[i] = bingoLine;
	}

	//On joue au jeu pour savoir le gagnant
	//pos servira a identifié le gagnant
	pos = -1;
	auto nbPicked = numbers.begin();

	std::uint32_t sol1 = 0;

	//On regarde chaque nombre 1 à 1
	for (; nbPicked != numbers.end(); ++nbPicked) {
		//Pour la partie 2, on créer un vector de tous les index de grille gagnante (et donc à supprimer)
		std::vector<std::size_t> finished;

		//On regarde toutes les grilles
		for (std::size_t i = 0; i < bingoGrids.size(); i++) {
			//On regarde toutes les lignes
			for (auto bingoLine = bingoGrids[i].begin(); bingoLine != bingoGrids[i].end(); ++bingoLine) {
				//On regarde tous les nombres, s'ils sont tous ok alors on a gagné !!!
				bool won = true;

				for (auto nb = bingoLine->begin(); nb != bingoLine->end(); ++nb) {
					//Si le nombre est ok, alors on dit que c'est coché
					if (nb->first == *nbPicked) {
						nb->second = true;
					}

					//Regarde si la ligne est bonne
					if (!nb->second) {
						won = false;
					}
				}

				//Si la ligne est bonne, on change la position
				if (won) {
					pos = i;
				}
			}

			//On regarde maintenant pour les colonnes
			for (std::size_t j = 0; j < 5 && pos == -1; j++) {
				bool won = true;

				for (std::size_t k = 0; k < 5; k++) {
					if (!bingoGrids[i][k][j].second) {
						won = false;
					}
				}

				if (won) {
					pos = i;
				}
			}

			if (pos != -1) {
				if (sol1 == 0)
					sol1 = countPoint(&bingoGrids[pos]) * *nbPicked;

				finished.push_back(pos);
				pos = -1;
			}
		}

		//Letzgong on a fini (que si la dernière grille est win
		if (bingoGrids.size() == 1 && finished.size() != 0) {
			pos = 0;
			break;
		}

		//Dans la part 2, une fois un nombre validé, on regarde si on a des grilles à éliminer, pour ne pas avoir de souci d'index on les supprime dans l'ordre inverse
		for (auto toDelete = finished.rbegin(); toDelete != finished.rend(); toDelete++) {
			bingoGrids.erase(bingoGrids.begin() + *toDelete);
		}
	}

	//On revient à celui de pile avant
	//nbPicked--;

	//On calcule le score gagnant
	std::uint32_t sum2 = countPoint(&bingoGrids[0]);
	
	//On arrête le chrono
	auto end = std::chrono::high_resolution_clock::now();

	std::cout << "Part 1: " << sol1 << std::endl;
	std::cout << "Part 2: " << sum2 * *nbPicked<< std::endl;
	std::cout << "Time: " << std::chrono::duration_cast<std::chrono::microseconds>(end-start).count() << " microseconds" << std::endl;

	//Pour afficher les grilles
	//for (auto grid = bingoGrids.begin(); grid != bingoGrids.end(); ++grid) {
	//	for (auto bingoLine = grid->begin(); bingoLine != grid->end(); ++bingoLine) {
	//		for (auto nb = bingoLine->begin(); nb != bingoLine->end(); ++nb) {
	//			if (nb->second) 
	//				std::cout << '[' << nb->first << "] ";
	//			else
	//				std::cout << nb->first << " ";
	//		}
	//	
	//		std::cout << std::endl;
	//	}
	//	std::cout << std::endl;
	//}

	return 0;
}
