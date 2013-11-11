

#include "pragma_handler/Node.h"


/*
 * ---- It's the root node of the annidation tree of the pragmas in a specific function 
 * 		and contains the first level pragmas.
 */
class Root {
private:	
	FunctionInfo fI;

	std::vector<Node *> *childrenVect;

	Node *lastNode;

public:
	Root(Node *n, FunctionInfo fI);

	void setLastNode(Node *n) {this->lastNode = n; };
	Node* getLastNode() { return this->lastNode; };

	void addChildNode(Node *n) { this->childrenVect->insert(this->childrenVect->end(), n); };

	void createXMLFunction(tinyxml2::XMLDocument *doc);

	void visitTree(); 
};