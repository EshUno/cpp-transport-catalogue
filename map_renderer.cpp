#include "map_renderer.h"

namespace renderer {
svg::Color& MapRenderer::ChoseColorById(int id) const {
    auto color_id = id % settings_.color_palette_.size();
    return settings_.color_palette_.at(color_id);
}

void MapRenderer::DrawRouteLines(const std::map<std::string_view, transport::Bus*>& buses, svg::Document &doc){
    auto id = 0;
    // draw Buses
    for (auto &bus: buses){
        svg::Polyline route;
        for (auto &stop: bus.second->stops){
            auto x = stop->coord;
            route.AddPoint(Converter(x));
        }
        if (bus.second->type == transport::BusType::DirectType){
            for (int i = bus.second->stops.size() - 2; i >= 0; --i){
                route.AddPoint(Converter(bus.second->stops.at(i)->coord));
            }
        }
        if (!bus.second->stops.empty()){
            doc.Add(route.SetStrokeColor(ChoseColorById(id))
                        .SetFillColor("none").SetStrokeWidth(settings_.line_width_)
                        .SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND));
            ++id;
        }

    }
}

void MapRenderer::MapRender(const std::map<std::string_view, transport::Bus*>& buses, std::ostream& os){
    svg::Document doc;
    DrawRouteLines(buses, doc);
    doc.Render(os);
}

} // namespace renderer
