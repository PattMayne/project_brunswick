
#include <SDL.h>
#include <iostream>
#include <vector>
#include <SDL_mixer.h>
export module Audio;

using namespace std;


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

    void freeAllAudio();

    /* TO DO: This should take a screenType parameter so we can play appropriate music. */
    void playMusic() {
        vector<Mix_Music*> musicFiles = { musicForMap , musicForMenu };
        Mix_Music* music = musicFiles[rand() % musicFiles.size()];

        if (Mix_PlayMusic(music, -1) == -1) {
            printf("Failed to play music! SDL_mixer Error: %s\n", Mix_GetError());
        }
    }

    void playKickDrum() {
        Mix_PlayChannel(-1, kickDrumSound, 0);
    }

    void playClick() {
        Mix_PlayChannel(-1, mouseClick, 0);
    }

    void playClickConnect() {
        Mix_PlayChannel(-1, clickConnect, 0);
    }

    void playCaw() {
        Mix_PlayChannel(-1, birdCaw, 0);
    }

    void playBirdUp() {
        Mix_PlayChannel(-1, birdUp, 0);
    }

    void playBirdDown() {
        Mix_PlayChannel(-1, birdDown, 0);
    }

    void playBird() {
        if (rand() % 1 == 0) {
            playBirdUp();
        }
        else {
            playBirdDown();
        }
    }

    void playAttack() {
        vector<Mix_Chunk*> attacks = {
            attack1, attack2, attack3
        };

        int attackIndex = rand() % attacks.size();
        Mix_Chunk* attack = attacks[attackIndex];

        Mix_PlayChannel(-1, attack, 0);
    }

    void playChorus() {
        vector<Mix_Chunk*> choruses = {
            chorus1, chorus2, chorus3
        };

        int chorusIndex = rand() % choruses.size();
        Mix_Chunk* chorus = choruses[chorusIndex];

        Mix_PlayChannel(-1, chorus, 0);
    }

    void playPickupSound() {
        vector<Mix_Chunk*> pickupSounds = {
            pickupSound1, pickupSound2, pickupSound3
        };

        int soundIndex = rand() % pickupSounds.size();
        Mix_Chunk* sound = pickupSounds[soundIndex];

        Mix_PlayChannel(-1, sound, 0);
    }

    void playNpcWalk() {
        Mix_PlayChannel(-1, walkNpcSound, 0);
    }

    void playPlayerWalk() {
        Mix_PlayChannel(-1, walkPlayerSound, 0);
    }

    void playSwoop() {
        Mix_PlayChannel(-1, swoopSound, 0);
    }

    void playBrainDrain() {
        Mix_PlayChannel(-1, brainDrainSound, 0);
    }

    void stopMusic() {
        Mix_HaltMusic();
    }


private:
    /* Constructor is private to prevent outside instantiation */
    AudioBooth();
    /* private destructor prevents deletion through a pointer to the base class */
    ~AudioBooth() = default;

    Mix_Music* musicForMenu = NULL;
    Mix_Music* musicForMap = NULL;

    Mix_Chunk* kickDrumSound = NULL;
    Mix_Chunk* mouseClick = NULL;
    Mix_Chunk* clickConnect = NULL;
    Mix_Chunk* brainDrainSound = NULL;
    Mix_Chunk* swoopSound = NULL;
    Mix_Chunk* walkPlayerSound = NULL;
    Mix_Chunk* walkNpcSound = NULL;
    Mix_Chunk* attack1 = NULL;
    Mix_Chunk* attack2 = NULL;
    Mix_Chunk* attack3 = NULL;
    Mix_Chunk* chorus1 = NULL;
    Mix_Chunk* chorus2 = NULL;
    Mix_Chunk* chorus3 = NULL;
    Mix_Chunk* pickupSound1 = NULL;
    Mix_Chunk* pickupSound2 = NULL;
    Mix_Chunk* pickupSound3 = NULL;
    Mix_Chunk* birdUp = NULL;
    Mix_Chunk* birdDown = NULL;
    Mix_Chunk* birdCaw = NULL;

};

