#pragma once //protects against multiple inclusions of this header file

#include <string>
#include <vector>
class LatLon; //Forward declaration

//use these values if you need earth radius or conversion from degrees to radians
constexpr double EARTH_RADIUS_IN_METERS = 6372797.560856;
constexpr double DEG_TO_RAD = 0.017453292519943295769236907684886;

//Loads a map streets.bin file. Returns true if successful, false if some error
//occurs and the map can't be loaded.
bool load_map(std::string map_name);

//Close the map (if loaded)
void close_map();

//Returns street id(s) for the given street name
//If no street with this name exists, returns a 0-length vector.
std::vector<unsigned> find_street_ids_from_name(std::string street_name);

//Returns the street segments for the given intersection 
std::vector<unsigned> find_intersection_street_segments(unsigned intersection_id);

//Returns the street names at the given intersection (includes duplicate street names in returned vector)
std::vector<std::string> find_intersection_street_names(unsigned intersection_id);

//Returns true if you can get from intersection1 to intersection2 using a single street segment (hint: check for 1-way streets too)
//corner case: an intersection is considered to be connected to itself
bool are_directly_connected(unsigned intersection_id1, unsigned intersection_id2);

//Returns all intersections reachable by traveling down one street segment 
//from given intersection (hint: you can't travel the wrong way on a 1-way street)
//the returned vector should NOT contain duplicate intersections
std::vector<unsigned> find_adjacent_intersections(unsigned intersection_id);

//Returns all street segments for the given street
std::vector<unsigned> find_street_street_segments(unsigned street_id);

//Returns all intersections along the a given street
std::vector<unsigned> find_all_street_intersections(unsigned street_id);

//Return all intersection ids for two intersecting streets
//This function will typically return one intersection id.
//However street names are not guarenteed to be unique, so more than 1 intersection id may exist
std::vector<unsigned> find_intersection_ids_from_street_names(std::string street_name1, std::string street_name2);

//Retruns the distance between two coordinates in meters
double find_distance_between_two_points(LatLon point1, LatLon point2);

//Returns the length of the given street segment in meters
double find_street_segment_length(unsigned street_segment_id);

//Returns the length of the specified street in meters
double find_street_length(unsigned street_id);

//Returns the travel time to drive a street segment in seconds (time = distance/speed_limit)
double find_street_segment_travel_time(unsigned street_segment_id);

//Returns the nearest point of interest to the given position
unsigned find_closest_point_of_interest(LatLon my_position);

//Returns the the nearest intersection to the given position
unsigned find_closest_intersection(LatLon my_position);
