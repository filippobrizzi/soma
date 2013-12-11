
#include "pragma_handler/ForNode.h"
/* Contains info about the locatation of a pragma in the source code */
/*struct SourceLocationStruct{
  std::string file_name_;
  unsigned start_line_;
  unsigned start_column_;

  unsigned end_line_;
  unsigned end_column_;
};
*/
/* Contains info about function */
struct FunctionInfo {
	clang::FunctionDecl *function_decl_;

	unsigned function_start_line_;
	unsigned function_end_line_;
	std::string function_name_;
	std::string function_return_type_;
	int num_params_;
	/* Matrix Nx2. Contains the list of the parameter of the functions: type name */
	std::string **function_parameters_;

	std::string function_class_name_;
};

/*
 * ---- Contains all the relevant information of a given pragma.
 */
class Node {

private:

	clang::OMPExecutableDirective *pragma_stmt_;

/* Stmt start and end line in the source file */
	std::string file_name_;
	int start_line_, start_column_;
	int end_line_, end_column_;
	
/*Line number of the function that contains this pragma */
	FunctionInfo parent_funct_info_;

/* Variables to construct the tree */
	Node *parent_node_;

	/*Pragma name with all the parameters */
	//std::string pragma_type_;

/* Function to exctract all the parameters of the pragma */
	void setPragmaClauses(clang::SourceManager& sm);

public:
	/*Pragma name with all the parameters */
	std::string pragma_type_;

	bool profiled_ = false;
	//SourceLocationStruct src_loc_;

	ForNode *for_node_;

	std::vector<Node *> *children_vect_;

	typedef std::map<std::string, std::string> VarList_;
	std::map<std::string, VarList_> *option_vect_;

	Node(clang::OMPExecutableDirective *pragma_stmt, clang::FunctionDecl *funct_decl, clang::SourceManager& sm);

	void setSourceLocation(const clang::SourceManager& sm);

/*
 * ---- Set the line, name, return type and parameters of the function containig the pragma ----
 */
	void setParentFunction(clang::FunctionDecl *funct_decl, const clang::SourceManager& sm);

	FunctionInfo getParentFunctionInfo() { return parent_funct_info_; }

	void AddChildNode(Node *n) { children_vect_->push_back(n); }

	void setParentNode(Node *n) { parent_node_ = n; }
	Node* getParentNode() { return parent_node_; }

	int getEndLine() { return end_line_; }
	int getStartLine() { return start_line_; }

	void CreateXMLPragmaNode(tinyxml2::XMLDocument *xml_doc, tinyxml2::XMLElement *pragmas_element);
	void CreateXMLPragmaOptions(tinyxml2::XMLDocument *xml_doc,tinyxml2:: XMLElement *options_element);



/* ------------------------------------------ */
	void visitNodeChildren();

	void getPragmaInfo();
	void printNode();
};