AudioBooth::AudioBooth() {
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

    /* LOAD MUSIC FILES */

    musicForMenu = Mix_LoadMUS("assets/audio/music_flutes_1.wav");
    if (musicForMenu == NULL) {
        printf("Failed to load WAV file! SDL_mixer Error: %s\n", Mix_GetError());
        Mix_CloseAudio();
        return;
    }

    /* POSSIBLY DELETE. Do we need music in the map? */
    musicForMap = Mix_LoadMUS("assets/audio/music_flutes_1.wav");
    if (musicForMap == NULL) {
        printf("Failed to load WAV file! SDL_mixer Error: %s\n", Mix_GetError());
        Mix_CloseAudio();
        return;
    }


    /* Load FX files. */

    kickDrumSound = Mix_LoadWAV("assets/audio/kick_drum.wav");
    if (kickDrumSound == NULL) {
        printf("Failed to load WAV file! SDL_mixer Error: %s\n", Mix_GetError());
        Mix_CloseAudio();
        return;
    }

    birdUp = Mix_LoadWAV("assets/audio/bird_up.wav");
    if (birdUp == NULL) {
        printf("Failed to load WAV file! SDL_mixer Error: %s\n", Mix_GetError());
        Mix_CloseAudio();
        return;
    }

    birdDown = Mix_LoadWAV("assets/audio/bird_down.wav");
    if (birdDown == NULL) {
        printf("Failed to load WAV file! SDL_mixer Error: %s\n", Mix_GetError());
        Mix_CloseAudio();
        return;
    }

    birdCaw = Mix_LoadWAV("assets/audio/bird_caw.wav");
    if (birdCaw == NULL) {
        printf("Failed to load WAV file! SDL_mixer Error: %s\n", Mix_GetError());
        Mix_CloseAudio();
        return;
    }

    clickConnect = Mix_LoadWAV("assets/audio/click_connect.wav");
    if (clickConnect == NULL) {
        printf("Failed to load WAV file! SDL_mixer Error: %s\n", Mix_GetError());
        Mix_CloseAudio();
        return;
    }

    mouseClick = Mix_LoadWAV("assets/audio/click_snap.wav");
    if (mouseClick == NULL) {
        printf("Failed to load WAV file! SDL_mixer Error: %s\n", Mix_GetError());
        Mix_CloseAudio();
        return;
    }

    attack1 = Mix_LoadWAV("assets/audio/attack_1.wav");
    if (attack1 == NULL) {
        printf("Failed to load WAV file! SDL_mixer Error: %s\n", Mix_GetError());
        Mix_CloseAudio();
        return;
    }

    attack2 = Mix_LoadWAV("assets/audio/attack_2.wav");
    if (attack2 == NULL) {
        printf("Failed to load WAV file! SDL_mixer Error: %s\n", Mix_GetError());
        Mix_CloseAudio();
        return;
    }

    attack3 = Mix_LoadWAV("assets/audio/attack_3.wav");
    if (attack3 == NULL) {
        printf("Failed to load WAV file! SDL_mixer Error: %s\n", Mix_GetError());
        Mix_CloseAudio();
        return;
    }

    chorus1 = Mix_LoadWAV("assets/audio/chorus_1.wav");
    if (chorus1 == NULL) {
        printf("Failed to load WAV file! SDL_mixer Error: %s\n", Mix_GetError());
        Mix_CloseAudio();
        return;
    }

    chorus2 = Mix_LoadWAV("assets/audio/chorus_2.wav");
    if (chorus2 == NULL) {
        printf("Failed to load WAV file! SDL_mixer Error: %s\n", Mix_GetError());
        Mix_CloseAudio();
        return;
    }

    chorus3 = Mix_LoadWAV("assets/audio/chorus_3.wav");
    if (chorus3 == NULL) {
        printf("Failed to load WAV file! SDL_mixer Error: %s\n", Mix_GetError());
        Mix_CloseAudio();
        return;
    }


    pickupSound1 = Mix_LoadWAV("assets/audio/flute_roll_1.wav");
    if (pickupSound1 == NULL) {
        printf("Failed to load WAV file! SDL_mixer Error: %s\n", Mix_GetError());
        Mix_CloseAudio();
        return;
    }

    pickupSound2 = Mix_LoadWAV("assets/audio/flute_roll_2.wav");
    if (pickupSound2 == NULL) {
        printf("Failed to load WAV file! SDL_mixer Error: %s\n", Mix_GetError());
        Mix_CloseAudio();
        return;
    }

    pickupSound3 = Mix_LoadWAV("assets/audio/flute_roll_3.wav");
    if (pickupSound3 == NULL) {
        printf("Failed to load WAV file! SDL_mixer Error: %s\n", Mix_GetError());
        Mix_CloseAudio();
        return;
    }

    brainDrainSound = Mix_LoadWAV("assets/audio/brain_drain.wav");
    if (brainDrainSound == NULL) {
        printf("Failed to load WAV file! SDL_mixer Error: %s\n", Mix_GetError());
        Mix_CloseAudio();
        return;
    }


    swoopSound = Mix_LoadWAV("assets/audio/swoop.wav");
    if (swoopSound == NULL) {
        printf("Failed to load WAV file! SDL_mixer Error: %s\n", Mix_GetError());
        Mix_CloseAudio();
        return;
    }


    walkPlayerSound = Mix_LoadWAV("assets/audio/walk_player.wav");
    if (walkPlayerSound == NULL) {
        printf("Failed to load WAV file! SDL_mixer Error: %s\n", Mix_GetError());
        Mix_CloseAudio();
        return;
    }


    walkNpcSound = Mix_LoadWAV("assets/audio/walk_npc.wav");
    if (walkNpcSound == NULL) {
        printf("Failed to load WAV file! SDL_mixer Error: %s\n", Mix_GetError());
        Mix_CloseAudio();
        return;
    }


    /* Set the volume. */

    int volume = 52; /* range: 0 to 128. */
    Mix_VolumeMusic(volume);
    Mix_Volume(-1, volume);
}

