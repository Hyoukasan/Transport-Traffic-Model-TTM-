#include "renderer.h"
#include "stb_easy_font.h"
#include "geometry.h"
#include "menu.h"
#include "graph.h"
#include "car.h"

#include <GL/glew.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

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
        return road->y1 - (lanes / 2);
    }
    return road->x1 - (lanes / 2);
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

// static void add_road_vertices(float *vertices, int *index, const Graph *graph, const RoadSegment *road) {
//     float x1 = grid_edge_to_normalized_x(road->x1, graph->chunk_size, graph->padding, graph->window_width);
//     float y1 = grid_edge_to_normalized_y(road->y1, graph->chunk_size, graph->padding, graph->window_height);
//     float x2 = grid_edge_to_normalized_x(road->x2, graph->chunk_size, graph->padding, graph->window_width);
//     float y2 = grid_edge_to_normalized_y(road->y2, graph->chunk_size, graph->padding, graph->window_height);

//     vertices[(*index)++] = x1;
//     vertices[(*index)++] = y1;
//     vertices[(*index)++] = x2;
//     vertices[(*index)++] = y2;

//     if (road->lanes > 1) {
//         if (road->type == ROAD_HORIZONTAL) {
//             if (road->y1 - 1 >= 0) {
//                 float offset_y = grid_center_to_normalized_y(road->y1 - 1, graph->chunk_size, graph->padding, graph->window_height);
//                 vertices[(*index)++] = x1;
//                 vertices[(*index)++] = offset_y;
//                 vertices[(*index)++] = x2;
//                 vertices[(*index)++] = offset_y;
//             }
//             if (road->y1 + 1 <= graph->grid_height) {
//                 float offset_y = grid_center_to_normalized_y(road->y1, graph->chunk_size, graph->padding, graph->window_height);
//                 vertices[(*index)++] = x1;
//                 vertices[(*index)++] = offset_y;
//                 vertices[(*index)++] = x2;
//                 vertices[(*index)++] = offset_y;
//             }
//         } else if (road->type == ROAD_VERTICAL) {
//             if (road->x1 - 1 >= 0) {
//                 float offset_x = grid_center_to_normalized_x(road->x1 - 1, graph->chunk_size, graph->padding, graph->window_width);
//                 vertices[(*index)++] = offset_x;
//                 vertices[(*index)++] = y1;
//                 vertices[(*index)++] = offset_x;
//                 vertices[(*index)++] = y2;
//             }
//             if (road->x1 + 1 <= graph->grid_width) {
//                 float offset_x = grid_center_to_normalized_x(road->x1, graph->chunk_size, graph->padding, graph->window_width);
//                 vertices[(*index)++] = offset_x;
//                 vertices[(*index)++] = y1;
//                 vertices[(*index)++] = offset_x;
//                 vertices[(*index)++] = y2;
//             }
//         }
//     }
// }

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

    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        printf("OpenGL error: %d\n", err);
    }
}

