#include <iostream>
#include "transport_catalogue.h"
#include "json_reader.h"

int main() {
    json_reader::JSONReader reader;
    transport_guide::TransportCatalogue catalogue;
    reader.ReadJSON(std::cin, std::cout, catalogue);
}
