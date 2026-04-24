#include "audio_manager.h"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"


static ma_engine engine;
static ma_sound menu_music;
static ma_sound simulation_music;
static ma_sound click;

static AudioType audio = {0};

int audio_init(void) {
    return ma_engine_init(NULL, &engine) == MA_SUCCESS;
}

void audio_shutdown(void) {
    ma_engine_uninit(&engine);
}


void audio_play_click(void) {

}


void audio_start_menu_music(void) {

}

void audio_start_simulation_music(void) {

}

void audio_stop_music(void) {

}