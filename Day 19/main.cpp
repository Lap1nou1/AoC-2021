#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>
#include <utility>
#include <vector>
#include <functional>
#include <set>
#include <algorithm>
#include <chrono>

struct Point {
	std::int64_t x = 0, y = 0, z = 0;

	//Constructeur
	Point(std::int64_t _x, std::int64_t _y, std::int64_t _z) : x(_x), y(_y), z(_z) {};
	Point() : x(0), y(0), z(0) {};

	//On défini différents opérateurs d'un point par un autre
	Point operator-(const Point& rhs) const {
		return Point(this->x - rhs.x, this->y - rhs.y, this->z - rhs.z);
	}

	Point operator+(const Point& rhs) const {
		return Point(this->x + rhs.x, this->y + rhs.y, this->z + rhs.z);
	}

	bool operator==(const Point& rhs) const {
		return this->x == rhs.x && this->y == rhs.y && this->z == rhs.z;
	}

	//La multiplication par un nombre
	Point operator*(const std::int64_t mult) const {
		return Point(this->x * mult, this->y * mult, this->z * mult);
	}
};

struct PointCmp {
	bool operator()(const Point& lhs, const Point& rhs) const {
		return lhs.z < rhs.z || (lhs.z == rhs.z && lhs.y < rhs.y || (lhs.y == rhs.y && lhs.x < rhs.x));
	}
};

//On crée 3 fonctions qui effectue une rotation à 90° sur un axe
void rotateX(Point& toRotate) {
	std::int64_t y = toRotate.y;
	toRotate.y = -toRotate.z;
	toRotate.z = y;
}

void rotateY(Point& toRotate) {
	std::int64_t x = toRotate.x;
	toRotate.x = toRotate.z;
	toRotate.z = -x;
}

void rotateZ(Point& toRotate) {
	std::int64_t x = toRotate.x;
	toRotate.x = -toRotate.y;
	toRotate.y = x;
}

//On crée un vecteur comportant les 3 différentes 
const std::vector<std::function<void(Point&)>> rotations = {rotateX, rotateY, rotateZ};

//On fait les différentes faces
void facingXP(Point& toFace) {
	//Dans ce cas là on ne fait rien
}

void facingYP(Point& toFace) {
	std::int64_t x = toFace.x;
	toFace.x = toFace.y;
	toFace.y = -x;
}

void facingZP(Point& toFace) {
	std::int64_t x = toFace.x;
	toFace.x = toFace.z;
	toFace.z = -x;
}

void facingXN(Point& toFace) {
	toFace.z *= -1;
	toFace.x *= -1;
}

void facingYN(Point& toFace) {
	std::int64_t x = toFace.x;
	toFace.x = -toFace.y;
	toFace.y = x;
}

void facingZN(Point& toFace) {
	std::int64_t x = toFace.x;
	toFace.x = -toFace.z;
	toFace.z = x;
}

const std::vector<std::function<void(Point&)>> facings = { facingXP, facingYP, facingZP, facingXN, facingYN, facingZN };

//Renvoie la liste de tous les vecteurs permettant de passer du point siart aux points destinations
template<class T>
std::vector<Point> calculateEveryVector(const T& destinations, const Point& start) {
	std::vector<Point> vectors;

	for (auto point = destinations.begin(); point != destinations.end(); ++point) {
		vectors.push_back(*point - start);
	}

	return vectors;
}

//Renvoie une pair, le premier élément correspond au nombre d'élément de vec1 correspondant à vec2, le second correspond à une pair des indices éléments correspondant
std::pair<std::size_t, std::vector<std::pair<std::size_t, std::size_t>>> findVectorsInCommon(const std::vector<Point>& vec1, const std::vector<Point>& vec2) {
	std::size_t count = 0;
	std::vector<std::pair<std::size_t, std::size_t>> indexes;

	//On regarde tous les éléments de vec1
	for (std::size_t i = 0; i < vec1.size(); i++) {
		//On regarde tous les éléments de vec2
		for (std::size_t j = 0; j < vec2.size(); j++) {
			//Si deux éléments sont égaux, on augmente le compteur et ajoutons leur indice
			if (vec1[i] == vec2[j]) {
				count++;
				indexes.push_back(std::make_pair(i, j));
			}
		}
	}

	return std::make_pair(count, indexes);
}

