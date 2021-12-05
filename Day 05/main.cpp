#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <utility>
#include <cstdint>

struct Position
{
	std::int32_t x;
	std::int32_t y;

	Position(std::int32_t _x, std::int32_t _y) : x(_x), y(_y) {}
	Position() : x(0), y(0) {}
	Position operator*(const std::int32_t& rhs) { return Position(this->x * rhs, this->y * rhs); }
	Position operator+(const Position& rhs) { return Position(this->x + rhs.x, this->y + rhs.y); }
	bool operator==(const Position& rhs) { return this->x == rhs.x && this->y == rhs.y; }
	bool operator!=(const Position& rhs) { return this->x != rhs.x || this->y != rhs.y; }
};

enum Directions {
	nonOrthogonal = 0,
	yStraight = 1,
	xStraight = 2
};

struct PointCmp {
	bool operator()(const Position& p1, const Position& p2) const {
		if (p1.x == p2.x)
			return p1.y < p2.y;
		return p1.x < p2.x;
	}
};

//Transforme une ligne de l'input en pair de Position compréhensible
std::pair<Position, Position> transformLineToVent(const std::string &line) {
	Position pos1, pos2;

	std::size_t pos = line.find(",")+1;

	//On prend x1
	pos1.x = std::stoi(line);

	//On prend y1
	pos1.y = std::stoi(line.substr(pos));

	//On trouver x2
	pos = line.find("-> ") + 3;
	
	//On prend x2
	pos2.x = std::stoi(line.substr(pos));

	//On trouve y2
	pos = line.find(",", pos)+1;

	//On prend y2
	pos2.y = std::stoi(line.substr(pos));

	//Le premier point sera celui avec le x le plus petit (et s'ils sont égaux le y le plus petit)
	std::pair<Position, Position> hydroVent;

	if (pos1.x > pos2.x || (pos1.x == pos2.x && pos1.y > pos2.y)) {
		hydroVent.first = pos2;
		hydroVent.second = pos1;
	}
	else {
		hydroVent.second = pos2;
		hydroVent.first = pos1;
	}

	return hydroVent;
}

//Donne la direction de la droite si elle suit le gradrillage (x ou y) sinon envoie false
Directions isVentOrthogonal(const std::pair<Position, Position>& vent) {
	if (vent.first.x == vent.second.x)
		return Directions::xStraight;
	if (vent.first.y == vent.second.y)
		return Directions::yStraight;
	return Directions::nonOrthogonal;
}

//Calcule le vecteur "uniforme" (qu'avec des 0 et des 1) directeur de la vent avec x > 0, si x = 0 alors y > 0 
//Si on change le sens du vecteur on inverse aussi l'ordre des points  de la vent
Position normalizeVector(std::pair<Position, Position>& vent) {
	Position normalized;

	if ((vent.first.x > vent.second.x) || (vent.first.x == vent.second.x && vent.first.y > vent.second.y)) {
		auto c = vent.first;
		vent.first = vent.second;
		vent.second = c;
	}

	normalized.x = vent.second.x - vent.first.x;
	normalized.y = vent.second.y - vent.first.y;

	//On le "normalise" car ce sont forcément des angles à 45°, on peut juste diviser chaque normalized.x (qui est toujours positif, exception lorsque normalized.x == 0)
	if (normalized.x == 0) {
		normalized.y /= normalized.y;
	}
	else {
		normalized.y /= normalized.x;
		normalized.x /= normalized.x;
	}

	return normalized;
}

