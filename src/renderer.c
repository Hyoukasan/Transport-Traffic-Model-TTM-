#include <GL/glew.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "stb_easy_font.h"

#include "renderer.h"
#include "geometry.h"
#include "menu.h"
#include "graph.h"
#include "car.h"

static GLuint VAO = 0, roadVBO = 0;
static GLuint helperRoadVAO = 0, helperRoadVBO = 0;
static GLuint gridVAO = 0, gridVBO = 0;
static GLuint nodeVAO = 0, nodeVBO = 0;
static int roadMainVertexCount = 0;
static int roadHelperVertexCount = 0;
static int nodeVertexCount = 0;
static GLuint carVAO = 0, carVBO = 0;

static int road_lane_count(const RoadSegment *road) {
    return (road != NULL && road->lanes > 0) ? road->lanes : 1;
}

static int road_start_edge(const RoadSegment *road) {
    int lanes = road_lane_count(road);
    if (road->type == ROAD_HORIZONTAL) {
        if (road->direction == ROAD_DIR_EAST) {
            return road->y1;
        } else if (road->direction == ROAD_DIR_WEST) {
            return road->y1 - (lanes - 1);
        }
        return road->y1 - (lanes / 2);
    }

    if (road->type == ROAD_VERTICAL) {
        if (road->direction == ROAD_DIR_NORTH) {
            return road->x1;
        } else if (road->direction == ROAD_DIR_SOUTH) {
            return road->x1 - (lanes - 1);
        }
        return road->x1 - (lanes / 2);
    }

    return road->x1 - (lanes / 2);
}

static bool is_direction_aligned_with_road(const RoadSegment *road, RoadDirection direction) {
    if (road == NULL) {
        return true;
    }

    if (road->type == ROAD_HORIZONTAL) {
        if (road->x2 >= road->x1) {
            return direction == ROAD_DIR_EAST;
        }
        return direction == ROAD_DIR_WEST;
    }

    if (road->type == ROAD_VERTICAL) {
        if (road->y2 >= road->y1) {
            return direction == ROAD_DIR_SOUTH;
        }
        return direction == ROAD_DIR_NORTH;
    }

    return true;
}

static float position_to_travel_fraction(const RoadSegment *road, RoadDirection direction, float position) {
    if (is_direction_aligned_with_road(road, direction)) {
        return position;
    }
    return 1.0f - position;
}

static int clamp_lane(const RoadSegment *road, int lane) {
    int lanes = road_lane_count(road);
    if (lane < 0) {
        return 0;
    }
    if (lane >= lanes) {
        return lanes - 1;
    }
    return lane;
}

static float normalize_angle(float angle) {
    while (angle > 180.0f) {
        angle -= 360.0f;
    }
    while (angle <= -180.0f) {
        angle += 360.0f;
    }
    return angle;
}

static void road_vertex_counts(const Graph *graph, int *mainCount, int *helperCount) {
    if (graph == NULL || mainCount == NULL || helperCount == NULL) {
        return;
    }

    *mainCount = 0;
    *helperCount = 0;

    for (int i = 0; i < graph->road_count; i++) {
        const RoadSegment *road = &graph->roads[i];
        int lanes = road_lane_count(road);

        if (lanes <= 1) {
            *mainCount += 2;
            continue;
        }

        int start_edge = road_start_edge(road);
        for (int edge = start_edge; edge <= start_edge + lanes; edge++) {
            if (road->type == ROAD_HORIZONTAL) {
                if (edge >= 0 && edge <= graph->grid_height) {
                    *helperCount += 2;
                }
            } else if (road->type == ROAD_VERTICAL) {
                if (edge >= 0 && edge <= graph->grid_width) {
                    *helperCount += 2;
                }
            }
        }

        for (int lane = 0; lane < lanes; lane++) {
            int center = start_edge + lane;
            if (road->type == ROAD_HORIZONTAL) {
                if (center >= 0 && center < graph->grid_height) {
                    *mainCount += 2;
                }
            } else if (road->type == ROAD_VERTICAL) {
                if (center >= 0 && center < graph->grid_width) {
                    *mainCount += 2;
                }
            }
        }
    }
}

