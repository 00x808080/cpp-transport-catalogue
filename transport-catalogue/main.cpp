#include <iostream>
#include "json_reader.h"

using namespace std;

int main() {
    json_reader::JSONReader reader;
    transport_guide::TransportCatalogue catalogue;
    reader.ReadJSON(std::cin, std::cout, catalogue);
}