int main() {
	//On charge le fichier
	std::ifstream inputFile("input.txt");

	//On parse
	std::vector<std::pair<Position, Position>> vents; 

	for (std::string line; std::getline(inputFile, line);) {
		vents.push_back(transformLineToVent(line));
	}

	//Maintenant on regarde chaque vents une par une, si c'est une ligne horizontale/verticale on la compare aux autres
	std::set<Position, PointCmp> simpleIntersections;

	//Pour les intersections diagonales
	std::set<Position, PointCmp> diagIntersections;
	
	for (std::size_t i = 0; i < vents.size(); i++) {
		//On garde un pointeur vers la vent en question, on calcul le vecteur normalisé ainsi que son "orthogonalité"
		auto vent1 = &vents[i];
		Directions dirVent1 = isVentOrthogonal(*vent1);
		Position normalizedVent1 = normalizeVector(*vent1);

		//On itère dans toutes les vents où on a pas encore fait les combinaisons
		for (std::size_t j = i + 1; j < vents.size(); j++) {
			//On garde un pointeur vers la vent en question, on calcul le vecteur normalisé ainsi que son "orthogonalité"
			auto vent2 = &vents[j];
			Directions dirVent2 = isVentOrthogonal(*vent2);
			Position normalizedVent2 = normalizeVector(*vent2);

			//Deux cas : 
			//Premier cas : droites sécantes
			if (normalizedVent1 != normalizedVent2) {
				//Dans ce cas, il y a le cas où une des coordonnées de chaque vecteur est nulle
				if (dirVent1 != Directions::nonOrthogonal && dirVent2 != Directions::nonOrthogonal) {
					//On regarde quel segment est constant en x et constant en y
					auto yConstant = vent1;
					auto xConstant = vent2;

					if (dirVent1 == Directions::xStraight) {
						yConstant = vent2;
						xConstant = vent1;
					}

					//Le point d'intersection est (a;b)
					//On calcul puis on regarde si le point d'intersection est dans segment constant en y
					Position inter(xConstant->first.x, yConstant->first.y);

					//On check si le point d'intersection est bien dans l'encadrement de la droite constante de y, si oui, on regarde si les y sont corrects
					if ((yConstant->first.x <= inter.x && inter.x <= yConstant->second.x) && (xConstant->first.y <= inter.y && inter.y <= xConstant->second.y)) {
						//L'intersection est dans le segment, on l'ajoute au set
						simpleIntersections.insert(inter);
						diagIntersections.insert(inter);
					}
				}
				//On a aussi le cas où seul un des vecteurs a une coordonnée nulle
				else if (dirVent1 != Directions::nonOrthogonal || dirVent2 != Directions::nonOrthogonal) {
					//Dans ce cas, on cherche quel droite est "orthogonal"
					auto straight = vent1;
					auto diag = vent2;
					auto normalizedDiag = &normalizedVent2;

					if (dirVent2 != Directions::nonOrthogonal) {
						straight = vent2;
						diag = vent1;
						normalizedDiag = &normalizedVent1;
					}

					Position inter;

					//Maintenant, on regarde quel position est constante
					//Ici, c'est x
					if (straight->first.x == straight->second.x) {
						inter.x = straight->first.x;

						//x (du vecteur) est toujours positif, donc pour calculer combien de "pas" il faut faire pour arriver à point d'intersection à partir du premier point de la diagonal, c'est une simple soustraction
						inter.y = diag->first.y + normalizedDiag->y * (inter.x - diag->first.x);
					
						//Maintenant on vérifie que l'intersection fait bien partie des segment
						//Pour ça on regarde si la coordonnée y d'inter est bien entre les bornnes du segment straight
						//Et si le point d'intersection est dans le segment diag, pour ça on regarde si les x correspondent
						if (straight->first.y <= inter.y && inter.y <= straight->second.y && diag->first.x <= inter.x && inter.x <= diag->second.x) {
							//On ajoute au set
							diagIntersections.insert(inter);
						}
					}
					//Ici, c'est y
					else {
						//On effectue la translation
						inter = diag->first + (*normalizedDiag * normalizedDiag->y) * (straight->first.y - diag->first.y);
					
						//Maintenant on vérifie que l'intersection fait bien partie du segment intersection
						//Pour ça on regarde si la coordonnée x d'inter est bien entre les bornnes du segment straight
						//Et si le point d'intersection est dans le segment diag, pour ça on regarde si les x correspondent
						if (straight->first.x <= inter.x && inter.x <= straight->second.x && diag->first.x <= inter.x && inter.x <= diag->second.x) {
							//On ajoute au set
							diagIntersections.insert(inter);
						}
					}
				}
				//Finalement, on a le cas des deux diagonales
				else {
					//Vu que x>0 (et donc x=1 dans ce cas), le coefficient directeur des deux droites est le y de leur vecteur normalisé
					//Donc on calcul les deux p
					std::int32_t p1 = vent1->first.y - normalizedVent1.y * vent1->first.x;
					std::int32_t p2 = vent2->first.y - normalizedVent2.y * vent2->first.x;

					//On a juste a résoudre l'équation mx + p = m'x + p' pour trouver le x, puis le y
					Position inter;
					
					inter.x = (p2 - p1) / (normalizedVent1.y - normalizedVent2.y);
					inter.y = normalizedVent1.y * inter.x + p1;

					//On vérifie que le point appartient bien aux deux segments
					//Pour ça, on regarde la position x du point d'intersections et des droites
					if (vent1->first.x <= inter.x && inter.x <= vent1->second.x && vent2->first.x <= inter.x && inter.x <= vent2->second.x) {
						diagIntersections.insert(inter);
					}
				}
			}
			//Second cas : droites parallèles
			else {
				//Si les droites sont parallèles, on cherche d'abord à savoir si elles ont un point en commun
				//Pour ça on a deux cas, soit les droites sont "orthogonales" dans quel cas c'est pareil à la part 1
				if (dirVent1 != Directions::nonOrthogonal) {
					//On regarde la partie qui n'est pas égale, pInter correspond au plus tard début et à la plus tôt fin
					std::pair<std::int32_t*, std::int32_t*> pInter;

					if (dirVent1 == Directions::xStraight) {
						if (vent1->first.y > vent2->first.y) {
							pInter.first = &vent1->first.y;
						}
						else {
							pInter.first = &vent2->first.y;
						}

						if (vent1->second.y < vent2->second.y) {
							pInter.second = &vent1->second.y;
						}
						else {
							pInter.second = &vent2->second.y;
						}
					}
					else {
						if (vent1->first.x > vent2->first.x) {
							pInter.first = &vent1->first.x;
						}
						else {
							pInter.first = &vent2->first.x;
						}

						if (vent1->second.x < vent2->second.x) {
							pInter.second = &vent1->second.x;
						}
						else {
							pInter.second = &vent2->second.x;
						}
					}

					//Maintenant, les intersections sont tous les points tels que on commence au début de p2 et on aille à la fin de p1
					//On vérifie avant le sens, et si les droites sont potentiellement superposés
					if (dirVent1 == Directions::xStraight && vent1->first.x == vent2->first.x) {
						for (std::int32_t _y = *pInter.first; _y <= *pInter.second; _y++) {
							simpleIntersections.insert(Position(vent1->first.x, _y));
							diagIntersections.insert(Position(vent1->first.x, _y));
						}
					}
					else if (dirVent1 == Directions::yStraight && vent1->first.y == vent2->first.y) {
						for (std::int32_t _x = *pInter.first; _x <= *pInter.second; _x++) {
							simpleIntersections.insert(Position(_x, vent1->first.y));
							diagIntersections.insert(Position(_x, vent1->first.y));
						}
					}
				}
				//Sinon nos droites sont diagonales
				else {
					//Dans ce cas on cherche d'abord à savoir si les droites sont confondues, on cherche le m et le p de la première
					//Le m est le y de normalizedVent1
					//Le p peut se calculer
					std::int32_t p = vent1->first.y - normalizedVent1.y * vent1->first.x;

					//Maintenant, on regarde si avec l'équation de la première droite y = mx + p, avec le x d'un point de la second on arrive bien au y de ce même point
					if (vent2->first.y == normalizedVent1.y * vent2->first.x + p) {
						//Maintenant, on cherche la vent le plus "proche" et le plus "loin"
						auto furthest = vent1;
						auto nearest = vent2;

						if (vent2->first.x > vent1->first.x) {
							furthest = vent2;
							nearest = vent1;
						}

						//Le "début" sera le segment le début du segment le plus "loin"
						//Tandis que la "fin" sera soit le fin du segment le plus "proche", soit celui du plus "loin"
						std::uint32_t end = nearest->second.x;

						if (end > furthest->second.x) {
							end = furthest->second.x;
						}

						//On commence du "début" à la "fin"
						for (std::size_t start = furthest->first.x; start <= end; start++) {
							diagIntersections.insert(Position(0,p) + normalizedVent1 * start);
						}
					}
				}
			}
		}
	}

	std::cout << "Part 1: " << simpleIntersections.size() << std::endl;
	std::cout << "Part 2: " << diagIntersections.size() << std::endl;

	//Pour le debut (vérification des points)
	/*
	for (auto el : diagIntersections) {
		std::cout << "(" << el.x << "," << el.y << ")" << std::endl;
	}
	*/

	return 0;
}
