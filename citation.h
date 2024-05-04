#pragma once
#ifndef CITATION_H
#define CITATION_H

#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
#include <cpp-httplib/httplib.h>
httplib::Client client{API_ENDPOINT}; 

class Citation {
private:
    std::string id;
    std::string type;
    std::string code;
public:
    Citation(const std::string& id, const std::string& type="", const std::string& code="") : id(id), type(type), code(code) {}
    virtual std::string cite() = 0;
    std::string getInfo();
    std::string getId() { return id; }
    std::string getType() { return type; } 
    std::string getCode() { return code; }
};


class BookCitation : public Citation {
public:
    BookCitation(const std::string& id, const std::string& isbn) : Citation(id, "isbn", isbn) {}
    virtual std::string cite() override {
        return "[" + getId() + "]" + " book: " + getInfo() + "\n";
    }
};

class WebCitation : public Citation {
public:
    WebCitation(const std::string& id, const std::string& url) : Citation(id, "title", url) {}
    virtual std::string cite() override {
        return "[" + getId() + "]" + " webpage: " + getInfo() + ". " + "Available at " + getCode() + "\n";
    }
};

class ArticleCitation : public Citation {
public:
    std::string info;
    ArticleCitation(const std::string& id, const std::string& info) : Citation(id), info(info){}
    virtual std::string cite() override{ 
        return "[" + getId() + "]" + " article: " + info + "\n";
    }
};

std::string Citation::getInfo(){
    std::string type = "/" + getType() + "/";
    std::string code = getCode();
    auto res = client.Get(type + encodeUriComponent(code));
    if (res && res->status == httplib::OK_200) {
        if (getType() == "title"){
            auto json = nlohmann::json::parse(res->body);
            try{return json["title"].get<std::string>();}
            catch(nlohmann::json::exception& e){
                std::cout << "Error in API response: " << e.what() << "\n ";
                exit(1);
            }
        }
        else {
            auto json = nlohmann::json::parse(res->body);
            try{
                std::string author = json["author"].get<std::string>();
                std::string title = json["title"].get<std::string>();
                std::string publisher = json["publisher"].get<std::string>();
                std::string year = json["year"].get<std::string>();
                return author + ", " + title + ", " + publisher + ", " + year;
                }
            catch(nlohmann::json::exception& e){
                std::cout << "Error in API response: " << e.what() << "\n ";
                exit(1);
            }
        }
    }
    else {
        auto err = res.error();
        std::string err_msg =  "HTTP error: " + httplib::to_string(err);
        return err_msg;
    }
}

#endif
