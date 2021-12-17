#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>
#include <vector>
#include <utility>
#include <chrono>

//On définit des constantes pour mieux comprendre le code, c'est pour les AND
constexpr std::uint64_t FIRST_BITS = 0x8000000000000000;	//Correspond au premier bit le plus gros d'une variable 64 bits
constexpr std::uint64_t THREE_FIRST_BITS = 0xE000000000000000;	//Correspond aux trois bits les plus gros d'une variable 64 bits
constexpr std::uint64_t FIVE_FIRST_BITS = 0xF800000000000000;	//Correspond aux cinq bits les plus gros d'une variable 64 bits
constexpr std::uint64_t ELEVEN_FIRST_BITS = 0xFFE0000000000000;	//Correspond aux onze bits les plus gros d'une variable 64 bits
constexpr std::uint64_t FIFTEEN_FIRST_BITS = 0xFFFE000000000000;	//Correspond aux quinze bits les plus gros d'une variable 64 bits;

//définition d'un packet pour simplifier la donne
//On aura deux sous constructions pour les literals et les operators
struct Packet {
	std::uint8_t version;
	std::uint8_t typeID;

	//Utile uniquement pour le type literal
	std::uint64_t value = 0;	

	//Utile que pour les operators
	bool lengthTypeID = 0;	//Si c'est false, alors c'est la longueur totale des subpackets, si c'est 1, le nombre de subpackets
	std::uint16_t length = 0;
	std::vector<Packet*> subpackets;
	
	Packet() { this->version = 0; this->typeID = 0; };
	Packet(std::uint8_t _v, std::uint8_t _type) { this->version = _v; this->typeID = _type; };

	//On évalue le packeeeet
	std::uint64_t evaluate() {
		//Ce qu'on fait dépend du typeID
		//Si c'est une valeur litéral, on renvoie la valeur
		std::uint64_t returnedValue;

		switch (typeID) {
		//Si c'est un 0, c'est une somme
		case 0:
			returnedValue = 0;

			//on regarde chaque sous packet
			for (auto subpacket = subpackets.begin(); subpacket != subpackets.end(); ++subpacket) {
				returnedValue += (*subpacket)->evaluate();
			}

			return returnedValue;
		//Si c'est un 1, c'est un produit
		case 1:
			returnedValue = 1;

			//on regarde chaque sous packet
			for (auto subpacket = subpackets.begin(); subpacket != subpackets.end(); ++subpacket) {
				returnedValue *= (*subpacket)->evaluate();
			}

			return returnedValue;
		//Si c'est un 2, on renvoie le minimum
		case 2:
			returnedValue = -1;

			//on regarde chaque sous packet
			for (auto subpacket = subpackets.begin(); subpacket != subpackets.end(); ++subpacket) {
				std::uint64_t newVal = (*subpacket)->evaluate();

				if (newVal < returnedValue) {
					returnedValue = newVal;
				}
			}

			return returnedValue;
		//Si c'est un 3, un maximum
		case 3:
			returnedValue = 0;

			//on regarde chaque sous packet
			for (auto subpacket = subpackets.begin(); subpacket != subpackets.end(); ++subpacket) {
				std::uint64_t newVal = (*subpacket)->evaluate();

				if (newVal > returnedValue) {
					returnedValue = newVal;
				}
			}

			return returnedValue;
		//Si c'est une valeur litéral, on renvoie la valeur
		case 4:
			return value;

		//Si c'est un 5, c'est le plus grand
		case 5:
			return subpackets[0]->evaluate() > subpackets[1]->evaluate();

		//Si c'est un 6, c'est le plus petit
		case 6:
			return subpackets[0]->evaluate() < subpackets[1]->evaluate();

		//Si c'est un 7, c'est l'égalité
		case 7:
			return subpackets[0]->evaluate() == subpackets[1]->evaluate();

		}
	}
};


/* J'ai pas réussi à faire comme ça :(
//Packet litéral (ID = 4), des nombres
struct LiteralPacket : Packet {
	std::uint64_t value = 0;

	LiteralPacket() { this->version = 0; this->typeID = 0; };
	LiteralPacket(std::uint8_t _v, std::uint8_t _type) { this->version = _v; this->typeID = _type; };	//Il y a certainement mieux pour faire ça mais j'ai pas troué
};

//Packet opérateur (ID != 4), des opérateurs comportants des sous-packets
struct OperatorPacket : Packet {
	bool lengthTypeID = 0;	//Si c'est false, alors c'est la longueur totale des subpackets, si c'est 1, le nombre de subpackets
	std::uint16_t length = 0;
	std::vector<Packet*> subpackets = {};

	OperatorPacket() { this->version = 0; this->typeID = 0; };
	OperatorPacket(std::uint8_t _v, std::uint8_t _type) { this->version = _v; this->typeID = _type; };	//Il y a certainement mieux pour faire ça mais j'ai pas troué
};
*/

