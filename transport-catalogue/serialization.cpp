#include "serialization.h"
#include <filesystem>
#include <fstream>

using namespace std;
using namespace constructions;
using namespace transport_catalogue;

serializer::Serializer::Serializer(TransportCatalogue& catalogue,
                                   renderer::RenderSettings& render_settings,
                                   transport_router::RoutingSettings& routing_settings)
    : catalogue_(catalogue)
    , render_settings_(render_settings)
    , routing_settings_(routing_settings) {
}

void serializer::Serializer::SaveTo(const string& output) {
    SerializeStops();
    SerializeBuses();
    SerializeDistances();
    SerializeRenderSettings();
    SerializeRoutingSettings();

    ofstream output_file(output, ios::binary);
    serialized_catalogue_.SerializeToOstream(&output_file);
}

void serializer::Serializer::DeserializeTo(const string& input) {
    ifstream input_file(input, ios::binary);
    if (!serialized_catalogue_.ParseFromIstream(&input_file)) {
        throw;
    }

    DeserializeStops();
    DeserializeBuses();
    DeserializeDistances();
    DeserializeRenderSettings();
    DeserializeRoutingSettings();
}

const transport_router::RoutingSettings& serializer::Serializer::GetRenderSettings() const {
    return reinterpret_cast<const transport_router::RoutingSettings &>(render_settings_);
}

void serializer::Serializer::SerializeStops() {
    const auto stops = catalogue_.GetStops();
    vector<serialize::Stop> serialized_stops_list(stops.size());
    *serialized_catalogue_.mutable_stops() = {serialized_stops_list.begin(), serialized_stops_list.end() };
    serialized_catalogue_.mutable_stops()->Reserve(stops.size());

    for (const auto& [stop_name, stop] : stops) {
        serialize::Stop* serialized_stop = serialized_catalogue_.mutable_stops(stop->id);
        *serialized_stop->mutable_name() = stop_name;
        serialized_stop->set_lat(stop->coordinates.lat);
        serialized_stop->set_lng(stop->coordinates.lng);
        serialized_stop->set_id(stop->id);
    }
}

void serializer::Serializer::SerializeBuses() {
    const auto buses = catalogue_.GetBuses();
    for (auto& [bus_name, bus] : buses) {
        serialize::Bus* serialized_bus = serialized_catalogue_.add_buses();
        *serialized_bus->mutable_name() = bus_name;
        serialized_bus->set_is_roundtrip(bus->isRoundTrip);
        for (const auto& stop : bus->stops) {
            serialized_bus->add_index_stops(stop->id);
        }
    }
}

void serializer::Serializer::SerializeDistances() {
    const auto distances = catalogue_.GetDistances();
    for (const auto& [stops, distance] : distances) {
        serialize::Distance* serialized_distance = serialized_catalogue_.add_distances();
        serialized_distance->set_id_stop_first(stops.first->id);
        serialized_distance->set_id_stop_second(stops.second->id);
        serialized_distance->set_distance(distance);
    }
}

serialize::Color serializer::Serializer::SerializeColor(const svg::Color &color) {
    serialize::Color serialized_color;
    if (std::holds_alternative<svg::Rgb>(color)) {
        serialize::RGB* rgb = serialized_color.mutable_rgb();
        rgb->set_red(std::get<svg::Rgb>(color).red);
        rgb->set_green(std::get<svg::Rgb>(color).green);
        rgb->set_blue(std::get<svg::Rgb>(color).blue);
    }
    else if (std::holds_alternative<svg::Rgba>(color)) {
        serialize::RGBA* rgba = serialized_color.mutable_rgba();
        rgba->set_red(std::get<svg::Rgba>(color).red);
        rgba->set_green(std::get<svg::Rgba>(color).green);
        rgba->set_blue(std::get<svg::Rgba>(color).blue);
        rgba->set_opacity(std::get<svg::Rgba>(color).opacity);
    }
    else if (std::holds_alternative<std::string>(color)) {
        serialized_color.set_name_color(std::get<std::string>(color));
    }
    else {
        serialized_color.set_name_color("monostate"s);
    }
    return serialized_color;
}

