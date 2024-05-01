#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <nlohmann/json.hpp>
#include "utils.hpp"
#include "citation.h"

std::vector<Citation*> loadCitations(const std::string& filename) {
    // load citations from file, meanwhile, check the validity of citations file
    std::vector<Citation*> citations{};
    std::ifstream file{filename};
    if(!file.is_open()){
        std::cerr << "Invalid file." << std::endl;
        std::exit(1);
    }
    nlohmann::json data = nlohmann::json::parse(file);
    try{auto cites = data["citations"];} 
    catch(nlohmann::json::exception& e){
        std::cerr << "Invalid citation collection, " << e.what() << std::endl;
        std::exit(1);
    }
    auto cites = data["citations"];
    for(int i=0; i<cites.size(); i++){
        auto citation = cites[i];

        // Basic exception handling
        try{
            if(citation["id"].is_string() == false){
                std::cerr << "Invalid id." << std::endl;
                std::exit(1);
            }
        }
        catch(nlohmann::json::exception& e){
            std::cerr << "Invalid citation, " << e.what() << std::endl;
            std::exit(1);
        }
        try{
            if(citation["type"].is_string() == false){
                std::cerr << "Invalid type." << std::endl;
                std::exit(1);
            }
            std::string type = citation["type"].get<std::string>();
            if(type != "book" && type != "webpage" && type != "article"){
                std::cerr << "Invalid citation type." << std::endl;
                std::exit(1);
            }
        }
        catch(nlohmann::json::exception& e){
            std::cerr << "Invalid citation, " << e.what() << std::endl;
            std::exit(1);
        }
        // end basic exception handling

        std::string id = citation["id"].get<std::string>();
        std::string type = citation["type"].get<std::string>();

        // type dispatch
        if (type == "book"){
            try{
                if(citation["isbn"].is_string() == false){
                    std::cerr << "Invalid isbn." << std::endl;
                    std::exit(1);
                }
                std::string isbn = citation["isbn"].get<std::string>();
                citations.push_back(new BookCitation(id, isbn));
            }
            catch(nlohmann::json::exception& e){
                std::cerr << "Invalid book citation, " << e.what() << " not found." << std::endl;
                std::exit(1);
            }
        }
        else if (type == "webpage"){
            try{
                if(citation["url"].is_string() == false){
                    std::cerr << "Invalid url." << std::endl;
                    std::exit(1);
                }
                std::string url = citation["url"].get<std::string>();
                auto pos = url.find("http://");
                std::string URL;
                if(pos == std::string::npos) URL = url;
                else URL = url.substr(pos+7);
                citations.push_back(new WebCitation(id, URL));
            }
            catch(nlohmann::json::exception& e){
                std::cerr << "Invalid webpage citation, " << e.what() << " not found." << std::endl;
                std::exit(1);
            }
        }
        else if (type == "article"){
            try{
                if(citation["title"].is_string() == false || citation["author"].is_string() == false || citation["journal"].is_string() == false || citation["year"].is_number() == false || citation["volume"].is_number() == false || citation["issue"].is_number() == false){
                    std::cerr << "Invalid article citation." << std::endl;
                    std::exit(1);
                }
                std::string title = citation["title"].get<std::string>();
                std::string author = citation["author"].get<std::string>();
                std::string journal = citation["journal"].get<std::string>();
                int year = citation["year"].get<int>();
                int volume = citation["volume"].get<int>();
                int issue = citation["issue"].get<int>();
                std::string info = title + ", " + author + ", " + journal + ", " + std::to_string(year) + ", " + std::to_string(volume) + ", " + std::to_string(issue);
                citations.push_back(new ArticleCitation(id, info));
            }
            catch(nlohmann::json::exception& e){
                std::cerr << "Invalid article citation, " << e.what() << std::endl;
                std::exit(1);
            }
        }
        else {
            std::cerr << "Invalid citation type." << std::endl;
            std::exit(1);
        }
        // end type dispatch
    }

    return citations;
}

