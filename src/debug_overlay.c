#include "debug_overlay.h"
#include "traffic_manager.h"
#include "graph.h"
#include "renderer.h"

#include <stdio.h>

void debug_overlay_draw(struct TrafficManager* manager, int screen_width, int screen_height) {
    if(manager == NULL || manager->graph == NULL) {
        return;
    }

    char line[128];
    int x = 20;
    int y = 20;
    int step = 18;

    renderer_draw_text(20, 20, "Transport-Traffic-Manager ver. Beta", screen_width, screen_height);

}