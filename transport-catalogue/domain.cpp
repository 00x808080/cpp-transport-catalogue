#include "domain.h"

#include <utility>

using namespace constructions;
using namespace std;

Stop::Stop(const std::string& stop, geo::Coordinates& n_coordinates)
        : name(stop)
        , coordinates(n_coordinates) {
}

bool constructions::Stop::operator==(const Stop &right) const {
    return name == right.name;
}

bool constructions::Bus::operator==(const Bus &right) const {
    return name == right.name;
}

size_t StopDistanceHasher::operator()( pair<const Stop*, const Stop*> stops_pair) const {
    size_t h_x = ptrHasher_(stops_pair.first);
    size_t h_y = ptrHasher_(stops_pair.second);

    return h_x + h_y * 37;
}

bool operator==(const constructions::Stop& left, const constructions::Stop& right) {
    return left.name == right.name;
}

bool operator==(const constructions::Bus& left, const constructions::Bus& right) {
    return left.name == right.name;
}