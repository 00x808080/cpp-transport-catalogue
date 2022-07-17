#include "json_reader.h"
#include "transport_router.h"

#include <deque>

using namespace std;
using namespace transport_guide;
using namespace constructions;
using namespace request_handler;
using namespace json_reader;

// Формат входных данных:
// { "base_requests":[],"stat_requests":[] }
std::ostream& JSONReader::ReadJSON(istream &input, ostream &output, TransportCatalogue &catalogue) {
    json::Dict requests = json::Load(input).GetRoot().AsDict();
    if (requests.count("base_requests"s)) {
        ReadBaseRequests(catalogue, requests.at("base_requests"s).AsArray());
    }

    renderer::RenderSettings renderSettings;
    if (requests.count("render_settings"s)) {
        renderSettings = ReadRenderSettings(requests.at("render_settings"s).AsDict());
    }

    transport_router::RoutingSettings routingSettings;
    if (requests.count("routing_settings")) {
        routingSettings = ReadRoutingSettings(requests.at("routing_settings").AsDict());
    }

    transport_router::TransportRouter transportRouter{routingSettings, catalogue};
    renderer::MapRenderer renderer(renderSettings, catalogue);
    RequestHandler requestHandler{catalogue, renderer, transportRouter};
    if (requests.count("stat_requests"s)) {
        if (!requests.at("stat_requests"s).AsArray().empty()) {
            PrintStatRequests(output, requestHandler, requests.at("stat_requests"s).AsArray());
        }
    }

    return output;
}

void JSONReader::ReadStopJSON(TransportCatalogue& catalogue, const json::Dict& stopJSON) {
    string mainStopName = stopJSON.at("name"s).AsString();

    Stop stop;
    stop.name = mainStopName;
    if (stopJSON.count("latitude"s)) {
        if (stopJSON.at("latitude"s).IsDouble()) {
            stop.coordinates.lat = stopJSON.at("latitude"s).AsDouble();
        }
        else if (stopJSON.at("latitude"s).IsInt()) {
            stop.coordinates.lat = stopJSON.at("latitude"s).AsInt();
        }
        else stop.coordinates.lat = stod(stopJSON.at("latitude"s).AsString());


        if (stopJSON.at("longitude"s).IsDouble()) {
            stop.coordinates.lng = stopJSON.at("longitude"s).AsDouble();
        }
        else if (stopJSON.at("longitude"s).IsInt()) {
            stop.coordinates.lng = stopJSON.at("longitude"s).AsInt();
        }
        else stop.coordinates.lng = stod(stopJSON.at("longitude"s).AsString());
    }
    catalogue.AddStop(move(stop));

    if (stopJSON.at("road_distances"s).AsDict().empty()) return;
        for (const auto& [name, distance] : stopJSON.at("road_distances"s).AsDict()) {
        if (!catalogue.ContainsStop(name)) {
            Stop nStop;
            nStop.name = name;
            catalogue.AddStop(move(nStop));
        }
        catalogue.AddDistance(mainStopName, name, distance.AsInt());
    }
}

void JSONReader::ReadBusJSON(TransportCatalogue& catalogue, const json::Dict& busJSON) {
    Bus bus;
    bus.name = busJSON.at("name"s).AsString();
    bus.isRoundTrip = busJSON.at("is_roundtrip"s).AsBool();

    deque<string> stops{};
    for (const json::Node& stop : busJSON.at("stops"s).AsArray()) {
        stops.emplace_back(stop.AsString());
    }
    catalogue.AddRoute(move(bus), move(stops));
}

void JSONReader::ReadBaseRequests(TransportCatalogue& catalogue, const json::Array& base) {

    // Считываем остановки
    for (const json::Node& stopOrBus : base) {
        json::Dict stopJSON = stopOrBus.AsDict();
        if (stopJSON.at("type"s).AsString() != "Stop"s) continue;
        ReadStopJSON(catalogue, stopJSON);
    }

    // Cчитываем автобусы
    for (const json::Node& stopOrBus : base) {
        json::Dict busJSON = stopOrBus.AsDict();
        if (busJSON.at("type"s).AsString() != "Bus"s) continue;
        ReadBusJSON(catalogue, busJSON);
    }
}

