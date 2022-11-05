#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "request_handler.h"
#include "map_renderer.h"
#include "json_builder.h"
#include "transport_router.h"

namespace json_reader {

class JSONReader {
public:
    void MakeBase(std::istream &input, transport_catalogue::TransportCatalogue &catalogue);
    void ProcessRequests(std::istream &input, transport_catalogue::TransportCatalogue& catalogue);

private:
    void ReadStopJSON(transport_catalogue::TransportCatalogue &catalogue, const json::Dict &stopJSON);
    void ReadBusJSON(transport_catalogue::TransportCatalogue &catalogue, const json::Dict &busJSON);
    void ReadBaseRequests(transport_catalogue::TransportCatalogue &catalogue, const json::Array &base);

    json::Dict StopRequest(request_handler::RequestHandler requestHandler, const json::Dict &stopRequestJSON);
    json::Dict BusRequest(request_handler::RequestHandler requestHandler, const json::Dict &busRequestJSON);
    json::Dict MapRequest(request_handler::RequestHandler requestHandler, const json::Dict &mapRequestJSON);
    json::Dict RouteRequest(request_handler::RequestHandler requestHandler, const json::Dict &routeRequestJSON);

    void PrintStatRequests(std::ostream &output,
                           request_handler::RequestHandler &requestHandler,
                           const json::Array &stat);

    svg::Point ReadPoint(const json::Array &arr);
    svg::Color ReadColor(const json::Node &node);
    renderer::RenderSettings ReadRenderSettings(const json::Dict &render);

    transport_router::RoutingSettings ReadRoutingSettings(const json::Dict& routingSettingsJSON);
};

} // namespace json_reader