std::string readFromFile(const std::string& filename){
    std::ifstream file(filename, std::ios::in);
    std::string content;
    if(!file.is_open()){
        std::cerr << "Invalid file." << std::endl;
        std::exit(1);
    }
    std::string line;
    while(std::getline(file, line)){
        content += line + "\n";
    }
    return content;
}

std::vector<Citation*> printedCitations{};
void parseInput(const std::string& input, const std::vector<Citation*>& citations){
    // check if all [ and ] are paired
    auto pos =  input.find("[");
    while(pos != std::string::npos){
        auto end = input.find("]", pos);
        if(end == std::string::npos || end < pos){
            std::cerr << "Invalid input." << std::endl;
            std::exit(1);
        }
        pos = input.find("[", end);
    }
    // check if all citation id in input text are found in citations
    auto ptr = input.find("["); 
    while(ptr != std::string::npos){
        auto end = input.find("]", ptr);
        auto id = input.substr(ptr+1, end-ptr-1);
        bool found = false;
        for(auto c : citations){
            if(c->getId() == id){
                found = true;
                auto it = std::find(printedCitations.begin(), printedCitations.end(), c);
                if(it == printedCitations.end()) printedCitations.push_back(c); // avoid repeated references
                break;
            }
        }
        if(!found){
            std::cerr << "Invalid input. " << id << " not found in citation collection. " << std::endl;
            std::exit(1);
        }
        ptr = input.find("[", end);
    }
}



int main(int argc, char** argv) {
    if (argc!=6 && argc!=4){
        std::cerr << "Invalid command." << std::endl;
        std::exit(1);
    }
    std::string citation_path;
    std::string output_path;
    std::string input_file;
    std::string input;
    for(int i=1; i<argc; i++){
        std::string command = std::string(argv[i]);
        if(command.substr(0,1) == "-"){
            if(command == "-c"){
                if(citation_path != ""){
                    std::cerr << "Invalid command. Can only cite from one file." << std::endl;
                    std::exit(1);
                }
                citation_path = argv[++i];
            }
            else if(command == "-o"){
                if(output_path != ""){
                    std::cerr << "Invalid command. Can only output to one file." << std::endl;
                    std::exit(1);
                }
                output_path = argv[++i];
            }
            else{
                if (i == argc-1 && command == "-"){
                    input_file = "-";
                }
                else{
                    std::cerr << "Invalid command." << std::endl;
                    std::exit(1);
                }
            }  
        }
        else{
            if(input_file != ""){
                std::cerr << "Invalid command. Can only input from one file." << std::endl;
                std::exit(1);
            }
            input_file = command;
        }
    }
    if(citation_path == ""){
        std::cerr << "Invalid command. Missing citation file." << std::endl;
        std::exit(1);
    }
    if(input_file == ""){
        std::cerr << "Invalid command. Missing input file." << std::endl;
        std::exit(1);
    }
    else if(input_file == "-"){
        // read from stdin
        std::string line;
        while(std::getline(std::cin, line)){
            input += line + "\n";
        }
    }
    else{
        // read from file
        input = readFromFile(input_file);
    }

    // main logic
    auto citations = loadCitations(citation_path);
    parseInput(input, citations);
    sort(printedCitations.begin(), printedCitations.end(), [](Citation* a, Citation* b){
        return std::stoi(a->getId()) < std::stoi(b->getId());
    });

    // output
    if(output_path == ""){
        // output to stdout
        std::ostream& output = std::cout;
        output << input;  
        output << "\nReferences: \n";
        for (auto c : printedCitations) {
            std::cout << c->cite();
        }
    }
    else{
        // output to file
        try{
        std::ofstream outfile{output_path};
        outfile << input;
        outfile << "\nReferences: \n";
        for (auto c : printedCitations) {
            outfile << c->cite();
        }
        }
        catch(std::exception&){
            std::cerr << "Output error." << std::endl;
            std::exit(1);
        }
    }

    for (auto c : citations) {
        delete c;
    }
}
