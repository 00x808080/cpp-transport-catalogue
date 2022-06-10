#pragma once

#include "domain.h"
#include "transport_catalogue.h"
#include "json.h"
#include "map_renderer.h"
#include <string>
#include <sstream>
#include <unordered_set>

namespace request_handler {

// Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
// с другими подсистемами приложения.
// См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)
class RequestHandler {
public:
    explicit RequestHandler(const transport_guide::TransportCatalogue& db, const renderer::MapRenderer& renderer)
        : db_(db)
        , renderer_(renderer) {
    }

    // Возвращает информацию о маршруте (запрос Bus)
    std::optional<constructions::BusStat> GetBusStat(const std::string_view &bus_name) const;

    // Возвращает маршруты, проходящие через
    const std::set<std::string_view>* GetBusesByStop(const std::string_view &stop_name) const;

    svg::Document RenderMap() const;

private:
    svg::Text GenerateBusNameToText(const std::string& bus_name,
                                    const constructions::Stop* stop,
                                    const svg::Color& color,
                                    const renderer::SphereProjector& proj) const;

    svg::Text GenerateUnderLayerColor (svg::Text text) const;

    svg::Text GenerateStopNameToText(const constructions::Stop* stop, const renderer::SphereProjector& proj) const;

    void AddMainStopNamesToMap(svg::Document& doc,
                               const std::set<std::string>& busNames,
                               const renderer::SphereProjector& proj) const;

    void AddLinesToMap(svg::Document& doc,
                       const std::set<std::string>& bus_names,
                       const renderer::SphereProjector& proj) const;

    void AddDotsToMap(svg::Document& doc,
                      const std::set<std::string>& stopNames,
                      const renderer::SphereProjector& proj) const;

    void AddStopNamesToMap(svg::Document& doc,
                           const std::set<std::string>& stopNames,
                           const renderer::SphereProjector& proj) const;

    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    const transport_guide::TransportCatalogue &db_;
    const renderer::MapRenderer& renderer_;
};

}