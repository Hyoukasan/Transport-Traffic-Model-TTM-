#include "audio_manager.h"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"


static AudioManager audio = {0};

int audio_init(void) {
    return ma_engine_init(NULL, &audio.engine) == MA_SUCCESS;
}

void audio_shutdown(void) {
    ma_engine_uninit(&audio.engine);
}


void audio_play_click(void) {

    if(audio.initialized != 0) {
        if(ma_sound_init_from_file(&audio.engine, "Data/audio/click.mp3", 
            0, NULL, NULL, &audio) != MA_SUCCESS) {
            return;
        }

        ma_sound_set_looping(&audio.click, MA_TRUE);
        ma_sound_set_volume(&audio.click, 0.6f);
    }

    ma_sound_start(&audio.click);
}

void audio_start_menu_music(void) {

    if(audio.initialized != 0) {
        if(ma_sound_init_from_file(&audio.engine, "Data/audio/menu.mp3", 
            MA_SOUND_FLAG_STREAM, NULL, NULL, &audio.menu_music) != MA_SUCCESS) {
            return;
        }

        ma_sound_set_looping(audio.menu_music, MA_TRUE);
        ma_sound_set_volume(audio.menu_music, 0.4f);
    }

    ma_sound_start(audio.menu_music);
}

void audio_start_simulation_music(void) {
    if(audio_init() == 0) {
        return;
    }

}

void audio_stop_music(void) {
    if(audio.menu_music_loaded != 0) {

    }

    if(audio.simulation_music_loaded != 0) {

    }
}