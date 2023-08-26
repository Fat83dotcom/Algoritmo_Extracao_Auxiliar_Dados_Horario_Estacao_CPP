#define FMT_HEADER_ONLY
#include <fmt/format.h>
#include <filesystem>
#include <iostream>
#include <pqxx/pqxx>

namespace fs = std::filesystem;
using std::exception;
using std::string;
using std::cout;
using std::endl;
using std::vector;

class DataBase {
private:
    pqxx::connection C;
public:
    DataBase(const string &config) : C(config){}
    ~DataBase(){}

    void execInsertData(const string &sql){
        try {
            pqxx::work W(C);
            W.exec(sql);
            W.commit();
        } catch (const exception &e) {
            throw e.what();
        }
    }
};

class CSVReader {
protected:
    string folderPath;
    vector<string> filesPath;
public:
    CSVReader(const string &fPath) : folderPath(fPath){}
    ~CSVReader(){}

    void searchFilesFromPath(const string &extensionFile){
        try {
            for(const auto &i : fs::directory_iterator(this->folderPath)){
                if(i.is_regular_file() && i.path().extension() == extensionFile){
                    this->filesPath.push_back(i.path().string());
                }
            }
        }  catch (const exception &e) {
            throw e.what();
        }
    }

    void diplayFilesPath(){
        for(const string &i : this->filesPath){
            cout << i << endl;
        }
    }
};

class StringHandler {

};

class DateHandler {

};

class TransferDataDB : public DataBase, public CSVReader {

};

int main(){
    const string &csvFolder = "/home/fernando/Área de Trabalho/Projeto_Estacao/csv_estacao";
    CSVReader *csv = new CSVReader(csvFolder);
    csv->searchFilesFromPath(".csv");
    csv->diplayFilesPath();
    delete csv;
    return 0;
}