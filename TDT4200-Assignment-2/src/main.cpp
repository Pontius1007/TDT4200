#include <iostream>
#include <cstring>
#include "utilities/OBJLoader.hpp"
#include "utilities/lodepng.h"
#include "rasteriser.hpp"
#include <mpi.h>

int main(int argc, char **argv) {
	std::string input("../input/sphere.obj");
	std::string output("../output/sphere.png");
	unsigned int width = 1920;
	unsigned int height = 1080;
	unsigned int depth = 3;
	//Initialize MPI environment
	MPI_Init(NULL, NULL);
	//Get number of processes
	int world_size;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	// Get the rank of the process
	int world_rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

	float rotationAngle = (world_rank) *30;
	float rotationAngleSend = 0;
	float calc_rotation_angle = world_rank;

	for (int i = 1; i < argc; i++) {
		if (i < argc -1) {
			if (std::strcmp("-i", argv[i]) == 0) {
				input = argv[i+1];
			} else if (std::strcmp("-o", argv[i]) == 0) {
				output = argv[i+1];
			} else if (std::strcmp("-w", argv[i]) == 0) {
				width = (unsigned int) std::stoul(argv[i+1]);
			} else if (std::strcmp("-h", argv[i]) == 0) {
				height = (unsigned int) std::stoul(argv[i+1]);
			} else if (std::strcmp("-d", argv[i]) == 0) {
				depth = (int) std::stoul(argv[i+1]);
			}
		}
	}
    std::cout << "Loading '" << input << "' file... " << std::endl;

    std::vector<Mesh> meshs = loadWavefront(input, false);



    //Is this correct? How can we check if it is asking?
    if (world_rank != 0) {
        MPI_Send (
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

    std::vector<unsigned char> frameBuffer = rasterise(meshs, width, height, depth, rotationAngleSend);

    //Give each picture a unique name so we can see the different pictures
    std::string str = ".";
    int position = output.find(str);
    output.insert(position, std::to_string(world_rank));

    std::cout << "Writing image to '" << output << "'..." << std::endl;

    // Finalize the MPI environment.
    MPI_Finalize();

    unsigned error = lodepng::encode(output, frameBuffer, width, height);

    if(error)
    {
        std::cout << "An error occurred while writing the image file: " << error << ": " << lodepng_error_text(error) << std::endl;
    }
    return 0;
}
