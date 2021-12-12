#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>
#include <array>
#include <vector>
#include <set>
#include <chrono>

struct Position {
	std::uint16_t x;
	std::uint16_t y;

	Position(std::uint16_t _x, std::uint16_t _y) : x(_x), y(_y) {};
	Position() : x(), y() {};
};

struct PositionCmp {
	bool operator()(const Position& lhs, const Position& rhs) const {
		return lhs.y < rhs.y || (lhs.y == rhs.y && lhs.x < rhs.x);
	}
};

//Donne les positions valides adjacentes à une position de base
std::vector<Position> getAdjacentsPosition(const Position& base) {
	std::vector<Position> adjacents;

	//On définit des bool bizarrement car j'ai la flemme de faire fois les conditions pour les diagonales
	bool up = base.y > 0, down = base.y < 9, right = base.x < 9, left = base.x > 0;

	//Si on a une case au dessus
	if (up) {
		adjacents.push_back(Position(base.x, base.y - 1));

		//Si on a une case en haut à droite
		if (right) {
			adjacents.push_back(Position(base.x + 1, base.y - 1));
		}

		//Pareil, mais en haut à gauche
		if (left) {
			adjacents.push_back(Position(base.x - 1, base.y - 1));
		}
	}

	//Si on a une case au dessous
	if (down) {
		adjacents.push_back(Position(base.x, base.y + 1));

		//Si on a une case en base à droite
		if (right) {
			adjacents.push_back(Position(base.x + 1, base.y + 1));
		}

		//Pareil, mais en base à gauche
		if (left) {
			adjacents.push_back(Position(base.x - 1, base.y + 1));
		}
	}

	//Si on a une case à gauche
	if (left) {
		adjacents.push_back(Position(base.x - 1, base.y));
	}

	//Si on a une case à droite
	if (right) {
		adjacents.push_back(Position(base.x + 1, base.y));
	}

	return adjacents;
}

//Sert à print
void printGrid(const std::array<std::array<std::uint8_t, 10>, 10>& grid, const std::set<Position, PositionCmp>& flashed = {}) {
	auto nextFlash = flashed.begin(); //Vu que flashed est trié dans l'ordre des y puis des x, alors on peut suivre l'ordre naturel du set

	//On regarde chaque ligne
	for (std::size_t y = 0; y < 10; y++) {
		for (std::size_t x = 0; x < 10; x++) {
			if (nextFlash != flashed.end() && nextFlash->x == x && nextFlash->y == y) {
				std::cout << "\x1b[33m";
				nextFlash++;
			}

			std::cout << int(grid[y][x]) << "\x1b[37m";
		}

		std::cout << std::endl;
	}

	std::cout << std::endl;
}

//Augmente l'intensiter de lumière d'une grille
//Renvoie le nombre de flash qu'il y a eu à cette étape
std::uint16_t lightUp(std::array<std::array<std::uint8_t, 10>, 10>& grid) {
	std::set<Position, PositionCmp> toFlash;	//Positions des octopuses à flash à la prochaine étape

		//D'abord, on augmente de un le niveau de chacun des octopuses
	for (std::size_t y = 0; y < 10; y++) {
		for (std::size_t x = 0; x < 10; x++) {
			grid[y][x]++;

			//Si on est au dessus du 9 (soit 10), on l'ajoute aux flashs
			if (grid[y][x] == 10) {
				toFlash.insert(Position(x, y));
			}
		}
	}

	//Une fois tous les niveaux augmentés de 1, on augmente le niveau des cases adjacentes aux flashs
	std::set<Position, PositionCmp> flashed;	//Position qu'on a déjà flashé

	while (toFlash.size() != 0) {
		auto posToFlash = toFlash.begin();

		//On obtient les positions à flasher
		std::vector<Position> adjacentsFlashed = getAdjacentsPosition(*posToFlash);

		std::vector<Position> nextToFlash;	//Prochaine position à flash si jamais on a assez d'énergie après ce flash (sert dans le cas où un flash se mettrait devant posToFlash

		//On les flashs
		for (auto posToLight = adjacentsFlashed.begin(); posToLight != adjacentsFlashed.end(); ++posToLight) {
			grid[posToLight->y][posToLight->x]++;

			//Si on est à 10, on flash aussi
			if (grid[posToLight->y][posToLight->x] == 10) {
				nextToFlash.push_back(*posToLight);
			}
		}

		//On ajoute le flash aux positions flashées
		flashed.insert(*posToFlash);

		//On supprime le flash qu'on vient de faire
		toFlash.erase(posToFlash);

		//On ajoute les nouveaux flashs à faire
		toFlash.insert(nextToFlash.begin(), nextToFlash.end());
	}

	//Maintenant, on reset les flashed à 0
	for (auto flashedPos = flashed.begin(); flashedPos != flashed.end(); ++flashedPos) {
		grid[flashedPos->y][flashedPos->x] = 0;
	}

	return flashed.size();
}

int main() {
	//On commence le temps
	auto start = std::chrono::steady_clock::now();

	//On charge le fichier
	std::ifstream inputFile("input.txt");

	if (!inputFile.is_open()) {
		return -1;
	}

	//On lit le fichier
	std::array<std::array<std::uint8_t, 10>, 10> grid;

	std::string line;
	for (std::size_t y = 0; std::getline(inputFile, line); y++) {
		for (std::size_t x = 0; x < 10; x++) {
			grid[y][x] = line[x] - '0';
		}
	}

	std::uint32_t numberOfFlash = 0;

	//Maintenant, pendant 100 étapes pour la p1, on applique la procédure
	std::size_t t = 0;
	for (; t < 100; t++) {
		//On fait avancer la grille
		auto nbFlashed = lightUp(grid);

		//On ajoute au compteur le nombre de flash à l'instant
		numberOfFlash += nbFlashed;
	}

	//Pour la p2 : on fait tourner jusqu'à ce qu'on ait 100 en nbFlashed
	do {
		t++;
	} while (lightUp(grid) != 100);

	//On finit le temps
	auto end = std::chrono::steady_clock::now();

	std::cout << "Part 1: " << numberOfFlash << std::endl;
	std::cout << "Part 2: " << t << std::endl;
	std::cout << "Time: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " microseconds" << std::endl;

	return 0;
}
