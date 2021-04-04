#pragma once

#include "dl_creationadapter.h"

#include <fstream>
#include <vector>
#include <map>
#include <string>

class DxfConverter : public DL_CreationAdapter {
private:
    std::ofstream fout;
    std::vector<std::string> layers;
    std::map<std::string, uint64_t> layerNameMap;
    std::vector<std::string> blocks;
    std::map<std::string, uint64_t> blockNameMap;
    bool inBlock;
    uint64_t currentBlock;

    enum class EntityType {
        None,
        Polyline
    } entityType;
    const char* nextPrefix;

    void checkPrefix(bool hideLayer);
    void writeEntity(const char* content);

public:
    DxfConverter(const char* input, const char* output);

    void addLayer(const DL_LayerData& data) override;

    //definition:
    //    block
    void addBlock(const DL_BlockData& data) override;
    void endBlock() override;

    //entity:
    //    shape
    void addPoint(const DL_PointData& data) override;
    void addLine(const DL_LineData& data) override;
    void addArc(const DL_ArcData& data) override;
    void addCircle(const DL_CircleData& data) override;
    //    block
    void addInsert(const DL_InsertData& data) override;
    //    hatch
    void addHatch(const DL_HatchData& data) override;
    void addHatchLoop(const DL_HatchLoopData& data) override;
    void addHatchEdge(const DL_HatchEdgeData& data) override;
    //    image
    void addImage(const DL_ImageData& data) override;
    //    image def
    void linkImage(const DL_ImageDefData& data) override;
    //    leader
    void addLeader(const DL_LeaderData& data) override;
    void addLeaderVertex(const DL_LeaderVertexData& data) override;
    //    polyline
    void addPolyline(const DL_PolylineData& data) override;
    void addVertex(const DL_VertexData& data) override;
    //    spline
    void addSpline(const DL_SplineData& data) override;
    //    text
    void addMText(const DL_MTextData& data) override;
    void addMTextChunk(const std::string& string) override;
    void addText(const DL_TextData& data) override;
    //finish
    void endEntity() override;
};
