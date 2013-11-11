
#include "pragma_handler/ForNode.h"

struct SourceLocationStruct{
  std::string fileName;
  unsigned startLine;
  unsigned startColumn;

  unsigned endLine;
  unsigned endColumn;
};

struct FunctionInfo {
	clang::FunctionDecl *fD;

	unsigned parentFunctionLine;
	std::string parentFunctionName;
	std::string parentFunctionReturnType;
	int nParams;
	std::string **parentFunctionParameter;

	std::string parentFunctionClassName;
};


class Node {

protected:

	clang::OMPExecutableDirective *pragma;

	ForNode *forNode;


/* Stmt start and end line in the source file */
	std::string fileName;
	int startLine, startColumn;
	int endLine, endColumn;

/*Pragma name with all the parameters */
	std::string pragmaName;
	typedef std::map<std::string, std::string> varList;
	std::map<std::string, varList> *optionVect;

/*Line number of the function that contains this pragma */
	FunctionInfo fI;

/* Variables to construct the tree */
	std::vector<Node *> *childrenVect;
	Node *parentNode;

/* Function to exctract all the parameters of the pragma */
	void setPragmaName(clang::SourceManager& sm);

public:
	bool profiled = false;
	SourceLocationStruct sL;

	Node(clang::OMPExecutableDirective *pragma, clang::FunctionDecl *fd, clang::SourceManager& sm);

	void toLocationStruct(const clang::SourceManager& sm);

/*
 * ---- Set the line, name, return type and parameters of the function containig the pragma ----
 */
	void setParentFunction(clang::FunctionDecl *functD);	

	FunctionInfo getParentFunctionInfo() { return this->fI; };

	void addChildNode(Node *n) { this->childrenVect->insert(this->childrenVect->end(), n); };

	void setParentNode(Node *n) { this->parentNode = n; };
	Node* getParentNode() { return this->parentNode; };

	int getEndLine() { return this->endLine; };


	void createXMLPragma(tinyxml2::XMLDocument *doc, tinyxml2::XMLElement *pragmasElement);
	void createXMLPragmaOptions(tinyxml2::XMLDocument *doc,tinyxml2:: XMLElement *optionsElement);



/* ------------------------------------------ */
	void visitNodeChildren();

	void getPragmaInfo();
	void printNode();
};


