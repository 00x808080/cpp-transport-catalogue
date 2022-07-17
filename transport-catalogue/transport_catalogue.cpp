#include "transport_catalogue.h"

using namespace std;
using namespace transport_guide;
using namespace constructions;

void TransportCatalogue::AddStop(Stop &&stop) {
    string stopName = stop.name;
    if (find(stops_data_.begin(), stops_data_.end(), stop) == stops_data_.end()) {
        stop.id = static_cast<int>(stops_data_.size());
        stops_data_.push_back(std::move(stop));
        stops_.insert({stops_data_.back().name, &stops_data_.back()});
    }
    else if (stops_[stop.name]->coordinates.lng == 0 &&
             stops_[stop.name]->coordinates.lat == 0) {
        stops_[stop.name]->coordinates = stop.coordinates;
    }
    stop_to_buses_[stops_.at(stopName)->name];
}

void TransportCatalogue::AddDistance(const string& stop1, const string& stop2, int distance) {
    stop_distance_.insert({{stops_.at(stop1), stops_.at(stop2)}, distance});
}

void TransportCatalogue::AddRoute(Bus &&bus, deque<std::string> &&stops) {
    for (std::string& stop_name : stops) {
        constructions::Stop stop;
        stop.name = stop_name;
        AddStop(std::move(stop));
        bus.stops.push_back(stops_.at(stop_name));
    }
    buses_data_.push_back(std::move(bus));
    buses_routes_.insert({buses_data_.back().name, &buses_data_.back()});

    //Заполнение stop_to_buses_
    for (Stop *stop : buses_data_.back().stops) {
        stop_to_buses_[stop->name].insert(buses_data_.back().name);
    }
}

const constructions::Bus *TransportCatalogue::FindRoute(const string_view& bus) const {
    return buses_routes_.at(bus);
}

const constructions::Stop *TransportCatalogue::FindStop(const string_view& stop) const {
    return stops_.at(stop);
}

bool TransportCatalogue::ContainsStop(const string_view& stop) const {
    return stops_.count(stop);
}

bool TransportCatalogue::ContainsBus(const string_view& bus) const {
    return buses_routes_.count(bus);
}

int TransportCatalogue::GetDistanceBtwStops(const Stop &stop1, const Stop &stop2) const {
    int distance;
    // проверяем расстояние stop1 - stop2, если в базе такого расстояние нет, то проверяется stop2 - stop1
    try {
        distance = stop_distance_.at({&stop1, &stop2});
        return distance;
    }
    catch (std::out_of_range&) {
        try {
            distance = stop_distance_.at({&stop2, &stop1});
            return distance;
        }
        catch (std::out_of_range&) {
            return 0;
        }
    }
}

const std::set<string_view>* TransportCatalogue::GetBusesByStop(const std::string_view& stop_name) const {
    if (stop_to_buses_.count(stop_name)) {
        return &stop_to_buses_.at(stop_name);
    }
    return nullptr;
}

const std::deque<constructions::Stop>& TransportCatalogue::GetStops() const {
    return stops_data_;
}

const std::deque<constructions::Bus>& TransportCatalogue::GetBuses() const {
    return buses_data_;
}