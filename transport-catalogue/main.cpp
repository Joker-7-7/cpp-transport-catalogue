#include <iostream>
#include <string>
#include <fstream>

#include "json_reader.h"
#include "request_handler.h"

using namespace std;

    /*
     * Примерная структура программы:
     *
     * Считать JSON из stdin
     * Построить на его основе JSON базу данных транспортного справочника
     * Выполнить запросы к справочнику, находящиеся в массива "stat_requests", построив JSON-массив
     * с ответами Вывести в stdout ответы в виде JSON
     */

int main() {
    catalogue::Transport catalogue;
    reader::JsonInput reader;
    ifstream in;
    in.open("/Users/ggevorgyan/Documents/practicum-projects/cpp-transport-catalogue/transport-catalogue/input.json");
    
    ofstream out;
    out.open("/Users/ggevorgyan/Documents/practicum-projects/cpp-transport-catalogue/transport-catalogue/out.json");

    const auto document = json::Load(in);
    reader.ReadFrom(document, catalogue);
    reader::ReadJsonRequests(document, catalogue, out);
}