void renderer_init(void) {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &roadVBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, roadVBO);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glGenVertexArrays(1, &helperRoadVAO);
    glGenBuffers(1, &helperRoadVBO);

    glBindVertexArray(helperRoadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, helperRoadVBO);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glGenVertexArrays(1, &gridVAO);
    glGenBuffers(1, &gridVBO);
    glBindVertexArray(gridVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glGenVertexArrays(1, &nodeVAO);
    glGenBuffers(1, &nodeVBO);
    glBindVertexArray(nodeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, nodeVBO);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glGenVertexArrays(1, &carVAO);
    glGenBuffers(1, &carVBO);
    glBindVertexArray(carVAO);
    glBindBuffer(GL_ARRAY_BUFFER, carVBO);

    float carVertices[4 * 4] = {
        -0.5f, -0.5f, 0.0f, 0.0f,
         0.5f, -0.5f, 1.0f, 0.0f,
         0.5f,  0.5f, 1.0f, 1.0f,
        -0.5f,  0.5f, 0.0f, 1.0f
    };

    glBufferData(GL_ARRAY_BUFFER, sizeof(carVertices), carVertices, GL_STATIC_DRAW);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(2, GL_FLOAT, 4 * sizeof(float), (void*)0);
    glTexCoordPointer(2, GL_FLOAT, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void renderer_draw_background(unsigned int texture) {
    if(texture == 0) {
        return;
    }

    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex2f(-1.0f, 1.0f);

    glTexCoord2f(1, 0);
    glVertex2f(1.0f, 1.0f);

    glTexCoord2f(1, 1);
    glVertex2f(1.0f, -1.0f);

    glTexCoord2f(0, 1);
    glVertex2f(-1.0f, -1.0f);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
}

void button_render(MenuButton_t* button, int offset_x, int offset_y, 
    int screen_width, int screen_height) {
    if(button == NULL) {
        return;
    }

    int x = offset_x + button->x;
    int y = offset_y + button->y;

    float left   = pixel_to_normalized_x(x, screen_width);
    float right  = pixel_to_normalized_x(x + button->width, screen_width);
    float top    = pixel_to_normalized_y(y, screen_height);
    float bottom = pixel_to_normalized_y(y + button->height, screen_height);

    glBindTexture(GL_TEXTURE_2D, button->texture);

    glBegin(GL_QUADS);

    glTexCoord2f(0, 1);
    glVertex2f(left, bottom);

    glTexCoord2f(1, 1);
    glVertex2f(right, bottom);

    glTexCoord2f(1, 0);
    glVertex2f(right, top);

    glTexCoord2f(0, 0);
    glVertex2f(left, top);

    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
}

void menu_render(Menu_t* menu, int screen_width, int screen_height) {
    if (menu == NULL) {
        return;
    }

    glColor3f(1.0f, 1.0f, 1.0f);
    if(menu->current_state != MENU_STATE_SIMULATION_PAUSE && menu->current_state != MENU_STATE_SIMULATION_CONFIG_PAUSE) {
        renderer_draw_background(menu->background_texture);
    } 

    float left   = pixel_to_normalized_x(menu->x, screen_width);
    float right  = pixel_to_normalized_x(menu->x + menu->width, screen_width);
    float top    = pixel_to_normalized_y(menu->y, screen_height);
    float bottom = pixel_to_normalized_y(menu->y + menu->height, screen_height);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, menu->texture);

    glBegin(GL_QUADS);
    if (menu->current_state == MENU_STATE_SIMULATION_TOOLS) {
        glTexCoord2f(0, 0);
        glVertex2f(left, top);

        glTexCoord2f(1, 0);
        glVertex2f(right, top);

        glTexCoord2f(1, 1);
        glVertex2f(right, bottom);

        glTexCoord2f(0, 1);
        glVertex2f(left, bottom);
    } else {
        glTexCoord2f(0, 1);
        glVertex2f(left, top);

        glTexCoord2f(1, 1);
        glVertex2f(right, top);

        glTexCoord2f(1, 0);
        glVertex2f(right, bottom);

        glTexCoord2f(0, 0);
        glVertex2f(left, bottom);
    }
    glEnd();

    for (int i = 0; i < menu->button_count; i++) {
        glColor3f(1.0f, 1.0f, 1.0f);
        glEnable(GL_TEXTURE_2D);
        button_render(&menu->buttons[i], menu->x, menu->y, screen_width, screen_height);

        if (menu->buttons[i].profile_text[0] != '\0') {
            int raw_text_width = stb_easy_font_width(menu->buttons[i].profile_text);
            float scale = 1.45f;
            float max_text_width = (float)(menu->buttons[i].width - 52);

            if (menu->current_state == MENU_STATE_SIMULATION_CONFIG_SETTING) {
                scale = 2.15f;
                max_text_width = (float)(menu->buttons[i].width - 150);
            }

            if (raw_text_width > 0 && (float)raw_text_width * scale > max_text_width) {
                scale = max_text_width / (float)raw_text_width;
            }

            if (scale < 0.85f) {
                scale = 0.85f;
            }

            float text_width  = (float)raw_text_width * scale;
            float text_height = (float)stb_easy_font_height(menu->buttons[i].profile_text) * scale;

            float button_x = (float)(menu->x + menu->buttons[i].x);
            float button_y = (float)(menu->y + menu->buttons[i].y);
            
            float text_x = button_x + ((float)menu->buttons[i].width - text_width) * 0.5f;
            float text_y = button_y + (float)menu->buttons[i].height - text_height - 10.0f;

            if (menu->current_state == MENU_STATE_SIMULATION_CONFIG_SETTING) {
                text_y = button_y + ((float)menu->buttons[i].height - text_height) * 0.5f + 6.0f;
            }

            renderer_draw_text(text_x + 1.0f, text_y + 1.0f, menu->buttons[i].profile_text, scale,
                0.0f, 0.0f, 0.0f, screen_width, screen_height);
            renderer_draw_text(text_x, text_y, menu->buttons[i].profile_text, scale,
                1.0f, 1.0f, 1.0f, screen_width, screen_height);

            glColor3f(1.0f, 1.0f, 1.0f);
            glEnable(GL_TEXTURE_2D);
        }
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
}

void renderer_upload_graph(Graph *graph) {
    if (graph == NULL) {
        return;
    }

    roadMainVertexCount = 0;
    roadHelperVertexCount = 0;

    if (graph->road_count > 0) {
        road_vertex_counts(graph, &roadMainVertexCount, &roadHelperVertexCount);
        float *mainVertices = roadMainVertexCount > 0 ? malloc(roadMainVertexCount * 2 * sizeof(float)) : NULL;
        float *helperVertices = roadHelperVertexCount > 0 ? malloc(roadHelperVertexCount * 2 * sizeof(float)) : NULL;
        if ((roadMainVertexCount > 0 && mainVertices == NULL) ||
            (roadHelperVertexCount > 0 && helperVertices == NULL)) {
            fprintf(stderr, "renderer_upload_graph: failed to allocate split road vertices buffers\n");
            free(mainVertices);
            free(helperVertices);
            return;
        }

        int mainIndex = 0;
        int helperIndex = 0;
        for (int i = 0; i < graph->road_count; i++) {
            const RoadSegment *road = &graph->roads[i];
            float x1 = grid_edge_to_normalized_x(road->x1, graph->chunk_size, graph->padding, graph->window_width);
            float y1 = grid_edge_to_normalized_y(road->y1, graph->chunk_size, graph->padding, graph->window_height);
            float x2 = grid_edge_to_normalized_x(road->x2, graph->chunk_size, graph->padding, graph->window_width);
            float y2 = grid_edge_to_normalized_y(road->y2, graph->chunk_size, graph->padding, graph->window_height);
            int lanes = road_lane_count(road);

            if (lanes > 1) {
                int start_edge = road_start_edge(road);
                for (int edge = start_edge; edge <= start_edge + lanes; edge++) {
                    if (road->type == ROAD_HORIZONTAL) {
                        if (edge >= 0 && edge <= graph->grid_height) {
                            helperVertices[helperIndex++] = x1;
                            helperVertices[helperIndex++] = grid_edge_to_normalized_y(edge, graph->chunk_size, graph->padding, graph->window_height);
                            helperVertices[helperIndex++] = x2;
                            helperVertices[helperIndex++] = grid_edge_to_normalized_y(edge, graph->chunk_size, graph->padding, graph->window_height);
                        }
                    } else if (road->type == ROAD_VERTICAL) {
                        if (edge >= 0 && edge <= graph->grid_width) {
                            helperVertices[helperIndex++] = grid_edge_to_normalized_x(edge, graph->chunk_size, graph->padding, graph->window_width);
                            helperVertices[helperIndex++] = y1;
                            helperVertices[helperIndex++] = grid_edge_to_normalized_x(edge, graph->chunk_size, graph->padding, graph->window_width);
                            helperVertices[helperIndex++] = y2;
                        }
                    }
                }

                for (int lane = 0; lane < lanes; lane++) {
                    int center = start_edge + lane;
                    if (road->type == ROAD_HORIZONTAL) {
                        if (center >= 0 && center < graph->grid_height) {
                            mainVertices[mainIndex++] = x1;
                            mainVertices[mainIndex++] = grid_center_to_normalized_y(center, graph->chunk_size, graph->padding, graph->window_height);
                            mainVertices[mainIndex++] = x2;
                            mainVertices[mainIndex++] = grid_center_to_normalized_y(center, graph->chunk_size, graph->padding, graph->window_height);
                        }
                    } else if (road->type == ROAD_VERTICAL) {
                        if (center >= 0 && center < graph->grid_width) {
                            mainVertices[mainIndex++] = grid_center_to_normalized_x(center, graph->chunk_size, graph->padding, graph->window_width);
                            mainVertices[mainIndex++] = y1;
                            mainVertices[mainIndex++] = grid_center_to_normalized_x(center, graph->chunk_size, graph->padding, graph->window_width);
                            mainVertices[mainIndex++] = y2;
                        }
                    }
                }
            } else {
                mainVertices[mainIndex++] = grid_center_to_normalized_x(road->x1, graph->chunk_size, graph->padding, graph->window_width);
                mainVertices[mainIndex++] = grid_center_to_normalized_y(road->y1, graph->chunk_size, graph->padding, graph->window_height);
                mainVertices[mainIndex++] = grid_center_to_normalized_x(road->x2, graph->chunk_size, graph->padding, graph->window_width);
                mainVertices[mainIndex++] = grid_center_to_normalized_y(road->y2, graph->chunk_size, graph->padding, graph->window_height);
            }
        }

        glBindVertexArray(VAO);

        if (roadMainVertexCount > 0) {
            glBindBuffer(GL_ARRAY_BUFFER, roadVBO);
            glBufferData(GL_ARRAY_BUFFER, roadMainVertexCount * 2 * sizeof(float), mainVertices, GL_STATIC_DRAW);
        }
        if (roadHelperVertexCount > 0) {
            glBindBuffer(GL_ARRAY_BUFFER, helperRoadVBO);
            glBufferData(GL_ARRAY_BUFFER, roadHelperVertexCount * 2 * sizeof(float), helperVertices, GL_STATIC_DRAW);
        }

        glBindVertexArray(0);
        free(mainVertices);
        free(helperVertices);
    }

    if (graph->intersection_count > 0) {
        nodeVertexCount = graph->intersection_count;
        float *vertices = malloc(nodeVertexCount * 2 * sizeof(float));
        if (vertices == NULL) {
            fprintf(stderr, "renderer_upload_graph: failed to allocate node vertices buffer\n");
            return;
        }

        int index = 0;
        for (int i = 0; i < graph->intersection_count; i++) {
            vertices[index++] = grid_center_to_normalized_x(graph->intersections[i].x, graph->chunk_size, graph->padding, graph->window_width);
            vertices[index++] = grid_center_to_normalized_y(graph->intersections[i].y, graph->chunk_size, graph->padding, graph->window_height);
        }

        glBindVertexArray(nodeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, nodeVBO);
        glBufferData(GL_ARRAY_BUFFER, nodeVertexCount * 2 * sizeof(float), vertices, GL_STATIC_DRAW);
        glBindVertexArray(0);
        free(vertices);
    }
}

static void renderer_draw_road_texture(const Graph *graph, const RoadSegment *road) {
    if (graph == NULL || road == NULL || road->texture == 0) {
        return;
    }

    float left, right, top, bottom;
    int lanes = road_lane_count(road);
    if (road->type == ROAD_HORIZONTAL) {
        int min_x = coord_min(road->x1, road->x2);
        int max_x = coord_max(road->x1, road->x2);

        int start_edge = road_start_edge(road);
        int end_edge = start_edge + lanes;

        left = grid_edge_to_normalized_x(min_x, graph->chunk_size, graph->padding, graph->window_width);
        right = grid_edge_to_normalized_x(max_x + 1, graph->chunk_size, graph->padding, graph->window_width);
        top = grid_edge_to_normalized_y(start_edge, graph->chunk_size, graph->padding, graph->window_height);
        bottom = grid_edge_to_normalized_y(end_edge, graph->chunk_size, graph->padding, graph->window_height);
    } else if (road->type == ROAD_VERTICAL) {
        int min_y = coord_min(road->y1, road->y2);
        int max_y = coord_max(road->y1, road->y2);

        int start_edge = road_start_edge(road);
        int end_edge = start_edge + lanes;

        left = grid_edge_to_normalized_x(start_edge, graph->chunk_size, graph->padding, graph->window_width);
        right = grid_edge_to_normalized_x(end_edge, graph->chunk_size, graph->padding, graph->window_width);
        top = grid_edge_to_normalized_y(min_y, graph->chunk_size, graph->padding, graph->window_height);
        bottom = grid_edge_to_normalized_y(max_y + 1, graph->chunk_size, graph->padding, graph->window_height);
    } else {
        return;
    }

    float repeat_u = 1.0f;
    float repeat_v = 1.0f;
    if (road->type == ROAD_HORIZONTAL) {
        int length_chunks = abs(road->x2 - road->x1) + 1;
        int width_chunks = lanes;
        repeat_u = length_chunks / 2.0f;
        repeat_v = width_chunks / 2.0f;
    } else if (road->type == ROAD_VERTICAL) {
        int length_chunks = abs(road->y2 - road->y1) + 1;
        int width_chunks = lanes;
        repeat_u = length_chunks / 2.0f;
        repeat_v = width_chunks / 2.0f;
    }

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, road->texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);

    if(road->type == ROAD_HORIZONTAL){
        glTexCoord2f(0.0f, 0.0f);
        glVertex2f(left, top);

        glTexCoord2f(repeat_u, 0.0f);
        glVertex2f(right, top);

        glTexCoord2f(repeat_u, repeat_v);
        glVertex2f(right, bottom);

        glTexCoord2f(0.0f, repeat_v);
        glVertex2f(left, bottom);
    } else {
        glTexCoord2f(0.0f, 0.0f);
        glVertex2f(left, top);

        glTexCoord2f(0.0f, repeat_v);
        glVertex2f(right, top);

        glTexCoord2f(repeat_u, repeat_v);
        glVertex2f(right, bottom);

        glTexCoord2f(repeat_u, 0.0f);
        glVertex2f(left, bottom);
    }

    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
}

static void renderer_draw_intersection_blocks(const Graph *graph) {
    if (graph == NULL || graph->intersection_count <= 0) {
        return;
    }

    for (int i = 0; i < graph->intersection_count; i++) {
        const Intersection *intersection = &graph->intersections[i];
        float left = grid_edge_to_normalized_x(intersection->left_edge, graph->chunk_size, graph->padding, graph->window_width);
        float right = grid_edge_to_normalized_x(intersection->right_edge, graph->chunk_size, graph->padding, graph->window_width);
        float top = grid_edge_to_normalized_y(intersection->top_edge, graph->chunk_size, graph->padding, graph->window_height);
        float bottom = grid_edge_to_normalized_y(intersection->bottom_edge, graph->chunk_size, graph->padding, graph->window_height);

        glDisable(GL_TEXTURE_2D);
        glColor3f(0.345f, 0.345f, 0.345f);

        glBegin(GL_QUADS);
        glVertex2f(left, top);
        glVertex2f(right, top);
        glVertex2f(right, bottom);
        glVertex2f(left, bottom);
        glEnd();
    }
}

void renderer_draw_roads(Graph *graph) {
    if (graph == NULL) {
        return;
    }

    for (int i = 0; i < graph->road_count; i++) {
        renderer_draw_road_texture(graph, &graph->roads[i]);
    }

    renderer_draw_intersection_blocks(graph);

    glLineWidth(1.0f);
    glColor3f(1.0f, 1.0f, 1.0f);
}

void renderer_draw_selected_lane(Graph *graph, int road_id, int lane) {
    if (graph == NULL || road_id < 0 || lane < 0) {
        return;
    }

    const RoadSegment *road = NULL;
    for (int i = 0; i < graph->road_count; i++) {
        if (graph->roads[i].id == road_id) {
            road = &graph->roads[i];
            break;
        }
    }

    if (road == NULL) {
        return;
    }

    int lanes = road_lane_count(road);
    if (lane >= lanes) {
        return;
    }

    int start_edge = road_start_edge(road);
    float left;
    float right;
    float top;
    float bottom;

    if (road->type == ROAD_HORIZONTAL) {
        int min_x = coord_min(road->x1, road->x2);
        int max_x = coord_max(road->x1, road->x2);
        int lane_y = start_edge + lane;

        left = grid_edge_to_normalized_x(min_x, graph->chunk_size, graph->padding, graph->window_width);
        right = grid_edge_to_normalized_x(max_x + 1, graph->chunk_size, graph->padding, graph->window_width);
        top = grid_edge_to_normalized_y(lane_y, graph->chunk_size, graph->padding, graph->window_height);
        bottom = grid_edge_to_normalized_y(lane_y + 1, graph->chunk_size, graph->padding, graph->window_height);
    } else if (road->type == ROAD_VERTICAL) {
        int min_y = coord_min(road->y1, road->y2);
        int max_y = coord_max(road->y1, road->y2);
        int lane_x = start_edge + lane;

        left = grid_edge_to_normalized_x(lane_x, graph->chunk_size, graph->padding, graph->window_width);
        right = grid_edge_to_normalized_x(lane_x + 1, graph->chunk_size, graph->padding, graph->window_width);
        top = grid_edge_to_normalized_y(min_y, graph->chunk_size, graph->padding, graph->window_height);
        bottom = grid_edge_to_normalized_y(max_y + 1, graph->chunk_size, graph->padding, graph->window_height);
    } else {
        return;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_TEXTURE_2D);
    glColor4f(1.0f, 1.0f, 1.0f, 0.35f);

    glBegin(GL_QUADS);
    glVertex2f(left, top);
    glVertex2f(right, top);
    glVertex2f(right, bottom);
    glVertex2f(left, bottom);
    glEnd();

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glDisable(GL_BLEND);
}

void renderer_draw_grid(Graph *graph) {
    if (graph == NULL || graph->grid_width <= 0 || graph->grid_height <= 0) {
        return;
    }

    glLineWidth(1.0f);
    glBegin(GL_LINES);

    // vertical lines
    for (int x = 0; x <= graph->grid_width; x++) {
        float nx = grid_edge_to_normalized_x(x, graph->chunk_size, graph->padding, graph->window_width);

        float y1 = grid_edge_to_normalized_y(0, graph->chunk_size, graph->padding, graph->window_height);
        float y2 = grid_edge_to_normalized_y(graph->grid_height, graph->chunk_size, graph->padding, graph->window_height);

        glVertex2f(nx, y1);
        glVertex2f(nx, y2);
    }

    // horizontal lines
    for (int y = 0; y <= graph->grid_height; y++) {
        float ny = grid_edge_to_normalized_y(y, graph->chunk_size, graph->padding, graph->window_height);

        float x1 = grid_edge_to_normalized_x(0, graph->chunk_size, graph->padding, graph->window_width);
        float x2 = grid_edge_to_normalized_x(graph->grid_width, graph->chunk_size, graph->padding, graph->window_width);

        glVertex2f(x1,ny);
        glVertex2f(x2,ny);
    }

    glEnd();
    glLineWidth(1.0f);
}

void renderer_draw_text(float x, float y, char* text, float scale, float r, float g, float b, int screen_width, int screen_height) {
    if(text == NULL || screen_width <= 0 || screen_height <= 0) {
        return;
    }

    char buffer[10000];

    int massage = stb_easy_font_print(0, 0, text, NULL, buffer, sizeof(buffer));

    glDisable(GL_TEXTURE_2D);
    glColor3f(r, g, b);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, screen_width, screen_height, 0.0, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glTranslatef(x, y, 0.0f);
    glScalef(scale, scale, 1.0f);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glEnableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(2, GL_FLOAT, 16, buffer);
    glDrawArrays(GL_QUADS, 0, massage * 4);

    glDisableClientState(GL_VERTEX_ARRAY);

    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);    
}

