#include "DxfConverter.h"

#include <iostream>
#include <memory>
#include <dl_dxf.h>

void assure(bool val) {
#ifndef NDEBUG
    if (!val) {
        __debugbreak();
    }
#endif
}


void DxfConverter::checkPrefix(bool hideLayer = false) {
    uint64_t layer = layerNameMap[attributes.getLayer()];
    if (inBlock) {
        fout << "on_block " << currentBlock << " ";
    } else if (!hideLayer) {
        fout << "on_layer " << layer << " ";
    }
}

void DxfConverter::writeEntity(const char* content) {
    fout << content;
    std::cout << content;
}


template<typename ... Args>
std::string stringFormat(const char* format, Args ... args) {
    size_t size = (size_t) snprintf(nullptr, 0, format, args ...) + 1; // Extra space for '\0'
    char* s = new char[size];
    std::unique_ptr<char[]> buf(s);
    snprintf(s, size, format, args ...);
    return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}


DxfConverter::DxfConverter(const char* input, const char* output) :
    fout(output) {
    DL_Dxf dxf;

    std::string s = R"(abcd'efgh")";

    inBlock = false;
    currentBlock = 0;
    entityType = EntityType::None;
    nextPrefix = "\0";

    if (!dxf.in(input, this)) { // if file open failed
        std::cerr << input << " could not be opened.\n";
        return;
    }

    fout.close();
}


void DxfConverter::addLayer(const DL_LayerData& data) {
    uint64_t i = layers.size();
    fout << "define_layer " << i << " " << data.name << "\n";
    std::cout << "define_layer " << i << " " << data.name << "\n";
    layerNameMap[data.name] = i;
    layers.push_back(data.name);
}


void DxfConverter::addBlock(const DL_BlockData& data) {
    uint64_t index = blocks.size();
    blockNameMap[data.name] = index;
    blocks.push_back(data.name);
    fout << "define_block " << index << " " << data.name << "\n";
    std::cout << "define_block " << index << " " << data.name << " " << index << "\n";
    inBlock = true;
    currentBlock = index;
    assure(data.bpx == 0 && data.bpy == 0);
}

void DxfConverter::endBlock() {
    std::cout << "endBlock\n";
    inBlock = false;
}


void DxfConverter::addPoint(const DL_PointData& data) {
    std::string string = stringFormat("point {x:%lf,y:%lf}\n", data.x, data.y);
    checkPrefix();
    writeEntity(string.c_str());
}

void DxfConverter::addLine(const DL_LineData& data) {
    std::string string = stringFormat("line {x1:%lf,y1:%lf,x2:%lf,y2:%lf}\n", data.x1, data.y1, data.x2, data.y2);
    checkPrefix();
    writeEntity(string.c_str());
}

void DxfConverter::addArc(const DL_ArcData& data) {
    std::string string = stringFormat("arc {cx:%lf,cy:%lf,r:%lf,angle1:%lf,angle2:%lf}\n",
                                      data.cx, data.cy, data.radius, data.angle1, data.angle2);
    checkPrefix();
    writeEntity(string.c_str());
}

void DxfConverter::addCircle(const DL_CircleData& data) {
    std::string string = stringFormat("circle {cx:%lf,cy:%lf,r:%lf}\n",
                                      data.cx, data.cy, data.radius);
    checkPrefix();
    writeEntity(string.c_str());
}


void DxfConverter::addInsert(const DL_InsertData& data) {
    uint64_t block = blockNameMap[data.name.c_str()];
    std::string string = stringFormat("block %llu {x:%lf,y:%lf,angle:%lf,sx:%lf,sy:%lf}\n", block, data.ipx, data.ipy, data.angle, data.sx, data.sy);
    checkPrefix(false);
    writeEntity(string.c_str());
    assure(data.rows == 1 && data.cols == 1);
}


void DxfConverter::addHatch(const DL_HatchData& data) {}

void DxfConverter::addHatchLoop(const DL_HatchLoopData& data) {}

void DxfConverter::addHatchEdge(const DL_HatchEdgeData& data) {}


void DxfConverter::addImage(const DL_ImageData& data) { assure(false); }

void DxfConverter::linkImage(const DL_ImageDefData& data) { assure(false); }


void DxfConverter::addLeader(const DL_LeaderData& data) { assure(false); }

void DxfConverter::addLeaderVertex(const DL_LeaderVertexData& data) { assure(false); }


void DxfConverter::addPolyline(const DL_PolylineData& data) {
    entityType = EntityType::Polyline;
    checkPrefix(false);
    bool closed = data.flags == 1;
    if (closed) {
        writeEntity("polyline 1 [");
    } else {
        writeEntity("polyline 0 [");
    }
    nextPrefix = "\0";
}

void DxfConverter::addVertex(const DL_VertexData& data) {
    writeEntity(nextPrefix);
    std::string string = stringFormat("%lf,%lf", data.x, data.y);
    writeEntity(string.c_str());
    nextPrefix = ",";
}


void DxfConverter::addSpline(const DL_SplineData& data) { assure(false); }


//const std::string&& escape(std::string& s, char from, const char* to) {
//    uint64_t i = 0;
//    size_t len = strlen(to);
//    while ((i = s.find(from, i)) != std::string::npos) {
//        s.replace(i, 1, to);
//        i += len;
//    }
//    return std::move(s);
//}
//
//std::string escapeAll(const std::string& text) {
//    std::string s = text;
//    escape(s, '\\', "\\\\");
//    escape(s, '\'', "\\'");
//    return s;
//}

void DxfConverter::addMText(const DL_MTextData& data) {
//    std::string s = escapeAll(data.text);
//    std::string string = stringFormat("mtext {text:\'%s\',x:%lf,y:%lf,type:%d,height:%lf}\n",
//                                      s.c_str(), data.ipx, data.ipy, data.attachmentPoint, data.height);
//    assure(data.drawingDirection != 3);
//    checkPrefix(false);
//    writeEntity(string.c_str());
}

void DxfConverter::addMTextChunk(const std::string& string) {
    assure(false);
}

void DxfConverter::addText(const DL_TextData& data) {
//    std::string s = escapeAll(data.text);
//    std::string string = stringFormat("text {text:\'%s\',x:%lf,y:%lf,alignH:%d,alignV:%d,height:%lf,xscale:%lf,angle:%lf}\n",
//                                      s.c_str(), data.ipx, data.ipy, data.hJustification, data.vJustification, data.height, data.xScaleFactor, data.angle);
//    checkPrefix(false);
//    writeEntity(string.c_str());
}


void DxfConverter::endEntity() {
    switch (entityType) {
        case EntityType::Polyline:
            writeEntity("]\n");
            break;
        case EntityType::None:
            break;
    }
    entityType = EntityType::None;
}
