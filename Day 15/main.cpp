#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <array>
#include <set>
#include <cstdint>
#include <chrono>
#include <algorithm>
#include <map>

struct Point {
	std::uint32_t x = 0;
	std::uint32_t y = 0;

	Point(std::uint32_t _x, std::uint32_t _y) : x(_x), y(_y) {};
	Point() : x(0), y(0) {};

	bool operator==(Point& rhs) const { return this->x == rhs.x && this->y == rhs.y; };
};

struct Path {
	Point pos;
	std::uint64_t cost = 0;

	Path(Point pt, std::uint64_t _cost) : pos(pt), cost(_cost) {};
	Path() : pos(), cost(0) {};
};

struct PathCmp {
	bool operator()(const Path& lhs, const Path& rhs) const {
		//On renvoie true soit si le coût est plus petit pour lhs, et si les coûts sont égaux, on renvoit la position y et x la plus petite
		return lhs.cost < rhs.cost || (lhs.cost == rhs.cost && (lhs.pos.y < rhs.pos.y || (lhs.pos.y == rhs.pos.y && lhs.pos.x < rhs.pos.x)));
	}
};

void insertPath(std::set<Path, PathCmp>& paths, Path& pathToAdd) {
	//On ajoute que s'il est unique, ou que ce chemin est plus court que l'ancien de même position
	bool willBeAdded = true;

	//On regarde tous les points
	for (auto checkedPath = paths.begin(); checkedPath != paths.end(); ++checkedPath) {
		if (checkedPath->pos == pathToAdd.pos) {
			//On regarde si le chemin est plus court, si oui, on change willBeAdded
			if (checkedPath->cost < pathToAdd.cost) {
				willBeAdded = false;
			}
			else {
				//S'il le nouveau chemin est plus court, alors on retire l'ancien
				paths.erase(checkedPath);
			}

			break;
		}
	}

	//On ajoute le chemin uniquement si willBeAdded est à true
	if (willBeAdded) {
		paths.insert(pathToAdd);
	}
}

//Trouve les côtés adjacents dans un carré (sans sortir dudit carré)
void findAdjacent(const Point& point, std::vector<Point>& adj, const std::size_t size) {
	//S'il y a une case à gauche, on l'ajoute
	if (point.x > 0) {
		adj.push_back(Point(point.x - 1, point.y));
	}

	//S'il y a une case à droite, on l'ajoute
	if (point.x < size - 1) {
		adj.push_back(Point(point.x + 1, point.y));
	}

	//S'il y a une case en haut, on l'ajoute
	if (point.y > 0) {
		adj.push_back(Point(point.x, point.y - 1));
	}

	//S'il y a une case en bas, on l'ajoute
	if (point.y < size - 1) {
		adj.push_back(Point(point.x, point.y + 1));
	}
}

//Initialisation de l'algorithme de Dijkstra (on met tout à la valeur max)
void initialisation(std::set<Path, PathCmp>& distanceDuDebut, std::map<std::uint64_t, bool>& taken, std::size_t size) {
	for (std::uint32_t y = 0; y < size; y++) {
		for (std::uint32_t x = 0; x < size; x++) {
			distanceDuDebut.insert(Path(Point(x, y), -1));
			taken[y * size + x] = false;
		}
	}

	//On met le premier à 0
	distanceDuDebut.erase(distanceDuDebut.begin());
	distanceDuDebut.insert(Path(Point(0, 0), 0));
}

//Mise à jour des distances
void majDistance(Path& s1, Point& s2Pos, std::vector<std::vector<std::uint8_t>>& grid, std::set<Path, PathCmp>& distanceDuDebut, std::map<std::uint64_t, bool>& taken) {
	//On cherche semi-manuellement les positions;
	//Avant on vérifie que la position n'a pas déjà été retiré
	if (!taken[s2Pos.y * grid.size() + s2Pos.x]) {
		auto s2 = std::find_if(distanceDuDebut.begin(), distanceDuDebut.end(), [s2Pos](Path p) { return s2Pos == p.pos; });;

		if (s2 != distanceDuDebut.end() && s2->cost > s1.cost + grid[s2Pos.y][s2Pos.x]) {
			Path shorter(s2Pos, s1.cost + grid[s2Pos.y][s2Pos.x]);

			distanceDuDebut.erase(s2);
			distanceDuDebut.insert(shorter);
		}
	}
}