void renderer_draw_cars(Graph *graph, Car *cars, int car_count) {
    if (graph == NULL || cars == NULL || car_count <= 0) {
        return;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindVertexArray(carVAO);
    glBindBuffer(GL_ARRAY_BUFFER, carVBO);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(2, GL_FLOAT, 4 * sizeof(float), (void*)0);
    glTexCoordPointer(2, GL_FLOAT, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    for (int i = 0; i < car_count; i++) {
        Car *car = &cars[i];
        if (car->road_id < 0 || car->road_id >= graph->road_count) {
            continue;
        }

        RoadSegment *road = &graph->roads[car->road_id];
        int lane = clamp_lane(road, car->lane);
        float lane_center = (float)road_start_edge(road) + lane;

        float car_grid_x;
        float car_grid_y;
        RoadDirection effective_direction = ROAD_DIR_NONE;

        if (road->direction == ROAD_DIR_NONE) {
            int lanes = road->lanes > 0 ? road->lanes : 1;
            int half = lanes / 2;
            if (road->type == ROAD_HORIZONTAL) {
                effective_direction = lane < half ? ROAD_DIR_WEST : ROAD_DIR_EAST;
            } else if (road->type == ROAD_VERTICAL) {
                effective_direction = lane < half ? ROAD_DIR_SOUTH : ROAD_DIR_NORTH;
            }
        } else {
            effective_direction = road->direction;
        }

        if (car->state == CAR_STATE_TURNING) {
            float t = smoothstep(car->turn_progress);
            float path_angle = lerp_angle(car->turn_path_angle_from, car->turn_path_angle_to, t);
            float delta_angle = normalize_angle(car->turn_path_angle_to - car->turn_path_angle_from);
            float tangent_offset = (delta_angle >= 0.0f) ? 90.0f : -90.0f;
            car->angle = normalize_angle(path_angle + tangent_offset);
            float rad = path_angle * (3.14159265f / 180.0f);
            car_grid_x = car->turn_center_x + car->turn_radius * cosf(rad);
            car_grid_y = car->turn_center_y + car->turn_radius * sinf(rad);
        } else if (road->type == ROAD_HORIZONTAL) {
            int min_x = road->x1 < road->x2 ? road->x1 : road->x2;
            int max_x = road->x1 > road->x2 ? road->x1 : road->x2;
            float start_x = (effective_direction == ROAD_DIR_EAST) ? (float)min_x : (float)max_x;
            float start_y = lane_center;
            float travel_fraction = position_to_travel_fraction(road, effective_direction, car->position);
            float s = (float)road->length * travel_fraction;
            float d = car->lane_offset;

            if (effective_direction == ROAD_DIR_WEST || effective_direction == ROAD_DIR_SOUTH) {
                d = -d;
            }

            road_local_to_grid(start_x, start_y, effective_direction, s, d, &car_grid_x, &car_grid_y);
        } else if (road->type == ROAD_VERTICAL) {
            int min_y = road->y1 < road->y2 ? road->y1 : road->y2;
            int max_y = road->y1 > road->y2 ? road->y1 : road->y2;
            float start_y = (effective_direction == ROAD_DIR_SOUTH) ? (float)min_y : (float)max_y;
            float start_x = lane_center;
            float travel_fraction = position_to_travel_fraction(road, effective_direction, car->position);
            float s = (float)road->length * travel_fraction;
            float d = car->lane_offset;

            if (effective_direction == ROAD_DIR_WEST || effective_direction == ROAD_DIR_SOUTH) {
                d = -d;
            }

            road_local_to_grid(start_x, start_y, effective_direction, s, d, &car_grid_x, &car_grid_y);
        } else {
            continue;
        }

        float cx = ((car_grid_x + graph->padding) * graph->chunk_size + graph->chunk_size * 0.5f) * 2.0f / graph->window_width - 1.0f;
        float cy = 1.0f - ((car_grid_y + graph->padding) * graph->chunk_size + graph->chunk_size * 0.5f) * 2.0f / graph->window_height;

        float car_width_px = 28.0f;
        float car_height_px = 44.0f;
        if (road->type == ROAD_HORIZONTAL) {
            car_width_px = 44.0f;
            car_height_px = 28.0f;
        }

        float scale_x = (car_width_px * 2.0f) / (float)graph->window_width;
        float scale_y = (car_height_px * 2.0f) / (float)graph->window_height;

        if (car->texture != 0) {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, car->texture);
        } else {
            glDisable(GL_TEXTURE_2D);
        }

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glTranslatef(cx, cy, 0.0f);
        glRotatef(car->angle, 0.0f, 0.0f, 1.0f);
        glScalef(scale_x, scale_y, 1.0f);
        glColor3f(1.0f, 1.0f, 1.0f);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glPopMatrix();

        glBindTexture(GL_TEXTURE_2D, 0);
    }

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
}

static unsigned int renderer_light_texture(unsigned int light_textures[2], LightState state) {
    if (state == LIGHT_GREEN) {
        return light_textures[LIGHT_GREEN];
    }
    return light_textures[LIGHT_RED];
}

static const Intersection *renderer_find_intersection(const Graph *graph, const TrafficLight *light) {
    for (int i = 0; i < graph->intersection_count; i++) {
        const Intersection *intersection = &graph->intersections[i];
        if (intersection->x == light->intersection_x && intersection->y == light->intersection_y) {
            return intersection;
        }
    }

    return NULL;
}

static float grid_point_to_normalized_x(float grid_x, const Graph *graph) {
    float pixel = (grid_x + graph->padding) * (float)graph->chunk_size;
    return (2.0f * pixel / (float)graph->window_width) - 1.0f;
}

static float grid_point_to_normalized_y(float grid_y, const Graph *graph) {
    float pixel = (grid_y + graph->padding) * (float)graph->chunk_size;
    return 1.0f - (2.0f * pixel / (float)graph->window_height);
}

static void renderer_draw_colored_grid_quad(const Graph *graph, float center_x, float center_y,
                                            float width, float height, float r, float g, float b) {
                                                
    float left = grid_point_to_normalized_x(center_x - width * 0.5f, graph);
    float right = grid_point_to_normalized_x(center_x + width * 0.5f, graph);
    float top = grid_point_to_normalized_y(center_y - height * 0.5f, graph);
    float bottom = grid_point_to_normalized_y(center_y + height * 0.5f, graph);

    glDisable(GL_TEXTURE_2D);
    glColor3f(r, g, b);

    glBegin(GL_QUADS);
    glVertex2f(left, top);
    glVertex2f(right, top);
    glVertex2f(right, bottom);
    glVertex2f(left, bottom);
    glEnd();
}

static void renderer_draw_light_sprite(const Graph *graph, float x, float y, float width, float height, float angle, unsigned int texture) {
    if (texture == 0) {
        return;
    }

    float draw_width = width;
    float draw_height = height;
    float tex_x0 = 0.0f;
    float tex_y0 = 0.0f;
    float tex_x1 = 1.0f;
    float tex_y1 = 0.0f;
    float tex_x2 = 1.0f;
    float tex_y2 = 1.0f;
    float tex_x3 = 0.0f;
    float tex_y3 = 1.0f;

    if (angle == 90.0f || angle == -90.0f) {
        draw_width = height;
        draw_height = width;
    }

    if (angle == 90.0f) {
        tex_x0 = 1.0f; tex_y0 = 0.0f;
        tex_x1 = 1.0f; tex_y1 = 1.0f;
        tex_x2 = 0.0f; tex_y2 = 1.0f;
        tex_x3 = 0.0f; tex_y3 = 0.0f;
    } else if (angle == -90.0f) {
        tex_x0 = 0.0f; tex_y0 = 1.0f;
        tex_x1 = 0.0f; tex_y1 = 0.0f;
        tex_x2 = 1.0f; tex_y2 = 0.0f;
        tex_x3 = 1.0f; tex_y3 = 1.0f;
    } else if (angle == 180.0f) {
        tex_x0 = 1.0f; tex_y0 = 1.0f;
        tex_x1 = 0.0f; tex_y1 = 1.0f;
        tex_x2 = 0.0f; tex_y2 = 0.0f;
        tex_x3 = 1.0f; tex_y3 = 0.0f;
    }

    float left = grid_point_to_normalized_x(x - draw_width * 0.5f, graph);
    float right = grid_point_to_normalized_x(x + draw_width * 0.5f, graph);
    float top = grid_point_to_normalized_y(y - draw_height * 0.5f, graph);
    float bottom = grid_point_to_normalized_y(y + draw_height * 0.5f, graph);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);
    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);
    glTexCoord2f(tex_x0, tex_y0);
    glVertex2f(left, top);

    glTexCoord2f(tex_x1, tex_y1);
    glVertex2f(right, top);

    glTexCoord2f(tex_x2, tex_y2);
    glVertex2f(right, bottom);

    glTexCoord2f(tex_x3, tex_y3);
    glVertex2f(left, bottom);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
}

