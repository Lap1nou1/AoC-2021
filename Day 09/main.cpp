#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <array>
#include <set>
#include <utility>
#include <cstdint>
#include <chrono>

struct Point {
	std::int16_t x;
	std::int16_t y;

	Point(std::uint16_t _x, std::uint16_t _y) : x(_x), y(_y) {};
	Point() : x(0), y(0) {};
	bool operator==(const Point& rhs) const { return this->x == rhs.x && this->y == rhs.y; };
};

struct PointCmp {
	bool operator()(const Point& lhs, const Point& rhs) const { return lhs.y < rhs.y || (lhs.y == rhs.y && lhs.x < rhs.x); };
};

//Affiche la grille, pour le debug
void printGrid(const std::vector<std::vector<uint16_t>>& grid) {
	for (auto gridLine = grid.begin(); gridLine != grid.end(); ++gridLine) {
		for (auto height = gridLine->begin(); height != gridLine->end(); ++height) {
			std::cout << int(*height);
		}
		std::cout << std::endl;
	}
}

bool isValidPoint(const std::vector<std::vector<uint16_t>>& grid, const Point& pos) {
	if (pos.y < 0 || pos.y >= grid.size() || pos.x < 0 || pos.x >= grid[pos.y].size()) {
		return 0;
	}

	return 1;
}

//Renvoie le niveau de risque, sinon renvoie 0 (false)
//grid correspond à la grille, pos à la position testé, et ignored aux points qui sont ignorés dans le calcul (pour la p2)
std::uint8_t isLowPoint(const std::vector<std::vector<std::uint16_t>>& grid, const Point& pos, const std::set<Point, PointCmp>& ignored = {}) {
	//On vérifie que la position est valide (dans les bornes de grid)
	if (!isValidPoint(grid, pos)) {
		return 0;
	}

	//Par facilité on garde un pointeur vers la valeur
	const std::uint16_t* val = &grid[pos.y][pos.x];

	//Si on a un 9 on sait que c'est pas un minimum
	if (*val == 9) {
		return 0;
	}

	//Ces bool disent si parmis ignored on retrouve la case à droite (right), à gauche (left), etc
	bool right = false, left = false, up = false, down = false;
	
	for (auto ignPos = ignored.begin(); ignPos != ignored.end(); ++ignPos) {
		//On regarde si ça correspond à une des cases adjacentes
		//Cas case gauche :
		if (*ignPos == Point(pos.x - 1, pos.y)) {
			left = true;
		}

		//Cas case droite :
		if (*ignPos == Point(pos.x + 1, pos.y)) {
			right = true;
		}

		//Cas case haute :
		if (*ignPos == Point(pos.x, pos.y - 1)) {
			up = true;
		}

		//Cas case basse :
		if (*ignPos == Point(pos.x, pos.y + 1)) {
			down = true;
		}
	}

	//On test les différentes possibilités
	//Si x>0 alors il y a un membre à gauche, on le test
	if (pos.x > 0 && grid[pos.y][pos.x - 1] <= *val && !left) {
		return 0;
	}

	//Si x<grid[pos.y].size() alors il y a un membre à droite, on le test
	if (pos.x < grid[pos.y].size()-1 && grid[pos.y][pos.x + 1] <= *val && !right) {
		return 0;
	}

	//Si y>0 alors il y a un membre au dessus, on le test
	if (pos.y > 0 && grid[pos.y-1][pos.x] <= *val && !up) {
		return 0;
	}

	//Si y<grid.size() alors il y a un membre en bas, on le test
	if (pos.y < grid.size()-1 && grid[pos.y+1][pos.x] <= *val && !down) {
		return 0;
	}

	//Tous les tests sont fait : on renvoie le niveau de risque
	return *val + 1;
}

//Place un nombre dans un array, l'array est triée, et place l'élément au bon emplacement, s'il est trop petit, ne change rien
//topSize[0] est l'élément le plus grand
void insertMax(std::array<std::uint64_t, 3>& topSize, std::uint32_t value) {
	for (std::size_t i = 0; i < 3; i++) {
		if (value > topSize[i]) {
			//Si l'élément est plus grand on fait un shift
			for (std::size_t j = 2; j > i; j--) {
				topSize[j] = topSize[j-1];
			}

			//On place l'élément
			topSize[i] = value;

			break;
		}
	}
}

