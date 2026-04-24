#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

int audio_init(void);
void audio_shutdown(void);

void audio_play_click(void);

void audio_start_menu_music(void);
void audio_start_simulation_music(void);
void audio_stop_music(void);

#endif 