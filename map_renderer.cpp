#include "map_renderer.h"

namespace renderer {
svg::Color& MapRenderer::ChoseColorById(int id) const {
    auto color_id = id % settings_.color_palette_.size();
    return settings_.color_palette_.at(color_id);
}

void MapRenderer::DrawRouteLines(const std::map<std::string_view, transport::Bus*>& buses){
    auto id = 0;
    for (auto &bus: buses){
        svg::Polyline route;
        for (auto &stop: bus.second->stops){
            route.AddPoint(Converter(stop->coord));
        }
        if (bus.second->type == transport::BusType::DirectType){
            for (int i = bus.second->stops.size() - 2; i >= 0; --i){
                route.AddPoint(Converter(bus.second->stops.at(i)->coord));
            }
        }
        if (!bus.second->stops.empty()){
            document_.Add(route.SetStrokeColor(ChoseColorById(id))
                        .SetFillColor("none").SetStrokeWidth(settings_.line_width_)
                        .SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND));
            ++id;
        }

    }
}

void MapRenderer::DrawRouteName(int id, const std::string_view& name, const geo::Coordinates & coords){
    document_.Add(svg::Text().
            SetPosition(Converter(coords)).
            SetOffset(settings_.bus_label_.offset).
            SetFontSize(settings_.bus_label_.font_size).
            SetFontFamily("Verdana").
            SetFontWeight("bold").
            SetData(name.data()).
            SetFillColor(settings_.underlayer_.color).
            SetStrokeColor(settings_.underlayer_.color).
            SetStrokeWidth(settings_.underlayer_.width).
            SetStrokeLineCap(svg::StrokeLineCap::ROUND).
            SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
            );

    document_.Add(svg::Text().
            SetPosition(Converter(coords)).
            SetOffset(settings_.bus_label_.offset).
            SetFontSize(settings_.bus_label_.font_size).
            SetFontFamily("Verdana").
            SetFontWeight("bold").
            SetData(name.data()).
            SetFillColor(ChoseColorById(id))
            );


}

void MapRenderer::DrawRouteNames(const std::map<std::string_view, transport::Bus*>& buses){
    auto id = 0;
    for (auto &bus : buses){
        if (!bus.second->stops.empty()){
            auto &first_stop = bus.second->stops.front()->coord;
            auto &last_stop = bus.second->stops.back()->coord;
            DrawRouteName(id, bus.first, first_stop);
            if ((bus.second->type == transport::BusType::DirectType) &&
                (first_stop != last_stop)){
                DrawRouteName(id, bus.first, last_stop);
            }
            ++id;
        }
    }
}

void MapRenderer::DrawStops(const std::map<std::string_view, transport::Stop*>& stops){
    for (auto &stop: stops){
        document_.Add(svg::Circle().
                SetFillColor("white").
                SetRadius(settings_.stop_radius_).
                SetCenter(Converter(stop.second->coord)));
    }
}

void MapRenderer::DrawStopsNames(const std::map<std::string_view, transport::Stop*>& stops){
    for (auto &stop: stops){
        document_.Add(svg::Text().
                SetPosition(Converter(stop.second->coord)).
                SetOffset(settings_.stop_label_.offset).
                SetFontSize(settings_.stop_label_.font_size).
                SetFontFamily("Verdana").
                SetData(stop.first.data()).
                SetFillColor(settings_.underlayer_.color).
                SetStrokeColor(settings_.underlayer_.color).
                SetStrokeWidth(settings_.underlayer_.width).
                SetStrokeLineCap(svg::StrokeLineCap::ROUND).
                SetStrokeLineJoin(svg::StrokeLineJoin::ROUND));
        document_.Add(svg::Text().
                SetPosition(Converter(stop.second->coord)).
                SetOffset(settings_.stop_label_.offset).
                SetFontSize(settings_.stop_label_.font_size).
                SetFontFamily("Verdana").
                SetData(stop.first.data()).
                SetFillColor("black"));
    }
}


void MapRenderer::MapRender(const std::map<std::string_view, transport::Bus*>& buses, const std::map<std::string_view, transport::Stop*>& stops,  std::ostream& os){
    DrawRouteLines(buses);
    DrawRouteNames(buses);
    DrawStops(stops);
    DrawStopsNames(stops);
    document_.Render(os);
}

} // namespace renderer
