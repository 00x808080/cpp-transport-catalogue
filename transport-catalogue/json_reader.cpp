#include "json_reader.h"

#include <deque>

using namespace std;
using namespace transport_guide;
using namespace constructions;
using namespace request_handler;

void ReadStopJSON(TransportCatalogue& catalogue, const json::Dict& stopJSON) {
    string mainStopName = stopJSON.at("name"s).AsString();

    Stop stop;
    stop.name = mainStopName;
    if (stopJSON.count("latitude"s)) {
        if (stopJSON.at("latitude"s).IsString()) {
            stop.coordinates.lat = stod(stopJSON.at("latitude"s).AsString());
        }
        else stop.coordinates.lat = stopJSON.at("latitude"s).AsInt();

        if (stopJSON.at("longitude"s).IsString()) {
            stop.coordinates.lng = stod(stopJSON.at("longitude"s).AsString());
        }
        else stop.coordinates.lng = stopJSON.at("longitude"s).AsInt();
    }
    catalogue.AddStop(move(stop));

    if (stopJSON.at("road_distances"s).AsMap().empty()) return;
    for (const auto& [name, distance] : stopJSON.at("road_distances"s).AsMap()) {
        if (!catalogue.ContainsStop(name)) {
            Stop nStop;
            nStop.name = name;
            catalogue.AddStop(move(nStop));
        }
        catalogue.AddDistance(mainStopName, name, distance.AsInt());
    }
}

void ReadBusJSON(TransportCatalogue& catalogue, const json::Dict& busJSON) {
    Bus bus;
    bus.name = busJSON.at("name"s).AsString();
    bus.isRoundTrip = busJSON.at("is_roundtrip"s).AsBool();

    deque<string> stops{};
    for (const json::Node& stop : busJSON.at("stops"s).AsArray()) {
        stops.emplace_back(stop.AsString());
    }
    catalogue.AddRoute(move(bus), move(stops));
}

void ReadBaseRequests(TransportCatalogue& catalogue, const json::Array& base) {

    // Считываем остановки
    for (const json::Node& stopOrBus : base) {
        json::Dict stopJSON = stopOrBus.AsMap();
        if (stopJSON.at("type"s).AsString() != "Stop"s) continue;
        ReadStopJSON(catalogue, stopJSON);
    }

    // Cчитываем автобусы
    for (const json::Node& stopOrBus : base) {
        json::Dict busJSON = stopOrBus.AsMap();
        if (busJSON.at("type"s).AsString() != "Bus"s) continue;
        ReadBusJSON(catalogue, busJSON);
    }
}

json::Dict StopRequest(RequestHandler requestHandler, const json::Dict& stopRequestJSON) {
    json::Dict result;
    result.insert({"request_id"s, stopRequestJSON.at("id"s).AsInt()});

    auto buses_ptr = requestHandler.GetBusesByStop(stopRequestJSON.at("name"s).AsString());
    if (buses_ptr == nullptr) {
        result.insert({"error_message"s, "not found"s});
        return result;
    }

    json::Array arr;
    for (const string_view bus_name : *buses_ptr) {
        arr.push_back(string{bus_name});
    }
    result.insert({"buses"s, arr});
    return result;
}

json::Dict BusRequest(RequestHandler requestHandler, const json::Dict& busRequestJSON) {
    json::Dict result;
    result.insert({"request_id"s, busRequestJSON.at("id"s).AsInt()});

    optional<BusStat> stat = requestHandler.GetBusStat(busRequestJSON.at("name"s).AsString());
    if (!stat.has_value()) {
        result.insert({"error_message"s, "not found"s});
        return result;
    }

    result.insert({"route_length"s, stat->routeLength});
    result.insert({"stop_count"s, stat->stopsCount});
    result.insert({"unique_stop_count"s, stat->uniqueStopsCount});
    result.insert({"curvature"s, stat->curvature});
    return result;
}

json::Dict MapRequest(request_handler::RequestHandler requestHandler, const json::Dict& mapRequestJSON) {
    json::Dict result;
    result.insert({"request_id"s, mapRequestJSON.at("id"s).AsInt()});
    ostringstream svgMap;
    requestHandler.RenderMap().Render(svgMap);
    result.insert({"map"s, svgMap.str()});
    return result;
}