void AudioBooth::freeAllAudio() {

    if (musicForMenu != NULL) {
        Mix_FreeMusic(musicForMenu);
    }

    if (musicForMap != NULL) {
        Mix_FreeMusic(musicForMap);
    }

    if (kickDrumSound != NULL) {
        Mix_FreeChunk(kickDrumSound);
    }

    if (chorus1 != NULL) {
        Mix_FreeChunk(chorus1);
    }

    if (chorus2 != NULL) {
        Mix_FreeChunk(chorus2);
    }

    if (chorus3 != NULL) {
        Mix_FreeChunk(chorus3);
    }

    if (pickupSound1 != NULL) {
        Mix_FreeChunk(pickupSound1);
    }

    if (pickupSound2 != NULL) {
        Mix_FreeChunk(pickupSound2);
    }

    if (pickupSound3 != NULL) {
        Mix_FreeChunk(pickupSound3);
    }

    if (brainDrainSound != NULL) {
        Mix_FreeChunk(brainDrainSound);
    }

    if (swoopSound != NULL) {
        Mix_FreeChunk(swoopSound);
    }

    if (walkPlayerSound != NULL) {
        Mix_FreeChunk(walkPlayerSound);
    }

    if (walkNpcSound != NULL) {
        Mix_FreeChunk(walkNpcSound);
    }

    if (attack1 != NULL) {
        Mix_FreeChunk(attack1);
    }

    if (attack2 != NULL) {
        Mix_FreeChunk(attack2);
    }

    if (attack3 != NULL) {
        Mix_FreeChunk(attack3);
    }

    if (pickupSound1 != NULL) {
        Mix_FreeChunk(pickupSound1);
    }

    if (pickupSound2 != NULL) {
        Mix_FreeChunk(pickupSound2);
    }

    if (pickupSound3 != NULL) {
        Mix_FreeChunk(pickupSound3);
    }


    if (clickConnect != NULL) {
        Mix_FreeChunk(clickConnect);
    }

}