json::Dict JSONReader::StopRequest(RequestHandler requestHandler, const json::Dict& stopRequestJSON) {
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

json::Dict JSONReader::BusRequest(RequestHandler requestHandler, const json::Dict& busRequestJSON) {
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

json::Dict JSONReader::MapRequest(request_handler::RequestHandler requestHandler, const json::Dict& mapRequestJSON) {
    json::Dict result;
    result.insert({"request_id"s, mapRequestJSON.at("id"s).AsInt()});
    ostringstream svgMap;
    requestHandler.RenderMap().Render(svgMap);
    result.insert({"map"s, svgMap.str()});
    return result;
}

json::Dict JSONReader::RouteRequest(request_handler::RequestHandler requestHandler, const json::Dict &routeRequestJSON) {
    json::Dict result;
    result.insert({"request_id"s, routeRequestJSON.at("id"s).AsInt()});

    string stop_name_from = routeRequestJSON.at("from"s).AsString();
    string stop_name_to = routeRequestJSON.at("to"s).AsString();
    auto routeInfo = requestHandler.BuildRouteBtwStops(stop_name_from, stop_name_to);

    if (!routeInfo.has_value()) {
        result.insert({"error_message"s, "not found"s});
        return result;
    }

    result.insert({"total_time"s, static_cast<double>(routeInfo.value().weight)});

    json::Array arr;
    const auto *edges = &routeInfo.value().edges;
    for (size_t i = 0; i < edges->size(); i++) {
        json::Dict incidence_wait;
        json::Dict incidence_bus;
        auto edge = requestHandler.GetEdge((*edges)[i]);

        {
            string stop_name = requestHandler.GetStopNameFromId(edge.from).data();
            incidence_wait.insert({"stop_name"s, stop_name});
            incidence_wait.insert({"type"s, "Wait"s});
            incidence_wait.insert({"time"s, static_cast<int>(requestHandler.BusWaitTime())});
        }

        {
            const auto *info = &requestHandler.GetEdgeInfo((*edges)[i]);
            incidence_bus.insert({"type"s, "Bus"s});
            incidence_bus.insert({"bus"s, info->bus_name.data()});
            incidence_bus.insert({"span_count"s, info->span_count});
            incidence_bus.insert({"time"s, edge.weight - static_cast<int>(requestHandler.BusWaitTime())});
        }

        arr.push_back(incidence_wait);
        arr.push_back(incidence_bus);
    }

    result.insert({"items"s, arr});

    return result;
}

void JSONReader::PrintStatRequests(ostream& output, RequestHandler& requestHandler, const json::Array& stat) {
    json::Array result;
    for (const json::Node &request: stat) {
        json::Dict requestJSON = request.AsDict();
        if (requestJSON.at("type"s).AsString() == "Stop"s) {
            result.push_back(StopRequest(requestHandler, requestJSON));
        }
        else if (requestJSON.at("type"s).AsString() == "Bus"s) {
            result.push_back(BusRequest(requestHandler, requestJSON));
        }
        else if (requestJSON.at("type"s).AsString() == "Map"s) {
            result.push_back(MapRequest(requestHandler, requestJSON));
        }
        else if (requestJSON.at("type"s).AsString() == "Route"s) {
            result.push_back(RouteRequest(requestHandler, requestJSON));
        }
    }
    json::Print(json::Document{json::Builder{}.Value(result).Build()}, output);
}

svg::Point JSONReader::ReadPoint(const json::Array& arr) {
    svg::Point point;
    if (arr[0].IsInt()) point.x = arr[0].AsInt();
    else point.x = arr[0].AsDouble();

    if (arr[1].IsInt()) point.y = arr[1].AsInt();
    else point.y = arr[1].AsDouble();

    return point;
}

svg::Color JSONReader::ReadColor(const json::Node& node) {
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
                           arr[3].AsDouble()};
            color = rgba;
        }
    }
    return color;
}

renderer::RenderSettings JSONReader::ReadRenderSettings(const json::Dict& render) {
    renderer::RenderSettings renderSettings;
    if (render.at("width").IsDouble()) renderSettings.width_ = render.at("width").AsDouble();
    else if (render.at("width").IsInt()) renderSettings.width_ = render.at("width").AsInt();
    else renderSettings.width_ = stod(render.at("width").AsString());

    if (render.at("height").IsDouble()) renderSettings.height_ = render.at("height").AsDouble();
    else if (render.at("height").IsInt()) renderSettings.height_ = render.at("height").AsInt();
    else renderSettings.height_ = stod(render.at("height").AsString());

    if (render.at("padding").IsDouble()) renderSettings.padding_ = render.at("padding").AsDouble();
    else if (render.at("padding").IsInt()) renderSettings.padding_ = render.at("padding").AsInt();
    else renderSettings.padding_ = stod(render.at("padding").AsString());

    if (render.at("line_width").IsDouble()) renderSettings.line_width_ = render.at("line_width").AsDouble();
    else if (render.at("line_width").IsInt()) renderSettings.line_width_ = render.at("line_width").AsInt();
    else renderSettings.line_width_ = stod(render.at("line_width").AsString());

    if (render.at("stop_radius").IsDouble()) renderSettings.stop_radius_ = render.at("stop_radius").AsDouble();
    else if (render.at("stop_radius").IsInt()) renderSettings.stop_radius_ = render.at("stop_radius").AsInt();
    else renderSettings.stop_radius_ = stod(render.at("stop_radius").AsString());

    renderSettings.bus_label_font_size_ = render.at("bus_label_font_size").AsInt();
    renderSettings.bus_label_offset_ = ReadPoint(render.at("bus_label_offset").AsArray());
    renderSettings.stop_label_font_size_ = render.at("stop_label_font_size").AsInt();
    renderSettings.stop_label_offset_ = ReadPoint(render.at("stop_label_offset").AsArray());
    renderSettings.underlayer_color_ = ReadColor(render.at("underlayer_color"));

    if (render.at("underlayer_width").IsDouble()) renderSettings.underlayer_width_ = render.at("underlayer_width").AsDouble();
    else if (render.at("underlayer_width").IsInt()) renderSettings.underlayer_width_ = render.at("underlayer_width").AsInt();
    else renderSettings.underlayer_width_ = stod(render.at("underlayer_width").AsString());

    for (const auto& colorJSON : render.at("color_palette").AsArray()) {
        renderSettings.color_palette_.push_back(ReadColor(colorJSON));
    }
    return renderSettings;
}

transport_router::RoutingSettings JSONReader::ReadRoutingSettings(const json::Dict& routingSettingsJSON) {
    transport_router::RoutingSettings routingSettings;

    routingSettings.bus_wait_time = routingSettingsJSON.at("bus_wait_time").AsInt();
    routingSettings.bus_velocity = routingSettingsJSON.at("bus_velocity").AsInt();

    return routingSettings;
}
