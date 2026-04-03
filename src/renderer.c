#include "renderer.h"
#include <GL/glew.h>
#include <stdlib.h>
#include <math.h>

static GLuint VAO = 0, VBO = 0;

void renderer_init(void) {
    // Создаем VAO и VBO для рисования линий
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    
    // Настраиваем атрибуты вершин
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void renderer_draw_roads(Graph *graph) {
    if (!graph || graph->edge_count == 0) {
        return;
    }
    
    // Преобразуем координаты в нормализованные координаты OpenGL (-1 до 1)
    float *vertices = (float*)malloc(graph->edge_count * 4 * sizeof(float));
    
    for (int i = 0; i < graph->edge_count; i++) {
        Edge *edge = &graph->edges[i];
        Node *from = &graph->nodes[edge->from];
        Node *to = &graph->nodes[edge->to];
        
        // Преобразуем пиксельные координаты в нормализованные
        float x1 = (2.0f * from->pixel_x / graph->window_width) - 1.0f;
        float y1 = 1.0f - (2.0f * from->pixel_y / graph->window_height);
        float x2 = (2.0f * to->pixel_x / graph->window_width) - 1.0f;
        float y2 = 1.0f - (2.0f * to->pixel_y / graph->window_height);
        
        vertices[i * 4 + 0] = x1;
        vertices[i * 4 + 1] = y1;
        vertices[i * 4 + 2] = x2;
        vertices[i * 4 + 3] = y2;
    }
    
    // Загружаем вершины в VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, graph->edge_count * 4 * sizeof(float), vertices, GL_DYNAMIC_DRAW);
    
    // Рисуем линии
    glBindVertexArray(VAO);
    glDrawArrays(GL_LINES, 0, graph->edge_count * 2);
    glBindVertexArray(0);
    
    free(vertices);
}

void renderer_draw_nodes(Graph *graph) {
    if (!graph || graph->node_count == 0) {
        return;
    }
    
    // Преобразуем координаты для точек (перекрестков)
    float *vertices = (float*)malloc(graph->node_count * 2 * sizeof(float));
    
    for (int i = 0; i < graph->node_count; i++) {
        Node *node = &graph->nodes[i];
        
        float x = (2.0f * node->pixel_x / graph->window_width) - 1.0f;
        float y = 1.0f - (2.0f * node->pixel_y / graph->window_height);
        
        vertices[i * 2 + 0] = x;
        vertices[i * 2 + 1] = y;
    }
    
    // Загружаем вершины в VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, graph->node_count * 2 * sizeof(float), vertices, GL_DYNAMIC_DRAW);
    
    // Рисуем точки
    glPointSize(8.0f);
    glBindVertexArray(VAO);
    glDrawArrays(GL_POINTS, 0, graph->node_count);
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
