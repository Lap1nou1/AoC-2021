#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>
#include <vector>
#include <utility>
#include <chrono>

//Différente possibilité de position de node chez le père
enum class NodePosition {
	left,	//Notre node est à gauche par rapport au node père
	right,	//Notre node est à droite par rapport au node père
	fatherOfAll	//Notre node est le père de tous les autres nodes (aucun père)
};

//Chaque nombre sera un node
struct Node
{
	bool isANumber = false;	//Dit si le node est un nombre
	NodePosition position = NodePosition::fatherOfAll;	//Donne la position du node par rapport au node "père"
	Node* father = nullptr;	//Node père

	//On a soit un pair de nombre, soit un nombre
	union {
		std::pair<Node*, Node*> nextNodes;
		std::uint8_t value;
	};

	//Constructeur
	Node(std::uint8_t _v) : isANumber(true), value(_v) {};
	Node(Node* left, Node* right) : isANumber(false), nextNodes(std::make_pair(left, right)) {};
	Node() {};

	//Constructeur de copie
	//Utile car quand on copie un Node, on a besoin que les enfants de la copie de soit pas les mêmes que l'original
	Node(const Node& toCopy) {
		//On copie toujours si c'est un nombre, et la position
		this->isANumber = toCopy.isANumber;
		this->position = toCopy.position;

		//Si c'est un nombre, on crée manuellement une copie
		if (toCopy.isANumber) {
			this->value = toCopy.value;
		}
		//Si c'est un node contenant d'autres nodes, on copie récursivement les bébés nodes, puis on change le parent
		else {
			this->nextNodes.first = new Node(*toCopy.nextNodes.first);
			this->nextNodes.second = new Node(*toCopy.nextNodes.second);

			//On change le parent
			this->nextNodes.first->father = this;
			this->nextNodes.second->father = this;
		}
	}

	//On change l'addition
	Node* operator+(Node* rhs) {
		//On crée de nouveaux nodes copies (utile pour pas modifier les originaux pour la p2)
		Node* newLhs = new Node(*this);
		Node* newRhs = new Node(*rhs);

		//On crée le nouveau node
		Node* newNode = new Node(newLhs, newRhs);

		//On change les positions
		newLhs->position = NodePosition::left;
		newRhs->position = NodePosition::right;

		//On change les pères
		newLhs->father = newNode;
		newRhs->father = newNode;

		return newNode;
	}

	//Réduit le node, depth correspond à la profondeur à laquelle on est
	void reduce() {
		//On fait d'abord reduceByExplosion, et si on réussi pas, on fait reduceBySpliting
		while (this->reduceByExplosion() || this->reduceBySpliting()) {};
	}

private:
	//On fait exploser un node
	void explode() {
		//On prend les valeurs
		std::pair<std::uint8_t, std::uint8_t> values = std::make_pair(this->nextNodes.first->value, this->nextNodes.second->value);
	
		//On supprime les deux anciens nodes
		delete this->nextNodes.first;
		delete this->nextNodes.second;

		//On change notre node comme étant un nombre (0)
		this->isANumber = true;
		this->value = 0;

		//Maintenant on cherche où mettre la valeur de gauche
		//On va devoir la mettre au premier élément à gauche, pour le trouve, on monte de père en père jusqu'à trouver un node qui ne soit pas à gauche de son père
		//Une fois qu'on la trouvé, on descend une fois à gauche, puis toujours à droite jusqu'à trouvé une valeur
		Node* searching = this->father;
		NodePosition lastPos = this->position;

		while (lastPos == NodePosition::left) {
			lastPos = searching->position;
			searching = searching->father;
		}

		//Avant, on vérifie qu'on était pas tout à gauche (cas où searching == nullptr)
		if (searching != nullptr) {
			//Maintenant, on redescend une fois à gauche, puis toujours à droite
			searching = searching->nextNodes.first;

			//Tant que c'est pas un nombre, on va à droite
			while (!searching->isANumber) {
				searching = searching->nextNodes.second;
			}

			//Maintenant, on additionne les nombres
			searching->value += values.first;
		}

		//Maintenant on fait pareil mais pour la valeur de droite (donc on remonte jusqu'à trouvé un node qui soit pas à droite, puis on va une fois à droite, puis toujours à gauche
		searching = this->father;
		lastPos = this->position;

		while (lastPos == NodePosition::right) {
			lastPos = searching->position;
			searching = searching->father;
		}

		//Avant, on vérifie qu'on était pas tout à droite (cas où searching == nullptr)
		if (searching != nullptr) {
			//Maintenant, on redescend une fois à droite, puis toujours à gauche
			searching = searching->nextNodes.second;

			//Tant que c'est pas un nombre, on va à droite
			while (!searching->isANumber) {
				searching = searching->nextNodes.first;
			}

			//Maintenant, on additionne les nombres
			searching->value += values.second;
		}

	}
	