void serializer::Serializer::SerializeRenderSettings() {
    serialize::RenderSettings* serialized_render_settings_ = serialized_catalogue_.mutable_render_settings();

    serialized_render_settings_->set_width(render_settings_.width_);
    serialized_render_settings_->set_height(render_settings_.height_);
    serialized_render_settings_->set_padding(render_settings_.padding_);
    serialized_render_settings_->set_line_width(render_settings_.line_width_);
    serialized_render_settings_->set_stop_radius(render_settings_.stop_radius_);
    serialized_render_settings_->set_bus_label_font_size(render_settings_.bus_label_font_size_);
    serialized_render_settings_->set_bus_label_offset_x(render_settings_.bus_label_offset_.x);
    serialized_render_settings_->set_bus_label_offset_y(render_settings_.bus_label_offset_.y);
    serialized_render_settings_->set_stop_label_font_size(render_settings_.stop_label_font_size_);
    serialized_render_settings_->set_stop_label_offset_x(render_settings_.stop_label_offset_.x);
    serialized_render_settings_->set_stop_label_offset_y(render_settings_.stop_label_offset_.y);
    *serialized_render_settings_->mutable_underlayer_color() = SerializeColor(render_settings_.underlayer_color_);
    serialized_render_settings_->set_underlayer_width(render_settings_.underlayer_width_);

    for (const svg::Color& color : render_settings_.color_palette_) {
        *serialized_render_settings_->add_color_palette() = SerializeColor(color);
    }
}

void serializer::Serializer::SerializeRoutingSettings() {
    serialize::RoutingSettings* serialized_routing_settings = serialized_catalogue_.mutable_routing_settings();
    serialized_routing_settings->set_bus_wait_time(routing_settings_.bus_wait_time);
    serialized_routing_settings->set_bus_velocity(routing_settings_.bus_velocity);
}

void serializer::Serializer::DeserializeStops() {
    for (const auto& stop : serialized_catalogue_.stops()) {
        catalogue_.AddStop(Stop{stop.name(), geo::Coordinates{ stop.lat(), stop.lng()}});
    }
}

void serializer::Serializer::DeserializeBuses() {
    for (const auto& bus : serialized_catalogue_.buses()) {
        deque<string> stops_names;
        for (const auto& id : bus.index_stops()) {
            string stop_name = serialized_catalogue_.mutable_stops(id)->name();
            stops_names.push_back(move(stop_name));
        }
        catalogue_.AddRoute(Bus{bus.name(), {}, bus.is_roundtrip()}, move(stops_names));
    }
}

void serializer::Serializer::DeserializeDistances() {
    for (const auto& distance : serialized_catalogue_.distances()) {
        const string first_stop_name = serialized_catalogue_.mutable_stops(distance.id_stop_first())->name();
        const string second_stop_name = serialized_catalogue_.mutable_stops(distance.id_stop_second())->name();
        catalogue_.AddDistance(first_stop_name, second_stop_name, distance.distance());
    }
}

void serializer::Serializer::DeserializeRoutingSettings() {
    routing_settings_.bus_wait_time = serialized_catalogue_.routing_settings().bus_wait_time();
    routing_settings_.bus_velocity = serialized_catalogue_.routing_settings().bus_velocity();
}

    svg::Color serializer::Serializer::DeserializeColor(const serialize::Color &serialized_color) {
    if (serialized_color.has_rgb()) {
        svg::Rgb rgb;
        rgb.red = serialized_color.rgb().red();
        rgb.green = serialized_color.rgb().green();
        rgb.blue = serialized_color.rgb().blue();
        return rgb;
    }
    else if (serialized_color.has_rgba()) {
        svg::Rgba rgba;
        rgba.red = serialized_color.rgba().red();
        rgba.green = serialized_color.rgba().green();
        rgba.blue = serialized_color.rgba().blue();
        rgba.opacity = serialized_color.rgba().opacity();
        return rgba;
    }
    else if (serialized_color.name_color() != "monostate"s) {
        return serialized_color.name_color();
    }
    return std::monostate();
}

void serializer::Serializer::DeserializeRenderSettings() {
    serialize::RenderSettings* serialized_render_settings = serialized_catalogue_.mutable_render_settings();

    render_settings_.width_ = serialized_render_settings->width();
    render_settings_.height_ = serialized_render_settings->height();
    render_settings_.padding_ = serialized_render_settings->padding();
    render_settings_.line_width_ = serialized_render_settings->line_width();
    render_settings_.stop_radius_ = serialized_render_settings->stop_radius();
    render_settings_.bus_label_font_size_ = serialized_render_settings->bus_label_font_size();
    render_settings_.bus_label_offset_ = {serialized_render_settings->bus_label_offset_x(), serialized_render_settings->bus_label_offset_y() };
    render_settings_.stop_label_font_size_ = serialized_render_settings->stop_label_font_size();
    render_settings_.stop_label_offset_ = {serialized_render_settings->stop_label_offset_x(), serialized_render_settings->stop_label_offset_y() };
    render_settings_.underlayer_color_ = DeserializeColor(serialized_render_settings->underlayer_color());
    render_settings_.underlayer_width_ = serialized_render_settings->underlayer_width();

    for (serialize::Color serialized_color : serialized_render_settings->color_palette()) {
        render_settings_.color_palette_.push_back(DeserializeColor(serialized_color));
    }
}

