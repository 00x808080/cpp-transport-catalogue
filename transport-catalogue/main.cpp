#include <fstream>
#include <iostream>
#include <string_view>

#include "json_builder.h"
#include "json_reader.h"
#include "serialization.h"

using namespace std::literals;

//void PrintUsage(std::ostream& stream = std::cerr) {
//    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
//}
//
//int main(int argc, char* argv[]) {
//    if (argc != 2) {
//        PrintUsage();
//        return 1;
//    }
//
//    const std::string_view mode(argv[1]);
//    json_reader::JSONReader jsonReader;
//    transport_catalogue::TransportCatalogue transportCatalogue;
//    if (mode == "make_base"sv) {
//        jsonReader.MakeBase(std::cin, transportCatalogue);
//    } else if (mode == "process_requests"sv) {
//        jsonReader.ProcessRequests(std::cin, transportCatalogue);
//    } else {
//        PrintUsage();
//        return 1;
//    }
//}

int main() {
    json_reader::JSONReader jsonReader;
    transport_catalogue::TransportCatalogue transportCatalogue;

//    jsonReader.MakeBase(std::cin, transportCatalogue);
    jsonReader.ProcessRequests(std::cin, transportCatalogue);
}