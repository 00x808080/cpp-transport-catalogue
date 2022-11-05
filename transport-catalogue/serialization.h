#pragma once
#include <transport_catalogue.pb.h>
#include <svg.pb.h>
#include <map_renderer.pb.h>
#include "request_handler.h"

namespace serializer {

class Serializer {
public:
    explicit Serializer(transport_catalogue::TransportCatalogue& catalogue,
                        renderer::RenderSettings& render_settings,
                        transport_router::RoutingSettings& routing_settings);
    void SaveTo(const std::string& output);
    void DeserializeTo(const std::string& input);

    const transport_router::RoutingSettings& GetRenderSettings() const;
private:
    void SerializeStops();
    void SerializeBuses();
    void SerializeDistances();
    void SerializeRoutingSettings();

    serialize::Color SerializeColor(const svg::Color& color);
    void SerializeRenderSettings();

    void DeserializeStops();
    void DeserializeBuses();
    void DeserializeDistances();
    void DeserializeRoutingSettings();

    svg::Color DeserializeColor(const serialize::Color& serialized_color);
    void DeserializeRenderSettings();

private:
    serialize::TransportCatalogue serialized_catalogue_;
    transport_catalogue::TransportCatalogue& catalogue_;
    renderer::RenderSettings& render_settings_;
    transport_router::RoutingSettings& routing_settings_;
};
}