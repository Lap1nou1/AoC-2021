#include <iostream>
#include <fstream>
#include <string>
#include <utility>
#include <cstdint>
#include <numeric>
#include <chrono>

struct Position {
	std::int32_t x;
	std::int32_t y;

	Position(std::int32_t _x, std::int32_t _y) : x(_x), y(_y) {};
	Position() : x(0), y(0) {};

	//Renvoie true si le point est dans les bornes des deux autres (avec lhs < rhs)
	bool isInside(const Position& lhs, const Position& rhs) {
		return lhs.x <= this->x && this->x <= rhs.x && lhs.y <= this->y && this->y <= rhs.y;
	}

	//Renvoie true si on est après le point (donc à droite ou en dessous)
	bool isAfter(const std::int32_t _x, std::int32_t _y) {
		return this->x > _x || this->y < _y;
	}

	Position operator+(const Position& rhs) {
		return Position(this->x + rhs.x, this->y + rhs.y);
	}
};

int main() {
	//On commence le chrono
	auto start = std::chrono::steady_clock::now();

	//On charge le fichier
	std::ifstream inputFile("input.txt");

	if (!inputFile.is_open()) {
		return -1;
	}

	//On lit la seule ligne
	//On stocke la position minimale et maximale dans un pair, le premier est le plus petit, le second le plus grand
	std::pair<Position, Position> targetArea;

	//On prend la ligne
	std::string line;
	std::getline(inputFile, line);

	//Le x= sera toujours en position
	std::size_t pos = 15;

	//On prend le minimal de x
	targetArea.first.x = std::stoi(line.substr(pos));

	//On cherche le maximal de x puis on le prend
	pos = line.find('.', pos) + 2;
	targetArea.second.x = std::stoi(line.substr(pos));

	//On cherche le minimal de y et on le prend
	pos = line.find('y', pos) + 2;
	targetArea.first.y = std::stoi(line.substr(pos));

	//On cherche le maximal de y puis on le prend
	pos = line.find('.', pos) + 2;
	targetArea.second.y = std::stoi(line.substr(pos));

	//On cherche les valeurs limites de x du vecteur vitesse initiale
	//Pour ça, il suffit juste que la somme de toutes les vitesses tel qu'à la fin la vitesse soit égal à 0 et que cette somme soit égal au minimum de la tergetArea en x
	//Donc n(n+1)/2 = x <=> n² + n - 2x = 0 <=> d = 1 + 8x, on ne veut que le n positif donc n = (sqrt(d) - 1)/2
	//Ici, on a le minimum (toute valeur inférieur atteindrait 0 en vélocité x avant d'être dans la zone
	//Le maximum est juste le x du point de targetArea le plus loin, si c'est au dessus, on est pas dessus
	std::pair<Position, Position> minmaxTry;
	minmaxTry.first.x = (std::sqrt(1 + 8 * targetArea.first.x) - 1) / 2;
	minmaxTry.second.x = targetArea.second.x;

	//Les valeurs limites en y sont simples : soit on va trop bas en une itération (donc min_y - 1)
	//soit, étant donné que l'on repassera forcément par x = 0 avec une vélocité opposé à celle de départ, -(min_y + 1)
	minmaxTry.first.y = targetArea.first.y - 1;
	minmaxTry.second.y = -targetArea.first.y - 1;

	//La réponse à la p1 est juste la somme de tous les nombres entiers de 0 à minmaxTry.second.y
	std::uint32_t p1 = (minmaxTry.second.y * (minmaxTry.second.y + 1)) / 2;

	//Pour la p2, on va tester toutes les points entre minmaxTry.first et minmaxTry.second
	std::size_t p2 = 0;

	for (std::int32_t y = minmaxTry.first.y; y <= minmaxTry.second.y; y++) {
		for (std::int32_t x = minmaxTry.first.x; x <= minmaxTry.second.x; x++) {
			//On itère jusqu'à soit qu'on soit dans la zone, soit qu'on soit derrière
			Position probe(x, y);
			Position velocity(x > 0 ? x-1 : 0, y-1);

			while (!probe.isAfter(targetArea.second.x, targetArea.first.y)) {
				if (probe.isInside(targetArea.first, targetArea.second)) {
					p2++;
					break;
				}

				//On modifie la position de prob et de la velocity
				probe = probe + velocity;
				velocity = velocity + Position(velocity.x > 0 ? -1 : 0, -1);
			}
		}
	}

	//On commence le chrono
	auto end = std::chrono::steady_clock::now();

	std::cout << "Part 1: " << p1 << std::endl;
	std::cout << "Part 2: " << p2 << std::endl;
	std::cout << "Time: " << std::chrono::duration_cast<std::chrono::microseconds>(end-start).count() << " microseconds" << std::endl;

	return 0;
}
