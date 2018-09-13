#include "sse_test.hpp"
#include <stdlib.h>
#include <time.h>

float randFloat() {
    return (float) (rand()) / (float) (RAND_MAX);
}

sse_float4 randFloat4() {
    sse_float4 res;
    res.elements[0] = randFloat();
    res.elements[1] = randFloat();
    res.elements[2] = randFloat();
    res.elements[3] = randFloat();
    return res;
}

void sse_test(Mesh &mesh) {
    //Not allowed to change:
    unsigned int const loadFactor = 1000;
    //
    std::vector<sse_float4> vertices;
    std::vector<sse_float4> rand1;
    std::vector<sse_float4> rand2;
    std::vector<sse_float4> rand3;
    std::vector<sse_float4> rand4;
    srand(time(NULL));

    vertices.resize(mesh.vertexCount);

    std::cout << "SSE_TEST: Initializing vectors... " << std::flush;
    for (unsigned int i=0; i < vertices.size(); i++) {
        vertices[i].elements[0] = mesh.vertices[i].x;
        vertices[i].elements[1] = mesh.vertices[i].y;
        vertices[i].elements[2] = mesh.vertices[i].z;
        vertices[i].elements[3] = mesh.vertices[i].w;

        rand1.push_back(randFloat4());
        rand2.push_back(randFloat4());
        rand3.push_back(randFloat4());
        rand4.push_back(randFloat4());
    }
    std::cout << "finished!"  << std::endl;
    for (unsigned int j = 0; j < loadFactor; j++) {
        std::cout << "SSE_TEST: " << (j+1) << "/" << loadFactor << " Crunching numbers on " << vertices.size() << " vertices... " << "\r" << std::flush;
        for (unsigned int i=0; i < vertices.size(); i++) {

            vertices[i].elements[0] = vertices[i].elements[0] + rand1[i].elements[0];
            vertices[i].elements[1] = vertices[i].elements[1] + rand1[i].elements[1];
            vertices[i].elements[2] = vertices[i].elements[2] + rand1[i].elements[2];
            vertices[i].elements[3] = vertices[i].elements[3] + rand1[i].elements[3];


            vertices[i].elements[0] = vertices[i].elements[0] - rand2[i].elements[0];
            vertices[i].elements[1] = vertices[i].elements[1] - rand2[i].elements[1];
            vertices[i].elements[2] = vertices[i].elements[2] - rand2[i].elements[2];
            vertices[i].elements[3] = vertices[i].elements[3] - rand2[i].elements[3];


            vertices[i].elements[0] = vertices[i].elements[0] * rand3[i].elements[0];
            vertices[i].elements[1] = vertices[i].elements[1] * rand3[i].elements[1];
            vertices[i].elements[2] = vertices[i].elements[2] * rand3[i].elements[2];
            vertices[i].elements[3] = vertices[i].elements[3] * rand3[i].elements[3];

            if (rand4[i].elements[0] != 0 || rand4[i].elements[1] != 0 || rand4[i].elements[2] != 0 || rand4[i].elements[3] != 0){
                vertices[i].elements[0] = vertices[i].elements[0] / rand4[i].elements[0];
                vertices[i].elements[1] = vertices[i].elements[1] / rand4[i].elements[1];
                vertices[i].elements[2] = vertices[i].elements[2] / rand4[i].elements[2];
                vertices[i].elements[3] = vertices[i].elements[3] / rand4[i].elements[3];
            }
        }
    }
    std::cout << std::endl;
}


