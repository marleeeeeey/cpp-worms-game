#include "sdl_colors.h"
#include <magic_enum.hpp>
#include <map>
#include <my_cpp_utils/math_utils.h>

SDL_Color GetSDLColor(ColorName colorName, Uint8 alpha)
{
    static const std::map<ColorName, SDL_Color> colorMap = {
        {ColorName::White, {255, 255, 255, 255}},  {ColorName::Red, {255, 0, 0, 255}},      {ColorName::Green, {0, 255, 0, 255}},    {ColorName::Blue, {0, 0, 255, 255}},
        {ColorName::Black, {0, 0, 0, 255}},        {ColorName::Yellow, {255, 255, 0, 255}}, {ColorName::Cyan, {0, 255, 255, 255}},   {ColorName::Magenta, {255, 0, 255, 255}},
        {ColorName::Orange, {255, 165, 0, 255}},   {ColorName::Purple, {128, 0, 128, 255}}, {ColorName::Grey, {128, 128, 128, 255}}, {ColorName::DarkGreen, {0, 100, 0, 255}},
        {ColorName::Navy, {0, 0, 128, 255}},       {ColorName::Maroon, {128, 0, 0, 255}},   {ColorName::Olive, {128, 128, 0, 255}},  {ColorName::Teal, {0, 128, 128, 255}},
        {ColorName::Silver, {192, 192, 192, 255}}, {ColorName::Lime, {0, 255, 0, 255}},     {ColorName::Aqua, {0, 255, 255, 255}},   {ColorName::Fuchsia, {255, 0, 255, 255}},
    };

    auto color = colorMap.at(colorName);
    color.a = alpha;

    return color;
}

void SetRenderDrawColor(SDL_Renderer* renderer, const SDL_Color& color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
}

void SetRenderDrawColor(SDL_Renderer* renderer, ColorName colorName)
{
    SetRenderDrawColor(renderer, GetSDLColor(colorName));
}

ColorName GetRandomColorName()
{
    auto colorNumber = utils::Random<size_t>(0, magic_enum::enum_count<ColorName>() - 1);
    return static_cast<ColorName>(colorNumber);
}