void menu_render(Menu_t* menu, int screen_width, int screen_height) {
    if (menu == NULL) {
        return;
    }

    glColor3f(1.0f, 1.0f, 1.0f);
    if(menu->current_state != MENU_STATE_SIMULATION_PAUSE) {
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
    glTexCoord2f(0, 1);
    glVertex2f(left, top);

    glTexCoord2f(1, 1);
    glVertex2f(right, top);

    glTexCoord2f(1, 0);
    glVertex2f(right, bottom);

    glTexCoord2f(0, 0);
    glVertex2f(left, bottom);
    glEnd();

    for (int i = 0; i < menu->button_count; i++) {
        button_render(&menu->buttons[i], menu->x, menu->y, screen_width, screen_height);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);

    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        printf("OpenGL error: %d\n", err);
    }
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
        int start_edge = road_start_edge(road);
        int end_edge = start_edge + lanes;
        left = grid_edge_to_normalized_x(road->x1, graph->chunk_size, graph->padding, graph->window_width);
        right = grid_edge_to_normalized_x(road->x2 + 1, graph->chunk_size, graph->padding, graph->window_width);
        top = grid_edge_to_normalized_y(start_edge, graph->chunk_size, graph->padding, graph->window_height);
        bottom = grid_edge_to_normalized_y(end_edge, graph->chunk_size, graph->padding, graph->window_height);
    } else if (road->type == ROAD_VERTICAL) {
        int start_edge = road_start_edge(road);
        int end_edge = start_edge + lanes;
        left = grid_edge_to_normalized_x(start_edge, graph->chunk_size, graph->padding, graph->window_width);
        right = grid_edge_to_normalized_x(end_edge, graph->chunk_size, graph->padding, graph->window_width);
        top = grid_edge_to_normalized_y(road->y1, graph->chunk_size, graph->padding, graph->window_height);
        bottom = grid_edge_to_normalized_y(road->y2 + 1, graph->chunk_size, graph->padding, graph->window_height);
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
        repeat_u = width_chunks / 2.0f;
        repeat_v = length_chunks / 2.0f;
    }

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, road->texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(left, top);

    glTexCoord2f(repeat_u, 0.0f);
    glVertex2f(right, top);

    glTexCoord2f(repeat_u, repeat_v);
    glVertex2f(right, bottom);

    glTexCoord2f(0.0f, repeat_v);
    glVertex2f(left, bottom);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
}

void renderer_draw_roads(Graph *graph) {
    if (graph == NULL) {
        return;
    }

    for (int i = 0; i < graph->road_count; i++) {
        renderer_draw_road_texture(graph, &graph->roads[i]);
    }

    if (roadMainVertexCount > 0) {
        glBindVertexArray(VAO);
        glLineWidth(2.5f);
        glColor3f(0.85f, 0.15f, 0.15f);
        glDrawArrays(GL_LINES, 0, roadMainVertexCount);
        glBindVertexArray(0);
    }

    if (roadHelperVertexCount > 0) {
        glBindVertexArray(helperRoadVAO);
        glLineWidth(1.0f);
        glColor3f(0.95f, 0.85f, 0.2f);
        glDrawArrays(GL_LINES, 0, roadHelperVertexCount);
        glBindVertexArray(0);
    }
    glLineWidth(1.0f);
    glColor3f(1.0f, 1.0f, 1.0f);
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
        int lane_center = road_start_edge(road) + lane;
        float x1;
        float y1;
        float x2;
        float y2;

        if (road->type == ROAD_HORIZONTAL) {
            x1 = grid_center_to_normalized_x(road->x1, graph->chunk_size, graph->padding, graph->window_width);
            y1 = grid_center_to_normalized_y(lane_center, graph->chunk_size, graph->padding, graph->window_height);
            x2 = grid_center_to_normalized_x(road->x2, graph->chunk_size, graph->padding, graph->window_width);
            y2 = y1;
        } else if (road->type == ROAD_VERTICAL) {
            x1 = grid_center_to_normalized_x(lane_center, graph->chunk_size, graph->padding, graph->window_width);
            y1 = grid_center_to_normalized_y(road->y1, graph->chunk_size, graph->padding, graph->window_height);
            x2 = x1;
            y2 = grid_center_to_normalized_y(road->y2, graph->chunk_size, graph->padding, graph->window_height);
        } else {
            continue;
        }

        float cx = x1 + (x2 - x1) * car->position;
        float cy = y1 + (y2 - y1) * car->position;

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

void renderer_draw_nodes(Graph *graph) {
    (void)graph;
    if (nodeVertexCount == 0) {
        return;
    }

    glPointSize(10.0f);
    glBindVertexArray(nodeVAO);
    glDrawArrays(GL_POINTS, 0, nodeVertexCount);
    glBindVertexArray(0);
    glPointSize(1.0f);
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
