#pragma once

#include "geo.h"
#include "domain.h"

#include <utility>
#include <string_view>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <set>
#include <algorithm>
#include <iostream>

namespace transport_catalogue {

class TransportCatalogue {
public:
    TransportCatalogue() = default;

    void AddStop(constructions::Stop&& stop);
    void AddDistance(const std::string& stop1, const std::string& stop2, int distance);
    void AddRoute(constructions::Bus&& bus, std::deque<std::string>&& stops);

    const constructions::Bus* FindRoute(const std::string_view& bus) const;
    const constructions::Stop* FindStop(const std::string_view& stop) const;

    bool ContainsStop(const std::string_view& stop) const;
    bool ContainsBus(const std::string_view& bus) const;

    double GetDistanceBtwStops(const constructions::Stop& stop1, const constructions::Stop& stop2) const;
    const std::set<std::string_view>* GetBusesByStop(const std::string_view& stop_name) const;
    const std::deque<constructions::Stop>& GetStopsData() const;
    const std::deque<constructions::Bus>& GetBusesData() const;
    const std::unordered_map<std::string_view, constructions::Stop*>& GetStops() const;
    const std::unordered_map<std::string_view, constructions::Bus*>& GetBuses() const;
    const std::unordered_map<std::pair<const constructions::Stop*, const constructions::Stop*>,
                             int, constructions::StopDistanceHasher>& GetDistances() const;


private:
    std::deque<constructions::Stop> stops_data_;
    std::deque<constructions::Bus> buses_data_;
    std::unordered_map<std::string_view, constructions::Stop*> stops_;
    std::unordered_map<std::string_view, constructions::Bus*> buses_routes_;
    std::unordered_map<std::string_view, std::set<std::string_view>> stop_to_buses_;
    std::unordered_map<std::pair<const constructions::Stop*, const constructions::Stop*>,
                       int, constructions::StopDistanceHasher> stops_distances_;
};

} // namespace transport_catalogue

