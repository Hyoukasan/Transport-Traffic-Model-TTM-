#include <stdio.h>

#include "debug_overlay.h"
#include "traffic_manager.h"
#include "graph.h"
#include "renderer.h"

void debug_overlay_draw(struct TrafficManager* manager, int screen_width, int screen_height) {
    if(manager == NULL || manager->graph == NULL) {
        return;
    }

    char line[128];
    int x = 20;
    int y = 20;
    int step = 25;

    renderer_draw_text(22, 22, "Transport-Traffic-Manager ver. Beta", 2.0f, 0.0f, 0.0f, 0.0f, screen_width, screen_height);
    renderer_draw_text(20, 20, "Transport-Traffic-Manager ver. Beta", 2.0f, 1.0f, 1.0f, 1.0f, screen_width, screen_height);

    snprintf(line, sizeof(line), "Time: %.1f sec", manager->time);
    renderer_draw_text(x + 2, y + step + 2, line, 2.0f, 0.0f, 0.0f, 0.0f, screen_width, screen_height);
    renderer_draw_text(x, y + step, line, 2.0f, 1.0f, 1.0f, 1.0f, screen_width, screen_height);

    snprintf(line, sizeof(line), "Cars: %d / %d", manager->car_count, manager->max_cars);
    renderer_draw_text(x + 4, y + step*2 + 2, line, 2.0f, 0.0f, 0.0f, 0.0f, screen_width, screen_height);
    renderer_draw_text(x + 2, y + step*2, line, 2.0f, 1.0f, 1.0f, 1.0f, screen_width, screen_height);

    snprintf(line, sizeof(line), "Roads: %d", manager->graph->road_count);
    renderer_draw_text(x + 4, y + step*3 + 2, line, 2.0f, 0.0f, 0.0f, 0.0f, screen_width, screen_height);
    renderer_draw_text(x + 2, y + step*3, line, 2.0f, 1.0f, 1.0f, 1.0f, screen_width, screen_height);

    snprintf(line, sizeof(line), "Intersections: %d", manager->graph->intersection_count);
    renderer_draw_text(x + 4, y + step*4 + 2, line, 2.0f, 0.0f, 0.0f, 0.0f, screen_width, screen_height);
    renderer_draw_text(x + 2, y + step*4, line, 2.0f, 1.0f, 1.0f, 1.0f, screen_width, screen_height);

    snprintf(line, sizeof(line), "Lights: %d", manager->light_count);
    renderer_draw_text(x + 4, y + step*5 + 2, line, 2.0f, 0.0f, 0.0f, 0.0f, screen_width, screen_height);
    renderer_draw_text(x + 2, y + step*5, line, 2.0f, 1.0f, 1.0f, 1.0f, screen_width, screen_height);

    snprintf(line, sizeof(line), "Accidents: %d / %d", manager->accident_count, manager->max_accidents);
    renderer_draw_text(x + 4, y + step*6 + 2, line, 2.0f, 0.0f, 0.0f, 0.0f, screen_width, screen_height);
    renderer_draw_text(x + 2, y + step*6, line, 2.0f, 1.0f, 1.0f, 1.0f, screen_width, screen_height);

    if(manager->selected_road_id != -1 && manager->selected_lane != -1) {
        bool selected_lane_accident_active = traffic_manager_selected_lane_has_accident(manager);

        snprintf(line, sizeof(line), "Selected road: %d", manager->selected_road_id);
        renderer_draw_text(x + 4, y + step*8 + 2, line, 2.0f, 0.0f, 0.0f, 0.0f, screen_width, screen_height);
        renderer_draw_text(x + 2, y + step*8, line, 2.0f, 1.0f, 1.0f, 1.0f, screen_width, screen_height);

        snprintf(line, sizeof(line), "Selected lane: %d", manager->selected_lane);
        renderer_draw_text(x + 4, y + step*9 + 2, line, 2.0f, 0.0f, 0.0f, 0.0f, screen_width, screen_height);
        renderer_draw_text(x + 2, y + step*9, line, 2.0f, 1.0f, 1.0f, 1.0f, screen_width, screen_height);

        RoadSegment *road = &manager->graph->roads[manager->selected_road_id];
        RoadDirection dir = graph_get_lane_direction(road, manager->selected_lane);

        switch (dir)
        {
            case ROAD_DIR_EAST:
                snprintf(line, sizeof(line), "Lane dir: EAST");
                break;
            case ROAD_DIR_WEST:
                snprintf(line, sizeof(line), "Lane dir: WEST");
                break;
            case ROAD_DIR_NORTH:
                snprintf(line, sizeof(line), "Lane dir: NORTH");
                break;
            case ROAD_DIR_SOUTH:
                snprintf(line, sizeof(line), "Lane dir: SOUTH");
                break;
            default:
                snprintf(line, sizeof(line), "Lane dir: NONE");
                break;
        }

        renderer_draw_text(x + 4, y + step*10 + 2, line, 2.0f, 0.0f, 0.0f, 0.0f, screen_width, screen_height);
        renderer_draw_text(x + 2, y + step*10, line, 2.0f, 1.0f, 1.0f, 1.0f, screen_width, screen_height);

        snprintf(line, sizeof(line), "Lane accident: %s", selected_lane_accident_active ? "ACTIVE" : "NONE");
        renderer_draw_text(x + 4, y + step*11 + 2, line, 2.0f, 0.0f, 0.0f, 0.0f, screen_width, screen_height);
        renderer_draw_text(x + 2, y + step*11, line, 2.0f, 1.0f, 1.0f, 1.0f, screen_width, screen_height);
    }
}
