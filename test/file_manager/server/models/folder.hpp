#pragma once
#include <map>
#include <iostream>
#include <vector>
#include <fstream>
#include <string>

class Folder {
    public:
        Folder();
        Folder(std::string path);
        void reg_file(std::string file);
        bool file_exist(std::string file);
        std::string getFilePath(std::string file); 
        std::vector<std::string> getFiles();
    private:
        std::string path;
        std::vector<std::string> File;
        std::map<std::string,bool> File_Exist;
};
Folder::Folder(){}
Folder::Folder(std::string path) {
    this->path = path;

    std::fstream File_list(path+"/files.list",std::ios::in);
    
    std::string linha;
    while(std::getline(File_list,linha)) {
        File.push_back(linha);
        File_Exist[linha] = true;
    }
    File_list.close();
}

void Folder::reg_file(std::string file) {
    std::fstream File_list(path+"/files.list",std::ios::out|std::ios::app);
    File_list << file <<'\n';
    File_list.close();

    File.push_back(file);
    File_Exist[file] = true;
}

bool Folder::file_exist(std::string file) {
    return (File_Exist.find(file)!=File_Exist.end());
}

std::string Folder::getFilePath(std::string file) {
    return path+"/"+file;
} 

std::vector<std::string> Folder::getFiles(){
    std::vector<std::string> files;
    for(auto i:File_Exist) {
        files.push_back(i.first);
    }
    return files;
}