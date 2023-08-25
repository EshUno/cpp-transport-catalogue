#include "svg.h"
namespace svg {


std::unordered_map<char, std::string> REP = {{'\"', "&quot;"},
                                             {'\'', "&apos;"},
                                             {'<', "&lt;"},
                                             {'>', "&gt;"},
                                             {'&', "&amp;"}};


std::ostream& operator<<(std::ostream& out, Color color){
    visit(OstreamSolutionPrinter{out}, color);
    return out;
}

std::ostream& operator<<(std::ostream& out, StrokeLineCap slc) {
    switch (slc) {
    case StrokeLineCap::BUTT:
        out << "butt"s;
        break;
    case StrokeLineCap::ROUND:
        out << "round"s;
        break;
    case StrokeLineCap::SQUARE:
        out << "square"s;
        break;
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, StrokeLineJoin slj){
    switch ( slj) {
    case StrokeLineJoin::ARCS:
        out << "arcs"s;
        break;
    case StrokeLineJoin::BEVEL:
        out << "bevel"s;
        break;
    case StrokeLineJoin::MITER:
        out << "miter"s;
        break;
    case StrokeLineJoin::MITER_CLIP:
        out << "miter-clip"s;
        break;
    case StrokeLineJoin::ROUND:
        out << "round"s;
        break;
    }
    return out;
}

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    // <circle cx="50" cy="50" r="50" />
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ ;
    (NeedPathProps())? (out << "\""):(out << "\" ");
    RenderAttrs(context.out);
    out << "/>"sv;
}

//----------Polilyne-------------------
Polyline& Polyline::AddPoint(Point point){
    points_.push_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const{
    // <polyline points="0,100 50,25 50,75 100,0" />
    auto &out = context.out;
    bool first = true;
    out << "<polyline points=\""sv;
    for (auto &point : points_){
        if (!first) out << " ";
        first = false;
        out << point.x << ","<< point.y;
    }
    (NeedPathProps())? (out << "\""):(out << "\" ");
    RenderAttrs(context.out);
    out << "/>"sv;
}

//-------------Text--------------------

Text& Text::SetPosition(Point pos){
    pos_ = pos;
    return *this;
}

Text& Text::SetOffset(Point offset){
    offset_ = offset;
    return *this;
}

Text& Text::SetFontSize(uint32_t size){
    font_size_ = size;
    return *this;
}

Text& Text::SetFontFamily(std::string font_family){
    font_family_ = std::move(font_family);
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight){
    font_weight_ = std::move(font_weight);
    return *this;
}

Text& Text::SetData(std::string data){
    data_ = std::move(data);
    return *this;
}

void Text::RenderObject(const RenderContext& context) const {
    //    <text x="0" y="0" dx="0" dy="0" font-size="1"></text>
    //    <text x="0" y="0" dx="0" dy="6" font-size="12"></text>
    //    <text x="35" y="20" dx="0" dy="6" font-size="12" font-family="Verdana" font-weight="bold">Hello C++</text>

    auto &out = context.out;
    out << "<text";
    RenderAttrs(context.out);
    out<< " x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\" "sv
        << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" "sv
        << "font-size=\""sv << font_size_ << "\""sv;
    if (!font_family_.empty()) out <<" font-family=\""sv << font_family_<< "\""sv;
    if (!font_weight_.empty()) out <<" font-weight=\""sv << font_weight_<< "\""sv;
    out << ">"sv;

    for (auto x: data_){
        if (svg::REP.count(x)) out << REP.at(x);
        else out << x;
    }
    out << "</text>"sv;
}


//--------------Document------------------------

void Document::AddPtr(std::unique_ptr<Object>&& obj){
    objects_.push_back(std::move(obj));
}

void Document::Render(std::ostream& out) const{
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
    RenderContext ctx(out);
    for (auto &obj: objects_){
        out << " ";
        obj.get()->Render(ctx);
    }
    out << "</svg>"sv;
}

}  // namespace svg