class BITSTransmission {
	std::string stringVersion;	//Toutes les données pas traité
	std::uint64_t currentData = 0;	//Donnée sous forme binaire (16 nombres hexadécimals), le plus grand bit est le prochain bit à prendre
	std::uint8_t dataFetched = 0;	//Nombre de caractère bits mis actuellement dans currentData

	//Rempli juste currentData
	void getData() {
		//Si on a au moins un hexa en trop
		if (dataFetched <= 60 && !stringVersion.empty()) {
			//On cherche un certain nombre d'hexa
			std::uint8_t nbOfData = std::min(std::size_t((64 - dataFetched) / 4), stringVersion.size());	//Nombre de chiffre hexa qu'on va prendre

			std::uint64_t newData = std::stoull(stringVersion.substr(0, nbOfData), 0, 16);

			//Maintenant, on va trouver où mettre ces données
			//Exemple : fetched = 3 (sur un total de 8 bits)
			//currentData : 0b101|00000 (barre = limite de fetched
			//newData = 0b1101
			//On fait un shift de 8 - fetched - nombre de bits de newData vers la gauche (ici, 8 - 3 - 4, soit 1)
			//0b10100000
			//0b---1101< On est au bon endroit

			newData <<= (64 - dataFetched) - nbOfData * 4; //nbOfData correspond au nombre de chiffre (hexa décimal), un chiffre hexadécimal = 4 bits

			//On augmente dataFetched
			dataFetched += nbOfData * 4;

			//Maintenant on change currentData
			currentData |= newData;

			//Maintenant on retire à stringVersion ce qu'on lui a pris
			stringVersion = stringVersion.substr(nbOfData > stringVersion.size() ? stringVersion.size() : nbOfData);
		}
	}

	//On baisse juste un vector de pointeur de nombre, pour la compréhension
	void decrement(std::vector<std::size_t>& nbOfBits, std::uint16_t decrement) {
		for (auto nb = nbOfBits.begin(); nb != nbOfBits.end(); ++nb) {
			*nb -= decrement;
		}

		currentData <<= decrement;
		dataFetched -= decrement;
	}

public:
	//Constructeur de BITS
	BITSTransmission(std::string& line) : stringVersion(line) {};

