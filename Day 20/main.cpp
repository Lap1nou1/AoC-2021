#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>
#include <vector>
#include <array>
#include <set>
#include <utility>
#include <algorithm>
#include <chrono>

struct Point {
	std::int32_t x = 0, y = 0;

	Point(std::int32_t _x, std::int32_t _y) : x(_x), y(_y) {};
	Point() : x(0), y(0) {};

	bool operator==(const Point& rhs) const {
		return this->x == rhs.x && this->y == rhs.y;
	}

	bool operator>(const Point& rhs) const {
		return this->y > rhs.y || (this->y == rhs.y && this->x > rhs.x);
	}

	Point operator+(const Point& rhs) const {
		return Point(this->x + rhs.x, this->y + rhs.x);
	}

	Point operator-(const Point& rhs) const {
		return Point(this->x - rhs.x, this->y - rhs.x);
	}
};

struct PointCmp {
	bool operator()(const Point& lhs, const Point& rhs) const {
		return lhs.y < rhs.y || (lhs.y == rhs.y && lhs.x < rhs.x);
	}
};

//Permet d'obtenir l'index qui nous permet de savoir le prochain pixel
std::size_t getNewPixelImageIndex(const std::set<Point, PointCmp>& activePoints, const Point toCheck, bool borderEffect, std::pair<Point, Point> border) {
	//On prend d'abord les pixels adjacents
	std::array<Point, 9> adjacents;

	for (std::int32_t y = -1; y <= 1; y++) {
		for (std::int32_t x = -1; x <= 1; x++) {
			//Pour l'index d'adjacents, c'est (y+1) * 3 + (x + 1), le +1 est pour compter le fait qu'on commence à -1, en réduisant ça fait : y * 3 + 3 + x + 1 = y * 3 + x + 4
			adjacents[y * 3 + x + 4] = Point(toCheck.x + x, toCheck.y + y);
		}
	}

	//Maintenant on cherche les points adjacents
	//On va compter sur le fait que le set soit trié (par les y puis les x) pour qu'on puisse savoir quand on a un point à partir du moment où un est "plus grand" qu'un autre
	std::size_t activePos = 0;
	
	auto nextPoint = adjacents.begin();

	for (auto point = activePoints.begin(); point != activePoints.end() && nextPoint != adjacents.end(); ++point) {
		//Si le point est derrière celui qu'on cherche, alors celui qu'on cherche n'est pas allumé
		//Donc on change pos et on avance nextPoint
		//On fait bien attention à regarder tous les points qui ne sont pas bon
		while (nextPoint != adjacents.end() && *point > *nextPoint) {
			activePos <<= 1;
			nextPoint++;
		}
		
		//Dans le cas où on soit sur le bon point (donc qu'il soit allumé)
		//On change pos, et on avance nextPoint
		if (nextPoint != adjacents.end() && *point == *nextPoint) {
			activePos = (activePos << 1) + 1;
			nextPoint++;
		}
	}

	//Il y a un cas où on a regardé tous les points actifs, mais qu'il reste des points adjacents, on fait un shift du nombre restant
	activePos <<= std::distance(nextPoint, adjacents.end());

	//Pour le cas bordure (si tout est on), on va faire une autre pos qui regarde le nombre de point dans la bordure
	//Pour on va faire | à activePos et borderPos
	std::size_t borderPos = 0;

	if (borderEffect) {
		nextPoint = adjacents.begin();

		for (; nextPoint != adjacents.end(); ++nextPoint) {
			borderPos <<= 1;

			//Si le point est en dehors de la bordure, alors il est on
			if (nextPoint->x <= border.first.x || nextPoint->y <= border.first.y || nextPoint->x >= border.second.x || nextPoint->y >= border.second.y) {
				borderPos += 1;
			}
		}
	}

	//On revoie pos
	return activePos | borderPos;
}