void PrintStatRequests(ostream& output, RequestHandler& requestHandler, const json::Array& stat) {
    json::Array result;
    for (const json::Node &request: stat) {
        json::Dict requestJSON = request.AsMap();
        if (requestJSON.at("type"s).AsString() == "Stop"s) {
            result.push_back(StopRequest(requestHandler, requestJSON));
        } else if (requestJSON.at("type"s).AsString() == "Bus"s) {
            result.push_back(BusRequest(requestHandler, requestJSON));
        } else if (requestJSON.at("type"s).AsString() == "Map"s) {
            result.push_back(MapRequest(requestHandler, requestJSON));
        }
    }
    json::Print(json::Document{result}, output);
}

svg::Point ReadPoint(const json::Array& arr) {
    svg::Point point;
    if (arr[0].IsInt()) point.x = arr[0].AsInt();
    else point.x = stod(arr[0].AsString());

    if (arr[1].IsInt()) point.y = arr[1].AsInt();
    else point.y = stod(arr[1].AsString());

    return point;
}

svg::Color ReadColor(const json::Node& node) {
    svg::Color color;
    if (node.IsString()) {
        color = node.AsString();
    }
    else {
        auto arr = node.AsArray();
        if (arr.size() == 3u) {
            svg::Rgb rgb{static_cast<uint8_t>(arr[0].AsInt()),
                         static_cast<uint8_t>(arr[1].AsInt()),
                         static_cast<uint8_t>(arr[2].AsInt())};
            color = rgb;
        }
        else if (arr.size() == 4u) {
            svg::Rgba rgba{static_cast<uint8_t>(arr[0].AsInt()),
                           static_cast<uint8_t>(arr[1].AsInt()),
                           static_cast<uint8_t>(arr[2].AsInt()),
                           stod(arr[3].AsString())};
            color = rgba;
        }
    }
    return color;
}

renderer::MapRenderer ReadRenderSettings(const json::Dict& render) {
    renderer::MapRenderer renderer;
    if (render.at("width").IsInt()) renderer.width_ = render.at("width").AsInt();
    else renderer.width_ = stod(render.at("width").AsString());

    if (render.at("height").IsInt()) renderer.height_ = render.at("height").AsInt();
    else renderer.height_ = stod(render.at("height").AsString());

    if (render.at("padding").IsInt()) renderer.padding_ = render.at("padding").AsInt();
    else renderer.padding_ = stod(render.at("padding").AsString());

    if (render.at("line_width").IsInt()) renderer.line_width_ = render.at("line_width").AsInt();
    else renderer.line_width_ = stod(render.at("line_width").AsString());

    if (render.at("stop_radius").IsInt()) renderer.stop_radius_ = render.at("stop_radius").AsInt();
    else renderer.stop_radius_ = stod(render.at("stop_radius").AsString());

    renderer.bus_label_font_size_ = render.at("bus_label_font_size").AsInt();
    renderer.bus_label_offset_ = ReadPoint(render.at("bus_label_offset").AsArray());
    renderer.stop_label_font_size_ = render.at("stop_label_font_size").AsInt();
    renderer.stop_label_offset_ = ReadPoint(render.at("stop_label_offset").AsArray());
    renderer.underlayer_color_ = ReadColor(render.at("underlayer_color"));

    if (render.at("underlayer_width").IsInt()) renderer.underlayer_width_ = render.at("underlayer_width").AsInt();
    else renderer.underlayer_width_ = stod(render.at("underlayer_width").AsString());

    for (const auto& colorJSON : render.at("color_palette").AsArray()) {
        renderer.color_palette_.push_back(ReadColor(colorJSON));
    }
    return renderer;
}

// Формат входных данных:
// { "base_requests":[],"stat_requests":[] }
std::ostream& ReadJSON(istream& input, ostream& output, TransportCatalogue& catalogue) {
    json::Dict requests = json::Load(input).GetRoot().AsMap();
    if (requests.count("base_requests"s)) {
        ReadBaseRequests(catalogue, requests.at("base_requests"s).AsArray());
    }

    renderer::MapRenderer renderer;
    if (requests.count("render_settings"s)) {
        renderer = ReadRenderSettings(requests.at("render_settings"s).AsMap());
    }

    RequestHandler requestHandler{catalogue, renderer};
    if (requests.count("stat_requests"s)) {
        if (!requests.at("stat_requests"s).AsArray().empty()) {
            PrintStatRequests(output, requestHandler, requests.at("stat_requests"s).AsArray());
        }
    }

//    requestHandler.RenderMap().Render(output);
    return output;
}