#define FMT_HEADER_ONLY
#include <pqxx/pqxx>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <fmt/format.h>

namespace fs = std::filesystem;

using std::chrono::system_clock;
using std::chrono::time_point;
using std::istringstream;
using std::ostringstream;
using std::exception;
using std::ifstream;
using std::ofstream;
using std::string;
using std::vector;
using std::map;
using std::ios;
using std::cout;
using std::setfill;
using fmt::format;
using std::endl;
using std::setw;
using std::time_t;
using std::ctime;
using std::strftime;

/* Classe LogFile: Registra os erros do banco em um arquivo */
class LogFile {
private:
    ofstream logFile;
public:
    LogFile(const string &nameF) : logFile(nameF, ios::app) {
        if (logFile.is_open()){
            logFile << "Log Operando -> " <<  this->currentTime() << endl;
            logFile << endl;
        }
        else{
            cout << "Log não está operando..." << endl;
        }
    }
    ~LogFile(){
        logFile.close();
        cout << "Arquivo Log fechado." << endl;
    }

    string currentTime(){
        char buffer[50]; 
        auto currentTime = system_clock::now();
        time_point<system_clock> timePoint = currentTime;
        time_t currentTimeT = system_clock::to_time_t(currentTime);
        std::tm* localTime = std::localtime(&currentTimeT);
        strftime(buffer, sizeof(buffer), "%A, %d/%m/%Y %H:%M:%S", localTime);
        return buffer;
    }

    void registerLog(const string &msg){
        logFile << msg << " -> " << this->currentTime() << endl;
        logFile << endl;
    }
};

/*Classe Error: Levanta erros do programa.*/
class Error : public exception {
private:
    string msg;
public:
    Error(const string &msg) : msg(msg){}

    const char *what() const noexcept override {
        return msg.c_str();
    }
};

/* Classe DateHandler: Formata as datas para o formato aceito pelo Postgres. */
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

    vector<int> _mainFormatAlgorithm(const string &rawDate){
        vector<int> resultExtractData;
        string monthStr;
        char discard;
        int day, month, year, hour, minute, second;
        day = month = year = hour = minute = second = 0;
        istringstream dateTarget(rawDate);
        dateTarget >> day >> monthStr >> year >> hour >> discard >> minute >> discard >> second;
        month = this->monthMap[monthStr];
        resultExtractData = {day, month, year, hour, minute, second};
        return resultExtractData;
    }
public:
    DateHandler(){}
    virtual ~DateHandler(){
        cout << "Date Handler Delete." << endl;
    }

    string formatTableNameDate(const string &rawDate){
        vector<int> result = this->_mainFormatAlgorithm(rawDate);
        ostringstream formatedDateStream;
        formatedDateStream << setfill('0') << setw(2) << result[0] << '-' <<
                              setfill('0') << setw(2) << result[1] << '-' <<
                              result[2];
        return formatedDateStream.str();
    }
    string formatFullDate(const string &rawDate){
        vector<int> result = this->_mainFormatAlgorithm(rawDate);
        ostringstream formatedDateStream;
        formatedDateStream << setfill('0') << setw(2) << result[0] << '-' <<
                              setfill('0') << setw(2) << result[1] << '-' <<
                              result[2] << ' ' << setfill('0') << setw(2) <<
                              result[3] << ':' << setfill('0') << setw(2) <<
                              result[4] << ':' << setfill('0') << setw(2) <<
                              result[5];
        return formatedDateStream.str();
    }
};

/* Classe DataBase: Conecta e executa o serviço de banco de dados Postgres. */
class DataBase {
private:
    pqxx::connection C;
    LogFile *log = new LogFile("LogFileAlgoritmoExtracaoCPP.txt");

    int _selectGerenciadorTabelas(const string &currentDate){
        try{
            if (currentDate != "00-00-0"){
                string fKeyStr;
                int fKey = 0;
                pqxx::work W(C);
                fKey = W.query_value<int>(
                    "SELECT codigo FROM "
                    "gerenciador_tabelas_horarias "
                    "WHERE data_tabela=" + W.quote(currentDate)
                );
                return fKey;
            }
        }
        catch(const exception& e) {
            this->log->registerLog(e.what());
        }
        return -1;
    }

