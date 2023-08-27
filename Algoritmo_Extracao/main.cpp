#define FMT_HEADER_ONLY
#include <pqxx/pqxx>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <fmt/format.h>

namespace fs = std::filesystem;
using std::istringstream;
using std::ostringstream;
using std::ifstream;
using std::ios;
using std::exception;
using std::string;
using std::cout;
using std::endl;
using std::vector;
using std::map;
using std::setfill;
using std::setw;
using fmt::format;

class Error : public exception {
private:
    string msg;
public:
    Error(const string &msg) : msg(msg){}

    const char *what() const noexcept override {
        return msg.c_str();
    }
};

class DateHandler {
private:
    map<string, int> monthMap = {
        {"Jan", 1}, {"Feb", 2}, {"Mar", 3}, {"Apr", 4},
        {"May", 5}, {"Jun", 6}, {"Jul", 7}, {"Aug", 8},
        {"Sep", 9}, {"Oct", 10}, {"Nov", 11}, {"Dec", 12},
        {"jan", 1}, {"fev", 2}, {"mar", 3}, {"abr", 4},
        {"mai", 5}, {"jun", 6}, {"jul", 7}, {"ago", 8},
        {"set", 9}, {"out", 10}, {"nov", 11}, {"dez", 12}
    };
public:
    DateHandler(){}
    ~DateHandler(){}

    vector<int> _mainFormatAlgorithm(const string & rawDate){
        vector<int> resultExtractData;
        string monthStr;
        char discard;
        int day, month, year, hour, minute, second;
        istringstream dateTarget(rawDate);
        dateTarget >> day >> monthStr >> year >> hour >> discard >> minute >> discard >> second;
        month = this->monthMap[monthStr];
        resultExtractData = {day, month, year, hour, minute, second};
        return resultExtractData;
    }

    string formatTableNameDate(const string &rawDate){
        vector<int> result = this->_mainFormatAlgorithm(rawDate);
        ostringstream formatedDateStream;
        formatedDateStream << setfill('0') << setw(2) << result[0] << '-' <<
                              setfill('0') << setw(2) << result[1] << '-' << result[2];
        return formatedDateStream.str();
    }
    string formatFullDate(const string &rawDate){
        vector<int> result = this->_mainFormatAlgorithm(rawDate);
        ostringstream formatedDateStream;
        formatedDateStream << setfill('0') << setw(2) << result[0] << '-' <<
                              setfill('0') << setw(2) << result[1] << '-' << result[2] << ' ' <<
                              setfill('0') << setw(2) << result[3] << ':' << setfill('0') <<
                              setw(2) << result[4] << ':' << setfill('0') << setw(2) << result[5];
        return formatedDateStream.str();
    }
};

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

class CSVRetriever {
protected:
    string folderPath;
    vector<string> filesPath;
public:
    CSVRetriever(const string &fPath) : folderPath(fPath){}
    ~CSVRetriever(){}

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
private:
    DateHandler *dtHand = new DateHandler();
    string rawData;
public:
    StringHandler(const string &rData) : rawData(rData){}
    ~StringHandler(){}

    vector<string> splitRawData(char delimiter){
        try {
            string dataToken;
            vector<string> dataTokens;
            istringstream tokenStream(this->rawData);
            while (getline(tokenStream, dataToken, delimiter)) {
                dataTokens.push_back(dataToken);
            }
            string formatedDate = this->dtHand->formatFullDate(dataTokens[0]);
            dataTokens[0] = formatedDate;
            return dataTokens;
        }  catch (const exception &e) {
            throw e.what();
        }
    }
};

class FileExtractor {
private:
    ifstream currentFile;
public:
    FileExtractor(const string &fPath) : currentFile(fPath, ios::in){
        if(this->currentFile.is_open()){
            string fileName = format("Arquivo aberto com sucesso! -> {}", fPath);
            cout << fileName << endl;
        }
        else{
            throw Error("Arquivo não foi aberto com sucesso...");
        }
    }
    ~FileExtractor(){}

    string getDataRawFile(){
        try {
            string line;
            if(!this->currentFile.eof()){
                getline(this->currentFile, line);
                return line;
            }
            return "eof";
        }  catch (const exception &e) {
            throw e.what();
        }
    }
};

class TransferDataDB : public DataBase, public CSVRetriever {
public:
    TransferDataDB(const string &dbConfig, const string &folderFiles) :
        DataBase(dbConfig), CSVRetriever(folderFiles){}
    ~TransferDataDB(){}

    void run(){
        try {
            this->searchFilesFromPath(".csv");
            if(this->filesPath.size() > 0){
                for(const string &file : this->filesPath){
                    FileExtractor *flExt = new FileExtractor(file);
                    string eofFlag = "foe";
                    while (eofFlag != "eof") {
                        string rawData = flExt->getDataRawFile();
                        StringHandler *strHand = new StringHandler(rawData);
                        vector<string> splitData = strHand->splitRawData(',');
//                        for(const string &i : splitData){
//                            cout << i << endl;
//                        }
                        cout << splitData[0];
                        cout << endl;
                        delete strHand;
                        if(rawData == "eof") eofFlag = rawData;
                    }
                    delete flExt;
                }
            }
        }  catch (const exception &e) {
            throw e.what();
        }
    }
};

int main(){
    const string &dbConfig = "dbname=teste user=postgres password=230383asD# hostaddr=127.0.0.1 port=5432";
    const string &csvFolder = "/home/fernando/Área de Trabalho/Projeto_Estacao/csv_estacao";
    try {
        TransferDataDB *exe = new TransferDataDB(dbConfig, csvFolder);
        exe->run();
    } catch (const exception &e) {
        cout << e.what() << endl;
    }
    return 0;
}
