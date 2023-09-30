#pragma once

#include "svg.h"
#include "domain.h"
#include <limits>
namespace renderer {

inline const double EPSILON = 1e-6;
inline bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

struct ImageSphere{
    // ширина и высота изображения в пикселях
    double width;
    double height;
    // отступ краёв карты от границ SVG-документа
    double padding;
};

// для названий остановок и маршрутов
struct Label{
    // размер надписи
    double font_size;
    // смещение надписи
    svg::Point offset;
};

struct Underlayer{
    // цвет подложки под названиями остановок и маршрутов.
    svg::Color color;
    // толщина подложки под названиями остановок и маршрутов.
    // задаёт значение атрибута stroke-width элемента <text>
    double width;
};

struct Settings{
    ImageSphere image_;
    // толщина линий, которыми рисуются автобусные маршруты
    double line_width_;
    // радиус окружностей, которыми обозначаются остановки
    double stop_radius_;
    Label bus_label_;
    Label stop_label_;
    Underlayer underlayer_;
    std::vector<svg::Color> color_palette_;
};


class MapRenderer{
public:
    MapRenderer(Settings &settings, geo::Coordinates min_coord, geo::Coordinates max_coord):
        settings_(settings), min_lon_(min_coord.lng), max_lat_(max_coord.lat){
            UpdateZoom(min_coord, max_coord);
        };

    void UpdateZoom(geo::Coordinates& min_coord, geo::Coordinates& max_coord){
        const auto [min_lat, min_lon] = min_coord;
        const auto [max_lat, max_lon] = max_coord;

        // Вычисляем коэффициент масштабирования вдоль координаты x
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (settings_.image_.width - 2 * settings_.image_.padding) / (max_lon - min_lon_);
        }
        // Вычисляем коэффициент масштабирования вдоль координаты y
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (settings_.image_.height - 2 * settings_.image_.padding) / (max_lat_ - min_lat);
        }
        if (width_zoom && height_zoom) {
            // Коэффициенты масштабирования по ширине и высоте ненулевые,
            // берём минимальный из них
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        } else if (width_zoom) {
            // Коэффициент масштабирования по ширине ненулевой, используем его
            zoom_coeff_ = *width_zoom;
       } else if (height_zoom) {
            // Коэффициент масштабирования по высоте ненулевой, используем его
            zoom_coeff_ = *height_zoom;
        }
    }
    const svg::Document& MapRender(const transport::BusesInfo& buses, const transport::StopsInfo& stops);

    // Проецирует широту и долготу в координаты внутри SVG-изображения
    svg::Point Converter(geo::Coordinates coords) const {
        auto x = (coords.lng - min_lon_) * zoom_coeff_ + settings_.image_.padding;
        auto y = (max_lat_ - coords.lat) * zoom_coeff_ + settings_.image_.padding;
        return {x, y};
    }
    const svg::Color& ChoseColorById(int id) const;
    void DrawRouteLines(const transport::BusesInfo& buses);
    void DrawRouteNames(const transport::BusesInfo& buses);
    void DrawRouteName(int id, const std::string_view& name, const geo::Coordinates & coords);
    void DrawStops(const transport::StopsInfo& stops);
    void DrawStopsNames(const transport::StopsInfo& stops);


private:
    const Settings &settings_;
    svg::Document document_;

    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

} // namespace renderer




