    string _queryCreateTimeTable(const string &nameTable, int fkey){
        string sql = format(
            "CREATE TABLE IF NOT EXISTS tabelas_horarias.\"{}\" ("
            "codigo serial not null PRIMARY KEY, "
            "data_hora timestamp not null UNIQUE, "
            "codigo_gerenciador bigint default {}, "
            "umidade double precision null, "
            "pressao double precision null, "
            "temp_int double precision null, "
            "temp_ext double precision null, "
            "FOREIGN KEY (codigo_gerenciador) "
            "REFERENCES gerenciador_tabelas_horarias (codigo) "
            "ON DELETE CASCADE)",
            nameTable.c_str(), fkey
        );
        return sql;
    }
    
    string _queryInsertTableGerenciamentoTabela(const string &currentDate){
        string sql = format(
            "INSERT INTO gerenciador_tabelas_horarias (data_tabela) VALUES ('{}')",
            currentDate.c_str()
        );
        return sql;
    }

    string _queryInserTimeTable(const string &tableName, const vector<string> values){
        string sql = format(
            "INSERT INTO tabelas_horarias.\"{}\" (data_hora, umidade, pressao, temp_int, temp_ext)"
            "VALUES ('{}', {}, {}, {}, {})", tableName.c_str(), values[0].c_str(), values[1].c_str(),
            values[2].c_str(), values[3].c_str(), values[4].c_str()
        );
        return sql;
    }
    
    string _querySelectGerenciadorTabela(const string &currentDate){
        string sql = format("SELECT codigo FROM gerenciador_tabelas_horarias WHERE data_tabela='{}'",
        currentDate);
        return sql;
    }

    void _execDB(const string &sql){
        try {
            pqxx::work W(C);
            W.exec(sql);
            W.commit();
        } 
        catch (const exception &e) {
            this->log->registerLog(e.what());
        }
    }
public:
    DataBase(const string &config) : C(config){}
    virtual ~DataBase(){
        delete log;
        cout << "DB delete." << endl;
    }

    void insertDataDB(const string currentDate, vector<string>data){
        try{
            if (currentDate != "00-00-0"){
                string sql = this->_queryInserTimeTable(currentDate, data);
                this->_execDB(sql);
            }
        }
        catch(const exception& e) {
            this->log->registerLog(e.what());
        }
    }

    void checkCreateTimeTable(const string &currentDate, const string &statusDate){
        try {
            if (currentDate != statusDate && currentDate != "00-00-0") {
                int fKey = -1;
                string sql = this->_queryInsertTableGerenciamentoTabela(currentDate);
                this->_execDB(sql);
                fKey = this->_selectGerenciadorTabelas(currentDate);
                if (fKey != -1){
                    string sql2 = this->_queryCreateTimeTable(currentDate, fKey);
                    this->_execDB(sql2);
                }
                else{
                    throw Error("Houve um erro em obter a chave estrangeira -> checkCreateTimeTable");
                }
            }
        }
        catch(const exception& e) {
            this->log->registerLog(e.what());
        }     
    }
};

/* Classe CSVRetriever: Busca os arquivos .csv na pasta destino. */
class CSVRetriever {
protected:
    string folderPath;
    vector<string> filesPath;
public:
    CSVRetriever(const string &fPath) : folderPath(fPath){}
    virtual ~CSVRetriever(){
        cout << "Retriever delete." << endl;
    }

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

/* Classe StringHandler: Manipula e prepara os dados de cada linha do arquivo. */
class StringHandler {
private:
    DateHandler *dtHand = new DateHandler();
    string tableDateInformation;
    string rawData;
public:

    StringHandler(){}
    ~StringHandler(){
        cout << "StringHanlder Delete." << endl;
        delete dtHand;
    }

    void setRawData(const string &rawData){
        this->rawData = rawData;
    }

    string getTableDateInformation(){
        return this->tableDateInformation;
    }

