// xmltest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <filesystem>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/filesystem.hpp>

#include <rapidxml.hpp>

namespace pt = boost::property_tree;
namespace fs = boost::filesystem;

using namespace rapidxml;
using namespace std;


int main(int argc, char* argv[])
{
   pt::ptree data;
   filesystem::path filename = argv[1];
   if (exists(filename)) {
      size_t fsize = file_size(filename);
      vector<char> file_contents(fsize + 1);
      ifstream inp;
      inp.open(filename, ifstream::in);
      auto ss = &file_contents.front();
      inp.read(ss, fsize);
      file_contents[fsize] = 0;

      xml_document<> doc; // character type defaults to char
      doc.parse<0>(file_contents.data()); // 0 means default parse flags

      xml_node<>* node = doc.first_node("BoardData");
      for (auto n = node->first_node(); n != node->last_node(); n = n->next_sibling()) {
         //cout << "Name of the node is: " << n->name() << "\n";
      }
   }
}
