#include "renderer.h"
#include "menu.h"
#include "graph.h"
#include "car.h"

#include <GL/glew.h>
#include <stdlib.h>
#include <stdio.h>

static GLuint VAO = 0, VBO = 0;

static float grid_to_normalized_x(const Graph *graph, int grid_x) {
    float pixel = (grid_x + graph->padding) * graph->chunk_size + graph->chunk_size * 0.5f;
    return (2.0f * pixel / graph->window_width) - 1.0f;
}

static float grid_to_normalized_y(const Graph *graph, int grid_y) {
    float pixel = (grid_y + graph->padding) * graph->chunk_size + graph->chunk_size * 0.5f;
    return 1.0f - (2.0f * pixel / graph->window_height);
}

void renderer_init(void) {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

static float pixel_to_normalized_x(const Graph *graph, int px) {
    return (2.0f * px / graph->window_width) - 1.0f;
}

static float pixel_to_normalized_y(const Graph *graph, int py) {
    return 1.0f - (2.0f * py / graph->window_height);
}

void renderer_draw_roads(Graph *graph) {
    if (!graph || graph->road_count == 0) {
        return;
    }

    float *vertices = malloc(graph->road_count * 4 * sizeof(float));
    if (!vertices) {
        fprintf(stderr, "renderer_draw_roads: failed to allocate vertices buffer\n");
        return;
    }

    for (int i = 0; i < graph->road_count; i++) {
        RoadSegment *road = &graph->roads[i];
        float x1 = grid_to_normalized_x(graph, road->x1);
        float y1 = grid_to_normalized_y(graph, road->y1);
        float x2 = grid_to_normalized_x(graph, road->x2);
        float y2 = grid_to_normalized_y(graph, road->y2);

        vertices[i * 4 + 0] = x1;
        vertices[i * 4 + 1] = y1;
        vertices[i * 4 + 2] = x2;
        vertices[i * 4 + 3] = y2;
    }

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, graph->road_count * 4 * sizeof(float), vertices, GL_DYNAMIC_DRAW);
    glBindVertexArray(VAO);
    glDrawArrays(GL_LINES, 0, graph->road_count * 2);
    glBindVertexArray(0);

    free(vertices);
}

void renderer_draw_grid(Graph *graph) {
    if (!graph || graph->grid_width <= 0 || graph->grid_height <= 0) {
        return;
    }

    int total_lines = graph->grid_width + graph->grid_height + 2;
    float *vertices = malloc(total_lines * 4 * sizeof(float));
    if (!vertices) {
        fprintf(stderr, "renderer_draw_grid: failed to allocate vertices buffer\n");
        return;
    }

    int index = 0;
    int left = graph->padding * graph->chunk_size;
    int right = left + graph->grid_width * graph->chunk_size;
    int top = graph->padding * graph->chunk_size;
    int bottom = top + graph->grid_height * graph->chunk_size;

    // vertical lines
    for (int x = 0; x <= graph->grid_width; x++) {
        int px = left + x * graph->chunk_size;
        vertices[index++] = pixel_to_normalized_x(graph, px);
        vertices[index++] = pixel_to_normalized_y(graph, top);
        vertices[index++] = pixel_to_normalized_x(graph, px);
        vertices[index++] = pixel_to_normalized_y(graph, bottom);
    }

    // horizontal lines
    for (int y = 0; y <= graph->grid_height; y++) {
        int py = top + y * graph->chunk_size;
        vertices[index++] = pixel_to_normalized_x(graph, left);
        vertices[index++] = pixel_to_normalized_y(graph, py);
        vertices[index++] = pixel_to_normalized_x(graph, right);
        vertices[index++] = pixel_to_normalized_y(graph, py);
    }

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, total_lines * 4 * sizeof(float), vertices, GL_DYNAMIC_DRAW);
    glBindVertexArray(VAO);
    glLineWidth(1.0f);
    glDrawArrays(GL_LINES, 0, total_lines * 2);
    glBindVertexArray(0);
    glLineWidth(1.0f);

    free(vertices);
}

void renderer_draw_cars(struct Graph *graph, struct Car *cars, int car_count) {
    if (!graph || !cars || car_count <= 0) {
        return;
    }

    glBindVertexArray(0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (int i = 0; i < car_count; i++) {
        Car *car = &cars[i];
        if (car->road_id < 0 || car->road_id >= graph->road_count) {
            continue;
        }
        RoadSegment *road = &graph->roads[car->road_id];

        float x1 = grid_to_normalized_x(graph, road->x1);
        float y1 = grid_to_normalized_y(graph, road->y1);
        float x2 = grid_to_normalized_x(graph, road->x2);
        float y2 = grid_to_normalized_y(graph, road->y2);

        float cx = x1 + (x2 - x1) * car->position;
        float cy = y1 + (y2 - y1) * car->position;

        float half_width = 0.03f;
        float half_height = 0.02f;
        if (road->type == ROAD_VERTICAL) {
            half_width = 0.02f;
            half_height = 0.04f;
        }

        if (car->texture != 0) {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, car->texture);
            glColor3f(1.0f, 1.0f, 1.0f);
            
            glBegin(GL_QUADS);
            glTexCoord2f(0.0f, 0.0f); glVertex2f(cx - half_width, cy - half_height);
            glTexCoord2f(1.0f, 0.0f); glVertex2f(cx + half_width, cy - half_height);
            glTexCoord2f(1.0f, 1.0f); glVertex2f(cx + half_width, cy + half_height);
            glTexCoord2f(0.0f, 1.0f); glVertex2f(cx - half_width, cy + half_height);
            glEnd();

            glBindTexture(GL_TEXTURE_2D, 0);
            glDisable(GL_TEXTURE_2D);
        } else {
            glDisable(GL_TEXTURE_2D);
            glColor3f(car->color[0], car->color[1], car->color[2]);
            
            glBegin(GL_QUADS);
            glVertex2f(cx - half_width, cy - half_height);
            glVertex2f(cx + half_width, cy - half_height);
            glVertex2f(cx + half_width, cy + half_height);
            glVertex2f(cx - half_width, cy + half_height);
            glEnd();
        }
    }

    glDisable(GL_BLEND);
    glBindVertexArray(VAO);
}

void renderer_draw_nodes(Graph *graph) {
    if (!graph || graph->road_count == 0) {
        return;
    }

    int point_count = graph->road_count * 2;
    float *vertices = malloc(point_count * 2 * sizeof(float));
    if (!vertices) {
        fprintf(stderr, "renderer_draw_nodes: failed to allocate vertices buffer\n");
        return;
    }

    int index = 0;
    for (int i = 0; i < graph->road_count; i++) {
        RoadSegment *road = &graph->roads[i];
        vertices[index++] = grid_to_normalized_x(graph, road->x1);
        vertices[index++] = grid_to_normalized_y(graph, road->y1);
        vertices[index++] = grid_to_normalized_x(graph, road->x2);
        vertices[index++] = grid_to_normalized_y(graph, road->y2);
    }

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, point_count * 2 * sizeof(float), vertices, GL_DYNAMIC_DRAW);
    glPointSize(8.0f);
    glBindVertexArray(VAO);
    glDrawArrays(GL_POINTS, 0, point_count);
    glPointSize(1.0f);
    glBindVertexArray(0);

    free(vertices);
}

void renderer_shutdown(void) {
    if (VAO != 0) {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }
}