	//On regarde si les pairs doivent être explosés
	bool reduceByExplosion(std::size_t depth = 0) {
		bool hasExploded = false;

		//Pour ça on regarde toutes les pairs récursivement, et s'il y en a une dépasse 5 de profondeur, on l'explose
		if (!this->isANumber) {
			if (depth >= 4) {
				this->explode();

				hasExploded = true;
			}
			else {
				//On regarde pour les deux enfants
				hasExploded = this->nextNodes.first->reduceByExplosion(depth + 1);
				hasExploded = this->nextNodes.second->reduceByExplosion(depth + 1) || hasExploded;
			}
		}

		return hasExploded;
	}

	//On regarde si les pairs doivent être séparés
	bool reduceBySpliting() {
		bool hasSplited = false;

		//Si c'est un nombre, on regarde si on le split
		if (this->isANumber) {
			if (this->value >= 10) {
				this->split();

				hasSplited = true;
			}
		}
		//Si ce n'est pas un nombre, on regarde si on doit spliter les deux enfants
		else {
			hasSplited = this->nextNodes.first->reduceBySpliting();

			//Si on a pas déjà splité, on regarde pour l'autre côté
			if (!hasSplited) {
				hasSplited = this->nextNodes.second->reduceBySpliting();
			}
		}

		return hasSplited;
	}

	//On split le node nombre en deux
	void split() {
		//Pour spliter c'est simple, on crée deux nouveaux nodes chiffres et on change notre node actuel par un node non chiffré
		std::uint8_t nodeValue = this->value;
		this->isANumber = false;

		this->nextNodes.first = new Node(nodeValue / 2);
		this->nextNodes.second = new Node(nodeValue / 2 + nodeValue%2);

		//On précise où se trouvent les deux qui se sont splités
		this->nextNodes.first->position = NodePosition::left;
		this->nextNodes.second->position = NodePosition::right;

		//On précise le parent
		this->nextNodes.first->father = this;
		this->nextNodes.second->father = this;
	}
};

//Lis une ligne de fichier
//Fonction récursivement, expressionNode est le node qu'on veut remplir, line la ligne donnant le node, et pos un pointeur vers un size_t d'où on en est dans la lecture
void readLine(Node** expressionNode, std::string& line, std::size_t* pos = nullptr) {
	//Si pos n'est pas définie, on la définie
	if (pos == nullptr)
		pos = new std::size_t(0);

	//On skip le '[' qui a ouvert ce node
	(*pos)++;

	//On crée le node expressionNode
	*expressionNode = new Node;

	//On prend le premier node enfant qu'on va écrire (à gauche)
	Node** currentNode = &(*expressionNode)->nextNodes.first;

	//Tant qu'on a pas lu toute la ligne ou qu'on tombe sur un ']', on continue
	bool finished = false;

	for (; *pos < line.size() && !finished; (*pos)++) {
		//On fait différentes actions par rapport au caractère
		switch (line[*pos]) {
		//Si on a un '[' on crée un nouveau node
		case '[':
			//On appelle notre fonction
			readLine(currentNode, line, pos);

			break;
		//Si on a un ',' on change currentNode
		case ',':
			currentNode = &(*expressionNode)->nextNodes.second;

			break;
		//Si on a un ']', on a fini ce node
		case ']':
			finished = true;

			break;
		//Si on a autre chose, c'est un chiffre
		default:
			(*currentNode) = new Node;
			(*currentNode)->isANumber = true;
			(*currentNode)->value = line[*pos] - '0';

			break;
		}
	}

	//On a fait un dernier (*pos)++ qu'il faut annuler
	(*pos)--;

	//On défini le papa et gauche/droite
	(*expressionNode)->nextNodes.first->father = *expressionNode;
	(*expressionNode)->nextNodes.second->father = *expressionNode;
	(*expressionNode)->nextNodes.first->position = NodePosition::left;
	(*expressionNode)->nextNodes.second->position = NodePosition::right;
}

