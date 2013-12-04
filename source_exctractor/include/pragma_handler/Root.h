

#include "pragma_handler/Node.h"


/*
 * ---- It's the root node of the annidation tree of the pragmas in a specific function 
 * 		and contains the first level pragmas.
 */
class Root {
private:	
	FunctionInfo function_info_;

	Node *last_node_;

public:
	Root(Node *n, FunctionInfo funct_info);

	std::vector<Node *> *children_vect_;

	void setLastNode(Node *n) {last_node_ = n; };
	Node* getLastNode() { return last_node_; };

	void AddChildNode(Node *n) { children_vect_->push_back(n); };

	void CreateXMLFunction(tinyxml2::XMLDocument *xml_doc);
	
	unsigned getFunctionLineStart(){ return function_info_.function_start_line_; }
	unsigned getFunctionLineEnd() {return function_info_.function_end_line_; }

	void VisitTree(); 
};