#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <cstdint>
#include <utility>
#include <chrono>

//map liant un caractère de début à un caractère de fin ainsi qu'à un nombre de point
std::map<char, char> openingClosing = {
	{'(', ')'},
	{'[', ']'},
	{'{', '}'},
	{'<', '>'}
};

std::map<char, std::uint16_t> pointByError{
	{')', 3},
	{']', 57},
	{'}', 1197},
	{'>', 25137}
};

std::map<char, std::uint16_t> pointByCompletion{
	{'(', 1},
	{'[', 2},
	{'{', 3},
	{'<', 4}
};

const std::string opening = "([{<"; //Caractères ouvrant

//Class node pour la pile
template<class T>
struct Node {
	T value;
	Node* next = nullptr;

	Node(T val) : value(val) {};
	Node() : value(0) {};
};

class Stack {
	Node<char>* first = nullptr;

public:
	void addNode(char value) {
		Node<char>* newNode = new Node<char>;
		newNode->value = value;

		newNode->next = this->first;
		this->first = newNode;
	};

	char takeNode() {
		char returned = this->first->value;

		Node<char>* newFirst = this->first->next;

		delete this->first;

		this->first = newFirst;

		return returned;
	}

	Node<char>* getFirst() {
		return this->first;
	}
};

//Pris de mon jour 7 (un peu modifié)
class LinkedSortedVector {
	Node<std::uint64_t>* first;
	std::size_t size = 0;		//Nombre d'élément total

public:
	LinkedSortedVector() : size(0), first(nullptr) {};

	//Ajoute un élément dans le vecteur, en le triant
	void insert(std::uint64_t value) {
		Node<std::uint64_t>* newNode = new Node<std::uint64_t>(value);

		//On cherche où on le place, en regardant toujours celà avant
		Node<std::uint64_t>** searching = &this->first;
		while (*searching != nullptr) {
			if ((*searching)->value > value) {
				break;
			}

			//On change le Node de recherche
			searching = &(*searching)->next;
		}

		//On place l'élément
		newNode->next = *searching;
		*searching = newNode;

		this->size++;
	}

	Node<std::uint64_t>* getFirst() {
		return this->first;
	}

	std::uint64_t getMedian() {
		std::size_t medianPos = this->size / 2;

		Node<std::uint64_t>* node = this->first;

		for (std::size_t i = 0; i < medianPos; i++) {
			node = node->next;
		}

		return node->value;
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

	//On lit le fichier
	std::uint32_t errorPoints = 0;
	LinkedSortedVector completionPoints;

	for (std::string line; std::getline(inputFile, line);) {
		Stack currentStack;

		//On regarde les caractères, et si la ligne est corrompue
		bool isCorrupted = false;

		for (auto chr = line.begin(); chr != line.end(); ++chr) {
			//On regarde si le caractère est un caractère ouvrant
			if (opening.find_first_of(*chr) != std::string::npos) {
				currentStack.addNode(*chr);
			}
			else {
				//On prend le fermant du dernier caractère ouvrant
				char supposedClosingChr = openingClosing[currentStack.takeNode()];

				//On regarde s'ils sont égaux
				//S'ils sont différents il y a une erreur donc on ajoute au compte
				if (*chr != supposedClosingChr) {
					errorPoints += pointByError[*chr];
					isCorrupted = true;
					break;
				}
			}
		}

		//Si c'est pas corrompu
		if (!isCorrupted) {
			//On calcul le nombre de point
			std::uint64_t points = 0;

			//On vide la pile
			Node<char>* currentNode = currentStack.getFirst();

			while (currentNode != nullptr) {
				//On calcul les points suivant la formule
				points *= 5;
				points += pointByCompletion[currentNode->value];

				//On passe au node suivant
				currentNode = currentNode->next;
			}

			completionPoints.insert(points);
		}
	}

	auto completionPoint = completionPoints.getMedian();

	//On termine le chrono
	auto end = std::chrono::steady_clock::now();

	std::cout << "Part 1: " << errorPoints << std::endl;
	std::cout << "Part 2: " << completionPoint << std::endl;
	std::cout << "Time: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " microseconds" << std::endl;

	return 0;
}
