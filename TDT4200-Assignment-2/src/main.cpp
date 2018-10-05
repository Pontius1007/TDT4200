#include <iostream>
#include <cstring>
#include "utilities/OBJLoader.hpp"
#include "utilities/lodepng.h"
#include "rasteriser.hpp"
#include <mpi.h>
#include "utilities/floats.hpp"
#include <stddef.h>

int main(int argc, char **argv) {
    std::string input("../input/sphere.obj");
    std::string output("../output/sphere.png");
    unsigned int width = 1920;
    unsigned int height = 1080;
    unsigned int depth = 3;

    std::vector<float4> verticestest;
    std::vector<float3> texturestest;
    std::vector<float3> normalstest;

    //Initialize MPI environment
    MPI_Init(NULL, NULL);
    //Get number of processes
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    // Get the rank of the process
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    //float rotationAngle = (world_rank) *30;
    float rotationAngleSend = 0;
    float calc_rotation_angle = world_rank;

    for (int i = 1; i < argc; i++) {
        if (i < argc - 1) {
            if (std::strcmp("-i", argv[i]) == 0) {
                input = argv[i + 1];
            } else if (std::strcmp("-o", argv[i]) == 0) {
                output = argv[i + 1];
            } else if (std::strcmp("-w", argv[i]) == 0) {
                width = (unsigned int) std::stoul(argv[i + 1]);
            } else if (std::strcmp("-h", argv[i]) == 0) {
                height = (unsigned int) std::stoul(argv[i + 1]);
            } else if (std::strcmp("-d", argv[i]) == 0) {
                depth = (int) std::stoul(argv[i + 1]);
            }
        }
    }
    std::cout << "Loading '" << input << "' file... " << std::endl;

    std::vector <Mesh> meshs = loadWavefront(input, false);

    //Remove verticies, normals and texture arrays for everyone except master
    if (world_rank != 0) {
        for (unsigned int i = 0; i < meshs.size(); i++) {
            for (unsigned int vertex = 0; vertex < meshs.at(i).vertices.size(); vertex++) {
                meshs.at(i).vertices.at(vertex).x = 0;
                meshs.at(i).vertices.at(vertex).y = 0;
                meshs.at(i).vertices.at(vertex).z = 0;
                meshs.at(i).vertices.at(vertex).w = 0;
            }
            if (meshs.at(i).hasTextures) {
                for(unsigned int t = 0; t < meshs.at(i).textures.size(); t++) {
                    meshs.at(i).textures.at(t).x = 0;
                    meshs.at(i).textures.at(t).y = 0;
                    meshs.at(i).textures.at(t).z = 0;
                }
            }
            if (meshs.at(i).hasNormals) {
                for(unsigned int n = 0; n < meshs.at(i).normals.size(); n++) {
                    meshs.at(i).normals.at(n).x = 0;
                    meshs.at(i).normals.at(n).y = 0;
                    meshs.at(i).normals.at(n).z = 0;
                }
            }
        }
    }

    //Creating the float3 and float4 datastructure
    //Float4
    int count = 4;
    int blocklengths[4] = {1, 1, 1, 1};
    MPI_Datatype types[4] = {MPI_FLOAT, MPI_FLOAT, MPI_FLOAT, MPI_FLOAT};
    MPI_Datatype MPI_FLOAT4;
    MPI_Aint offsets[4];

    offsets[0] = offsetof(float4, x);
    offsets[1] = offsetof(float4, y);
    offsets[2] = offsetof(float4, z);
    offsets[3] = offsetof(float4, w);

    MPI_Type_create_struct(count, blocklengths, offsets, types, &MPI_FLOAT4);
    MPI_Type_commit(&MPI_FLOAT4);

    //Float3
    int count_3 = 3;
    MPI_Aint offsets_3[3];
    int blocklengths_3[3] = {1, 1, 1};
    MPI_Datatype types_3[3] = {MPI_FLOAT, MPI_FLOAT, MPI_FLOAT};
    MPI_Datatype MPI_FLOAT3;

    offsets_3[0] = offsetof(float3, x);
    offsets_3[1] = offsetof(float3, y);
    offsets_3[2] = offsetof(float3, z);

    MPI_Type_create_struct(count_3, blocklengths_3, offsets_3, types_3, &MPI_FLOAT3);
    MPI_Type_commit(&MPI_FLOAT3);

    //Send the value
    for (unsigned int x = 0; x < meshs.size(); x++) {
        verticestest = meshs.at(x).vertices;
        texturestest = meshs.at(x).textures;
        normalstest = meshs.at(x).normals;

        MPI_Bcast(&verticestest.front(), verticestest.size(), MPI_FLOAT4, 0, MPI_COMM_WORLD);
        MPI_Bcast(&texturestest.front(), texturestest.size(), MPI_FLOAT3, 0, MPI_COMM_WORLD);
        MPI_Bcast(&normalstest.front(), normalstest.size(), MPI_FLOAT3, 0, MPI_COMM_WORLD);

        MPI_Barrier(MPI_COMM_WORLD);

        meshs.at(x).vertices = verticestest;
        meshs.at(x).textures = texturestest;
        meshs.at(x).normals = normalstest;
    }

    //Is this correct? How can we check if it is asking?
    if (world_rank != 0) {
        MPI_Send(
                &calc_rotation_angle, 1, MPI_FLOAT, 0, 0, MPI_COMM_WORLD
        );
        MPI_Recv(
                &rotationAngleSend, 1, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE
        );
    } else {
        for (int i = 1; i < world_size; i++) {
            MPI_Recv(
                    &calc_rotation_angle, 1, MPI_FLOAT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE
            );
            rotationAngleSend = calc_rotation_angle * 30;
            MPI_Send(
                    &rotationAngleSend, 1, MPI_FLOAT, i, 0, MPI_COMM_WORLD
            );
        }
    }

    std::vector<unsigned char> frameBuffer = rasterise(meshs, width, height, depth, rotationAngleSend,
            world_rank, world_size);

    //Give each picture a unique name so we can see the different pictures
    std::string str = ".";
    int position = output.find(str);
    output.insert(position, std::to_string(world_rank));

    std::cout << "Writing image to '" << output << "'..." << std::endl;

    // Finalize the MPI environment.
    MPI_Finalize();

    unsigned error = lodepng::encode(output, frameBuffer, width, height);

    if (error) {
        std::cout << "An error occurred while writing the image file: " << error << ": " << lodepng_error_text(error)
                  << std::endl;
    }
    return 0;
}
