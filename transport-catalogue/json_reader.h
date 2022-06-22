#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "request_handler.h"
#include "map_renderer.h"
#include "json_builder.h"

namespace json_reader {

class JSONReader {
public:
    std::ostream &ReadJSON(std::istream &input, std::ostream &output, transport_guide::TransportCatalogue &catalogue);

private:
    void ReadStopJSON(transport_guide::TransportCatalogue &catalogue, const json::Dict &stopJSON);
    void ReadBusJSON(transport_guide::TransportCatalogue &catalogue, const json::Dict &busJSON);
    void ReadBaseRequests(transport_guide::TransportCatalogue &catalogue, const json::Array &base);

    json::Dict StopRequest(request_handler::RequestHandler requestHandler, const json::Dict &stopRequestJSON);
    json::Dict BusRequest(request_handler::RequestHandler requestHandler, const json::Dict &busRequestJSON);
    json::Dict MapRequest(request_handler::RequestHandler requestHandler, const json::Dict &mapRequestJSON);

    void PrintStatRequests(std::ostream &output,
                           request_handler::RequestHandler &requestHandler,
                           const json::Array &stat);

    svg::Point ReadPoint(const json::Array &arr);
    svg::Color ReadColor(const json::Node &node);
    renderer::RenderSettings ReadRenderSettings(const json::Dict &render);
};

} // namespace json_reader