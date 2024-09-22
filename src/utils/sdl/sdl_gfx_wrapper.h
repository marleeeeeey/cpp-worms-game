#pragma once
#include <SDL.h>
#include <glm/vec2.hpp>
#include <vector>

enum class FillOption
{
    Filled,
    Outlined
};

int DrawPoligon(SDL_Renderer* renderer, std::vector<glm::vec2> vertices, SDL_Color color, FillOption fillOption = FillOption::Outlined);

int DrawCircle(SDL_Renderer* renderer, glm::vec2 center, float radius, SDL_Color color, FillOption fillOption = FillOption::Outlined);

void DrawCross(SDL_Renderer* renderer, int centerX, int centerY, int size, const SDL_Color& color);

void DrawCross(SDL_Renderer* renderer, const glm::vec2& center, int size, const SDL_Color& color);
