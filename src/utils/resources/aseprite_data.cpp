#include "aseprite_data.h"
#include <regex>

AsepriteData LoadAsepriteData(const nlohmann::json& asepriteJsonData)
{
    AsepriteData asepriteData;

    // Load frame tags.
    for (const auto& frameTagJson : asepriteJsonData["meta"]["frameTags"])
    {
        AsepriteData::FrameTag frameTag;
        frameTag.name = frameTagJson["name"];
        frameTag.from = frameTagJson["from"];
        frameTag.to = frameTagJson["to"];
        asepriteData.frameTags[frameTag.name] = frameTag;
    }

    // Load frames.
    for (const auto& [frameName, frameData] : asepriteJsonData["frames"].items())
    {
        AsepriteData::Frame frame;

        // Read originalIndex from frameName `something 120.aseprite` via regex
        std::regex re(".* (\\d+)\\.aseprite");
        std::smatch match;
        std::regex_search(frameName, match, re);

        // The only one frame in the animation file.
        if (match.size() == 0)
            frame.originalIndex = 0;
        // The frame has an originalIndex.
        else if (match.size() == 2)
            frame.originalIndex = std::stoul(match[1]);
        // Unrecognized frame name format.
        else
            throw std::runtime_error("[LoadAsepriteData] Unrecognized frame name format");

        auto frameDurationMs = frameData["duration"];
        frame.duration_seconds = static_cast<float>(frameDurationMs) / 1000.0f;
        frame.rectInTexture.x = frameData["frame"]["x"];
        frame.rectInTexture.y = frameData["frame"]["y"];
        frame.rectInTexture.w = frameData["frame"]["w"];
        frame.rectInTexture.h = frameData["frame"]["h"];
        asepriteData.frames.push_back(frame);
    }

    // Sort frames by originalIndex.
    std::sort(asepriteData.frames.begin(), asepriteData.frames.end(), [](const auto& a, const auto& b) { return a.originalIndex < b.originalIndex; });

    // Load texture path.
    asepriteData.texturePath = asepriteJsonData["meta"]["image"];

    return asepriteData;
}