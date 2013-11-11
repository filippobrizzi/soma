
#include "xml_creator/XMLcreator.h"


void createXML(std::vector<Root *> *rootVect, char *fileName) {
  
  tinyxml2::XMLDocument *doc = new tinyxml2::XMLDocument();
  tinyxml2::XMLElement *fileElement = doc->NewElement("File");
  doc->InsertEndChild(fileElement);

  tinyxml2::XMLElement *nameElement = doc->NewElement("Name");
  tinyxml2::XMLText* nameText = doc->NewText(fileName);
  nameElement->InsertEndChild(nameText);
  fileElement->InsertEndChild(nameElement);


  for(std::vector<Root *>::iterator itr = rootVect->begin(); itr != rootVect->end(); ++ itr)     
    (*itr)->createXMLFunction(doc);
  

  std::string outXML (fileName);
  size_t ext = outXML.find_last_of(".");
  if (ext == std::string::npos)
    ext = outXML.length();
  outXML = outXML.substr(0, ext);
  std::cout << outXML << std::endl;
  outXML.insert(ext, "_pragmas.xml");
  std::cout << outXML << std::endl;

  doc->SaveFile(outXML.c_str());
}