//Fait une itération pour passer d'une image à une autre
//image est l'image qui va être modifié, imageEnhancementAlgorithm l'algorithme utilisé
//Dans le cas où le vide devient plein (où imageEnhancementAlgotihme[0] = '#'), border permet de savoir à partir d'où tout est plein/vide (dépend de la parité donné par oddIteration)
void doOneTurn(std::set<Point, PointCmp>& image, const std::array<bool, 512>& imageEnhancementAlgorithm, std::pair<Point, Point>& border, const bool& borderEffect, const bool& oddIteration) {
	//D'abord, on copie toute l'image
	const std::set<Point, PointCmp> originalImage = image;

	//On reset notre image
	image.clear();

	//Maintenant, on va regarder tous les points à l'intérieur de la zone définie par la bordure
	for (std::int32_t y = border.first.y; y <= border.second.y; y++) {
		for (std::int32_t x = border.first.x; x <= border.second.x; x++) {
			Point newPoint(x, y);

			//On regarde le résultat de l'algorithme pour la région de notre pixel
			bool isOn = imageEnhancementAlgorithm[getNewPixelImageIndex(originalImage, newPoint, borderEffect && oddIteration, border)];

			//Si c'est on, on l'ajoute à notre nouvelle image
			if (isOn) {
				image.insert(newPoint);

				/*
				* Finalement on fait pas car on peut dire que c'est toujours un élargissement de 1
				//On modifie aussi la bordure
				if (newBorder.first.x >= x) {
					newBorder.first.x = x - 1;
				}
				else if (newBorder.second.x <= x) {
					newBorder.second.x = x = 1;
				}

				if (newBorder.first.y >= y) {
					newBorder.first.y = y - 1;
				}
				else if (newBorder.second.y >= y) {
					newBorder.second.y = y + 1;
				}
				*/
			}
		}
	}


	////Si on a un effet de bordure, on élargit d'un la bordure
	//if (borderEffect && !oddIteration) {
	//	border.first = border.first - Point(1, 1);
	//	border.second = border.second + Point(1, 1);
	//}

	//La bordure ne peux que s'agrandir d'un par étape
	border.first = border.first - Point(1, 1);
	border.second = border.second + Point(1, 1);
}

void printGrid(const std::set<Point, PointCmp>& points, std::pair<Point, Point> border) {
	auto point = points.begin();
	
	for (std::int32_t y = border.first.y; y <= border.second.y; y++) {
		for (std::int32_t x = border.first.x; x <= border.second.x; x++) {
			if (y == 0 && x == 0) {
				std::cout << "\x1b[32m";
			}
			
			if (point != points.end() && *point == Point(x, y)) {
				std::cout << '#';
				point++;
			}
			else
				std::cout << '.';
			if (y == 0 && x == 0) {
				std::cout << "\x1b[37m";
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

	//On interprète le fichier
	//On commence par prendre l'algorithme
	std::string line;
	std::getline(inputFile, line);

	std::array<bool, 512> imageEnhancementAlgorithm;

	std::size_t pos = 0;
	std::for_each(line.begin(), line.end(), [&imageEnhancementAlgorithm, line, &pos](char chr) { (imageEnhancementAlgorithm)[pos] = chr == '#'; pos++; });

	//On "jette" la ligne de vide
	std::getline(inputFile, line);

	//Maintenant on lit l'image de base
	std::set<Point, PointCmp> image;

	//On en profite pour définir les bordures
	auto border = std::make_pair(Point(-2, -2), Point(2, 2));

	//Chaque ligne est une donnée y (on commence arbitrairement à 0)
	for (std::int32_t y = 0; std::getline(inputFile, line); y++) {
		//On regarde chaque caractère de la ligne
		for (auto xChar = line.begin(); xChar != line.end(); ++xChar) {
			//Si ce caractère est un '#', on l'ajoute à l'image
			if (*xChar == '#') {
				Point newPoint = Point(std::distance(line.begin(), xChar), y);

				image.insert(newPoint);

				//Si on est en dehors de la bordure, on l'élargit
				if (border.second.x <= newPoint.x) {
					border.second.x = newPoint.x + 2;
				}

				if (border.second.y <= newPoint.y) {
					border.second.y = newPoint.y + 2;
				}
			}
		}
	}

	//La bordure à un effet si et seulement si le premier élément de l'algorithme est un '#'
	bool borderEffect = imageEnhancementAlgorithm[0];

	//Pour la p1 on fait que deux itérations
	for (std::size_t it = 0; it < 2; it++) {
		doOneTurn(image, imageEnhancementAlgorithm, border, borderEffect, it%2);

		//printGrid(image, border);
	}

	std::size_t p1 = image.size();

	//Pour la p2 on va à 50 itérations
	for (std::size_t it = 2; it < 50; it++) {
		doOneTurn(image, imageEnhancementAlgorithm, border, borderEffect, it % 2);

		//printGrid(image, border);
	}
	
	//On finit le chrono
	auto end = std::chrono::steady_clock::now();

	std::cout << "Part 1: " << p1 << std::endl;
	std::cout << "Part 2: " << image.size() << std::endl;
	std::cout << "Time: " << std::chrono::duration_cast<std::chrono::microseconds>(end-start).count() << " microseconds" << std::endl;

	return 0;
}
