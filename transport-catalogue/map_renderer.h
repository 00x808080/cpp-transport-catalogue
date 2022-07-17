#pragma once

#include "geo.h"
#include "svg.h"
#include "json.h"
#include "domain.h"
#include "transport_catalogue.h"
#include <string>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <optional>

namespace renderer {

inline const double EPSILON = 1e-6;
bool IsZero(double value);

struct RenderSettings {
    RenderSettings() = default;

    double width_= {};
    double height_ = {};
    double padding_ = {};
    double line_width_ = {};
    double stop_radius_ = {};
    int bus_label_font_size_ = {};
    svg::Point bus_label_offset_ = {};
    int stop_label_font_size_ = {};
    svg::Point stop_label_offset_ = {};
    svg::Color underlayer_color_;
    double underlayer_width_ = {};
    std::vector<svg::Color> color_palette_;
};

class SphereProjector {
public:
    // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                    double max_width, double max_height, double padding)
            : padding_(padding) //
    {
        // Если точки поверхности сферы не заданы, вычислять нечего
        if (points_begin == points_end) {
            return;
        }

        // Находим точки с минимальной и максимальной долготой
        const auto [left_it, right_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        // Находим точки с минимальной и максимальной широтой
        const auto [bottom_it, top_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        // Вычисляем коэффициент масштабирования вдоль координаты x
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        // Вычисляем коэффициент масштабирования вдоль координаты y
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            // Коэффициенты масштабирования по ширине и высоте ненулевые,
            // берём минимальный из них
            zoom_coefficient_ = std::min(*width_zoom, *height_zoom);
        } else if (width_zoom) {
            // Коэффициент масштабирования по ширине ненулевой, используем его
            zoom_coefficient_ = *width_zoom;
        } else if (height_zoom) {
            // Коэффициент масштабирования по высоте ненулевой, используем его
            zoom_coefficient_ = *height_zoom;
        }
    }

    // Проецирует широту и долготу в координаты внутри SVG-изображения
    svg::Point operator()(geo::Coordinates coords) const;

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coefficient_ = 0;
};

class MapRenderer {
public:
    explicit MapRenderer(renderer::RenderSettings& settings, transport_guide::TransportCatalogue& db);

    [[nodiscard]] svg::Document RenderMap() const;

private:
    svg::Text GenerateBusNameToText(const std::string& bus_name,
                                    const constructions::Stop* stop,
                                    const svg::Color& color,
                                    const renderer::SphereProjector& proj) const;

    [[nodiscard]] svg::Text GenerateUnderLayerColor (svg::Text text) const;
    svg::Text GenerateStopNameToText(const constructions::Stop* stop, const renderer::SphereProjector& proj) const;

    void AddMainStopNamesToMap(svg::Document& doc,
            const std::set<std::string>& busNames, const renderer::SphereProjector& proj) const;

    void AddLinesToMap(svg::Document& doc,
                       const std::set<std::string>& bus_names, const renderer::SphereProjector& proj) const;

    void AddDotsToMap(svg::Document& doc,
                      const std::set<std::string>& stopNames, const renderer::SphereProjector& proj) const;

    void AddStopNamesToMap(svg::Document& doc,
                           const std::set<std::string>& stopNames, const renderer::SphereProjector& proj) const;

private:
    const renderer::RenderSettings settings_;
    const transport_guide::TransportCatalogue &db_;
};

} // namespace renderer