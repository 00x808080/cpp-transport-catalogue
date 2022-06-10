#include "request_handler.h"

using namespace std;
using namespace constructions;
using namespace request_handler;
using namespace svg;

optional<BusStat> RequestHandler::GetBusStat(const string_view& bus_name) const {
    BusStat stat;
    if (!db_.ContainsBus(bus_name)) return nullopt;
    Bus route = *db_.FindRoute(bus_name);
    if (!route.stops.empty() && !route.isRoundTrip) {
        for (int i = route.stops.size() - 2; i != -1; i--) {
            route.stops.push_back(route.stops[i]);
        }
    }
    stat.stopsCount = route.stops.size();

    std::set<std::string_view> unique_stops;
    for (const constructions::Stop *stop : route.stops) {
        unique_stops.insert(stop->name);
    }
    stat.uniqueStopsCount = unique_stops.size();



    double distance = 0;
    double routeLength = 0;
    for (int i = 0; i < stat.stopsCount - 1; i++) {
        routeLength  += ComputeDistance(route.stops[i]->coordinates,
                                        route.stops[i + 1]->coordinates);

        distance += db_.GetDistanceBtwStops(*route.stops[i],
                                            *route.stops[i + 1]);
    }
    stat.curvature = distance / routeLength ;
    stat.routeLength = static_cast<int>(routeLength * stat.curvature);

    return { stat };
}

const set<string_view>* RequestHandler::GetBusesByStop(const string_view &stop_name) const {
    return db_.GetBusesByStop(stop_name);
}

svg::Document RequestHandler::RenderMap() const {
    set<string> stopNames;
    vector<geo::Coordinates> geo_coords;
    for (const auto& stop : db_.GetStops()) {
        auto buses = db_.GetBusesByStop(stop.name);
        if (buses != nullptr && !buses->empty()) {
            stopNames.insert(stop.name);
            geo_coords.push_back(stop.coordinates);
        }
    }
    set<string> busNames;
    for (const auto& bus : db_.GetBuses()) {
        if (!bus.stops.empty()) {
            busNames.insert(bus.name);
        }
    }
    const renderer::SphereProjector proj{
            geo_coords.begin(),geo_coords.end(),
            renderer_.width_,renderer_.height_,renderer_.padding_
    };

    svg::Document doc;
    AddLinesToMap(doc, busNames, proj);
    AddMainStopNamesToMap(doc, busNames, proj);
    AddDotsToMap(doc, stopNames, proj);
    AddStopNamesToMap(doc, stopNames, proj);
    return doc;
}

svg::Text RequestHandler::GenerateBusNameToText(const string& bus_name,const Stop* stop,const Color& color,
                                                const renderer::SphereProjector& proj) const {
    svg::Text text;
    text.SetData(bus_name);
    text.SetPosition(proj(stop->coordinates));
    text.SetOffset(renderer_.bus_label_offset_);
    text.SetFontSize(renderer_.bus_label_font_size_);
    text.SetFontFamily("Verdana");
    text.SetFontWeight("bold");
    text.SetFillColor(color);
    return text;
}

svg::Text RequestHandler::GenerateUnderLayerColor (Text text) const {
    text.SetFillColor(renderer_.underlayer_color_);
    text.SetStrokeColor(renderer_.underlayer_color_);
    text.SetStrokeWidth(renderer_.underlayer_width_);
    text.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    text.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    return text;
}

svg::Text RequestHandler::GenerateStopNameToText(const Stop* stop, const renderer::SphereProjector& proj) const {
    svg::Text text;
    text.SetData(stop->name);
    text.SetPosition(proj(stop->coordinates));
    text.SetOffset(renderer_.stop_label_offset_);
    text.SetFontSize(renderer_.stop_label_font_size_);
    text.SetFontFamily("Verdana");
    text.SetFillColor("black");
    return text;
}

void RequestHandler::AddMainStopNamesToMap(svg::Document& doc,
                                           const set<string>& busNames,
                                           const renderer::SphereProjector& proj) const {

    auto it_palette = begin(renderer_.color_palette_);
    for (const auto& busName : busNames) {
        auto bus = db_.FindRoute(busName);
        if (bus->stops.front() != bus->stops.back()){
            svg::Text name = GenerateBusNameToText(bus->name, bus->stops[0], *it_palette, proj);
            svg::Text under = GenerateUnderLayerColor(name);
            doc.Add(under);
            doc.Add(name);

            svg::Text back_name = GenerateBusNameToText(bus->name, bus->stops.back(), *it_palette, proj);
            svg::Text back_under = GenerateUnderLayerColor(back_name);
            doc.Add(back_under);
            doc.Add(back_name);
        }
        else {
            svg::Text name = GenerateBusNameToText(bus->name, bus->stops[0], *it_palette, proj);
            svg::Text under = GenerateUnderLayerColor(name);
            doc.Add(under);
            doc.Add(name);
        }

        if (it_palette != --end(renderer_.color_palette_)) it_palette++;
        else it_palette = begin(renderer_.color_palette_);
    }
}

void RequestHandler::AddLinesToMap(svg::Document& doc,
                                   const set<string>& bus_names,
                                   const renderer::SphereProjector& proj) const {
    auto it_palette = begin(renderer_.color_palette_);
    for (auto& busName : bus_names) {
        svg::Polyline polyline;
        auto route = db_.FindRoute(busName);
        for (const auto& stop : route->stops) {
            polyline.AddPoint(proj(stop->coordinates));
        }
        if (!route->isRoundTrip) {
            for (int i = route->stops.size() - 2; i != -1; i--) {
                polyline.AddPoint(proj(route->stops[i]->coordinates));
            }
        }
        polyline.SetStrokeColor(*it_palette);
        polyline.SetStrokeWidth(renderer_.line_width_);
        polyline.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        polyline.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        polyline.SetFillColor(svg::NoneColor);
        doc.Add(polyline);

        if (it_palette != --end(renderer_.color_palette_)) it_palette++;
        else it_palette = begin(renderer_.color_palette_);
    }
}

void RequestHandler::AddDotsToMap(svg::Document& doc,
                  const std::set<std::string>& stopNames,
                  const renderer::SphereProjector& proj) const {
    for (const auto& name : stopNames) {
        svg::Circle circle;
        circle.SetCenter(proj(db_.FindStop(name)->coordinates));
        circle.SetRadius(renderer_.stop_radius_);
        circle.SetFillColor("white");
        doc.Add(circle);
    }
}

void RequestHandler::AddStopNamesToMap(svg::Document& doc,
                       const std::set<std::string>& stopNames,
                       const renderer::SphereProjector& proj) const {
    for (const auto& name : stopNames) {
        Text stopName = GenerateStopNameToText(db_.FindStop(name), proj);
        Text underStopName = GenerateUnderLayerColor(stopName);
        doc.Add(underStopName);
        doc.Add(stopName);
    }
}
