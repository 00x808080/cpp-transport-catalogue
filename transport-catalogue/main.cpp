#include <iostream>
#include "transport_catalogue.h"
#include "json_reader.h"

int main() {
    transport_guide::TransportCatalogue catalogue;
    ReadJSON(std::cin, std::cout, catalogue);
}

//{"base_requests": [
//{
//"type": "Bus",
//"name": "114",
//"stops": ["Морской вокзал"],
//"is_roundtrip": false
//},
//{
//"type": "Bus",
//"name": "116",
//"stops": ["Морской вокзал", "Ривьерский мост"],
//"is_roundtrip": false
//},
//{
//"type": "Stop",
//"name": "Ривьерский мост",
//"latitude": 43.587795,
//"longitude": 39.716901,
//"road_distances": {"Морской вокзал": 850}
//},
//{
//"type": "Stop",
//"name": "Морской вокзал",
//"latitude": 43.581969,
//"longitude": 39.719848,
//"road_distances": {"Ривьерский мост": 850}
//}
//],
//"stat_requests": [
//{ "id": 1, "type": "Stop", "name": "NULL" }
//]}

//{
//  "base_requests": [
//      {"type": "Stop", "name": "Stop A", "latitude": 1.0, "longitude": 0.001 "road_distances": {}},
//      {"type": "Stop", "name": "Stop B", "latitude": 1.0, "longitude": 0.002, "road_distances": {"Stop A": 1000, "Stop C": 1000}},
//      {"type": "Stop", "name": "Stop C", "latitude": 1.0, "longitude": 0.003, "road_distances": {"Stop A": 1000}},
//      {"type": "Bus", "name": "Bus1", "stops": ["Stop A", "Stop B", "Stop C", "Stop A"], "is_roundtrip": true}
//  ],
//  "stat_requests": [
//          {"id": 1, "type": "Bus", "name": "Bus1"}
//      ]
//}


//{"base_requests":
//[{
//"type": "Stop", "name": "A", "latitude": 0.01, "longitude": 0.02, "road_distances": {"B": 10000, "C": 15000}}, {
//"type": "Stop", "name": "B", "latitude": 0.01, "longitude": 0.03, "road_distances": {}}, {
//"type": "Stop", "name": "С", "latitude": 0.01, "longitude": 0.04, "road_distances": {}}, {
//"type": "Bus", "name": "Bus", "stops": ["A", "B", "C"], "is_roundtrip": false
//}],
//"stat_requests":
//[{
//"id": 1, "type": "Bus", "name": "Bus"
//}]}



//{"base_requests": [
//    {"type": "Bus", "name": "117", "stops": ["Stop B"], "is_roundtrip": false},
//    {"type": "Bus", "name": "114", "stops": ["Stop B", "Stop A"], "is_roundtrip": false},
//    {"type": "Stop", "name": "Stop A", "latitude": 0, "longitude": 0, "road_distances": {}},
//    {"type": "Stop", "name": "Stop B", "latitude": 0, "longitude": 0, "road_distances": {}}
//],
//"stat_requests": [
//    { "id": 2, "type": "Stop", "name": "Stop X" }
//]}


//{"base_requests": [
//    {"type": "Bus", "name": "10к", "stops": ["Stop B"], "is_roundtrip": false},
//    {"type": "Bus", "name": "20к", "stops": ["Stop A", "Stop B"], "is_roundtrip": false},
//    {"type": "Stop", "name": "Stop A", "latitude": 2342134, "longitude": 1413.341341, "road_distances": {"B": 10000, "C": 15000}},
//    {"type": "Stop", "name": "Stop B", "latitude": -341341, "longitude": 3441, "road_distances": {}},
//    {"type": "Stop", "name": "Stop С", "latitude": 12441.2, "longitude": -55135.3, "road_distances": {"A": 30000}}
//],
//"stat_requests": [
//      {"id": 1, "type": "Stop", "name": "Stop B"},
//      {"id": 24243, "type": "Bus", "name": "20к"},
//      {"id": 2342423, "type": "Stop", "name": "Stop A"},
//      {"id": 1235235523, "type": "Bus", "name": "10к"},
//      {"id": 2147483647, "type": "Stop", "name": "Stop C"}
//]}