//Met dans fixedPoints tous les points résultant de la translation de foundPoint par les vecteurs de foundVectors (on modifie aussi foundPoints)
void addPoint(std::set<Point, PointCmp>& fixedPoints, Point foundPoint, std::vector<Point>& foundVectors, std::vector<Point>& foundPoints) {
	//On regarde tous les points et les vecteurs en même temps et on les ajoute dans fixedPoints
	for (std::size_t i = 0; i < foundVectors.size(); i++) {
		fixedPoints.insert(foundPoint + foundVectors[i]);
		foundPoints[i] = foundPoint + foundVectors[i];
	}
}

void printScanners(const std::vector<std::vector<Point>>& scanners) {
	for (std::size_t i = 0; i < scanners.size(); i++) {
		std::cout << "--- scanner " << i << " ---" << std::endl;
		for (auto p = scanners[i].begin(); p != scanners[i].end(); ++p) {
			std::cout << p->x << ',' << p->y << ',' << p->z << std::endl;
		}
		std::cout << std::endl;
	}
}

void printPoints(const std::vector<Point>& points) {
	for (auto p = points.begin(); p != points.end(); ++p) {
		std::cout << p->x << ',' << p->y << ',' << p->z << std::endl;
	}
}

int main() {
	//On commence le temps
	auto start = std::chrono::steady_clock::now();

	//On charge le fichier
	std::ifstream inputFile("input.txt");

	if (!inputFile.is_open()) {
		return -1;
	}

	//On interprète le fichier
	//On va sauvegarder tous les enregistrements des scanners comme étant une liste des points d'un scanner
	//Le i-ième élément indiquera donc tous les points enregistrer par le i-ième scanner
	std::vector<std::vector<Point>> scannersDetection;

	for (std::string line; std::getline(inputFile, line);) {
		//On est ici soit lorsqu'on commence la lecture soit lorsqu'on atteint un nouveau scanner
		std::vector<Point> detection;	//Va garder tous les points qu'on lira avec ce scanner

		//On va maintenant lire les points
		while (std::getline(inputFile, line)) {
			//Si on a une ligne vide, on est à un nouveau scanner donc on se barre
			if (line.empty()) 
				break;

			//On lit le point
			Point newPoint;

			newPoint.x = std::stoll(line);

			std::size_t pos = line.find(',') + 1;
			newPoint.y = std::stoll(line.substr(pos));

			pos = line.find(',', pos) + 1;
			newPoint.z = std::stoll(line.substr(pos));

			detection.push_back(newPoint);
		}

		//On a fini de lire ce scanner, on met tout dans scannersDetection
		scannersDetection.push_back(detection);
	}

	//Maintenant on va chercher à savoir la position des points par rapport au scanner 0 (qu'on définit comme l'origine du repère)
	//On crée le set qui va contenir tous les points trouvés, au début nous n'avons que les points du premier détecteur
	std::set<Point, PointCmp> beacons(scannersDetection[0].begin(), scannersDetection[0].end());

	//On crée maintenant un vecteur de pointeur de vecteur de point (scanner) dont il nous reste à trouver la position sur notre repère
	std::vector<std::vector<Point>*> toFind;
	std::for_each(scannersDetection.begin() + 1, scannersDetection.end(), [&toFind](std::vector<Point>& scan) { toFind.push_back(&scan); }); //On y ajoute un pointeur vers tous les scanners excepté le premier

	std::vector<std::vector<Point>*> found = { &scannersDetection[0] };

	std::vector<Point> scannersPosition = { Point(0,0,0) };	//On va stocker la position de tous les scanners pour la p2

	//Tant qu'on a pas trouver tous les scanners, on continu de chercher
	while (!toFind.empty()) {
		bool isFound = false;	//Permet de quitter tous les for une fois des points fixés
		std::size_t toErase = -1;

		//On va regarder tous les scanners qu'on a pas encore identifié
		for (auto scanDetect = toFind.begin(); !isFound && scanDetect != toFind.end(); ++scanDetect) {
			//On va regarder chaque scanner trouvé
			for (auto foundScan = found.begin(); !isFound && foundScan != found.end(); ++foundScan) {
				for (auto foundPoint = (*foundScan)->begin(); !isFound && foundPoint != (*foundScan)->end(); ++foundPoint) {
					//On prend la liste des vecteurs du point vers les autres
					std::vector<Point> vectorsFromFixed = calculateEveryVector<std::vector<Point>>(**foundScan, *foundPoint);

					//Maintenant, on regarde les différentes faces de scanDetect
					for (std::size_t i = 0; !isFound && i < facings.size(); i++) {
						//On copie scanDetect
						std::vector<Point> facedScan = (**scanDetect);

						//On applique le fonction pour que ça face le bon endroit
						std::for_each(facedScan.begin(), facedScan.end(), facings[i]);

						//Maintenant on fait chaque rotation
						for (std::size_t j = 0; !isFound && j < 4; j++) {
							//On regarde maintenant chaque point de scan Detect
							for (auto scannedPoint = facedScan.begin(); !isFound && scannedPoint != facedScan.end(); ++scannedPoint) {
								//On prend la liste des vecteurs du point vers les autres
								std::vector<Point> vectorsFromScan = calculateEveryVector<std::vector<Point>>(facedScan, *scannedPoint);

								//On compte les vecteurs en commun
								auto inCommon = findVectorsInCommon(vectorsFromFixed, vectorsFromScan);

								//Si on a plus de 12 points en commun, alors on peut deviner la position des beacons de scanDetect dans le repère
								if (inCommon.first >= 12) {
									isFound = true;
									addPoint(beacons, *foundPoint, vectorsFromScan, *toFind[std::distance(toFind.begin(), scanDetect)]);
									scannersPosition.push_back(*foundPoint - *scannedPoint);
									toErase = std::distance(toFind.begin(), scanDetect);
								}
							}

							//On applique la rotation (que si c'est pas trouvé)
							if (!isFound)
								std::for_each(facedScan.begin(), facedScan.end(), rotations[i % 3]);
						}
					}
				}
			}
		}

		//On ajoute celui qu'on a trouvé
		found.push_back(toFind[toErase]);

		//On efface celui qu'on a trouvé
		toFind.erase(toFind.begin() + toErase);
	}

	//On cherche la distance minimal entre les scanners
	std::uint64_t distanceMax = 0;

	for (std::size_t i = 0; i < scannersPosition.size(); i++) {
		for (std::size_t j = i + 1; j < scannersPosition.size(); j++) {
			//on calcul la distance
			std::uint64_t distance = abs(scannersPosition[i].x - scannersPosition[j].x) + abs(scannersPosition[i].y - scannersPosition[j].y) + abs(scannersPosition[i].z - scannersPosition[j].z);

			if (distance > distanceMax) {
				distanceMax = distance;
			}
		}
	}

	//On finit le temps
	auto end = std::chrono::steady_clock::now();

	std::cout << "Part 1: " << beacons.size() << std::endl;
	std::cout << "Part 2: " << distanceMax << std::endl;
	std::cout << "Time: " << std::chrono::duration_cast<std::chrono::microseconds>(end-start).count() << " microseconds" << std::endl;
	std::cout << "Time (long): " << std::chrono::duration_cast<std::chrono::hours>(end - start).count() << ":" << std::chrono::duration_cast<std::chrono::minutes>(end - start).count() << ":" << std::chrono::duration_cast<std::chrono::seconds>(end - start).count() << "sec" << std::endl;

	return 0;
}