Path findOptimal(std::vector<std::vector<std::uint8_t>>& grid) {
	std::set<Path, PathCmp> distanceDuDebut; 
	std::map<std::uint64_t, bool> taken;

	initialisation(distanceDuDebut, taken, grid.size());

	Path* finalPath = nullptr;
	while (finalPath == nullptr) {
		Path s1 = *distanceDuDebut.begin();	//On prend le plus petit sommet
		distanceDuDebut.erase(distanceDuDebut.begin());	//On retire le plus petit sommet

		//On regarde si on a finit (si s1 == (size-1, size-1)
		if (s1.pos.x == grid.size() - 1 && s1.pos.y == grid.size() - 1) {
			finalPath = new Path(s1);
			break;
		}

		//On regarde les voisins
		std::vector<Point> adjacents;
		findAdjacent(s1.pos, adjacents, grid.size());

		//Pour chaque nouveau voisin on calcul la distance
		for (auto v = adjacents.begin(); v != adjacents.end(); ++v) {
			majDistance(s1, *v, grid, distanceDuDebut, taken);
		}

		//On dit qu'on a check s1
		taken[s1.pos.y * grid.size() + s1.pos.x] = true;
	}

	return *finalPath;
}

int main() {
	//On commence le temps
	auto start = std::chrono::steady_clock::now();

	//On charge le fichier
	std::ifstream inputFile("input.txt");

	if (!inputFile.is_open()) {
		return -1;
	}

	//On lit la grille
	std::vector<std::vector<std::uint8_t>> grid;

	for (std::string line; std::getline(inputFile, line);) {
		std::vector<std::uint8_t> gridLine;

		//On lit la ligne
		for (auto chr = line.begin(); chr != line.end(); ++chr) {
			gridLine.push_back(*chr - '0');	//On converti en nombre de 0 à 9
		}

		//On ajoute gridLine à grid
		grid.push_back(gridLine);
	}

	auto p1 = findOptimal(grid).cost;

	//On agrandit la grille en x
	for (std::size_t y = 0; y < grid.size(); y++) {
		for (std::size_t i = 1; i < 5; i++) {
			for (std::size_t x = 0; x < grid.size(); x++) {
				grid[y].push_back((grid[y][x] + i > 9) ? (grid[y][x] + i + 1) % 10 : grid[y][x] + i);
			}
		}
	}

	//On agrandit la grille en y
	std::size_t baseSize = grid.size();
	for (std::uint8_t i = 1; i < 5; i++) {
		for (std::size_t y = 0; y < baseSize; y++) {
			std::vector<std::uint8_t> gridLine = grid[y];

			for (auto el = gridLine.begin(); el != gridLine.end(); ++el) {
				(*el) += i;

				if ((*el) > 9) {
					(*el) -= 9;
				}
			}

			grid.push_back(gridLine);
		}
	}
	
	auto p2 = findOptimal(grid).cost;

	//On finit le temps
	auto end = std::chrono::steady_clock::now();

	std::cout << "Part 1: " << p1 << std::endl;
	std::cout << "Part 2: " << p2 << std::endl;
	std::cout << "Time: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " microseconds" << std::endl;
	std::cout << "Time (long): " << std::chrono::duration_cast<std::chrono::hours>(end - start).count() << ":" << std::chrono::duration_cast<std::chrono::minutes>(end - start).count() << ":" << std::chrono::duration_cast<std::chrono::seconds>(end - start).count() << "sec" << std::endl;

	return 0;
}
