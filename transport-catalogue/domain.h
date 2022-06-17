#pragma once

#include "geo.h"

#include <string>
#include <deque>
#include <set>

namespace constructions {

struct Stop {
    Stop() = default;
    Stop(std::string&  stop, geo::Coordinates& n_coordinates);
    Stop(Stop&&) = default;
    Stop& operator=(Stop&&) = default;

    bool operator==(const Stop& right) const;

    std::string name;
    geo::Coordinates coordinates{};
};

struct Bus {
    Bus() = default;

    bool operator==(const Bus& right) const;

    std::string name;
    std::deque<Stop*> stops;
    bool isRoundTrip = false;
};

struct BusStat {
    int stopsCount = 0;
    int uniqueStopsCount = 0;
    int routeLength = 0;
    double curvature = 0;
};

struct StopDistanceHasher {
    size_t operator() (std::pair<const Stop*, const Stop*> stops_pair) const;

    std::hash<const void*> ptrHasher_;
};

} // namespace constructions

bool operator==(const constructions::Stop& left, const constructions::Stop& right);

bool operator==(const constructions::Bus& left, const constructions::Bus& right);