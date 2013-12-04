
#include "pragma_handler/Root.h"

//#include "ProfileTracker.h"

Root::Root(Node *n, FunctionInfo funct_info) {

	children_vect_ = new std::vector<Node *>();
	children_vect_->push_back(n);

	last_node_ = n;
  function_info_ = funct_info;
}



void Root::CreateXMLFunction(tinyxml2::XMLDocument *xml_doc) {

  tinyxml2::XMLElement *function_element = xml_doc->NewElement("Function");
  xml_doc->LastChild()->InsertEndChild(function_element);

  tinyxml2::XMLElement *name_element = xml_doc->NewElement("Name");
  function_element->InsertEndChild(name_element); 
  tinyxml2::XMLText* name_text = xml_doc->NewText(function_info_.function_name_.c_str());
  name_element->InsertEndChild(name_text);

  if(function_info_.function_class_name_.compare("") != 0){
    tinyxml2::XMLElement *class_name_element = xml_doc->NewElement("ClassName");
    function_element->InsertEndChild(class_name_element); 
    tinyxml2::XMLText* class_name_text = xml_doc->NewText(function_info_.function_class_name_.c_str());
    class_name_element->InsertEndChild(class_name_text);
  }
    
  tinyxml2::XMLElement *return_type_element = xml_doc->NewElement("ReturnType");
  function_element->InsertEndChild(return_type_element);
  tinyxml2::XMLText* return_type_text = xml_doc->NewText(function_info_.function_return_type_.c_str());
  return_type_element->InsertEndChild(return_type_text);

  if(function_info_.num_params_ > 0) {
    tinyxml2::XMLElement *parameters_element = xml_doc->NewElement("Parameters");
    function_element->InsertEndChild(parameters_element);

    for(int i = 0; i < function_info_.num_params_; i ++) {
      tinyxml2::XMLElement *parameter_element = xml_doc->NewElement("Parameter");
      parameters_element->InsertEndChild(parameter_element);

      tinyxml2::XMLElement *type_element = xml_doc->NewElement("Type");
      parameter_element->InsertEndChild(type_element);
      tinyxml2::XMLText* param_type_text = xml_doc->NewText(function_info_.function_parameters_[i][0].c_str());
      type_element->InsertEndChild(param_type_text);

      tinyxml2::XMLElement *param_name_element = xml_doc->NewElement("Name");
      parameter_element->InsertEndChild(param_name_element);
      tinyxml2::XMLText* param_name_text = xml_doc->NewText(function_info_.function_parameters_[i][1].c_str());
      param_name_element->InsertEndChild(param_name_text);

    }
  }

  tinyxml2::XMLElement *line_element = xml_doc->NewElement("Line");
  function_element->InsertEndChild(line_element); 
  char line[100];
  sprintf(line, "%d", function_info_.function_start_line_);
  tinyxml2::XMLText* line_text = xml_doc->NewText(line);
  line_element->InsertEndChild(line_text);


  tinyxml2::XMLElement *pragmas_element = xml_doc->NewElement("Pragmas");
  function_element->InsertEndChild(pragmas_element);

  for(std::vector<Node *>::iterator node_itr = children_vect_->begin(); node_itr != children_vect_->end(); ++ node_itr) {
    (*node_itr)->CreateXMLPragmaNode(xml_doc, pragmas_element);
  }

}



void Root::VisitTree() {

  Node *n;

  std::cout << "FUNCTION INFO " << std::endl;

  std::cout << "Name = " << function_info_.function_name_ << "  ReturnType = " << function_info_.function_return_type_ << std::endl;

  for(std::vector<Node *>::iterator itn = this->children_vect_->begin(); itn != this->children_vect_->end(); itn ++) {
    (*itn)->getPragmaInfo();
    (*itn)->printNode();
    std::cout << std::endl;
        
    (*itn)->visitNodeChildren();

    }
  } 