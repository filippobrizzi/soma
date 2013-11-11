
#include "pragma_handler/Root.h"

//#include "ProfileTracker.h"

Root::Root(Node *n, FunctionInfo fI) {

	this->childrenVect = new std::vector<Node *>();
	this->childrenVect->insert(this->childrenVect->end(), n);

	this->lastNode = n;
  this->fI = fI;
}



void Root::visitTree() {

	Node *n;

  std::cout << "FUNCTION INFO " << std::endl;

  std::cout << "Name = " << fI.parentFunctionName << "ReturnType = " << fI.parentFunctionReturnType << std::endl;

	for(std::vector<Node *>::iterator itn = this->childrenVect->begin(); itn != this->childrenVect->end(); itn ++) {
		(*itn)->getPragmaInfo();
   	(*itn)->printNode();
   	std::cout << std::endl;
    		
   	(*itn)->visitNodeChildren();

		}
	} 



void Root::createXMLFunction(tinyxml2::XMLDocument *doc) {

  tinyxml2::XMLElement *functionElement = doc->NewElement("Function");
  doc->LastChild()->InsertEndChild(functionElement);

  tinyxml2::XMLElement *nameElement = doc->NewElement("Name");
  functionElement->InsertEndChild(nameElement); 
  tinyxml2::XMLText* nameText = doc->NewText(this->fI.parentFunctionName.c_str());
  nameElement->InsertEndChild(nameText);

  if(fI.parentFunctionClassName.compare("") != 0){
    tinyxml2::XMLElement *classNameElement = doc->NewElement("ClassName");
    functionElement->InsertEndChild(classNameElement); 
    tinyxml2::XMLText* classNameText = doc->NewText(this->fI.parentFunctionClassName.c_str());
    classNameElement->InsertEndChild(classNameText);
  }
    
  tinyxml2::XMLElement *typeElement = doc->NewElement("ReturnType");
  functionElement->InsertEndChild(typeElement);
  tinyxml2::XMLText* typeText = doc->NewText(this->fI.parentFunctionReturnType.c_str());
  typeElement->InsertEndChild(typeText);

  if(this->fI.nParams > 0) {
    tinyxml2::XMLElement *parametersElement = doc->NewElement("Parameters");
    functionElement->InsertEndChild(parametersElement);

    for(int i = 0; i < this->fI.nParams; i ++) {
      tinyxml2::XMLElement *parameterElement = doc->NewElement("Parameter");
      parametersElement->InsertEndChild(parameterElement);

      tinyxml2::XMLElement *typeElement = doc->NewElement("Type");
      parameterElement->InsertEndChild(typeElement);
      tinyxml2::XMLText* paramTypeText = doc->NewText(this->fI.parentFunctionParameter[i][0].c_str());
      typeElement->InsertEndChild(paramTypeText);

      tinyxml2::XMLElement *paramNameElement = doc->NewElement("Name");
      parameterElement->InsertEndChild(paramNameElement);
      tinyxml2::XMLText* paramNameText = doc->NewText(this->fI.parentFunctionParameter[i][1].c_str());
      paramNameElement->InsertEndChild(paramNameText);

    }
  }

  tinyxml2::XMLElement *lineElement = doc->NewElement("Line");
  functionElement->InsertEndChild(lineElement); 
  char line[100];
  sprintf(line, "%d", this->fI.parentFunctionLine);
  tinyxml2::XMLText* lineText = doc->NewText(line);
  lineElement->InsertEndChild(lineText);


  tinyxml2::XMLElement *pragmasElement = doc->NewElement("Pragmas");
  functionElement->InsertEndChild(pragmasElement);

  for(std::vector<Node *>::iterator itn = this->childrenVect->begin(); itn != this->childrenVect->end(); ++ itn) {
    (*itn)->createXMLPragma(doc, pragmasElement);
  }

}