int main() {
	//On commence le chrono
	auto start = std::chrono::steady_clock::now();

	//On charge le fichier
	std::ifstream inputFile("input.txt");

	if (!inputFile.is_open()) {
		return -1;
	}

	//On lit le fichier
	//Pour ça on aura un vector de vector de unsigned char, le vector sera la taille, chaque vector dedans correspondra à une ligne
	std::vector<std::vector<std::uint16_t>> grid;

	for (std::string line; std::getline(inputFile, line);) {
		std::vector<std::uint16_t> gridLine;
		
		for (auto chr = line.begin(); chr != line.end(); ++chr) {
			gridLine.push_back(*chr - '0');
		}

		grid.push_back(gridLine);
	}

	//Maintenant, on test la grille pour trouver le niveau de risque
	std::uint32_t totalRiskLevel = 0;
	std::array<std::uint64_t, 3> topSize = { 0, 0, 0 }; //Correspond aux 3 meilleurs tailles de bassins

	for (std::uint16_t y = 0; y < grid.size(); y++) {
		for (std::uint16_t x = 0; x < grid[y].size(); x++) {
			std::uint16_t riskLevel = isLowPoint(grid, Point(x, y));

			//Si le niveau de risque n'est pas 0 (donc qu'on est sur un low point)
			//On regarde les cases adjacentes, puis on "propage" nos cases tant que ce sont des "low point", si on ne compte pas les cases déjà visités
			if (riskLevel) {
				std::set<Point, PointCmp> bassin = { Point(x, y) };	//Tous les points du bassin, il sert aussi pour savoir les points à ignorer
				std::vector<std::pair<Point, Point>> toCheck = { std::pair<Point, Point>(Point(x,y), Point(x, y - 1)), std::pair<Point, Point>(Point(x,y), Point(x, y + 1)), std::pair<Point, Point>(Point(x,y), Point(x - 1, y)), std::pair<Point, Point>(Point(x,y), Point(x + 1, y)) }; //Point à check s'ils font partie du bassin, l'état de début correspond aux points adjacents à notre case

				//On regarde les points un par un
				//Vu qu'à la fin on efface le point toCheck, on ne bouge pas le pointer
				while (toCheck.size() != 0) {
					Point pointToCheck = toCheck.begin()->second;
					Point base = toCheck.begin()->first;

					//On regarde si le point est dans le bassin
					//C'est-à-dire s'il est inférieur à son referent
					if (isValidPoint(grid, pointToCheck) && grid[pointToCheck.y][pointToCheck.x] != 9 && grid[base.y][base.x] < grid[pointToCheck.y][pointToCheck.x]) {
						//S'il est dans le bassin, alors on l'ajoute à bassin et on ajoute les points adjacents
						if (bassin.insert(pointToCheck).second) {
							toCheck.push_back(std::pair<Point, Point>(pointToCheck, Point(pointToCheck.x, pointToCheck.y - 1)));
							toCheck.push_back(std::pair<Point, Point>(pointToCheck, Point(pointToCheck.x, pointToCheck.y + 1)));
							toCheck.push_back(std::pair<Point, Point>(pointToCheck, Point(pointToCheck.x - 1, pointToCheck.y)));
							toCheck.push_back(std::pair<Point, Point>(pointToCheck, Point(pointToCheck.x + 1, pointToCheck.y)));
						}
					}

					toCheck.erase(toCheck.begin());
				}

				totalRiskLevel += riskLevel;

				insertMax(topSize, bassin.size());
			}
		}
	}

	//On finit le chrono
	auto end = std::chrono::steady_clock::now();

	std::cout << "Part 1: " << totalRiskLevel << std::endl;
	std::cout << "Part 2: " << topSize[0] * topSize[1] * topSize[2] << std::endl;
	std::cout << "Time: " << std::chrono::duration_cast<std::chrono::microseconds>(end-start).count() << " microseconds" << std::endl;

	return 0;
}
