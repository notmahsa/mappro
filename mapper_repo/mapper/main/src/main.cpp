#include <iostream>
#include <string>
#include <vector>
#include "m1.h"
#include "m2.h"
#include "StreetsDatabaseAPI.h"
#include <unordered_map>
#include "graphics.h"


/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */


const t_bound_box initial_coords;

extern std::vector<double> street_segment_length;

int main(int argc, char** argv) {

    std::string map_path;
    if(argc == 1) {
        //Use a default map
        map_path = "/cad2/ece297s/public/maps/toronto_canada.streets.bin";
    } else if (argc == 2) {
        //Get the map from the command line
        map_path = argv[1];
    } else {
        //Invalid arguments
        std::cerr << "Usage: " << argv[0] << " [map_file_path]\n";
        std::cerr << "  If no map_file_path is provided a default map is loaded.\n";
        return 1;
    }
    //Load the map and related data structures
    bool load_success = load_map(map_path);
    if(!load_success) {
        std::cerr << "Failed to load map '" << map_path << "'\n";
        return 2;
    }
    
    draw_map();

    std::cout << "Successfully loaded map '" << map_path << "'\n";
    //You can now do something with the map
    //Clean-up the map related data structures
    std::cout << "Closing map\n";
    close_map(); 
    close_graphics();
    return 0;
}