	//Interprète stringVersion jusqu'à ce qu'il soit vide, met le résultat dans le packet extérieur, il n'y en a qu'un seul
	//nbOfBits correspond aux nombres de bit qu'il nous reste à faire (utile pour opérateur de type de longueur 0)
	void process(Packet** packet, std::vector<std::size_t>& nbOfBits) {
		//On recharge currentData
		getData();

		//On regarde la version et le type
		//On prend d'abord la version
		std::uint8_t version = (THREE_FIRST_BITS & currentData) >> 61;

		decrement(nbOfBits, 3);

		//On prend maintenant le typeID
		std::uint8_t typeID = (THREE_FIRST_BITS & currentData) >> 61;

		decrement(nbOfBits, 3);

		//Maintenant on divise en deux catégories, soit c'est un type litéral (4) soit c'est un opérateur
		if (typeID == 4) {
			//On précise le nouveau packet
			Packet* newLiteralPacket = new Packet(version, typeID);

			std::uint8_t newLiteralData;

			do {
				//Juste au cas où on aurait une longue chaîne, on rallonge currentData
				getData();

				//On prend la data
				newLiteralData = (currentData & FIVE_FIRST_BITS) >> 59;

				//Maintenant on "pousse" de 4 bits le nombre du packet
				newLiteralPacket->value <<= 4;
				newLiteralPacket->value |= newLiteralData & 0xF;	//Maintenant on ajoute le petit bout de nombre

				//Si on doit compter le nombre de bits, on le baisse de 5
				decrement(nbOfBits, 5);

			//On continu tant que le premier bit est un 1
			} while (newLiteralData & 0x10);

			//On a finit ce packet, on dit à quoi il est égal, puis on se barre
			*packet = newLiteralPacket;
		}
		//Sinon c'est un opérateur
		else {
			//On initialise le nouveau packet
			Packet* newOperatorPacket = new Packet(version, typeID);

			//On trouve le type de longueur
			newOperatorPacket->lengthTypeID = (FIRST_BITS & currentData) >> 63;

			//Si on doit compter le nombre de bits, on le baisse de 1
			decrement(nbOfBits, 1);

			std::size_t lengthOfPacket = -1;	//Utile dans le cas où le type de longueur est 1, sinon est égal à -1, jamais on a autant de packet

			//Si la longueur est de type 0 (longueur total en bits donnée dans les 15 bits suivant)
			if (!newOperatorPacket->lengthTypeID) {
				//On prend la longueur
				newOperatorPacket->length = (currentData & FIFTEEN_FIRST_BITS) >> 49;

				//Si on doit compter le nombre de bits, on le baisse de 15
				decrement(nbOfBits, 15);

				//On ajoute la longueur au packet operator
				nbOfBits.push_back(newOperatorPacket->length);
			}
			//Si la longueur est de type 1 (nombre de subpackets donnée dans les 11 bits suivant
			else {
				//On prend la longueur
				newOperatorPacket->length = (currentData & ELEVEN_FIRST_BITS) >> 53;

				//On sauvegarde la longueur pour l'interpretation
				lengthOfPacket = newOperatorPacket->length;

				decrement(nbOfBits, 11);
			}

			//Maintenant, on boucle jusqu'avoir réglé la condition
			//La boucle va juste appeler la fonction process avec comme subpacket un élément qui va être ajouté au vector subpackets d'OperatorPacket
			//Les conditions : on arrête soit on a plus de subpacket à prendre (type de longueur = 1), soit le dernier élément de nbOfBits est inférieur ou égal à 0
			while ((newOperatorPacket->lengthTypeID == 1 && lengthOfPacket > 0) || (newOperatorPacket->lengthTypeID == 0 && nbOfBits[nbOfBits.size() - 1] > 0)) {
				//On crée un nouveau subpacket
				Packet* subpacket = nullptr;

				//On fait le processus
				//Note : on ne regénère pas currentData car on le fait au début de la fonction
				process(&subpacket, nbOfBits);

				//On ajoute le subpacket au vector
				newOperatorPacket->subpackets.push_back(subpacket);

				//On a fait un sous paquet, on retire un au nombre de sous paquet à faire
				lengthOfPacket--;
			}

			//Si on avait comme type de longueur 0 (la taille en bit), ça veut dire qu'on est à 0, on le retire pour ne pas perturber les autres packet
			if (newOperatorPacket->lengthTypeID == 0)
				nbOfBits.pop_back();

			//Le packet c'est newOperatorPacket
			*packet = newOperatorPacket;
		}
	}
};

//Obtiens la réponse à la p1 (additionne toutes les versions)
//Fonctionne recursivement, de la même manière dont on parcours un arbre
std::size_t getTotalSize(Packet* packet) {
	//Si on a une valeur litteral, on renvoie juste sa taille
	if (packet->typeID == 4) {
		return packet->version;
	}

	//on initialise à la version de ce packet
	std::size_t sum = packet->version;

	//Pour chaque subpackets, on ajoute la somme de leur version
	for (auto subpacket = packet->subpackets.begin(); subpacket != packet->subpackets.end(); ++subpacket) {
		sum += getTotalSize(*subpacket);
	}

	return sum;
}

int main() {
	//On commence le temps
	auto start = std::chrono::steady_clock::now();

	//On charge le fichier
	std::ifstream inputFile("input.txt");

	if (!inputFile.is_open()) {
		return -1;
	}

	//On interprète l'entrée
	std::string line;
	std::getline(inputFile, line);

	BITSTransmission transmission(line);

	Packet* firstPacket = nullptr;
	std::vector<std::size_t> nbOfBits;	//C'est pas parfait, je devrais pas à faire ça mais flemme

	transmission.process(&firstPacket, nbOfBits);

	std::uint64_t p1 = getTotalSize(firstPacket);
	std::uint64_t p2 = firstPacket->evaluate();

	//On finit le temps
	auto end = std::chrono::steady_clock::now();

	std::cout << "Part 1: " << p1 << std::endl;
	std::cout << "Part 2: " << p2 << std::endl;
	std::cout << "Time: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " microseconds" << std::endl;

	return 0;
}
