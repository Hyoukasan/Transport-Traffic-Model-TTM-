#include "audio_manager.h"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include <stdio.h>

static ma_engine engine;
static ma_sound menu_music;
static ma_sound simulation_music;
static ma_sound click;

static AudioManager audio = {0};

int audio_init(void) {
    audio.engine = &engine;
    audio.menu_music = &menu_music;
    audio.simulation_music = &simulation_music;
    audio.click = &click;

    if (ma_engine_init(NULL, audio.engine) != MA_SUCCESS) {
        fprintf(stderr, "Failed initialization audio_engine!\n");
        audio.initialized = 0;
        return 0;
    }

    audio.initialized = 1;
    return 1;
}

void audio_shutdown(void) {
    if (audio.menu_music_loaded != 0) {
        ma_sound_uninit(audio.menu_music);
        audio.menu_music_loaded = 0;
    }

    if (audio.simulation_music_loaded != 0) {
        ma_sound_uninit(audio.simulation_music);
        audio.simulation_music_loaded = 0;
    }

    if (audio.initialized != 0) {
        ma_engine_uninit(audio.engine);
        audio.initialized = 0;
    }
}


void audio_play_click(void) {

    if(audio.initialized == 0) {
        return;
    }

    if(ma_sound_init_from_file(audio.engine, "Data/audio/click.mp3", 
            0, NULL, NULL, audio.click) != MA_SUCCESS) {
            return;
    }

    ma_sound_set_looping(audio.click, MA_FALSE);
    ma_sound_set_volume(audio.click, 0.6f);

    ma_sound_start(audio.click);
    ma_sound_uninit(audio.click);
}

void audio_start_menu_music(void) {

    if(audio.initialized == 0) {
        return;
    }

    if(audio.menu_music_loaded == 0) {
        if(ma_sound_init_from_file(audio.engine, "Data/audio/menu.mp3", 
            MA_SOUND_FLAG_STREAM, NULL, NULL, audio.menu_music) != MA_SUCCESS) {
            return;
        }

        ma_sound_set_looping(audio.menu_music, MA_TRUE);
        ma_sound_set_volume(audio.menu_music, 0.4f);
        audio.menu_music_loaded = 1;
    }

    ma_sound_start(audio.menu_music);
}

void audio_start_simulation_music(void) {
    if(audio.initialized == 0) {
        return;
    }

}

void audio_stop_music(void) {
    if (audio.menu_music_loaded != 0) {
        ma_sound_stop(audio.menu_music);
    }

    if (audio.simulation_music_loaded != 0) {
        ma_sound_stop(audio.simulation_music);
    }
}
