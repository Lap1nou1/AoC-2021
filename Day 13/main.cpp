#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>
#include <set>
#include <chrono>

struct Point {
	std::int32_t x = 0;
	std::int32_t y = 0;

	Point(std::int32_t _x, std::int32_t _y) : x(_x), y(_y) {};
	Point() : x(0), y(0) {};
};

struct PointCmp {
	bool operator()(const Point& lhs, const Point& rhs) const {
		return lhs.y < rhs.y || (lhs.y == rhs.y && lhs.x < rhs.x);
	}
};

void flip(std::int32_t& coordBase, std::int32_t& coordModified) {
	//On ne flip que si la coordonnée du modifié est plus grande
	if (coordModified > coordBase) {
		//Pour flip, on regarde la distance coordModified - corrdBase, et on la retire à coordModified
		//Donc coordModified = coordBase - (coordModified - coordBase) = coordBase - coordModified + coordBase = 2*coordBase - coordModified
		coordModified = 2*coordBase - coordModified;
	}
}

void printPoints(const std::set<Point, PointCmp>* points) {
	//On part du principe que la plus petite position est 0,0
	//On prend d'abord les max x et y pour savoir jusqu'où aller
	Point maxPos;

	for (auto point = points->begin(); point != points->end(); ++point) {
		maxPos.x = std::max(maxPos.x, point->x);
		maxPos.y = std::max(maxPos.y, point->y);
	}

	//Maintenant on itère parmis tous les x puis y du plan et on place les points au fur et à mesure
	//Les points étant trié dans l'ordre y puis x, on regarde juste les points un par un
	auto nextPoint = points->begin();

	for (std::int32_t y = 0; y <= maxPos.y; y++) {
		for (std::int32_t x = 0; x <= maxPos.x; x++) {
			if (nextPoint != points->end() && nextPoint->x == x && nextPoint->y == y) {
				std::cout << 'X';

				nextPoint++;
			}
			else {
				std::cout << ' ';
			}
		}

		std::cout << std::endl;
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

	//On interprète les donnés
	//On lit d'abord les points
	std::set<Point, PointCmp>* points = new std::set<Point, PointCmp>;

	std::string line;
	while (std::getline(inputFile, line) && !line.empty()) {
		//On crée le point
		Point newPoint;

		//On prend la coordonnée x
		newPoint.x = std::stoi(line);

		//maintenant la coordonnée y qui se trouve avec la virgule
		newPoint.y = std::stoi(line.substr(line.find(',') + 1));

		//On ajoute le point au set
		points->insert(newPoint);
	}

	//Maintenant on interprète les pliures
	for (std::size_t count = 0; std::getline(inputFile, line); count++) {
		//Le "fold along" est toujours présent, on regarde donc à quoi correspond le caractère d'index 11 (soit 'x' soit 'y')
		char axis = line[11];	//Axe de pliage
		std::int32_t position = std::stoi(line.substr(13));	//On prend le nombre après le égale

		//On regarde tous les points, on ajoute les versions modifiées dans un autre set qu'on changera après
		std::set<Point, PointCmp>* newPoints = new std::set<Point, PointCmp>;

		for (auto point = points->begin(); point != points->end(); ++point) {
			Point changedPoint(point->x, point->y);

			if (axis == 'x') {
				flip(position, changedPoint.x);
			}
			else {
				flip(position, changedPoint.y);
			}

			newPoints->insert(changedPoint);
		}

		delete points;
		points = newPoints;
	
		//Pour la p1
		if (count == 0)
			std::cout << "Part 1: " << points->size() << std::endl;
	}

	printPoints(points);

	//On finit le chrono
	auto end = std::chrono::steady_clock::now();

	std::cout << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " microseconds" << std::endl;

	return 0;
}
