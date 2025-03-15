
#include <SDL.h>
#include <iostream>
#include <vector>
#include <SDL_mixer.h>
export module Audio;

using namespace std;

export void playThing() {




    // Initialize SDL
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return;
    }

    // Initialize SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
        return;
    }

    // Load WAV file
    Mix_Music* music = Mix_LoadMUS("assets/audio/kick_drum.wav");
    if (music == NULL) {
        printf("Failed to load WAV file! SDL_mixer Error: %s\n", Mix_GetError());
        Mix_CloseAudio();
        return;
    }

    // Play music
    if (Mix_PlayMusic(music, -1) == -1) {
        printf("Failed to play music! SDL_mixer Error: %s\n", Mix_GetError());
        Mix_FreeMusic(music);
        Mix_CloseAudio();
        return;
    }

    // Clean up
    Mix_FreeMusic(music);
    Mix_CloseAudio();





}



export class AudioBooth {
public:
    /* Deleted copy constructor and assignment operator to prevent copies */
    AudioBooth(const AudioBooth&) = delete;
    AudioBooth& operator=(const AudioBooth&) = delete;

    /* static method to get instance of the singleton */
    static AudioBooth& getInstance() {
        static AudioBooth instance; /* will be destroyed when program exits */
        return instance;
    }

    void freeAllAudio() {
        if (kickDrumSound != NULL) {
            Mix_FreeMusic(music);
            Mix_FreeChunk(kickDrumSound);
        }

    }

    void playMusic() {
        if (Mix_PlayMusic(music, -1) == -1) {
            printf("Failed to play music! SDL_mixer Error: %s\n", Mix_GetError());
        }
    }

    void playKickDrum() {
        Mix_PlayChannel(-1, kickDrumSound, 0);
    }


private:
    /* Constructor is private to prevent outside instantiation */
    AudioBooth() {
        cout << "AudioBooth created\n";

        /* Initialize SDL audio. */
        if (SDL_Init(SDL_INIT_AUDIO) < 0) {
            printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
            return;
        }

        // Initialize SDL_mixer
        if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
            printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
            Mix_CloseAudio();
            return;
        }

        /* Load WAV files. */
        kickDrumSound = Mix_LoadWAV("assets/audio/kick_drum.wav");
        if (kickDrumSound == NULL) {
            printf("Failed to load WAV file! SDL_mixer Error: %s\n", Mix_GetError());
            Mix_CloseAudio();
            return;
        }

        music = Mix_LoadMUS("assets/audio/neptovian.wav");
        if (music == NULL) {
            printf("Failed to load WAV file! SDL_mixer Error: %s\n", Mix_GetError());
            Mix_CloseAudio();
            return;
        }


    }
    /* private destructor prevents deletion through a pointer to the base class */
    ~AudioBooth() = default;

    Mix_Music* music = NULL;
    Mix_Chunk* kickDrumSound = NULL;
};