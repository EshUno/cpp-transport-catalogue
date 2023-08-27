#include "json_reader.h"

namespace reader {
using namespace std::literals;
void FillTheTransportCatalogue(transport::TransportCatalogue &tc, const json::Array &requests){
    std::vector<reader::Bus> bus_queries;
    bus_queries.reserve(requests.size());
    std::vector<reader::Stop> stop_queries;
    stop_queries.reserve(requests.size());
    for (auto &req : requests){
        const auto& req_map= req.AsMap();
        if (req_map.at("type"s) == "Stop"s){
            auto x = reader::CreateStop(req_map);
            stop_queries.push_back(x);
            tc.AddStop(x.stop);
        }
        else if (req_map.at("type"s) == "Bus"s){
            bus_queries.push_back(std::move(reader::CreateBus(req_map)));
        }
    }
    for (auto &x : stop_queries){
        for (auto &one : x.dm){
            tc.AddDistance(tc.FindStop(x.stop.name), tc.FindStop(one.first), one.second);
        }
    }

    for (auto &x : bus_queries){
        tc.AddBus(x.name, x.type, x.stops_name);
    }

}
reader::Stop CreateStop(const json::Dict &request){
    reader::Stop stop;
    stop.stop.name = request.at("name"s).AsString();
    stop.stop.coord.lat = request.at("latitude"s).AsDouble();
    stop.stop.coord.lng = request.at("longitude"s).AsDouble();

    for (auto &dist : request.at("road_distances").AsMap()){
        stop.dm[dist.first] = dist.second.AsDouble();
    }
    return stop;
}

reader::Bus CreateBus(const json::Dict &request){
    reader::Bus bus;
    bus.type = (request.at("is_roundtrip").AsBool())?(transport::BusType::CircularType):(transport::BusType::DirectType);
    bus.name = request.at("name"s).AsString();
    for (auto &stop : request.at("stops").AsArray()){
        bus.stops_name.push_back(stop.AsString());
    }

    return bus;
}

//--------------------- stat ----------------------//
void LoadStatQueries(transport::TransportCatalogue &tc, const json::Array &requests, std::ostream& os){
    json::Array arr;
    for (auto &req : requests){
        const auto& req_map= req.AsMap();
        if (req_map.at("type"s) == "Bus"s){
            auto bus = tc.FindBus(req_map.at("name"s).AsString());
            transport::BusPrintInfo bus_info;
            bus_info.id = req_map.at("id"s).AsInt();
            if (bus != nullptr) bus_info = tc.GetBusPrintInfo(bus, req_map.at("id"s).AsInt());
            else {
                bus_info.name = req_map.at("name"s).AsString();
                bus_info.exist = false;
            }
            arr.push_back(PrintBus( &bus_info));
        }
        else if (req_map.at("type"s) == "Stop"s){
            arr.push_back(PrintStop(req_map.at("id"s).AsInt(), tc.GetInfoAboutStop(req_map.at("name"s).AsString())));
        }
    }

    json::Print(json::Document(arr), os);
}

json::Dict PrintBus(transport::BusPrintInfo* info){
    json::Dict res;
    if (info->exist == false){
        res["request_id"s] = info->id;
        res["error_message"s] = "not found"s;
    }
    else {
        res["curvature"s] = info->curvature;
        res["request_id"s] = info->id;
        res["route_length"s] = info->route_length;
        res["stop_count"s] = info->stops_route;
        res["unique_stop_count"s] = info->unique_stops;
    }
    return res;
}

json::Dict PrintStop(int id, std::optional<const std::set<std::string_view>*> st){
    json::Dict res;
    res["request_id"s] = id;
    if (st == std::nullopt){
        res["error_message"s] = "not found"s;
    }
    else if (*st == nullptr) {
        res["buses"s] = json::Array{};
    }
    else if (*st != nullptr) {
        json::Array arr;
        for (auto &x: **st){
            arr.push_back(std::string(x));
        }
        res["buses"s] = arr;
    }
    return res;
}

//-----------------map renderer----------------//

svg::Color DetectColor(const json::Node& node){
    if (node.IsString()) return node.AsString();
    json::Array color = node.AsArray();
    if (color.size() == 3){
        return svg::Rgb(color[0].AsInt(), color[1].AsInt(), color[2].AsInt());
    }
    return svg::Rgba(color[0].AsInt(), color[1].AsInt(), color[2].AsInt(), color[3].AsDouble());
}

renderer::Settings LoadMapRendererSettings(const json::Dict &settings){
    renderer::Settings st;
    st.image_.height = settings.at("height").AsDouble();
    st.image_.width = settings.at("width").AsDouble();
    st.image_.padding = settings.at("padding").AsDouble();

    st.line_width_ = settings.at("line_width").AsDouble();
    st.stop_radius_ = settings.at("stop_radius").AsDouble();

    st.bus_label_.font_size = settings.at("bus_label_font_size").AsDouble();
    json::Array offset = settings.at("bus_label_offset").AsArray();
    st.bus_label_.offset.x = offset[0].AsDouble();
    st.bus_label_.offset.y = offset[1].AsDouble();

    st.stop_label_.font_size = settings.at("stop_label_font_size").AsDouble();
    offset = settings.at("stop_label_offset").AsArray();
    st.stop_label_.offset.x = offset[0].AsDouble();
    st.stop_label_.offset.y = offset[1].AsDouble();

    st.underlayer_.width = settings.at("underlayer_width").AsDouble();
    st.underlayer_.color = DetectColor(settings.at("underlayer_color"));

    json::Array palette = settings.at("color_palette").AsArray();
    for (auto &color: palette){
        st.color_palette_.push_back(DetectColor(color));
    }
    return st;
}



} // reader
