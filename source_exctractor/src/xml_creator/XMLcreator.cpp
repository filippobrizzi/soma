
#include "xml_creator/XMLcreator.h"


void CreateXML(std::vector<Root *> *root_vect, char *file_name) {
  
  tinyxml2::XMLDocument *xml_doc = new tinyxml2::XMLDocument();
  tinyxml2::XMLElement *file_element = xml_doc->NewElement("File");
  xml_doc->InsertEndChild(file_element);

  tinyxml2::XMLElement *name_element = xml_doc->NewElement("Name");
  tinyxml2::XMLText* name_text = xml_doc->NewText(file_name);
  name_element->InsertEndChild(name_text);
  file_element->InsertEndChild(name_element);


  for(std::vector<Root *>::iterator root_itr = root_vect->begin(); root_itr != root_vect->end(); ++ root_itr)     
    (*root_itr)->CreateXMLFunction(xml_doc);
  

  std::string out_xml_file (file_name);
  size_t ext = out_xml_file.find_last_of(".");
  if (ext == std::string::npos)
    ext = out_xml_file.length();
  out_xml_file = out_xml_file.substr(0, ext);
  std::cout << out_xml_file << std::endl;
  
  out_xml_file.insert(ext, "_pragmas.xml");
  std::cout << out_xml_file << std::endl;

  xml_doc->SaveFile(out_xml_file.c_str());
}

