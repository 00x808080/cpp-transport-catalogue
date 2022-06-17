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
    explicit RequestHandler(const transport_guide::TransportCatalogue& db, const renderer::MapRenderer& renderer);

    // Возвращает информацию о маршруте (запрос Bus)
    [[nodiscard]] std::optional<constructions::BusStat> GetBusStat(const std::string_view &bus_name) const;

    // Возвращает маршруты, проходящие через
    [[nodiscard]] const std::set<std::string_view>* GetBusesByStop(const std::string_view &stop_name) const;

    [[nodiscard]] svg::Document RenderMap() const;

private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    const transport_guide::TransportCatalogue& db_;
    const renderer::MapRenderer& renderer_;
};

}