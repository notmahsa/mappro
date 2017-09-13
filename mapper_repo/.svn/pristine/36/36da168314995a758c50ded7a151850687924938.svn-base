/* 
 * File:   m3.h
 * Author: naserifa
 *
 * Created on March 12, 2017, 10:44 AM
 */
#pragma once //protects against multiple inclusions of this header file

#include <vector>
#include <map>
#include <string>
#include <utility>

#ifndef M3_H
#define M3_H

#define INF 0x3f3f3f3f
double compute_path_travel_time(const std::vector<unsigned>& path, const double turn_penalty);
std::vector<unsigned> find_pois_from_name(std::string name);
std::vector<unsigned> directly_connected_segment(unsigned intersection_id1, unsigned intersection_id2);
std::vector<unsigned> find_path_between_intersections(const unsigned intersect_id_start, const unsigned intersect_id_end, const double turn_penalty);
std::vector<unsigned> find_path_to_point_of_interest(const unsigned intersect_id_start,const std::string point_of_interest_name,const double turn_penalty);
void le_directions(const std::vector<unsigned>& path);
bool positive_cross_product(double x1, double y1, double x2, double y2);
bool zero_cross_product(double x1, double y1, double x2, double y2);
std::vector <unsigned> reconstruct_path(std::vector<unsigned>& cameFrom, unsigned current, unsigned intersect_id_start) ;
bool found_POI(unsigned current, std::string POI_name) ;
double heuristic_cost_estimate(unsigned intersection, unsigned intersect_id_end) ;
std::vector <unsigned> intersection_to_street_seg(std::vector<unsigned>& intersection_path, double last_intersection);
unsigned find_fastest_street_segment(const std::vector<unsigned>& shared_street_segments, unsigned intersection1);
std::vector<unsigned>  find_street_segments(unsigned intersection_id1, unsigned intersection_id2);
#endif /* M3_H */

