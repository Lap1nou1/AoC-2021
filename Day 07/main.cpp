#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>
#include <numeric>
#include <chrono>

struct Node {
	std::uint16_t value;	//Valeur du noeud
	std::uint16_t coeff;	//Nombre de fois qu'on a notre valeur (de base égal à 1)
	Node* next;				//Noeud suivant

	Node(std::uint16_t v) : value(v), coeff(1), next(nullptr) {};
	Node() : value(0), coeff(1), next(nullptr) {};
};

class LinkedSortedVector {
	Node* first;
	Node* last;
	std::size_t size = 0;		//Nombre d'élément total
	std::size_t trueSize = 0;	//Nombre d'élément différent

public:
	LinkedSortedVector() : size(0), first(nullptr) {};

	//Ajoute un élément dans le vecteur, en le triant
	void insert(std::uint16_t value) {
		//Cas particulier où on place le premier élément
		if (this->size == 0) {
			this->first = new Node(value);
			this->last = this->first;
			this->trueSize++;
		}
		//Cas particulier : On est au dessous le premier élément
		else if (value < this->first->value) {
			Node* newNode = new Node(value);

			newNode->next = this->first;
			this->first = newNode;
			this->trueSize++;
		}
		else {

			Node* newNode = new Node(value);

			//On cherche où on le place, en regardant toujours celà avant
			Node* searching = this->first;
			for (std::size_t i = 1; i < this->trueSize - 1; i++) {
				//On change le Node de recherche
				searching = searching->next;

				if (searching->next->value > value) {
					break;
				}
			}

			//Une fois l'élément trouvé, si il existe déjà, on augmente juste le coeff, sinon on ajoute entièrement un nouvelle élément
			if (searching->value == value) {
				searching->coeff++;
			}
			else {
				//On place l'élément
				newNode->next = searching->next;
				searching->next = newNode;
				this->trueSize++;
			
				//Si jamais on ajoute le node tout à la fin, on le précise
				if (newNode->next == nullptr) {
					this->last = newNode;
				}
			}
		}

		this->size++;
	}

	Node* getFirst() {
		return this->first;
	}

	Node* getLast() {
		return this->last;
	}

	std::uint16_t getMedian() {
		std::size_t medianPos = this->size / 2;

		Node* node = this->first;

		for (std::size_t i = node->coeff; i < medianPos; i += node->coeff) {
			node = node->next;
		}

		return node->value;
	}

	float getMean() {
		std::uint16_t sum = 0;

		Node* node = this->first;

		while (node != nullptr) {
			sum += node->value * node->coeff;

			node = node->next;
		}

		return sum / float(this->size);
	}
};

void printLinked(LinkedSortedVector& v) {
	Node* n = v.getFirst();

	while (n != nullptr) {
		std::cout << n->value << std::endl;

		n = n->next;
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

	//On va mettre toute nos variables dans un set pour pouvoir ensuite prendre la médiane
	LinkedSortedVector crabSubmarines;

	//On lit la ligne
	std::size_t pos = -1;
	std::string line;
	std::getline(inputFile, line);

	do {
		pos++;

		//On lit et ajoute la position du crab
		crabSubmarines.insert(std::stoi(line.substr(pos)));

		//On cherche le prochain nombre
		pos = line.find(',', pos);
	} while (pos != -1);

	//Maintenant on prend la médiane, puis on calcul la distance entre chaque nombre et la médiane
	std::uint16_t median = crabSubmarines.getMedian();
	std::uint32_t sumDistancePart1 = 0;

	Node* node = crabSubmarines.getFirst();
	while (node != nullptr) {
		//Pour la p1 on effectue une simple addition
		sumDistancePart1 += abs(node->value - median) * node->coeff;

		node = node->next;
	}

	//Pour la p2 on fait, on bruteforce, mais intelligemment
	std::uint32_t sumDistancePart2 = -1;

	//Pour ça, on va itérer dans chaque possibilité de position entre le minimum et le maximum des valeurs
	//Pour chacune de ces positions on va calculer le nombre de fuel requis jusqu'à que ça dépasse notre meilleur valeur,
	//si elle ne la dépasse pas, c'est la bonne

	for (std::uint16_t i = crabSubmarines.getFirst()->value; i < crabSubmarines.getLast()->value; i++) {
		std::uint32_t newSum = 0;

		//On initialise le node
		node = crabSubmarines.getFirst();
		
		//On s'arrête soit quand le node == nullptr, soit une fois que la valeur soit dépassé
		while (node != nullptr && sumDistancePart2 > newSum) {
			//On calcul la distance du node au point
			std::uint16_t n = abs(i - node->value);

			newSum += node->coeff * n * (n + 1) / 2;

			//On passe au nouveau node
			node = node->next;
		}

		//Si on a un nouveau minimal, on le choisi
		if (sumDistancePart2 > newSum)
			sumDistancePart2 = newSum;
	}

	//On finit le temps
	auto end = std::chrono::steady_clock::now();

	std::cout << "Part 1: " << sumDistancePart1 << std::endl;
	std::cout << "Part 2: " << sumDistancePart2 << std::endl;
	std::cout << "Time: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " microseconds" << std::endl;

	return 0;
}
