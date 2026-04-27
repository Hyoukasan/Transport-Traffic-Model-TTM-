#include "renderer.h"
#include "geometry.h"
#include "menu.h"
#include "graph.h"
#include "car.h"

#include <GL/glew.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

static GLuint VAO = 0, VBO = 0;
static GLuint gridVAO = 0, gridVBO = 0;
static GLuint nodeVAO = 0, nodeVBO = 0;
static int roadVertexCount = 0;
static int nodeVertexCount = 0;
static GLuint carVAO = 0, carVBO = 0;

void renderer_init(void) {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
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

    renderer_draw_background(menu->background_texture);

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

    if (graph->road_count > 0) {
        roadVertexCount = graph->road_count * 2;
        float *vertices = malloc(roadVertexCount * 2 * sizeof(float));
        if (vertices == NULL) {
            fprintf(stderr, "renderer_upload_graph: failed to allocate road vertices buffer\n");
            return;
        }

        for (int i = 0; i < graph->road_count; i++) {
            RoadSegment *road = &graph->roads[i];
            float x1 = grid_center_to_normalized_x(road->x1, graph->chunk_size, graph->padding, graph->window_width);
            float y1 = grid_center_to_normalized_y(road->y1, graph->chunk_size, graph->padding, graph->window_height);
            float x2 = grid_center_to_normalized_x(road->x2, graph->chunk_size, graph->padding, graph->window_width);
            float y2 = grid_center_to_normalized_y(road->y2, graph->chunk_size, graph->padding, graph->window_height);
            vertices[i * 4 + 0] = x1;
            vertices[i * 4 + 1] = y1;
            vertices[i * 4 + 2] = x2;
            vertices[i * 4 + 3] = y2;
        }

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, roadVertexCount * 2 * sizeof(float), vertices, GL_STATIC_DRAW);
        glBindVertexArray(0);
        free(vertices);
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

void renderer_draw_roads(Graph *graph) {
    (void)graph;
    if (roadVertexCount == 0) {
        return;
    }

    glBindVertexArray(VAO);
    glDrawArrays(GL_LINES, 0, roadVertexCount);
    glBindVertexArray(0);
}

void renderer_draw_grid(Graph *graph) {
    if (graph == NULL || graph->grid_width <= 0 || graph->grid_height <= 0) {
        return;
    }

    int total_lines = graph->grid_width + graph->grid_height + 2;
    float *vertices = malloc(total_lines * 4 * sizeof(float));
    if (vertices == NULL) {
        fprintf(stderr, "renderer_draw_grid: failed to allocate vertices buffer\n");
        return;
    }

    int index = 0;
    // vertical lines
    for (int x = 0; x <= graph->grid_width; x++) {
        vertices[index++] = grid_edge_to_normalized_x(x, graph->chunk_size, graph->padding, graph->window_width);
        vertices[index++] = grid_edge_to_normalized_y(0, graph->chunk_size, graph->padding, graph->window_height);
        vertices[index++] = grid_edge_to_normalized_x(x, graph->chunk_size, graph->padding, graph->window_width);
        vertices[index++] = grid_edge_to_normalized_y(graph->grid_height, graph->chunk_size, graph->padding, graph->window_height);
    }

    // horizontal lines
    for (int y = 0; y <= graph->grid_height; y++) {
        vertices[index++] = grid_edge_to_normalized_x(0, graph->chunk_size, graph->padding, graph->window_width);
        vertices[index++] = grid_edge_to_normalized_y(y, graph->chunk_size, graph->padding, graph->window_height);
        vertices[index++] = grid_edge_to_normalized_x(graph->grid_width, graph->chunk_size, graph->padding, graph->window_width);
        vertices[index++] = grid_edge_to_normalized_y(y, graph->chunk_size, graph->padding, graph->window_height);
    }

    glBindVertexArray(gridVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
    glBufferData(GL_ARRAY_BUFFER, total_lines * 4 * sizeof(float), vertices, GL_DYNAMIC_DRAW);
    glLineWidth(1.0f);
    glDrawArrays(GL_LINES, 0, total_lines * 2);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glLineWidth(1.0f);

    free(vertices);
}

void renderer_draw_cars(Graph *graph, Car *cars, int car_count) {
    if (graph == NULL || !cars || car_count <= 0) {
        return;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindVertexArray(carVAO);

    for (int i = 0; i < car_count; i++) {
        Car *car = &cars[i];
        if (car->road_id < 0 || car->road_id >= graph->road_count) {
            continue;
        }

        RoadSegment *road = &graph->roads[car->road_id];
        float x1 = grid_center_to_normalized_x(road->x1, graph->chunk_size, graph->padding, graph->window_width);
        float y1 = grid_center_to_normalized_y(road->y1, graph->chunk_size, graph->padding, graph->window_height);
        float x2 = grid_center_to_normalized_x(road->x2, graph->chunk_size, graph->padding, graph->window_width);
        float y2 = grid_center_to_normalized_y(road->y2, graph->chunk_size, graph->padding, graph->window_height);
        float cx = x1 + (x2 - x1) * car->position;
        float cy = y1 + (y2 - y1) * car->position;

        float scale_x = 0.06f;
        float scale_y = 0.04f;
        if (road->type == ROAD_VERTICAL) {
            scale_x = 0.04f;
            scale_y = 0.06f;
        }

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
        glColor3f(car->color[0], car->color[1], car->color[2]);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glPopMatrix();

        glBindTexture(GL_TEXTURE_2D, 0);
    }

    glBindVertexArray(0);
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
    if (VAO != 0) {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }
}
