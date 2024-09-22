#include "sdl_audio_RAII.h"
#include <stdexcept>
#include <utils/logger.h>

SDLAudioInitializerRAII::SDLAudioInitializerRAII()
{
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
        throw std::runtime_error(MY_FMT("Failed to initialize SDL_mixer: {}", Mix_GetError()));
}

SDLAudioInitializerRAII::~SDLAudioInitializerRAII()
{
    Mix_CloseAudio();
}

MusicRAII::MusicRAII(const std::string& filename)
{
    music = Mix_LoadMUS(filename.c_str());
    if (!music)
        throw std::runtime_error(MY_FMT("Failed to load music: {} {}", filename, Mix_GetError()));
}

MusicRAII::~MusicRAII()
{
    if (music != nullptr)
    {
        Mix_FreeMusic(music);
    }
}

SoundEffectRAII::SoundEffectRAII(const std::string& filename)
{
    chunk = Mix_LoadWAV(filename.c_str());
    if (!chunk)
        throw std::runtime_error(MY_FMT("Failed to load sound effect: {} {}", filename, Mix_GetError()));
}

SoundEffectRAII::~SoundEffectRAII()
{
    if (chunk != nullptr)
    {
        Mix_FreeChunk(chunk);
    }
}
