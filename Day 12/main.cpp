#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>
#include <vector>
#include <map>
#include <utility>
#include <chrono>

struct Cave {
	std::string name;
	std::vector<Cave*> links;	//Lien vers les autres nodes
	bool open = true;			//Dis si le chemin est ouvert
	bool isSmallCave = false;	//Dis si c'est une cave petite (nom en minuscule)

	//Constructeur
	Cave(std::string _name, bool _small) : name(_name), isSmallCave(_small) {};
	Cave(std::string _name) : name(_name) {};
};

struct Route {
	Cave* here;	//Là où on est maintenant dans la recherche
	std::uint8_t smallCavePassage;	//Nombre de passage autorisé dans une même petite cave pour cette route
	std::string path;

	Route(Cave* _h, std::uint8_t _nbPassage, std::string _path) : here(_h), smallCavePassage(_nbPassage), path(_path) {};
	Route() : here(nullptr), smallCavePassage(1), path("") {};
};

//Pour le debug
void printCaves(const std::map<std::string, Cave*>& caves) {
	//On regarde chaque cave
	for (auto currentCave = caves.begin(); currentCave != caves.end(); ++currentCave) {
		//On regarde le nom
		std::cout << currentCave->second->name;
		
		if (currentCave->second->links.size() != 0) {
			std::cout << " => " << currentCave->second->links[0]->name;

			for (auto caveLink = currentCave->second->links.begin() + 1; caveLink != currentCave->second->links.end(); ++caveLink) {
				std::cout << ", " << (*caveLink)->name;
			}
		}

		std::cout << std::endl;
	}
}

//Ajoute une route à un vector de route
void addRoute(std::vector<Route>& routes, Cave* cave, std::uint8_t smallCavePassage, std::string& takenRoute) {
	//D'abord on crée la nouvelle route
	Route newRoute;

	//Le chemin de cette nouvelle route est le chemin de route avec; en plus, le nom de la route qu'on ajoute
	newRoute.path = takenRoute + cave->name + ',';

	//Le pointeur vers cette cave est *way (way est un pointeur de pointeur
	newRoute.here = cave;

	newRoute.smallCavePassage = smallCavePassage;

	//Maintenant on ajoute way aux nouvelles routes
	routes.push_back(newRoute);
}

//Compte le nombre de fois qu'apparait une suite de caractère dans un string
std::uint8_t countSequenceString(std::string& base, std::string searched) {
	std::uint8_t nbOfTime = 0;

	std::size_t pos = base.find(searched);
	while (pos != std::string::npos) {
		nbOfTime++;

		pos = base.find(searched, pos+1);
	}

	return nbOfTime;
}

//Compte le nombre de chemin possible à partir de start, pour un certain nombre de passage par petite cave donné (qu'une fois, après on retourne à 1)
std::uint32_t countNumberOfRoutes(Cave* start, std::uint8_t smallCavePassage) {
	//On compte naïvement
	//Pour ça on va faire le chemin de toutes les routes
	//Chaque membre de ce vector contient une route unique sous la forme d'un pointeur vers la cave où il est actuellement
	//et d'un string sous la forme "start,A,b,end" correspondant au chemin pris
	std::vector<Route> routes = { Route(start, smallCavePassage, start->name + ',')};
	std::uint32_t nbOfRoutes = 0;

	while (routes.size() != 0) {

		//Ce vector prendra toutes les nouvelles routes obtenus
		std::vector<Route> newRoutes;

		//On regarde toutes les routes actuelles
		for (auto route = routes.begin(); route != routes.end(); ++route) {
			//On regarde si on est à la fin (donc si route->first->name == "end"), si on est à la fin on ajoute 1 au compteur et testons le prochain chemin
			if (route->here->name == "end") {
				nbOfRoutes++;
				continue;
			}

			//On regarde chaque chemin possible à partir de la route
			for (auto way = route->here->links.begin(); way != route->here->links.end(); ++way) {
				//Si ce n'est pas une petite cave on le fait toujours
				if (!(*way)->isSmallCave || countSequenceString(route->path, ',' + (*way)->name + ',') < 1) {
					addRoute(newRoutes, *way, route->smallCavePassage, route->path);
				}
				//Si c'est une petite cave, alors on regarde si on l'a pas déjà trop prise (on ajoute une virgule dans la position pour pas que ça trouve le "a" dans "start" par exemple)
				//C'est dans une consition différente pour pouvoir changé le nombre de passage autorisé en 1
				else if (countSequenceString(route->path, ',' + (*way)->name + ',') < route->smallCavePassage) {
					addRoute(newRoutes, *way, 1, route->path);
				}
			}
		}

		routes = newRoutes;
	}

	return nbOfRoutes;
}

int main() {
	//On commence le temps
	auto start = std::chrono::steady_clock::now();

	//On charge le fichier
	std::ifstream inputFile("input.txt");

	if (!inputFile.is_open()) {
		return -1;
	}

	//On lit et interprète le fichier
	std::map<std::string, Cave*> caves;

	for (std::string line; std::getline(inputFile, line);) {
		//On divise l'instruction en deux caves, la première cave (avant le -) et la seconde (après le -)
		std::string firstCaveName = line.substr(0, line.find('-'));
		std::string secondCaveName = line.substr(line.find('-') + 1);

		//On prend la correspondance dans le map
		Cave** firstCave = &caves[firstCaveName];
		Cave** secondCave = &caves[secondCaveName];

		//Si jamais c'est la première fois qu'on voit ses caves, on les crée
		if (*firstCave == nullptr) {
			*firstCave = new Cave(firstCaveName, 'a' <= firstCaveName[0]); //'a' <= firstCaveName[0] renverra true si c'est une petite cave toutes les lettres après 'a' sont minuscules (les majuscules sont avant)
		}

		if (*secondCave == nullptr) {
			*secondCave = new Cave(secondCaveName, 'a' <= secondCaveName[0]); //'a' <= firstCaveName[0] renverra true si c'est une petite cave toutes les lettres après 'a' sont minuscules (les majuscules sont avant)
		}

		//Maintenant on ajoute à chacun d'eux un pointeur vers l'autre
		//On a deux cas particulier : end ne doit pouvoir aller nul part, et personne ne doit pouvoir aller à start
		if (secondCaveName != "start" && firstCaveName != "end") {
			(*firstCave)->links.push_back(*secondCave);
		}

		if (firstCaveName != "start" && secondCaveName != "end") {
			(*secondCave)->links.push_back(*firstCave);
		}
	}

	//printCaves(caves);

	//On utilise la fonction pour la p1 et la p2
	std::uint32_t p1 = countNumberOfRoutes(caves["start"], 1);
	std::uint32_t p2 = countNumberOfRoutes(caves["start"], 2);

	//On finit le temps
	auto end = std::chrono::steady_clock::now();

	std::cout << "Part 1: " << p1 << std::endl;
	std::cout << "Part 2: " << p2 << std::endl;
	std::cout << "Time: " << std::chrono::duration_cast<std::chrono::microseconds>(end-start).count() << " microseconds" << std::endl;

	return 0;
}
