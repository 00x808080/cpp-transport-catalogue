#include "map_renderer.h"

using namespace std;
using namespace svg;
using namespace renderer;
using namespace constructions;

bool renderer::IsZero(double value) {
    return std::abs(value) < renderer::EPSILON;
}

// ------------- SphereProjector ------------------

svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
    return {
            (coords.lng - min_lon_) * zoom_coefficient_ + padding_,
            (max_lat_ - coords.lat) * zoom_coefficient_ + padding_
    };
}

// ------------- MapRenderer ----------------------

renderer::MapRenderer::MapRenderer(renderer::RenderSettings& settings, transport_catalogue::TransportCatalogue& db)
    : settings_(settings)
    , db_(db) {
}

svg::Document MapRenderer::RenderMap() const {
    set<string> stopNames;
    vector<geo::Coordinates> geo_coords;
    for (const auto& stop : db_.GetStopsData()) {
        auto buses = db_.GetBusesByStop(stop.name);
        if (buses != nullptr && !buses->empty()) {
            stopNames.insert(stop.name);
            geo_coords.push_back(stop.coordinates);
        }
    }
    set<string> busNames;
    for (const auto& bus : db_.GetBusesData()) {
        if (!bus.stops.empty()) {
            busNames.insert(bus.name);
        }
    }
    const renderer::SphereProjector proj{
            geo_coords.begin(),geo_coords.end(),
            settings_.width_,settings_.height_,settings_.padding_
    };

    svg::Document doc;
    AddLinesToMap(doc, busNames, proj);
    AddMainStopNamesToMap(doc, busNames, proj);
    AddDotsToMap(doc, stopNames, proj);
    AddStopNamesToMap(doc, stopNames, proj);
    return doc;
}

svg::Text MapRenderer::GenerateBusNameToText(const string& bus_name,const Stop* stop,const Color& color,
                                                const renderer::SphereProjector& proj) const {
    svg::Text text;
    text.SetData(bus_name);
    text.SetPosition(proj(stop->coordinates));
    text.SetOffset(settings_.bus_label_offset_);
    text.SetFontSize(settings_.bus_label_font_size_);
    text.SetFontFamily("Verdana");
    text.SetFontWeight("bold");
    text.SetFillColor(color);
    return text;
}

svg::Text MapRenderer::GenerateUnderLayerColor (Text text) const {
    text.SetFillColor(settings_.underlayer_color_);
    text.SetStrokeColor(settings_.underlayer_color_);
    text.SetStrokeWidth(settings_.underlayer_width_);
    text.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    text.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    return text;
}

svg::Text MapRenderer::GenerateStopNameToText(const Stop* stop, const renderer::SphereProjector& proj) const {
    svg::Text text;
    text.SetData(stop->name);
    text.SetPosition(proj(stop->coordinates));
    text.SetOffset(settings_.stop_label_offset_);
    text.SetFontSize(settings_.stop_label_font_size_);
    text.SetFontFamily("Verdana");
    text.SetFillColor("black");
    return text;
}

void MapRenderer::AddMainStopNamesToMap(svg::Document& doc,
                                        const set<string>& busNames, const renderer::SphereProjector& proj) const {

    auto it_palette = begin(settings_.color_palette_);
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

        if (it_palette != --end(settings_.color_palette_)) it_palette++;
        else it_palette = begin(settings_.color_palette_);
    }
}

void MapRenderer::AddLinesToMap(svg::Document& doc,
                                const set<string>& bus_names, const renderer::SphereProjector& proj) const {
    auto it_palette = begin(settings_.color_palette_);
    for (auto& busName : bus_names) {
        svg::Polyline polyline;
        auto route = db_.FindRoute(busName);
        for (const auto& stop : route->stops) {
            polyline.AddPoint(proj(stop->coordinates));
        }
        if (!route->isRoundTrip) {
            for (int i = static_cast<int>(route->stops.size()) - 2; i != -1; i--) {
                polyline.AddPoint(proj(route->stops[i]->coordinates));
            }
        }
        polyline.SetStrokeColor(*it_palette);
        polyline.SetStrokeWidth(settings_.line_width_);
        polyline.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        polyline.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        polyline.SetFillColor(svg::NoneColor);
        doc.Add(polyline);

        if (it_palette != --end(settings_.color_palette_)) it_palette++;
        else it_palette = begin(settings_.color_palette_);
    }
}

void MapRenderer::AddDotsToMap(svg::Document& doc,
                               const std::set<std::string>& stopNames,
                               const renderer::SphereProjector& proj) const {
    for (const auto& name : stopNames) {
        svg::Circle circle;
        circle.SetCenter(proj(db_.FindStop(name)->coordinates));
        circle.SetRadius(settings_.stop_radius_);
        circle.SetFillColor("white");
        doc.Add(circle);
    }
}

void MapRenderer::AddStopNamesToMap(svg::Document& doc,
                                    const std::set<std::string>& stopNames,
                                    const renderer::SphereProjector& proj) const {
    for (const auto& name : stopNames) {
        Text stopName = GenerateStopNameToText(db_.FindStop(name), proj);
        Text underStopName = GenerateUnderLayerColor(stopName);
        doc.Add(underStopName);
        doc.Add(stopName);
    }
}