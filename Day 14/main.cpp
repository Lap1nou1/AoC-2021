#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <array>
#include <cstdint>
#include <chrono>

std::uint64_t getSolution(std::map<std::string, std::uint64_t>& polymerPairs, char lastChar) {
	//D'abord, on regarde le nombre de caractère au total
	std::map<char, std::uint64_t> count;
	
	for (auto pair = polymerPairs.begin(); pair != polymerPairs.end(); ++pair) {
		//On compte que le début des pairs, car la fin de l'un est le début de l'autre (sauf le tout dernier, mais c'est un cas spécial un peu et c'est qu'un caractère donc NIK (je prie pour que ça me nique pas))
		//NB : isok j'ai ajouté le cas
		count[pair->first[0]] += pair->second;
	}

	//On ajoute le tout TOUT dernier caractère
	count[lastChar]++;

	//Maintenant on cherche le min et le max
	std::uint64_t min = -1, max = 0;

	for (auto cnt = count.begin(); cnt != count.end(); ++cnt) {
		if (cnt->second > max)
			max = cnt->second;
		if (cnt->second < min)
			min = cnt->second;
	}

	return max - min;
}

//Fait la polymérisation d'une séquence de 2 caractère n fois à partir de pairInsertion, modifie l'array counter (qui va de 0 à 25, pour chacune des lettres de l'alphabet)
void polymerization(std::map<std::string, std::uint64_t>& polymerPairs, std::map<std::string, char>& pairInsertion) {
	//On initialise un nouveau map
	std::map<std::string, std::uint64_t> newPolymer;
	
	//On regarde chaque membre de la map
	for (auto pair = polymerPairs.begin(); pair != polymerPairs.end(); ++pair) {
		//D'abord on regarde l'enfant des deux
		char newChar = pairInsertion[pair->first];

		//Puis on cherche les deux côtés
		std::string lhs, rhs;
		lhs = (lhs + pair->first[0]) + newChar;
		rhs = (rhs + newChar) + pair->first[1];

		//Maintenant on dit que n * [AB] est égal à n * [AC] + n * [CB]
		newPolymer[lhs] += pair->second;
		newPolymer[rhs] += pair->second;
	}

	polymerPairs = newPolymer;
}

int main() {
	//On commence le chrono
	auto start = std::chrono::steady_clock::now();

	//On charge le fichier
	std::ifstream inputFile("input.txt");

	if (!inputFile.is_open()) {
		return -1;
	}

	//On lit d'abord le template de base
	std::string polymer;
	std::getline(inputFile, polymer);

	//On interprète la suite, avec un hashmap
	std::map<std::string, char> pairInsertion;
	std::map<std::string, std::uint64_t> polymerPairs;	//Compte les différentes paires qu'on a (et le nombre dont on a à un niveau t)

	std::string line; 
	std::getline(inputFile, line); //On retire le ligne vide
	while (std::getline(inputFile, line)) {
		//On regarde les deux premiers caractères
		std::string input = line.substr(0,2);
		char output = line[6];

		//On met dans le map
		pairInsertion[input] = output;
		polymerPairs[input] = 0;
	}

	//On initialise le compteur de pair
	for (std::size_t i = 1; i != polymer.size(); i++) {
		std::string pair;
		pair = (pair + polymer[i - 1]) + polymer[i];
		
		polymerPairs[pair]++;
	}

	//Maintenant on fait la polymérisation (10x)
	for (std::size_t i = 0; i < 10; i++) {
		polymerization(polymerPairs, pairInsertion);
	}

	std::uint64_t p1 = getSolution(polymerPairs, polymer[polymer.size()-1]);

	//Maintenant on fait la polymérisation (30x) pour la p2
	for (std::size_t i = 0; i < 30; i++) {
		polymerization(polymerPairs, pairInsertion);
	}

	std::uint64_t p2 = getSolution(polymerPairs, polymer[polymer.size() - 1]);

	//On fini le chrono
	auto end = std::chrono::steady_clock::now();

	std::cout << "Part 1: " << p1 << std::endl;
	std::cout << "Part 2: " << p2 << std::endl;
	std::cout << "Time: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " microseconds" << std::endl;

	return 0;
}