void printNode(const Node* printedNode, std::size_t depth = 0) {
	//Si notre node est un nombre, on print juste sa valeur
	if (printedNode->isANumber) {
		std::cout << int(printedNode->value);
	}
	//Sinon, on print '[', puis le node, puis ',', puis le node, puis ']'
	else {
		std::cout << "\x1b[" << 31 + depth << "m" << '[';
		printNode(printedNode->nextNodes.first, depth+1);
		std::cout << "\x1b[" << 31 + depth << "m" << ',';
		printNode(printedNode->nextNodes.second, depth + 1);
		std::cout << "\x1b[" << 31 + depth << "m" << ']';
	}

	//Si c'est le père de tous on fait un retour à la ligne
	if (printedNode->father == nullptr) {
		std::cout << std::endl;
	}

	std::cout << "\x1b[37m";
}

std::uint64_t checkSum(const Node* checkedNode) {
	//On regarde si le node est un nombre
	if (checkedNode->isANumber) {
		//Si c'en est un, on renvoie juste le nombre
		return checkedNode->value;
	}

	//Si ce n'est pas un nombre, on renvoie 3*checkSum(left) + 2*checkSum(left)
	return 3 * checkSum(checkedNode->nextNodes.first) + 2 * checkSum(checkedNode->nextNodes.second);
}

int main() {
	//On commence le chrono
	auto start = std::chrono::steady_clock::now();

	//On charge le fichier
	std::ifstream inputFile("input.txt");

	if (!inputFile.is_open()) {
		return -1;
	}

	//On lit le fichier
	std::vector<Node*> linesNode;

	for (std::string line; std::getline(inputFile, line);) {
		Node* lineNode = nullptr;

		readLine(&lineNode, line);

		//On reduit le node
		lineNode->reduce();

		linesNode.push_back(lineNode);
	}

	//Maintenant qu'on a tous les nodes, les additionnes tous
	Node* finalNode = new Node(*linesNode[0]);

	for (std::size_t i = 1; i < linesNode.size(); i++) {
		//std::cout << "\n   ";
		//printNode(finalNode);
		//std::cout << "+  ";
		//printNode(linesNode[i]);

		finalNode = *finalNode + linesNode[i];

		//On réduit
		finalNode->reduce();

		//std::cout << "=  ";
		//printNode(finalNode);
	}

	std::uint64_t p1 = checkSum(finalNode);

	//On fait la p2 lol
	std::uint64_t p2 = 0;	//On sauvegarde le maximum

	//On regarde tous les éléments
	for (auto y = linesNode.begin(); y != linesNode.end(); ++y) {
		for (auto x = linesNode.begin(); x != linesNode.end(); ++x) {
			//On s'assure qu'on fait pas y + y
			if (y == x)
				continue;

			//On fait l'addition
			Node* addNode = **y + *x;

			//On reduit
			addNode->reduce();

			//On regarde le somme
			std::uint64_t newValue = checkSum(addNode);

			//On regarde si on a un nouveau maximum
			if (p2 < newValue)
				p2 = newValue;
		}
	}

	//On finit le chrono
	auto end = std::chrono::steady_clock::now();

	std::cout << "Part 1: " << p1 << std::endl;
	std::cout << "Part 2: " << p2 << std::endl;
	std::cout << "Time: " << std::chrono::duration_cast<std::chrono::microseconds>(end-start).count() << " microseconds" << std::endl;

	return 0;
}
