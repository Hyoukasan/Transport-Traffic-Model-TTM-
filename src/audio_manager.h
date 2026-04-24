#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

typedef struct {
    ma_engine engine;
    ma_sound menu_music;
    ma_sound simulation_music;
    ma_sound click;

    int initialized;
    int menu_music_loaded;
    int simulation_music_loaded;

} AudioManager;

int audio_init(void);
void audio_shutdown(void);

void audio_play_click(void);

void audio_start_menu_music(void);
void audio_start_simulation_music(void);
void audio_stop_music(void);

#endif 