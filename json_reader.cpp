#include "json_reader.h"
#include "json_builder.h"
#include <sstream>
namespace reader {
using namespace std::literals;
void FillTheTransportCatalogue(transport::TransportCatalogue &tc, const json::Array &requests){
    std::vector<reader::Bus> bus_queries;
    bus_queries.reserve(requests.size());
    std::vector<reader::Stop> stop_queries;
    stop_queries.reserve(requests.size());
    for (auto &req : requests){
        const auto& req_map= req.AsDict();
        if (req_map.at("type"s) == "Stop"s){
            auto x = reader::CreateStop(req_map);
            stop_queries.push_back(x);
            tc.AddStop(x.stop);
        }
        else if (req_map.at("type"s) == "Bus"s){
            bus_queries.push_back(std::move(reader::CreateBus(req_map)));
        }
    }
    for (auto &stop : stop_queries){
        for (auto &one : stop.dm){
            tc.AddDistance(tc.FindStop(stop.stop.name), tc.FindStop(one.first), one.second);
        }
    }
    for (auto &bus : bus_queries){
        tc.AddBus(bus.name, bus.type, bus.stops_name);
    }
}

reader::Stop CreateStop(const json::Dict &request){
    reader::Stop stop;
    stop.stop.name = request.at("name"s).AsString();
    stop.stop.coord.lat = request.at("latitude"s).AsDouble();
    stop.stop.coord.lng = request.at("longitude"s).AsDouble();
    for (auto &[name, dist] : request.at("road_distances").AsDict()){
        stop.dm[name] = dist.AsDouble();
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
void LoadQueries(const transport::TransportCatalogue &tc, renderer::MapRenderer &mr, const json::Array &requests, std::ostream& os, const routing::TransportRouter &tr){
    json::Array arr;
    for (auto &req : requests){
        const auto& req_map= req.AsDict();
        if (req_map.at("type"s) == "Bus"s){
            auto bus = tc.FindBus(req_map.at("name"s).AsString());
            transport::BusPrintInfo bus_info;
            bus_info.id = req_map.at("id"s).AsInt();
            if (bus != nullptr) bus_info = tc.GetBusPrintInfo(bus, req_map.at("id"s).AsInt());
            else {
                bus_info.name = req_map.at("name"s).AsString();
                bus_info.exist = false;
            }
            arr.push_back(PrintBus(&bus_info));
        }
        else if (req_map.at("type"s) == "Stop"s){
            arr.push_back(PrintStop(req_map.at("id"s).AsInt(), tc.GetInfoAboutStop(req_map.at("name"s).AsString())));
        }
        else if (req_map.at("type"s) == "Map"s){
            arr.push_back(PrintMap(req_map.at("id"s).AsInt(), mr.MapRender(tc.GetBuses(), tc.GetUsedStops())));
        }
        else if (req_map.at("type"s) == "Route"s){
            arr.push_back(PrintRoute(req_map.at("id"s).AsInt(), tr.BuildRoute(req_map.at("from"s).AsString(), req_map.at("to"s).AsString())));
        }
    }

    json::Print(json::Document(arr), os);
}

void LoadStats(const transport::TransportCatalogue &tc, renderer::MapRenderer &mr, const json::Array &requests, std::ostream& os){
    json::Array arr;
    for (auto &req : requests){
        const auto& req_map= req.AsDict();

        if (req_map.at("type"s) == "Bus"s){
            auto bus = tc.FindBus(req_map.at("name"s).AsString());
            transport::BusPrintInfo bus_info;
            bus_info.id = req_map.at("id"s).AsInt();
            if (bus != nullptr) bus_info = tc.GetBusPrintInfo(bus, req_map.at("id"s).AsInt());
            else {
                bus_info.name = req_map.at("name"s).AsString();
                bus_info.exist = false;
            }
            arr.push_back(PrintBus(&bus_info));
        }
        else if (req_map.at("type"s) == "Stop"s){
            arr.push_back(PrintStop(req_map.at("id"s).AsInt(), tc.GetInfoAboutStop(req_map.at("name"s).AsString())));
        }
        else if (req_map.at("type"s) == "Map"s){
            arr.push_back(PrintMap(req_map.at("id"s).AsInt(), mr.MapRender(tc.GetBuses(), tc.GetUsedStops())));
        }
    }
    json::Print(json::Document(arr), os);
}

json::Dict PrintBus(const transport::BusPrintInfo* info){
    json::Builder result;
    result.StartDict();
    result.Key("request_id"s).Value(info->id);
    if (info->exist == false){
        result.Key("error_message"s).Value("not found"s);
    }
    else{
        result.Key("curvature"s).Value(info->curvature);
        result.Key("route_length"s).Value(info->route_length);
        result.Key("stop_count"s).Value(info->stops_route);
        result.Key("unique_stop_count"s).Value(info->unique_stops);
    }
    result.EndDict();
    return result.Build().AsDict();
}

json::Dict PrintStop(int id, std::optional<const std::set<std::string_view>*> st){
    json::Builder result;
    result.StartDict();
    result.Key("request_id"s).Value(id);
    if (st == std::nullopt){
        result.Key("error_message"s).Value("not found"s);
    }
    else if (*st == nullptr) {
        result.Key("buses"s).StartArray().EndArray();
    }
    else if (*st != nullptr) {
        result.Key("buses"s).StartArray();
        for (auto &x: **st){
            result.Value(std::string(x));
        }
        result.EndArray();
    }
    result.EndDict();
    return result.Build().AsDict();
}

json::Dict PrintMap(int id, const svg::Document& doc){
    json::Builder result;
    result.StartDict();
    result.Key("request_id"s).Value(id);
    std::ostringstream str;
    doc.Render(str);
    result.Key("map"s).Value(str.str());
    result.EndDict();
    return result.Build().AsDict();
}

json::Dict PrintRoute(int id, std::optional<routing::RoutePrintInfo> route){
    json::Builder result;
    result.StartDict();
    result.Key("request_id"s).Value(id);
    if (route == std::nullopt){
        result.Key("error_message"s).Value("not found"s);
    }
    else {
        result.Key("total_time"s).Value(route->total_time);
        result.Key("items"s).StartArray();
        for (auto edge : route->edges){
            result.StartDict();
            if (std::holds_alternative<routing::WaitEdge>(edge)){
                result.Key("type"s).Value("Wait"s);
                auto wait_edge  = std::get<routing::WaitEdge>(edge);
                result.Key("stop_name"s).Value(wait_edge.stop_name.data());
                result.Key("time"s).Value(wait_edge.time);
            }
            else{
                result.Key("type"s).Value("Bus"s);
                auto bus_edge  = std::get<routing::BusEdge>(edge);
                result.Key("bus"s).Value(bus_edge.bus_name.data());
                result.Key("span_count"s).Value(bus_edge.span_count);
                result.Key("time"s).Value(bus_edge.time);
            }
            result.EndDict();
        }
        result.EndArray();
    }
    result.EndDict();
    return result.Build().AsDict();
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

//---------------------routing---------------------------------//

routing::Settings LoadRoutingSettings(const json::Dict &settings){
    routing::Settings st;
    st.bus_wait_time_ = settings.at("bus_wait_time").AsInt();

    double km_h_velocity = settings.at("bus_velocity").AsDouble();
    st.bus_velocity_ = (km_h_velocity * 1000) / 60.0;

    return st;
}

// =-----------------------serialization------------------------//

serialization::Settings LoadSerializationSettings(const json::Dict &settings){
    serialization::Settings st;
    st.path = settings.at("file").AsString();
    return st;
}
} // reader
