#include "audio_system.h"
#include <SDL_mixer.h>
#include <utils/logger.h>

AudioSystem::AudioSystem(ResourceManager& resourceManager) : resourceManager(resourceManager), masterVolume(utils::GetConfig<float, "AudioSystem.masterVolume">())
{}

void AudioSystem::PlayMusic(const std::string& musicName)
{
    if (masterVolume == 0.0f)
        return;

    auto musicRAII = resourceManager.GetMusic(musicName);
    Mix_PlayMusic(musicRAII->get(), -1);

    int volume = static_cast<int>(masterVolume * MIX_MAX_VOLUME);
    Mix_VolumeMusic(volume);
}

void AudioSystem::PlaySoundEffect(const std::string& soundEffectName)
{
    if (masterVolume == 0.0f)
        return;

    const auto& soundEffectInfo = resourceManager.GetSoundEffect(soundEffectName);

    // Find a free channel to play the sound.
    int channel = Mix_PlayChannel(-1, soundEffectInfo.soundEffect->get(), 0);
    if (channel == -1)
    {
        MY_LOG(debug, "Failed to play sound effect: No available channels.");
        return;
    }

    auto volumeShift = std::clamp(soundEffectInfo.volumeShift, -0.5f, 0.5f);
    float volumeCoef = 0.5f + volumeShift;
    int volume = static_cast<int>(volumeCoef * masterVolume * MIX_MAX_VOLUME);

    MY_LOG(trace, "Playing sound effect: {} with volume: {}", soundEffectName, volume);

    Mix_Volume(channel, volume);
}