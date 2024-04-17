#pragma once
#ifndef CITATION_H
#define CITATION_H

#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
#include <cpp-httplib/httplib.h>
httplib::Client client{API_ENDPOINT}; 

class Citation {
public:
    std::string id;
    std::string lookup[2];
    Citation(const std::string& id, const std::string& type="", const std::string& code="") : id(id) {
        lookup[0] = type;
        lookup[1] = code;
    }
    virtual std::string cite() = 0;
    auto getInfo(){
        std::string type = "/" + lookup[0] + "/";
        std::string code = lookup[1];
        auto res = client.Get(type + encodeUriComponent(code));
        if (res && res->status == httplib::OK_200) {
            if (lookup[0] == "title"){
                auto json = nlohmann::json::parse(res->body);
                return json["title"].get<std::string>();
            }
            else {
                auto json = nlohmann::json::parse(res->body);
                std::string author = json["author"].get<std::string>();
                std::string title = json["title"].get<std::string>();
                std::string publisher = json["publisher"].get<std::string>();
                std::string year = json["year"].get<std::string>();
                return author + ", " + title + ", " + publisher + ", " + year;
            }
        }
        else {
            auto err = res.error();
            std::string err_msg =  "HTTP error: " + httplib::to_string(err);
            return err_msg;
        }
    }
};


class BookCitation : public Citation {
public:
    BookCitation(const std::string& id, const std::string& isbn) : Citation(id, "isbn", isbn) {}
    virtual std::string cite() override {
        return "[" + id + "]" + " book: " + getInfo() + "\n";
    }
};

class WebCitation : public Citation {
public:
    WebCitation(const std::string& id, const std::string& url) : Citation(id, "title", url) {}
    virtual std::string cite() override {
        return "[" + id + "]" + " webpage: " + getInfo() + ". " + "Available at " + "http://" + lookup[1] + "\n";
    }
};

class ArticleCitation : public Citation {
public:
    std::string info;
    ArticleCitation(const std::string& id, const std::string& info) : Citation(id), info(info){}
    virtual std::string cite() override{ 
        return "[" + id + "]" + " article: " + info + "\n";
    }
};

#endif