    vector<vector<string>> splitRawData(char delimiter){
        try {
            string dataToken;
            vector<vector<string>> dataTokensAll;
            vector<string> dataTokensFull;
            vector<string> dataTokensTable;
            istringstream tokenStream(this->rawData);
            while (getline(tokenStream, dataToken, delimiter)) {
                dataTokensFull.push_back(dataToken);
            }
            string formatFullDate = this->dtHand->formatFullDate(dataTokensFull[0]);
            string formatTableDate = this->dtHand->formatTableNameDate(dataTokensFull[0]);
            this->tableDateInformation = formatTableDate;
            dataTokensFull[0] = formatFullDate;
            dataTokensTable.push_back(formatTableDate);
            dataTokensAll.push_back(dataTokensTable);
            dataTokensAll.push_back(dataTokensFull);
            return dataTokensAll;
        }  catch (const exception &e) {
            throw e.what();
        }
    }
};

/* Classe FileExtractor: Abre e fornece cada linha do arquivo quando solicitado.*/
class FileExtractor {
private:
    ifstream currentFile;
    string fileName;
public:
    FileExtractor(const string &fPath) : currentFile(fPath, ios::in){
        if(this->currentFile.is_open()){
            fileName = fPath;
            string msg = ("Arquivo aberto com sucesso! -> {}", fileName);
            cout << msg << endl;
        }
        else{
            throw Error("Arquivo não foi aberto com sucesso...");
        }
    }
    ~FileExtractor(){
        currentFile.close();
        cout << "Arquivo: " << fileName << " foi fechado." << endl;
    }

    string nextLineFile(){
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

/* Classe DateStatus: Controla quando deve ser criada uma nova tabela,
 conservando o estado da ultima data inserida*/
class DateStatus {
private:
    string dateStatus = "01-01-1900";
public:
    DateStatus(){}
    ~DateStatus(){
        cout << "DateStatus delete." << endl;
    }

    void updateStatusDate(const string &date){
        if (this->dateStatus != date) {
            this->dateStatus = date;
        }
    }

    string getDateStatus(){
        return this->dateStatus;
    }
};

/* Classe Counter: Implementa um contador */
class Counter {
private:
    long int count;
public:
    Counter(){
        count = 0;
    }
    ~Counter(){
        cout << "Counter delete" << endl;
    }

    bool executeCounter(
        long int *pCount, const string &currentDate, const string &statusDate
    ){
        if (currentDate == statusDate){
            count++;
            return false;
        }
        else{
            *pCount = this->count + 1;
            count = 0;
            return true;
        }
    }
};

/* Classe TransferDataDB: Integra todas as partes do algoritmo.*/
class TransferDataDB : public DataBase, public CSVRetriever {
private:
    DateStatus *dtStatus = new DateStatus();
    Counter *counter = new Counter();
public:
    TransferDataDB(const string &dbConfig, const string &folderFiles) :  
    DataBase(dbConfig), CSVRetriever(folderFiles){}
    virtual ~TransferDataDB(){
        delete dtStatus;
        delete counter;
    }

    void run(){
        try {
            string eofFlag;
            string rawData;
            string currentDate;
            string tableInformation;
            unsigned long int count = 0;
            
            this->searchFilesFromPath(".csv");
            if(this->filesPath.size() > 0){
                for(const string &file : this->filesPath){
                    FileExtractor *flExt = new FileExtractor(file);
                    StringHandler *strHand = new StringHandler();
                    rawData = flExt->getDataRawFile();
                    strHand->setRawData(rawData);
                    eofFlag = "foe";
                    while (eofFlag != "eof") {
                        vector<vector<string>> splitData = strHand->splitRawData(',');
                        string currentDate = splitData[0][0];
                        string statusDate = this->dtStatus->getDateStatus();
                        if (this->counter->executeCounter(&count, currentDate, statusDate) && count != 1){
                            cout << count << " linhas de arquivo processadas." << endl;
                        }
                        if (currentDate != statusDate){
                            cout << "Inserindo a tabela: " <<
                            strHand->getTableDateInformation() << endl;
                        }
                        this->checkCreateTimeTable(currentDate, statusDate);
                        this->insertDataDB(currentDate, splitData[1]);
                        this->dtStatus->updateStatusDate(currentDate);
                        rawData = flExt->getDataRawFile();
                        strHand->setRawData(rawData);
                        count++;
                        if(rawData == "eof")eofFlag = rawData; 
                    }
                    delete flExt;
                    delete strHand;
                }
            }
        }  catch (const exception &e) {
            throw e.what();
        }
    }
};

int main(int argc, char *argv[]){
    try {
        string dbConfig;
        string csvFolder;
        if (argc == 3) {
            dbConfig = argv[1];
            csvFolder = argv[2];
        }
        else{
            cout << "Insira a linha de configuração do banco de dados e o caminho da pasta dos arquivos csv! Argumento Inválido." << endl;
            return -1;
        }
    
        TransferDataDB *exe = new TransferDataDB(dbConfig, csvFolder);
        exe->run();
        
    } catch (const exception &e) {
        cout << e.what() << endl;
    }
    return 0;
}