void renderer_draw_traffic_lights(Graph *graph, TrafficLight *lights, int light_count, unsigned int light_textures[2]) {
    if (graph == NULL || lights == NULL || light_count <= 0 || light_textures == NULL) {
        return;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for (int i = 0; i < light_count; i++) {
        TrafficLight *light = &lights[i];
        const Intersection *intersection = renderer_find_intersection(graph, light);
        if (intersection == NULL) {
            continue;
        }

        unsigned int horizontal_texture = renderer_light_texture(light_textures, light->horizontal_state_light);
        unsigned int vertical_texture = renderer_light_texture(light_textures, light->vertical_state_light);
        float center_x = ((float)intersection->left_edge + (float)intersection->right_edge) * 0.5f;
        float center_y = ((float)intersection->top_edge + (float)intersection->bottom_edge) * 0.5f;
        float block_size = 1.0f;
        float light_height = 1.6f;
        float light_width = light_height * (98.0f / 235.0f);
        float horizontal_offset = block_size * 0.5f + light_height * 0.5f;
        float vertical_offset = block_size * 0.5f + light_height * 0.5f;

        renderer_draw_light_sprite(graph, center_x - horizontal_offset, center_y,
                                   light_width, light_height, 90.0f, horizontal_texture);
        renderer_draw_light_sprite(graph, center_x + horizontal_offset, center_y,
                                   light_width, light_height, -90.0f, horizontal_texture);
        renderer_draw_light_sprite(graph, center_x, center_y - vertical_offset,
                                   light_width, light_height, 0.0f, vertical_texture);
        renderer_draw_light_sprite(graph, center_x, center_y + vertical_offset,
                                   light_width, light_height, 180.0f, vertical_texture);
        renderer_draw_colored_grid_quad(graph, center_x, center_y, block_size, block_size, 0.02f, 0.02f, 0.02f);
    }

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
}

void renderer_shutdown(void) {
    if (carVAO != 0) {
        glDeleteVertexArrays(1, &carVAO);
        glDeleteBuffers(1, &carVBO);
    }
    if (nodeVAO != 0) {
        glDeleteVertexArrays(1, &nodeVAO);
        glDeleteBuffers(1, &nodeVBO);
    }
    if (gridVAO != 0) {
        glDeleteVertexArrays(1, &gridVAO);
        glDeleteBuffers(1, &gridVBO);
    }
    if (helperRoadVAO != 0) {
        glDeleteVertexArrays(1, &helperRoadVAO);
        glDeleteBuffers(1, &helperRoadVBO);
    }
    if (VAO != 0) {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &roadVBO);
    }
}
