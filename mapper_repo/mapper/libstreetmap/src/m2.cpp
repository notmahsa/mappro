/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include "m3.h"
#include "m2.h"
#include "m1.h"
#include "m3.h"
#include "graphics.h"
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include "MapParser.h"
#include <X11/keysym.h>
#include <graphics.cpp>
#include <deque>
#include "Surface.h"
#include <math.h>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <string>
#include <chrono>

/*******************************************/
/*             FEATURE VARIABLES           */
/*******************************************/
const t_bound_box initial_coords = t_bound_box(0, 0, 1000, 1000);
//t_point minScrn,maxScrn;
double lat_avg;
float xmin, xmax, ymin, ymax;
LatLon Aclick = LatLon(0, 0);
LatLon Bclick = LatLon(0, 0);
bool ACLICK = false;
bool BCLICK = false;
std::string searchText, mapText;
bool shift = false;
unsigned poiClick, intersectionClick, inter;
LatLon pointClick;
unsigned interA, interB;
std::vector<std::string> directions_to_print;

/*******************************************/
/*             FEATURE VECTORS             */
/*******************************************/
std::vector<string> street1Auto, street2Auto;
extern std::vector<unsigned> buildings;

//extern std::deque<featureInfo> features_by_area;
extern std::vector<featureInfo> zero_area_features;
extern std::vector<featureInfo> open_features;

mapData mapInfo;
std::vector<double> coords;

/*******************************************/
/*           HIGHLIGHT VECTORS             */
/*******************************************/
std::vector<unsigned> found_intersections;
bool highlightIntersections;


std::unordered_map<string, string> tags;

extern t_transform_coordinates trans_coord;


/*******************************************/
/*            VOID FUNCTION                */

/*******************************************/

void find(std::string search) {
    // if finding path, don't highlight intersection
    if (mapInfo.path == false) {
        highlightIntersections = true;
    } else {
        highlightIntersections = true;
    }
    
    //clear the vector
    found_intersections.clear();
    string findStr, street1, street2;

    stringstream findStrStm(search);
    findStrStm>>street1;

    string next;
    findStrStm>>next;
    
    // take in street 1, if and isn't found by the end of file, only one street, so return
    while (next != "and" && next != "&" ) {
        if (findStrStm.eof()) {
            return;
        }
        street1 = street1 + " " + next;
        findStrStm>>next;
    }

    // rest of string is street 2
    findStrStm>>street2;
    while (!findStrStm.eof()) {
        findStrStm>>next;
        street2 = street2 + " " + next;
    }

    string upSt1 = boost::to_upper_copy<std::string>(street1);
    string upSt2 = boost::to_upper_copy<std::string>(street2);

    std::unordered_map<std::string, std::vector<unsigned>>::iterator streetIt;

    string st1Actual, st2Actual;

    bool oneFound = false, twoFound = false;
    std::vector<string> street1Auto, street2Auto;
    int s1 = 0, s2 = 0;
    
    // find all the strings that contain the substring
    for (streetIt = mapInfo.NameToStreetID.begin(); streetIt != mapInfo.NameToStreetID.end(); ++streetIt) {
        std::string itKey = boost::to_upper_copy<std::string>(streetIt->first);

        if (upSt1 == itKey) {
            oneFound = true;
            st1Actual = streetIt->first;
        } else if (upSt2 == itKey) {
            twoFound = true;
            st2Actual = streetIt->first;
        } else if (oneFound && twoFound) {
            break;
        } else if (!oneFound && upSt1 == (itKey).substr(0, upSt1.length())) {
            street1Auto.push_back(streetIt->first);

        } else if (!twoFound && upSt2 == (itKey).substr(0, upSt2.length())) {
            street2Auto.push_back(streetIt->first);
        }
    }

    // update messages and terminal based on if the streets were found
    if (oneFound && twoFound)
        update_message("Finding intersection: " + st1Actual + " & " + st2Actual);

    else if (oneFound) {
        update_message("Street2 not found");
        cout << "\nOptions for street 2\n";
        for (int i = 0; i < street2Auto.size(); i++) {
            cout << street2Auto[i] << std::endl;
        }
        return;
    } else if (twoFound) {
        update_message("Street1 not found");
        cout << "\nOptions for street 1\n";
        for (int i = 0; i < street1Auto.size(); i++) {
            cout << street1Auto[i] << std::endl;
        }
        return;
    } else {
        update_message("Both streets not found");

        cout << "\nOptions for street 1\n";
        for (int i = 0; i < street1Auto.size(); i++) {
            cout << street1Auto[i] << std::endl;
        }

        cout << "\nOptions for street 2\n";
        for (int i = 0; i < street2Auto.size(); i++) {
            cout << street2Auto[i] << std::endl;
        }

        return;
    }

    // update all the intersections found between the two streets
    found_intersections = find_intersection_ids_from_street_names(st1Actual, st2Actual);
    //std::cout<<found_intersections[0]<<std::endl;
    
    // if finding not finding a path, output the information of the found intersections
    if (!mapInfo.highlight_travel_path) {
        for (int i = 0; i < found_intersections.size(); i++) {
            cout << getIntersectionName(found_intersections[i]) << std::endl;
            cout << mapInfo.intersection_position[found_intersections[i]].lat() << std::endl;
            cout << mapInfo.intersection_position[found_intersections[i]].lon() << std::endl << std::endl;
        }
        
        //auto zoom into the first found intersection
        if (found_intersections.size() > 0 && !mapInfo.path) {
            t_point pos = getXY(mapInfo.intersection_position[found_intersections[0]]);
            zoom_fit(drawscreen);
            while (zoom_level < 6) {
                handle_zoom_in(pos.x, pos.y, drawscreen);
            }
        }
    }

}

/*******************************************/
/*             DRAW MAP                    */

/*******************************************/

void draw_map() {
    coords = calcInitialCoordinates(true);


    init_graphics("Map of the City", MAPGREY);

    //create_button("Window", "Find", find);
    create_button("Window", "Load map", act_on_load_new_map);
    create_button("Load map", "POI Names", show_poi_names);
    create_button("POI Names", "Draw POIs", more_poi_display);
    create_button("Draw POIs", "Help", help);

    set_drawing_buffer(OFF_SCREEN);
    set_visible_world(coords[0], coords[1], coords[2], coords[3]);

    set_keypress_input(true);
    set_mouse_move_input(true);

    event_loop(act_on_mousebutton, act_on_mouse_move, act_on_key_press, drawscreen);


    drawscreen();

}


/*******************************************/
/*               DRAW SCREEN               */

/*******************************************/
void drawscreen(void) {

    /* The redrawing routine for still pictures.  The graphics package calls  
     * this routine to do redrawing after the user changes the window 
     * in any way.                                                    
     */

    set_drawing_buffer(OFF_SCREEN);
    set_draw_mode(DRAW_NORMAL); // Should set this if your program does any XOR drawing in callbacks.
    /* Should precede drawing for all drawscreens */

    set_coordinate_system(GL_WORLD);

    setfontsize(10);
    setlinestyle(SOLID);
    setlinewidth(0);
    setcolor(WHITE);
    {
        if (!mapInfo.action) {
            clearscreen();
            // call the zoom helper function based on what the zoom level is
            switch (zoom_level) {
                case 0:
                    zoom0_draw();
                    break;
                case 1:
                    zoom1_draw();
                    break;
                case 2:
                    zoom2_draw();
                    break;
                case 3:
                    zoom3_draw();
                    break;
                case 4:
                    zoom4_draw();
                    break;
                case 5:
                    zoom5_draw();
                case 6:
                    zoom6_draw();
                    if (mapInfo.more_pois)
                        draw_poi(5);
                    break;
                case 7:
                    zoom7_draw();
                    if (mapInfo.more_pois)
                        draw_poi(2);
                    break;
                case 8:
                    zoom8_draw();
                    if (mapInfo.more_pois)
                        draw_poi(2);
                    break;
                case 9:
                    zoom9_draw();
                    if (mapInfo.more_pois)
                        draw_poi(1);
                    break;
                case 10:
                    zoom10_draw();
                    break;
                default:
                    std::cout << "zoom error" << std::endl;
                    break;
            }
        }
        if (!mapInfo.help) {
            if (highlightIntersections) {

                // highlight the intersection that the find function locates
                std::string lat, lon, found;
                for (unsigned i = 0; i < found_intersections.size(); i++) {
                    highlight_intersection(found_intersections[i]);
                }
            }

            // draw a search bar
            {
                set_coordinate_system(GL_SCREEN);
                // draw a shadow

                if (mapInfo.highlight_travel_path) {
                    highlight_path();
                    mapInfo.draw_pin = false;

                } else {
                    
                    // draw search bar if directions panel is not open
                    setcolor(DARKGREY);
                    fillrect(10, 10, 270, 62);
                    setcolor(RED);
                    fillrect(270, 10, 322, 62);

                    setcolor(WHITE);
                    setlinestyle(SOLID);
                    setlinewidth(1);
                    fillrect(10, 10, 370, 60);

                    // draw search bar icons
                    draw_surface(load_png_from_file("/nfs/ug/homes-5/r/roscoeol/mapper_repo/mapper/libstreetmap/src/icons/search.png"), 280, 25);
                    drawline(320, 20, 320, 50);
                    draw_surface(load_png_from_file("/nfs/ug/homes-5/r/roscoeol/mapper_repo/mapper/libstreetmap/src/icons/travel.png"), 330, 20);

                }
                
                // call draw functions
                draw_search_hover();
                draw_directions_hover();
                search_typing();
                draw_directions();
                enter_new_map();

                set_coordinate_system(GL_WORLD);


            }
        } else {
            clearscreen();
            set_coordinate_system(GL_SCREEN);
            setcolor(WHITE);
            t_bound_box rect = t_bound_box(0, 0, trans_coord.display_width, trans_coord.display_height);
            fillrect(rect);
            draw_surface(load_png_from_file("/nfs/ug/homes-5/r/roscoeol/mapper_repo/mapper/libstreetmap/src/icons/help.png"), 400, 100);
            set_coordinate_system(GL_WORLD);
        }
        mapInfo.action = false;

    }
    set_coordinate_system(GL_WORLD);
    // Need to copy the off screen buffer to screen if we're using double buffering.
    // Harmless to call this if we're not using double-buffering.
    copy_off_screen_buffer_to_screen();
}

/*******************************************/
/*     CALCULATE INITAL COORDINATES        */

/*******************************************/

std::vector<double> calcInitialCoordinates(bool xy) {
    // holds initial coordinates to display the map
    std::vector<double> map_coords;
    double minLon, minLat, maxLon, maxLat;

    LatLon point, point2, compare;
    // find the first and last intersection coordinates
    point = mapInfo.intersection_position[0];
    point2 = mapInfo.intersection_position[mapInfo.interNum - 1];

    // set the first and last lat/lons as the initial min/max
    minLon = point.lon();
    minLat = point.lat();
    maxLon = point2.lon();
    maxLat = point2.lat();

    // check all intersections, to find max and min of the map
    for (unsigned i = 0; i < mapInfo.interNum; i += 1000) {
        compare = mapInfo.intersection_position[i];
        if (compare.lon() < minLon) {
            minLon = compare.lon();
        } else if (compare.lon() > maxLon) {
            maxLon = compare.lon();
        }

        if (compare.lat() < minLat) {
            minLat = compare.lat();
        } else if (compare.lat() > maxLat) {
            maxLat = compare.lat();
        }
    }
    // store the global value of lat_avg to use when converting to XY
    lat_avg = (minLat + maxLat) / 2;

    // converts lat/lon to XY
    if (xy) {
        minLon = minLon * DEG_TO_RAD * cos(lat_avg * DEG_TO_RAD);
        maxLon = maxLon * DEG_TO_RAD * cos(lat_avg * DEG_TO_RAD);

        minLat = minLat * DEG_TO_RAD;
        maxLat = maxLat * DEG_TO_RAD;
    }

    // stores max/min in a vector in the order they are
    // used to plot the initial map
    map_coords.push_back(minLon);
    map_coords.push_back(minLat);
    map_coords.push_back(maxLon);
    map_coords.push_back(maxLat);
    return map_coords;
}

/*******************************************/
/*             GET X,Y COOR                */

/*******************************************/
t_point getXY(LatLon point) {
    float x1, y1;

    x1 = point.lon() * DEG_TO_RAD * cos(lat_avg * DEG_TO_RAD);

    y1 = point.lat() * DEG_TO_RAD;

    return t_point(x1, y1);
}

/*******************************************/
/*          GET LAT LON COOR               */

/*******************************************/
LatLon getLatLon(float x, float y) {
    float lat, lon;

    lon = x / (DEG_TO_RAD * cos(lat_avg * DEG_TO_RAD));
    lat = y / DEG_TO_RAD;

    return LatLon(lat, lon);
}

/*******************************************/
/*           DRAW STREET SEGMENTS          */

/*******************************************/
void draw_street_segment(unsigned segmentIndex, t_color color, int width) {

    setlinewidth(width);
    setcolor(color);

    t_point p1, p2;
    std::vector<LatLon> points;
    points.push_back(mapInfo.intersection_position[mapInfo.street_segment_info[segmentIndex].from]);

    double length = 0;
    LatLon curvePoint;

    // add the lat/lon of any curve points in the segment to the back of the vector
    if (mapInfo.street_segment_info[segmentIndex].curvePointCount != 0) {
        for (unsigned i = 0; i < mapInfo.street_segment_info[segmentIndex].curvePointCount; i++) {
            curvePoint = getStreetSegmentCurvePoint(segmentIndex, i);
            points.push_back(curvePoint);
        }
    }
    // add the ending intersection to the vector
    points.push_back(mapInfo.intersection_position[mapInfo.street_segment_info[segmentIndex].to]);

    // sum the distance between each intersection/curve point
    for (unsigned i = 0; i < points.size() - 1; i++) {
        p1 = getXY(points[i]);
        p2 = getXY(points[i + 1]);

        drawline(p1, p2);
    }
}

// this function checks to see whether or not the feature has the same 
// starting point and ending point. If they are the same, the feature
// is considered a closed feature and the function returns true

bool is_closed(unsigned featureIndex) {
    // assume that the start and end points of a feature are different
    bool closed = false;

    // get the number of points in the feature
    unsigned count = getFeaturePointCount(featureIndex);

    // create t points of the first and last feature points
    t_point A = getXY(getFeaturePoint(featureIndex, 0));
    t_point B = getXY(getFeaturePoint(featureIndex, count - 1));

    // compare the x and y values of the start and end point
    // if they match return true
    if (A.x == B.x) {
        if (A.y == B.y) {
            closed = true;
        }
    }

    return closed;
}

/*******************************************/
/*             DRAW FEATURES               */
/*******************************************/

// this function takes in the index of the feature, and the color that 
// the feature should be drawn with. It then checks to see if the
// feature should be drawn as a closed polygon, or a multi segmented line
// then it draws it to the screen

void draw_features(unsigned featureIndex, t_color colour, bool closed) {
    // get the number of points in the feature and set the colour of the line/polygon
    unsigned count = getFeaturePointCount(featureIndex);
    setcolor(colour);

    // check if it is a closed or open feature
    if (closed) {

        // create an array of t_points to store the vertices of the polygon
        t_point polypts[count];

        // add the vertices to the array
        for (unsigned i = 0; i < count; i++) {
            polypts[i] = getXY(getFeaturePoint(featureIndex, i));
        }

        // draw the polygon
        fillpoly(polypts, count);
    } else {
        // create two points to draw a line between
        t_point p1, p2;

        // draw lines between each set of two points in the feature
        for (unsigned i = 0; i < count - 1; i++) {
            p1 = getXY(getFeaturePoint(featureIndex, i));
            p2 = getXY(getFeaturePoint(featureIndex, i + 1));
            drawline(p1, p2);
        }
    }
}

/*******************************************/
/*             DRAW ICONS                  */
/*******************************************/

// this function takes the path to the png file, and the lat lon position
// that it should be drawn at and converts it to xy coordinates, and draws it

void draw_icons(const char* path, LatLon point, int height, int width) {
    t_point t = getXY(point);
    float X, Y;
    X = xworld_to_scrn_fl(t.x);
    Y = yworld_to_scrn_fl(t.y);
    set_coordinate_system(GL_SCREEN);

    // draw the icon centered on the lat/lon point
    draw_surface(load_png_from_file(path), X - width / 2, Y - height / 2);

    set_coordinate_system(GL_WORLD);
}

/*******************************************/
/*           POLYGON AREA                  */

/*******************************************/
double poly_area(std::vector<t_point> points) {
    double area = 0;
    unsigned size = points.size();

    // calculate the determinant -> aka the area of the polygon
    for (unsigned i = 0; i < size - 1; i++) {
        area += (points[i].x * points[i + 1].y);
        area -= (points[i + 1].x * points[i].y);
    }
    area += (points[size - 1].x * points[0].y);
    area -= (points[0].x * points[size - 1].y);

    // take the absolute value and divide it by 2
    area = std::abs(area) / 2;

    return area * 1000000;
}

/*******************************************/
/*           DRAW STREET NAMES             */

/*******************************************/

void draw_street_name(unsigned segment_id, unsigned zoom_level) {
    t_point begin = getXY(getIntersectionPosition(mapInfo.street_segment_info[segment_id].from));
    t_point end = getXY(getIntersectionPosition(mapInfo.street_segment_info[segment_id].to));

    unsigned stID = mapInfo.street_segment_info[segment_id].streetID;
    if (mapInfo.street_segment_info[segment_id].oneWay && begin.x < end.x)
        return;
    std::string name = getStreetName(stID);
    vector<unsigned> allIDs = find_street_ids_from_name(name);
    if (stID != allIDs[0])
        return;

    if (name == "<unknown>")
        return;

    t_point center;

    //unsigned curvePoints = getStreetSegmentCurvePoint(segment_id);
    unsigned curveNum = mapInfo.street_segment_info[segment_id].curvePointCount;
    if (curveNum < 2) {
        center.x = (begin.x + end.x) / 2;
        center.y = (begin.y + end.y) / 2;
    } else {
        LatLon curvePoint = getStreetSegmentCurvePoint(segment_id, curveNum / 2);
        //std::cout << curvePoint.lat() << std::endl;
        center.x = (begin.x + end.x) / 2;
        center.y = (begin.y + end.y) / 2;
    }



    double length = find_street_segment_length(segment_id);
    unsigned min = 6700 / (zoom_level * zoom_level);

    if (length < min) {
        return;
    }

    double angle = find_text_angle(begin, end);

    double boundx, boundy;
    boundx = length * cos(angle);
    boundy = length * sin(angle);

    unsigned fSize = 10 - zoom_level / 3 - name.length() / 15;
    settextrotation(angle);
    if (zoom_level > 7) {
        setcolor(t_color(255, 255, 255));
        setfontsize(fSize);
        drawtext(center, name, length / 1000000, length / 1000000);
        setcolor(t_color(10, 10, 10));
        setfontsize(fSize);
        drawtext(center, name, length / 1000000, length / 1000000);
    } else {
        setcolor(t_color(255, 255, 255));
        setfontsize(fSize);
        drawtext(center, name, (boundx / 600000) / 2, (boundy / 600000) / 2);
        setcolor(t_color(10, 10, 10));
        setfontsize(fSize);
        drawtext(center, name, (boundx / 600000) / 2, (boundy / 600000) / 2);
    }
}

/*******************************************/
/*           FIND TEXT ANGLES              */

/*******************************************/
double find_text_angle(t_point begin, t_point end) {
    double angle = 0;
    if (begin.y == end.y) {
        return 0;
    } else if (begin.x == end.x) {
        return 90;
    }

    // convert to degrees
    angle = atan((end.y - begin.y) / (end.x - begin.x))*180 / PI;

    return angle;
}

/*******************************************/
/*          ACT ON LOAD NEW MAP            */
/*******************************************/

// this function is called when the "Load map" button is pressed within the menu bar
// it asks the user whether they want to use a custom map path or an ece297 path
// it gets rid of the osm or streets ending if there is one
// and closes the current map and loads a new one

void act_on_load_new_map(void (*drawscreen_ptr) (void)) {
    
    // set all unused draw functinos to false and redraw the screen
    if (!mapInfo.map) {
        mapInfo.map = true;
        mapInfo.highlight_travel_path = false;
        mapInfo.search_type = false;
        mapInfo.directions = false;
        mapInfo.draw_pin = false;
        mapText = "";
        enable_or_disable_button(12, false);
    }
    drawscreen();
}

void load_new_map() {
    if (mapText == "") {
        update_message("No map path entered");
        return;
    }
    mapInfo.draw_pin = false;
    highlightIntersections = false;
    std::string mapPath, s, o;
    s = ".streets.bin";
    o = ".osm.bin";
    zoom_level = 0;

    // give the user the beginning of the path
    //mapText = "";

    // look for the suffix .osm and .street
    std::string::size_type streetData = mapText.find(s);
    std::string::size_type osmData = mapText.find(o);

    // if either ending is found, remove it, since load map accounts for these endings
    if (streetData != std::string::npos) {
        mapText.erase(streetData, s.length());
    }
    if (osmData != std::string::npos) {
        mapText.erase(streetData, o.length());
    }

    // set the map path

    mapPath = "/cad2/ece297s/public/maps/" + mapText;

    bool loaded = false;

    // close the current map
    close_map();

    // load a new map and set the value of loaded
    loaded = load_map(mapPath);

    // if the new map was successfully loaded, fill in the necessary info
    if (loaded) {

        // find new display coordinates
        coords = calcInitialCoordinates(true);
        set_visible_world(coords[0], coords[1], coords[2], coords[3]);

        // update the map message bar to say which map was loaded
        update_message(mapText + " was loaded");
    }
    mapInfo.map = false;
    enable_or_disable_button(12, true);
}

/*******************************************/
/*        DRAW SEGMENT DIRECTION           */

/*******************************************/

void draw_segment_direction(unsigned index) {
    //if (mapInfo.street_segment_info[index].curvePointCount > 4) return;
    t_point from = getXY(getIntersectionPosition(mapInfo.street_segment_info[index].from));
    t_point to = getXY(getIntersectionPosition(mapInfo.street_segment_info[index].to));
    bool oneway = mapInfo.street_segment_info[index].oneWay;

    string arrow;
    if (oneway)
        arrow = "↑";
    else
        arrow = "↕";
    double length = find_street_segment_length(index);
    if (length < 150) return;

    t_point center;
    center.x = (from.x + to.x) / 2;
    center.y = (from.y + to.y) / 2;

    double angle = find_text_angle(from, to) + 90;
    setcolor(t_color(100, 100, 100));
    setfontsize(9);

    settextrotation(angle);
    drawtext(center, arrow, length * cos(angle) / 1000, length * sin(angle) / 1000);

}

/*******************************************/
/*              ZOOM0 DRAW                 */

/*******************************************/
void zoom0_draw() {

    // draw the features in the order of decreasing area
    for (unsigned i = 0; i < (mapInfo.features_by_area.size()); i++) {
        draw_features(mapInfo.open_features[mapInfo.features_by_area[i].first].index, mapInfo.open_features[mapInfo.features_by_area[i].first].colour, mapInfo.open_features[mapInfo.features_by_area[i].first].closed);
    }

    // draw major roads
    for (unsigned j = 0; j < mapInfo.major_roads.size(); j++) {
        draw_street_segment(mapInfo.major_roads[j], WHITE, 3);

    }

    // draw highways on top
    for (unsigned j = 0; j < mapInfo.highways.size(); j++) {
        draw_street_segment(mapInfo.highways[j], ORANGE, 4);
    }

}

/*******************************************/
/*             ZOOM1 DRAW                  */

/*******************************************/
void zoom1_draw() {
    zoom0_draw();
}

/*******************************************/
/*             ZOOM2 DRAW                  */

/*******************************************/
void zoom2_draw() {
    // draw the features from the outermost zoom level
    zoom0_draw();
    //draw_poi(mapInfo.features_by_area.size() / 50);
    for (unsigned j = 0; j < mapInfo.major_roads.size(); j++) {
        draw_street_name(mapInfo.major_roads[j], 2);
        //if (j%2 == 0)
        //    draw_segment_direction(mapInfo.major_roads[j]);
    }
    for (unsigned j = 0; j < mapInfo.highways.size(); j++) {
        draw_street_name(mapInfo.highways[j], 3);
        //if (j%2 == 0)
        //    draw_segment_direction(mapInfo.highways[j]);
    }

}

/*******************************************/
/*               ZOOM3 DRAW                */

/*******************************************/
void zoom3_draw() {

    for (unsigned i = 0; i < (mapInfo.features_by_area.size()); i++) {
        draw_features(mapInfo.open_features[mapInfo.features_by_area[i].first].index, mapInfo.open_features[mapInfo.features_by_area[i].first].colour, mapInfo.open_features[mapInfo.features_by_area[i].first].closed);
    }
    
    

    for (unsigned j = 0; j < mapInfo.major_roads.size(); j++) {
        draw_street_segment(mapInfo.major_roads[j], WHITE, 4);
        //draw_street_name(mapInfo.major_roads[j], 3);
    }
    for (unsigned j = 0; j < mapInfo.highways.size(); j++) {
        draw_street_segment(mapInfo.highways[j], ORANGE, 5);
        //draw_street_name(mapInfo.highways[j], 3);
    }
    //draw_poi(mapInfo.features_by_area.size() / 100);
    for (unsigned j = 0; j < mapInfo.major_roads.size(); j++) {
        draw_street_name(mapInfo.major_roads[j], 3);
        //if (j%2 == 0)
        //    draw_segment_direction(mapInfo.major_roads[j]);
    }
    for (unsigned j = 0; j < mapInfo.highways.size(); j++) {
        draw_street_name(mapInfo.highways[j], 3);
        //if (j%2 == 0)
        //    draw_segment_direction(mapInfo.highways[j]);
    }

}

/*******************************************/
/*            ZOOM4 DRAW                   */

/*******************************************/
void zoom4_draw() {
for (unsigned i = 0; i < (mapInfo.features_by_area.size()); i++) {
        draw_features(mapInfo.open_features[mapInfo.features_by_area[i].first].index, mapInfo.open_features[mapInfo.features_by_area[i].first].colour, mapInfo.open_features[mapInfo.features_by_area[i].first].closed);
    }

    for (unsigned j = 0; j < mapInfo.major_roads.size(); j++) {
        draw_street_segment(mapInfo.major_roads[j], WHITE, 4);
        //        draw_street_name(mapInfo.major_roads[j], 4);
        //        if (j % 2 == 0)
        //            draw_segment_direction(mapInfo.major_roads[j]);
    }
    for (unsigned j = 0; j < mapInfo.highways.size(); j++) {
        draw_street_segment(mapInfo.highways[j], ORANGE, 5);
        //        draw_street_name(mapInfo.highways[j], 3);
        //        if (j % 2 == 0)
        //            draw_segment_direction(mapInfo.highways[j]);
    }
    //draw_poi(mapInfo.features_by_area.size() / 200);
    for (unsigned i = 0; i < mapInfo.subway_stations.size(); i++) {
        draw_icons("/nfs/ug/homes-5/r/roscoeol/mapper_repo/mapper/libstreetmap/src/icons/transit.png", mapInfo.subway_stations[i], 20, 20);
    }
    for (unsigned j = 0; j < mapInfo.major_roads.size(); j++) {
        draw_street_name(mapInfo.major_roads[j], 4);
        if (j % 2 == 0)
            draw_segment_direction(mapInfo.major_roads[j]);
    }
    for (unsigned j = 0; j < mapInfo.highways.size(); j++) {
        draw_street_name(mapInfo.highways[j], 3);
        if (j % 2 == 0)
            draw_segment_direction(mapInfo.highways[j]);
    }



}

/*******************************************/
/*                ZOOM5 DRAW               */

/*******************************************/

void zoom5_draw() {
    double max = mapInfo.major_roads.size();
    if (mapInfo.minor_roads.size() > max) {
        max = mapInfo.minor_roads.size();
    }

for (unsigned i = 0; i < (mapInfo.features_by_area.size()); i++) {
        draw_features(mapInfo.open_features[mapInfo.features_by_area[i].first].index, mapInfo.open_features[mapInfo.features_by_area[i].first].colour, mapInfo.open_features[mapInfo.features_by_area[i].first].closed);
    }

    for (unsigned j = 0; j < max; j++) {
        if (j < mapInfo.major_roads.size())
            draw_street_segment(mapInfo.major_roads[j], WHITE, 6);
        if (j < mapInfo.minor_roads.size())
            draw_street_segment(mapInfo.minor_roads[j], WHITE, 3);
    }

    for (unsigned j = 0; j < mapInfo.highways.size(); j++)
        draw_street_segment(mapInfo.highways[j], ORANGE, 7);

    for (unsigned i = 0; i < mapInfo.subway_stations.size(); i++) {
        draw_icons("/nfs/ug/homes-5/r/roscoeol/mapper_repo/mapper/libstreetmap/src/icons/transit.png", mapInfo.subway_stations[i], 20, 20);
    }

    double maxName = max;
    if (mapInfo.highways.size() > maxName) {
        maxName = mapInfo.highways.size();
    }
    for (unsigned j = 0; j < maxName; j++) {
        if (j < mapInfo.major_roads.size()) {
            draw_street_name(mapInfo.major_roads[j], 5);
            if (j % 2 == 0)
                draw_segment_direction(mapInfo.major_roads[j]);
        }
        if (j < mapInfo.highways.size()) {
            draw_street_name(mapInfo.highways[j], 5);
            if (j % 2 == 0)
                draw_segment_direction(mapInfo.highways[j]);
        }
    }
    //draw_poi(mapInfo.features_by_area.size() / 400);

}


/*******************************************/
/*                ZOOM6 DRAW               */

/*******************************************/
void zoom6_draw() {
    double max = mapInfo.major_roads.size();
    if (mapInfo.minor_roads.size() > max) {
        max = mapInfo.minor_roads.size();
    }

for (unsigned i = 0; i < (mapInfo.features_by_area.size()); i++) {
        draw_features(mapInfo.open_features[mapInfo.features_by_area[i].first].index, mapInfo.open_features[mapInfo.features_by_area[i].first].colour, mapInfo.open_features[mapInfo.features_by_area[i].first].closed);
    }

    for (unsigned i = 0; i < (mapInfo.zero_area_features.size()); i++) {
        draw_features(mapInfo.zero_area_features[i].index, mapInfo.zero_area_features[i].colour, mapInfo.zero_area_features[i].closed);
    }
    
    for (unsigned j = 0; j < max; j++) {
        if (j < mapInfo.major_roads.size())
            draw_street_segment(mapInfo.major_roads[j], WHITE, 6);
        if (j < mapInfo.minor_roads.size())
            draw_street_segment(mapInfo.minor_roads[j], WHITE, 5);
    }

    for (unsigned j = 0; j < mapInfo.highways.size(); j++)
        draw_street_segment(mapInfo.highways[j], ORANGE, 7);


    for (unsigned i = 0; i < mapInfo.subway_stations.size(); i++) {
        draw_icons("/nfs/ug/homes-5/r/roscoeol/mapper_repo/mapper/libstreetmap/src/icons/transit.png", mapInfo.subway_stations[i], 20, 20);
    }
    //draw_poi(5);

    double maxName = max;
    if (mapInfo.highways.size() > maxName) {
        maxName = mapInfo.highways.size();
    }
    for (unsigned j = 0; j < maxName; j++) {
        if (j < mapInfo.major_roads.size()) {
            draw_street_name(mapInfo.major_roads[j], 6);
            if (j % 2 == 0)
                draw_segment_direction(mapInfo.major_roads[j]);
        }
        if (j < mapInfo.highways.size()) {
            draw_street_name(mapInfo.highways[j], 6);
            if (j % 2 == 0)
                draw_segment_direction(mapInfo.highways[j]);
        }
    }

}

/*******************************************/
/*                ZOOM7 DRAW               */

/*******************************************/
void zoom7_draw() {
    double max = mapInfo.major_roads.size();
    if (mapInfo.minor_roads.size() > max) {
        max = mapInfo.minor_roads.size();
    }

    
for (unsigned i = 0; i < (mapInfo.features_by_area.size()); i++) {
        draw_features(mapInfo.open_features[mapInfo.features_by_area[i].first].index, mapInfo.open_features[mapInfo.features_by_area[i].first].colour, mapInfo.open_features[mapInfo.features_by_area[i].first].closed);
    }
    for (unsigned i = 0; i < (mapInfo.zero_area_features.size()); i++) {
        draw_features(mapInfo.zero_area_features[i].index, mapInfo.zero_area_features[i].colour, mapInfo.zero_area_features[i].closed);
    }

    for (unsigned j = 0; j < max; j++) {
        if (j < mapInfo.major_roads.size())
            draw_street_segment(mapInfo.major_roads[j], WHITE, 6);
        if (j < mapInfo.minor_roads.size())
            draw_street_segment(mapInfo.minor_roads[j], WHITE, 5);
    }

    for (unsigned j = 0; j < mapInfo.highways.size(); j++)
        draw_street_segment(mapInfo.highways[j], ORANGE, 7);

    for (unsigned i = 0; i < mapInfo.subway_stations.size(); i++) {
        draw_icons("/nfs/ug/homes-5/r/roscoeol/mapper_repo/mapper/libstreetmap/src/icons/transit.png", mapInfo.subway_stations[i], 20, 20);
    }
    //draw_poi(2);

    double maxName = max;
    if (mapInfo.highways.size() > maxName) {
        maxName = mapInfo.highways.size();
    }
    for (unsigned j = 0; j < maxName; j++) {
        if (j < mapInfo.major_roads.size()) {
            draw_street_name(mapInfo.major_roads[j], 7);
            if (j % 2 == 0)
                draw_segment_direction(mapInfo.major_roads[j]);
        }
        if (j < mapInfo.highways.size()) {
            draw_street_name(mapInfo.highways[j], 7);
            if (j % 2 == 0)
                draw_segment_direction(mapInfo.highways[j]);
        }
        if (j < mapInfo.minor_roads.size()) {
            draw_street_name(mapInfo.minor_roads[j], 7);
            if (j % 2 == 0)
                draw_segment_direction(mapInfo.minor_roads[j]);
        }
    }
}

/*******************************************/
/*                ZOOM8 DRAW               */

/*******************************************/
void zoom8_draw() {
    double max = mapInfo.major_roads.size();
    if (mapInfo.minor_roads.size() > max) {
        max = mapInfo.minor_roads.size();
    }

for (unsigned i = 0; i < (mapInfo.features_by_area.size()); i++) {
        draw_features(mapInfo.open_features[mapInfo.features_by_area[i].first].index, mapInfo.open_features[mapInfo.features_by_area[i].first].colour, mapInfo.open_features[mapInfo.features_by_area[i].first].closed);
    }
    for (unsigned i = 0; i < (mapInfo.zero_area_features.size()); i++) {
        draw_features(mapInfo.zero_area_features[i].index, mapInfo.zero_area_features[i].colour, mapInfo.zero_area_features[i].closed);
    }

    for (unsigned j = 0; j < max; j++) {
        if (j < mapInfo.major_roads.size())
            draw_street_segment(mapInfo.major_roads[j], WHITE, 10);
        if (j < mapInfo.minor_roads.size())
            draw_street_segment(mapInfo.minor_roads[j], WHITE, 7);
    }

    for (unsigned j = 0; j < mapInfo.highways.size(); j++)
        draw_street_segment(mapInfo.highways[j], ORANGE, 10);

    for (unsigned i = 0; i < mapInfo.buildings.size(); i++) {
        bool closed = is_closed(i);
        draw_features(mapInfo.buildings[i], t_color(222, 198, 182), closed);
    }
    for (unsigned i = 0; i < mapInfo.subway_stations.size(); i++) {
        draw_icons("/nfs/ug/homes-5/r/roscoeol/mapper_repo/mapper/libstreetmap/src/icons/transit.png", mapInfo.subway_stations[i], 20, 20);
    }

    double maxName = max;
    if (mapInfo.highways.size() > maxName) {
        maxName = mapInfo.highways.size();
    }
    for (unsigned j = 0; j < maxName; j++) {
        if (j < mapInfo.major_roads.size()) {
            draw_street_name(mapInfo.major_roads[j], 8);
            if (j % 2 == 0)
                draw_segment_direction(mapInfo.major_roads[j]);
        }
        if (j < mapInfo.highways.size()) {
            draw_street_name(mapInfo.highways[j], 8);
            if (j % 2 == 0)
                draw_segment_direction(mapInfo.highways[j]);
        }
        if (j < mapInfo.minor_roads.size()) {
            draw_street_name(mapInfo.minor_roads[j], 8);
            if (j % 2 == 0)
                draw_segment_direction(mapInfo.minor_roads[j]);
        }
    }
}

/*******************************************/
/*                ZOOM9 DRAW               */

/*******************************************/
void zoom9_draw() {
    double max = mapInfo.major_roads.size();
    if (mapInfo.minor_roads.size() > max) {
        max = mapInfo.minor_roads.size();
    }

for (unsigned i = 0; i < (mapInfo.features_by_area.size()); i++) {
        draw_features(mapInfo.open_features[mapInfo.features_by_area[i].first].index, mapInfo.open_features[mapInfo.features_by_area[i].first].colour, mapInfo.open_features[mapInfo.features_by_area[i].first].closed);
    }

    for (unsigned i = 0; i < (mapInfo.zero_area_features.size()); i++) {
        draw_features(mapInfo.zero_area_features[i].index, mapInfo.zero_area_features[i].colour, mapInfo.zero_area_features[i].closed);
    }
    
    for (unsigned j = 0; j < max; j++) {
        if (j < mapInfo.major_roads.size())
            draw_street_segment(mapInfo.major_roads[j], WHITE, 10);
        if (j < mapInfo.minor_roads.size())
            draw_street_segment(mapInfo.minor_roads[j], WHITE, 7);
    }

    for (unsigned j = 0; j < mapInfo.highways.size(); j++)
        draw_street_segment(mapInfo.highways[j], ORANGE, 10);

    for (unsigned i = 0; i < mapInfo.buildings.size(); i++) {
        bool closed = is_closed(i);
        draw_features(mapInfo.buildings[i], t_color(222, 198, 182), closed);
    }
    for (unsigned i = 0; i < mapInfo.subway_stations.size(); i++) {
        draw_icons("/nfs/ug/homes-5/r/roscoeol/mapper_repo/mapper/libstreetmap/src/icons/transit.png", mapInfo.subway_stations[i], 20, 20);
    }

    double maxName = max;
    if (mapInfo.highways.size() > maxName) {
        maxName = mapInfo.highways.size();
    }
    for (unsigned j = 0; j < maxName; j++) {
        if (j < mapInfo.major_roads.size()) {
            draw_street_name(mapInfo.major_roads[j], 9);
            if (j % 2 == 0)
                draw_segment_direction(mapInfo.major_roads[j]);
        }
        if (j < mapInfo.highways.size()) {
            draw_street_name(mapInfo.highways[j], 9);
            if (j % 2 == 0)
                draw_segment_direction(mapInfo.highways[j]);
        }
        if (j < mapInfo.minor_roads.size()) {
            draw_street_name(mapInfo.minor_roads[j], 9);
            if (j % 2 == 0)
                draw_segment_direction(mapInfo.minor_roads[j]);
        }
    }

}

/*******************************************/
/*                ZOOM10 DRAW               */

/*******************************************/
void zoom10_draw() {
    zoom9_draw();
}

/*******************************************/
/*          HIGHLIGHT FEATURE              */

/*******************************************/
void highlight_feature(unsigned featureIndex) {
    bool closed = is_closed(featureIndex);
    if (closed) {
        draw_features(featureIndex, t_color(255, 0, 0, 255 / 3), closed);
        draw_features(featureIndex, t_color(255, 0, 0), false);
    } else {
        draw_features(featureIndex, t_color(255, 0, 0), false);
    }
}

// draws the street in semi-transparent red

void highlight_street(unsigned streetIndex) {
    std::vector<unsigned> segsToHighlight;
    segsToHighlight = find_street_street_segments(streetIndex);

    for (unsigned i = 0; i < segsToHighlight.size(); i++) {
        draw_street_segment(segsToHighlight[i], t_color(255, 0, 0, 255 / 3), 7);
    }
}

/*******************************************/
/*        HIGHLIGHT INTERSECTION           */

/*******************************************/
void highlight_intersection(unsigned intersectionIndex) {
    t_point position = getXY(mapInfo.intersection_position[intersectionIndex]);

    setcolor(t_color(255, 0, 0, 255 / 3));
    fillarc(position.x, position.y, 0.000005, 0, 360);
    setlinewidth(2);
    setcolor(t_color(255, 0, 0));
    drawarc(position.x, position.y, 0.000005, 0, 360);

}

/*******************************************/
/*            HIGHLIGHT POI                */
/*******************************************/
void highlight_POI(unsigned POIindex) {
    t_point position = getXY(mapInfo.poi_position[POIindex]);

    //    setcolor(t_color(255, 0, 0));
    //    fillarc(position.x, position.y, 0.000005, 0, 360);

    setcolor(t_color(255, 0, 0));

    drawarc(position.x, position.y, 0.000005, 0, 360);

}


/*******************************************/
/*          ACT ON KEY PRESS               */
/*******************************************/
#include <X11/keysym.h>

void act_on_key_press(char c, int keysym) {
    // function to handle keyboard press event, the ASCII character is returned
    // along with an extended code (keysym) on X11 to represent non-ASCII
    // characters like the arrow keys.
    
    // add typed character to the current string 
    if (mapInfo.search_type) {
        searchText.push_back(c);
    } else if (mapInfo.directions) {
        if (mapInfo.start) {
            mapInfo.startLoc.push_back(c);
        } else if (mapInfo.end) {
            mapInfo.endLoc.push_back(c);
        }
    } else if (mapInfo.map) {
        mapText.push_back(c);
    }
#ifdef X11 // Extended keyboard codes only supported for X11 for now
    switch (keysym) {
        case XK_Left:
            translate_left;
            break;
        case XK_Right:
            translate_right;
            break;
        case XK_Up:
            translate_up;
            break;
        case XK_Down:
            translate_down;
            break;
        case XK_BackSpace:
            //remove the backspace character from the string once pressed
            if (mapInfo.search_type) {
                if (searchText.length() != 0)
                    searchText.pop_back();
            } else if (mapInfo.directions) {
                if (mapInfo.start) {
                    if (mapInfo.startLoc.length() != 0)
                        mapInfo.startLoc.pop_back();
                } else if (mapInfo.end) {
                    if (mapInfo.endLoc.length() != 0)
                        mapInfo.endLoc.pop_back();
                }
            } else if (mapInfo.map) {
                if (mapText.length() != 0)
                    mapText.pop_back();
            }
            break;
        case XK_Shift_L:
            // remove the shift character from the string
            if (mapInfo.search_type) {
                searchText.pop_back();
            } else if (mapInfo.directions) {
                if (mapInfo.start) {
                    mapInfo.startLoc.pop_back();
                } else if (mapInfo.end) {
                    mapInfo.endLoc.pop_back();
                }
            } else if (mapInfo.map) {
                mapText.pop_back();
            }
            break;
        case XK_Shift_R:
            // remove the shift character from the string
            if (mapInfo.search_type) {
                searchText.pop_back();
            } else if (mapInfo.directions) {
                if (mapInfo.start) {
                    mapInfo.startLoc.pop_back();
                } else if (mapInfo.end) {
                    mapInfo.endLoc.pop_back();
                }
            } else if (mapInfo.map) {
                mapText.pop_back();
            }
            break;
        default:

            break;

            //std::cout << "keysym (extended code) is " << keysym << std::endl;
    }

    // run delete again to take an extra character off the string
    if (keysym == 65288) {
        if (mapInfo.search_type) {
            if (searchText.length() != 0)
                searchText.pop_back();
        } else if (mapInfo.directions) {
            if (mapInfo.start) {
                if (mapInfo.startLoc.length() != 0)
                    mapInfo.startLoc.pop_back();
            } else if (mapInfo.end) {
                if (mapInfo.endLoc.length() != 0)
                    mapInfo.endLoc.pop_back();
            }
        } else if (mapInfo.map) {
            if (mapText.length() != 0)
                mapText.pop_back();
        }
    }
    mapInfo.action = true;
    drawscreen();
#endif
}

void act_on_mouse_move(float x, float y) {
    // function to handle mouse move event, the current mouse position in the current world coordinate
    // system (as defined in your call to init_world) is returned
    //std::cout << "x: " << x << "   y: " << y <<std::endl;

    float x1, y1;
    x1 = xworld_to_scrn_fl(x);
    y1 = yworld_to_scrn_fl(y);

    
    // if mouse is on the search icon, turn on search hover        
    if (x1 >= 270 && x1 <= 320 && y1 <= 60 && y1 >= 10) {
        mapInfo.draw_search_hover = true;
        mapInfo.draw_directions_hover = false;
        drawscreen();
    } 
    // if the mouse is on the directions icon, turn on directions hover
    else if (x1 >= 320 && x1 <= 370 && y1 <= 60 && y1 >= 10) {
        mapInfo.draw_search_hover = false;
        mapInfo.draw_directions_hover = true;
        drawscreen();
    } 
    // otherwise turn hover off
    else {
        mapInfo.draw_search_hover = false;
        mapInfo.draw_directions_hover = false;
        drawscreen();
    }

}
/*******************************************/
/*         ACT ON MOUSEBUTTON              */

/*******************************************/
void act_on_mousebutton(float x, float y, t_event_buttonPressed button_info) {

    //convert to screen coordinates
    mapInfo.clickX = xworld_to_scrn_fl(x);
    mapInfo.clickY = yworld_to_scrn_fl(y);
    mapInfo.action = true;


    // if the click was on the search bar
    if (mapInfo.clickX <= 270 && mapInfo.clickX >= 10 && mapInfo.clickY <= 60 && mapInfo.clickY >= 10) {

        //SEARCH BAR CLICKED!
        mapInfo.draw_pin = false;
        mapInfo.search_type = true;

        // If it's in directions mode, not searching
        if (mapInfo.directions) {
            mapInfo.search_type = false;
            mapInfo.start = true;
            mapInfo.end = false;

            if (ACLICK) {
                mapInfo.startLoc = "";
                ACLICK = false;
            }
        } else {
            mapInfo.start = false;
        }

    }// if the click was in the area of the search icon
    else if (mapInfo.clickX >= 270 && mapInfo.clickX <= 320 && mapInfo.clickY <= 60 && mapInfo.clickY >= 10) {
        if (searchText.length() > 0) {
            find(searchText);
            update_message("Finding intersection...");
            searchText = "";
        } else {
            update_message("Please enter a location to find");
        }
        mapInfo.draw_pin = false;

    }
    // if the click was in the area of the directions icon
    else if (mapInfo.clickX >= 320 && mapInfo.clickX <= 370 && mapInfo.clickY <= 60 && mapInfo.clickY >= 10) {
        mapInfo.draw_pin = false;
        mapInfo.search_type = false;
        
        // if user clicks this location while highlight is on, user clicked x from directions to close the panel
        if (mapInfo.highlight_travel_path) {
            mapInfo.highlight_travel_path = false;
        }
        if (!mapInfo.directions) {
            // if directions is clicked and not on, turn it on
            mapInfo.directions = true;
        } else {
            // if you turn directions off, clear the strings and turn off highlight
            mapInfo.directions = false;
            mapInfo.startLoc = "";
            mapInfo.endLoc = "";
            mapInfo.highlight_travel_path = false;
        }
        mapInfo.start = true;

    }// user clicked on end location enter
    else if (mapInfo.clickX >= 10 && mapInfo.clickX <= 280 && mapInfo.clickY >= 60 && mapInfo.clickY <= 120 && mapInfo.directions) {
        mapInfo.start = false;
        mapInfo.end = true;

        // if user clicks on end when end holds a clicked point, clear the clicked point for user input
        if (BCLICK) {
            mapInfo.endLoc = "";
            BCLICK = false;
        }
    }// USER CLICKED SWITCH for directions, switches start point and end point
    else if (mapInfo.clickX >= 330 && mapInfo.clickX <= 370 && mapInfo.clickY >= 60 && mapInfo.clickY <= 120 && mapInfo.directions) {
        std::string temp = mapInfo.startLoc;
        mapInfo.startLoc = mapInfo.endLoc;
        mapInfo.endLoc = temp;

        LatLon tempPos = Aclick;
        Aclick = Bclick;
        Bclick = tempPos;

        bool tempBool = ACLICK;
        ACLICK = BCLICK;
        BCLICK = tempBool;
    }// User clicked Go from LOAD NEW MAP
    else if (mapInfo.clickX >= 630 && mapInfo.clickX <= 670 && mapInfo.clickY >= 60 && mapInfo.clickY <= 120 && mapInfo.map) {
        update_message("Loading new map");
        load_new_map();
    }// user clicked the GO button in directions
    else if (mapInfo.clickX >= 280 && mapInfo.clickX <= 330 && mapInfo.clickY >= 60 && mapInfo.clickY <= 120 && mapInfo.directions) {
        bool startValid = false;
        bool endValid = false;
        mapInfo.highlight_travel_path = true;
        //
        update_message("Finding path");
        // if start holds a click value, find the closest intersection to start
        if (ACLICK) {
            interA = find_closest_intersection(Aclick);
            startValid = true;
        } 
        // otherwise check if the string is a valid intersection
        else {
            // check if start has a string in
            if (mapInfo.startLoc.length() != 0) {
                find(mapInfo.startLoc);
                
                // if intersections are found, set the first one in the vector to start
                if (found_intersections.size() != 0) {
                    interA = found_intersections[0];
                    startValid = true;
                } 
                // otherwise location isn't found
                else {
                    std::cout << "Error: Start Location Not Found!\n";
                    startValid = false;
                }
            }
        }

        // if end holds a click value, find the closest intersection to end
        if (BCLICK) {
            interB = find_closest_intersection(Bclick);
            endValid = true;
        } 
        
        // otherwise check if the string is a valid intersection
        else {
            // check if end has a string in
            if (mapInfo.endLoc.length() != 0) {
                find(mapInfo.endLoc);
                
                // if intersections are found, set the first one in the vector to end
                if (found_intersections.size() != 0) {
                    interB = found_intersections[0];
                    endValid = true;
                } 
                // otherwise check if the end point is a poi
                else {
                    mapInfo.found_poi = find_pois_from_name(mapInfo.endLoc);
                    if (mapInfo.found_poi.size() != 0) {
                        mapInfo.poi_travel_path = true;
                        endValid = true;
                    } else {
                        // otherwise location isn't found
                        std::cout << "Error: Destination Not Found!\n";
                        endValid = false;
                    }
                }
            }
        }

        // if both are found, find path between them and zoom in to the first intersection
        if (startValid && endValid) {
            if (mapInfo.poi_travel_path) {
                mapInfo.pathIDs = find_path_to_point_of_interest(interA, mapInfo.endLoc, 15);
                mapInfo.poi_travel_path = false;
            } else {
                mapInfo.pathIDs = find_path_between_intersections(interA, interB, 15);
            }
            t_point pos = getXY(mapInfo.intersection_position[interA]);
            zoom_fit(drawscreen);
            while (zoom_level < 5) {
                handle_zoom_in(pos.x, pos.y, drawscreen);
            }
            le_directions(mapInfo.pathIDs);
        } else {
            update_message("Invalid location");
            mapInfo.highlight_travel_path = false;
            mapInfo.startLoc = "";
            mapInfo.endLoc = "";
            mapInfo.directions = false;
        }
        //mapInfo.directions = false;
        ACLICK = false;
        BCLICK = false;
    }// if the user clicks somewhere on the map
    else {
        pointClick = getLatLon(x, y);
        intersectionClick = find_closest_intersection(pointClick);

        //If in directions mode, sets the start/end point to the location clicked
        if (mapInfo.directions) {
            if (mapInfo.start) {
                if (mapInfo.startLoc.length() == 0) {
                    mapInfo.startLoc = std::to_string(pointClick.lat()) + ", " + std::to_string(pointClick.lon());
                    mapInfo.start = false;
                    Aclick = pointClick;
                    ACLICK = true;
                }
                if (mapInfo.endLoc.length() == 0) {
                    mapInfo.end = true;
                }
            } else {
                if (mapInfo.endLoc.length() == 0) {
                    mapInfo.endLoc = std::to_string(pointClick.lat()) + ", " + std::to_string(pointClick.lon());
                    mapInfo.end = false;
                    Bclick = pointClick;
                    BCLICK = true;
                }
                if (mapInfo.startLoc.length() == 0) {
                    mapInfo.start = true;
                }
            }
        } else {
            // if rest of map is clicked, set the inputs to false
            mapInfo.search_type = false;

            // update the information panel with the information on the closest points to where the user clicked
            poiClick = find_closest_point_of_interest(pointClick);

            mapInfo.draw_pin = true;
            mapInfo.action = false;
        }
    }

    drawscreen();

}
/*******************************************/
/*           DRAW POI NAME                 */
/*******************************************/
void show_poi_names(void (*drawscreen_ptr) (void)) {
    if (!mapInfo.poi_name_drawn) {
        mapInfo.poi_name_drawn = true;
    } else {
        mapInfo.poi_name_drawn = false;
    }
    drawscreen();
}


/*******************************************/
/*      DRAW POIs FROM BUTTON              */
/*******************************************/
void more_poi_display(void (*drawscreen_ptr) (void)) {
    char old_button_name[200], new_button_name[200];

    if (!mapInfo.more_pois) {
        sprintf(old_button_name, "Show POIs");
        sprintf(new_button_name, "Hide POIs");
        change_button_text(old_button_name, new_button_name);

        mapInfo.more_pois = true;
    } else {
        sprintf(old_button_name, "Hide POIs");
        sprintf(new_button_name, "Show POIs");
        change_button_text(old_button_name, new_button_name);

        mapInfo.more_pois = false;
    }
    drawscreen();
}
/*******************************************/
/*           DRAW POI NAME                 */
/*******************************************/
void draw_poi_name(unsigned poi) {
    if (!mapInfo.poi_name_drawn)
        return;
    t_point position = getXY(mapInfo.poi_position[poi]);
    string name = getPointOfInterestName(poi);
    setcolor(BLACK);
    position.y += 0.0001 / zoom_level;
    setfontsize(7);
    settextrotation(0);
    drawtext(position, name, 1000, 1000);
}

/*******************************************/
/*              DRAW POI                   */

/*******************************************/
void draw_poi(int amount) {
    int max = mapInfo.schools.size();
    if (mapInfo.banks.size() > max) {
        max = mapInfo.banks.size();
    } else if (mapInfo.restaurants.size() > max) {
        max = mapInfo.restaurants.size();
    } else if (mapInfo.cafes.size() > max) {
        max = mapInfo.cafes.size();
    } else if (mapInfo.hospitals.size() > max) {
        max = mapInfo.hospitals.size();
    } else if (mapInfo.hotels.size() > max) {
        max = mapInfo.hotels.size();
    }

    //    if (mapInfo.other.size() > max && mapInfo.more_pois) {
    //        max = mapInfo.other.size();
    //    }

    //print out all the pois in the vector
    for (unsigned i = 0; i < max; i += amount) {
        if (i < mapInfo.schools.size()) {
            if (zoom_level > 6) draw_poi_name(mapInfo.schools[i]);
            draw_icons("/nfs/ug/homes-5/r/roscoeol/mapper_repo/mapper/libstreetmap/src/icons/school.png", mapInfo.poi_position[mapInfo.schools[i]], 20, 20);
            draw_poi_name(mapInfo.schools[i]);
        }
        if (i < mapInfo.banks.size()) {
            if (zoom_level > 6) draw_poi_name(mapInfo.banks[i]);
            draw_icons("/nfs/ug/homes-5/r/roscoeol/mapper_repo/mapper/libstreetmap/src/icons/bank.png", mapInfo.poi_position[mapInfo.banks[i]], 20, 20);
            draw_poi_name(mapInfo.banks[i]);
        }
        if (i < mapInfo.restaurants.size()) {
            if (zoom_level > 6) draw_poi_name(mapInfo.restaurants[i]);
            draw_icons("/nfs/ug/homes-5/r/roscoeol/mapper_repo/mapper/libstreetmap/src/icons/restaurant.png", mapInfo.poi_position[mapInfo.restaurants[i]], 20, 20);
            draw_poi_name(mapInfo.restaurants[i]);
        }
        if (i < mapInfo.cafes.size()) {
            if (zoom_level > 6) draw_poi_name(mapInfo.cafes[i]);
            draw_icons("/nfs/ug/homes-5/r/roscoeol/mapper_repo/mapper/libstreetmap/src/icons/cafe.png", mapInfo.poi_position[mapInfo.cafes[i]], 20, 20);
            draw_poi_name(mapInfo.cafes[i]);
        }
        if (i < mapInfo.hospitals.size()) {
            if (zoom_level > 6) draw_poi_name(mapInfo.hospitals[i]);

            draw_icons("/nfs/ug/homes-5/r/roscoeol/mapper_repo/mapper/libstreetmap/src/icons/hospital.png", mapInfo.poi_position[mapInfo.hospitals[i]], 20, 20);
            draw_poi_name(mapInfo.hospitals[i]);
        }
        if (i < mapInfo.hotels.size()) {
            if (zoom_level > 6) draw_poi_name(mapInfo.hotels[i]);

            draw_icons("/nfs/ug/homes-5/r/roscoeol/mapper_repo/mapper/libstreetmap/src/icons/hotel.png", mapInfo.poi_position[mapInfo.hotels[i]], 20, 20);
            draw_poi_name(mapInfo.hotels[i]);
        }
        //
        //        if (mapInfo.more_pois) {
        //            if (i < mapInfo.other.size()) {
        //                setcolor(153, 0, 153);
        //                t_point position = getXY(mapInfo.poi_position[mapInfo.other[i]]);
        //                if (zoom_level > 8) draw_poi_name(mapInfo.other[i]);
        //                fillarc(position.x, position.y, 0.0000005, 0, 360);
        //            }
        //        }
    }

}


/*******************************************/
/*              DRAW SCHOOLS               */

/*******************************************/
void draw_schools(float amount) {
    for (unsigned i = 0; i < (mapInfo.schools.size()) * amount; i++) {
        unsigned id = mapInfo.schools[i];
        draw_icons("/nfs/ug/homes-5/r/roscoeol/mapper_repo/mapper/libstreetmap/src/icons/school.png", mapInfo.poi_position[id], 20, 20);
    }
}

/*******************************************/
/*              DRAW BANKS                 */

/*******************************************/
void draw_banks(float amount) {
    for (unsigned i = 0; i < (mapInfo.banks.size()) * amount; i++) {
        unsigned id = mapInfo.banks[i];
        draw_icons("/nfs/ug/homes-5/r/roscoeol/mapper_repo/mapper/libstreetmap/src/icons/bank.png", mapInfo.poi_position[id], 20, 20);
    }
}

/*******************************************/
/*              DRAW CAFES                 */

/*******************************************/
void draw_cafes(float amount) {
    for (unsigned i = 0; i < (mapInfo.cafes.size()) * amount; i++) {
        unsigned id = mapInfo.cafes[i];
        draw_icons("/nfs/ug/homes-5/r/roscoeol/mapper_repo/mapper/libstreetmap/src/icons/cafe.png", mapInfo.poi_position[id], 20, 20);
    }
}

/*******************************************/
/*             DRAW HOTELS                 */

/*******************************************/
void draw_hotels(float amount) {
    for (unsigned i = 0; i < (mapInfo.hotels.size()) * amount; i++) {
        unsigned id = mapInfo.hotels[i];
        draw_icons("/nfs/ug/homes-5/r/roscoeol/mapper_repo/mapper/libstreetmap/src/icons/hotel.png", mapInfo.poi_position[id], 20, 20);
    }
}

/*******************************************/
/*            DRAW RESTAURANTS             */

/*******************************************/
void draw_restaurants(float amount) {
    for (unsigned i = 0; i < (mapInfo.restaurants.size()) * amount; i++) {
        unsigned id = mapInfo.restaurants[i];
        draw_icons("/nfs/ug/homes-5/r/roscoeol/mapper_repo/mapper/libstreetmap/src/icons/restaurant.png", mapInfo.poi_position[id], 20, 20);
    }
}

/*******************************************/
/*            DRAW HOSPITALS               */

/*******************************************/
void draw_hospitals(float amount) {
    for (unsigned i = 0; i < (mapInfo.hospitals.size()) * amount; i++) {
        unsigned id = mapInfo.hospitals[i];
        draw_icons("/nfs/ug/homes-5/r/roscoeol/mapper_repo/mapper/libstreetmap/src/icons/hospital.png", mapInfo.poi_position[id], 20, 20);
    }
}

/*******************************************/
/*             DRAW OTHER                  */

/*******************************************/
void draw_other(float amount) {
    for (unsigned i = 0; i < (mapInfo.other.size()) * amount; i++) {
        int type = mapInfo.poi_type[i];
        t_point position = getXY(mapInfo.poi_position[i]);
        setcolor(153, 0, 153);

        fillarc(position.x, position.y, 0.0000005, 0, 360);
    }
}

/*******************************************/
/*              DRAW PIN                   */

/*******************************************/

// this function is not commented because we didn't end up calling it - unused
//void draw_pin() {
//    if (mapInfo.draw_pin && zoom_level >= 1) {
//        std::cout << "CLICKED\n";
//        draw_icons("/nfs/ug/homes-5/r/roscoeol/mapper_repo/mapper/libstreetmap/src/icons/pin.png", pointClick, 70, 35);
//
//
//        bool drawName, drawStreet;
//        int fontSize = 12;
//
//        setcolor(WHITE);
//        fillrect(0, 500, 250, 800);
//        setcolor(DARKGREY);
//        setlinewidth(2);
//        settextrotation(0);
//        setfontsize(15);
//        drawrect(0, 500, 251, 801);
//
//        t_bound_box rect = t_bound_box(0, 500, 250, 530);
//        drawtext_in(rect, "Pin Info:");
//        setfontsize(12);
//        rect = t_bound_box(0, 500, 250, 570);
//        drawtext_in(rect, "Point of Interest:");
//        rect = t_bound_box(0, 500, 250, 610);
//        drawName = drawtext_in(rect, getPointOfInterestName(poiClick));
//        while (!drawName && fontSize >= 9) {
//            fontSize--;
//            setfontsize(fontSize);
//            drawName = drawtext_in(rect, getPointOfInterestName(poiClick));
//        }
//
//        setfontsize(12);
//        rect = t_bound_box(0, 500, 250, 650);
//        drawtext_in(rect, "Type: " + getPointOfInterestType(poiClick));
//
//        rect = t_bound_box(0, 500, 250, 700);
//        drawtext_in(rect, "Closest Intersection: ");
//
//        std::vector<std::string> streetNames;
//
//        // update the information panel with the information on the closest points to where the user clicked
//        streetNames = find_intersection_street_names(intersectionClick);
//        std::sort(streetNames.begin(), streetNames.end());
//        streetNames.erase(std::unique(streetNames.begin(), streetNames.end()), streetNames.end());
//        for (unsigned i = 0; i < streetNames.size(); i++) {
//            if (streetNames[i] != "<unknown>") {
//                rect = t_bound_box(0, 500, 250, 740 + 40 * i);
//                drawStreet = drawtext_in(rect, streetNames[i]);
//                while (!drawStreet && fontSize >= 9) {
//                    fontSize--;
//                    setfontsize(fontSize);
//                    drawStreet = drawtext_in(rect, streetNames[i]);
//                }
//            }
//        }
//
//
//    }
//    //mapInfo.draw_pin = false;
//}

/*******************************************/
/*        DRAW SEARCH HOVER                */

/*******************************************/
void draw_search_hover() {
    if (mapInfo.draw_search_hover) {
        draw_surface(load_png_from_file("/nfs/ug/homes-5/r/roscoeol/mapper_repo/mapper/libstreetmap/src/icons/search-hover.png"), 275, 60);
    }
}

/*******************************************/
/*        DRAW DIRECTIONS HOVER            */

/*******************************************/
void draw_directions_hover() {
    if (mapInfo.draw_directions_hover && !mapInfo.directions) {
        draw_surface(load_png_from_file("/nfs/ug/homes-5/r/roscoeol/mapper_repo/mapper/libstreetmap/src/icons/directions-hover.png"), 315, 60);
    }
}

/*******************************************/
/*    HANDLE TYPING IN THE SEARCH BAR      */
/*******************************************/
void search_typing() {
    // if you're typing in the search bar, to search not to navigate
    if (mapInfo.search_type && !mapInfo.directions) {
        settextrotation(0);
        setfontsize(15);
        setcolor(DARKGREY);
        bool draw;
        int fontSize = 15;


        t_bound_box rect = t_bound_box(10, 10, 270, 60);

        // check if text draws, if it doesn't, decrease fontSize and try again until font size is too small
        draw = drawtext_in(rect, searchText);
        while (!draw && fontSize > 8) {
            fontSize--;
            setfontsize(fontSize);
            draw = drawtext_in(rect, searchText);
        }

        setfontsize(15);

        // if it never prints, output that the name is too long
        if (fontSize <= 8 && !draw) {
            drawtext(90, 35, "Name too long...");
        }
        
        // if the search bar is empty, print placeholder
        if (searchText.length() == 0 && !mapInfo.directions) {
            drawtext(90, 35, "Search...");
        }

        setlinewidth(4);
        setcolor(BLUE);
        drawline(10, 10, 10, 61);
    } else if (!mapInfo.directions) {
        setcolor(DARKGREY);

        settextrotation(0);
        setfontsize(15);
        drawtext(90, 35, "Search...");

    }
}

/*******************************************/
/*              DRAW DIRECTIONS            */

/*******************************************/
void draw_directions() {
    if (mapInfo.directions) {
        settextrotation(0);
        setcolor(WHITE);
        setlinestyle(SOLID);
        setlinewidth(1);
        fillrect(10, 60, 370, 120);
        bool draw;
        int fontSize = 10;

        // draw switch image and go button
        draw_surface(load_png_from_file("/nfs/ug/homes-5/r/roscoeol/mapper_repo/mapper/libstreetmap/src/icons/switch.png"), 330, 70);
        draw_surface(load_png_from_file("/nfs/ug/homes-5/r/roscoeol/mapper_repo/mapper/libstreetmap/src/icons/go.png"), 280, 70);

        setcolor(DARKGREY);
        drawline(20, 60, 360, 60);
        setfontsize(10);
        
        // if search text was entered, transfer it to directions
        if (searchText.length() != 0) {
            mapInfo.startLoc = searchText;
        }
        
        // if start is selected
        if (mapInfo.start) {
            // if nothing is entered, print placeholder saying you can click
            if (mapInfo.startLoc.length() == 0) {
                drawtext(150, 40, "Enter starting point, or click the map");
            } else {
                t_bound_box rect = t_bound_box(10, 10, 270, 60);

                // check if text draws, if it doesn't, decrease fontSize and try again until font size is too small
                draw = drawtext_in(rect, mapInfo.startLoc);
                while (!draw && fontSize > 8) {
                    fontSize--;
                    setfontsize(fontSize);
                    draw = drawtext_in(rect, mapInfo.startLoc);
                }

                //fontSize = 10;
            }

            // if end is empty
            if (mapInfo.endLoc.length() == 0) {
                drawtext(80, 95, "Destination...");
            } else {
                t_bound_box rect = t_bound_box(10, 60, 270, 120);

                // check if text draws, if it doesn't, decrease fontSize and try again until font size is too small
                draw = drawtext_in(rect, mapInfo.endLoc);
                while (!draw && fontSize > 8) {
                    fontSize--;
                    setfontsize(fontSize);
                    draw = drawtext_in(rect, mapInfo.endLoc);
                }
                //fontSize = 10;
            }
            setlinewidth(4);
            setcolor(BLUE);
            drawline(10, 10, 10, 61);
        } 
        
        // if end is selected, print placeholders/current text
        else if (mapInfo.end) {
            if (mapInfo.startLoc.length() == 0) {
                drawtext(85, 40, "Starting point...");
            } else {
                t_bound_box rect = t_bound_box(10, 10, 270, 60);

                // check if text draws, if it doesn't, decrease fontSize and try again until font size is too small
                draw = drawtext_in(rect, mapInfo.startLoc);
                while (!draw && fontSize > 8) {
                    fontSize--;
                    setfontsize(fontSize);
                    draw = drawtext_in(rect, mapInfo.startLoc);
                }
                //fontSize = 10;
            }

            if (mapInfo.endLoc.length() == 0) {
                drawtext(145, 95, "Enter destination, or click the map");
            } else {
                t_bound_box rect = t_bound_box(10, 60, 270, 120);

                // check if text draws, if it doesn't, decrease fontSize and try again until font size is too small
                draw = drawtext_in(rect, mapInfo.endLoc);
                while (!draw && fontSize > 8) {
                    fontSize--;
                    setfontsize(fontSize);
                    draw = drawtext_in(rect, mapInfo.endLoc);
                }
                //fontSize = 10;
            }
            setlinewidth(4);
            setcolor(BLUE);
            drawline(10, 60, 10, 121);
        } 
        
        // if neither is selected, don't permit typing or clicking, but print current values
        else {
            if (mapInfo.startLoc.length() == 0) {
                drawtext(85, 40, "Starting point...");
            } else {
                t_bound_box rect = t_bound_box(10, 10, 270, 60);

                // check if text draws, if it doesn't, decrease fontSize and try again until font size is too small
                draw = drawtext_in(rect, mapInfo.startLoc);
                while (!draw && fontSize > 8) {
                    fontSize--;
                    setfontsize(fontSize);
                    draw = drawtext_in(rect, mapInfo.startLoc);
                }
                //fontSize = 10;
            }
            if (mapInfo.endLoc.length() == 0) {
                drawtext(80, 95, "Destination...");
            } else {
                t_bound_box rect = t_bound_box(10, 60, 270, 120);

                // check if text draws, if it doesn't, decrease fontSize and try again until font size is too small
                draw = drawtext_in(rect, mapInfo.endLoc);
                while (!draw && fontSize > 8) {
                    fontSize--;
                    setfontsize(fontSize);
                    draw = drawtext_in(rect, mapInfo.endLoc);
                }
            }
        }



        //        if (BCLICK) {
        //            draw_icons("/nfs/ug/homes-5/r/roscoeol/mapper_repo/mapper/libstreetmap/src/icons/B.png", Bclick, 70, 21);
        //
        //
        //            draw_icons("/nfs/ug/homes-5/r/roscoeol/mapper_repo/mapper/libstreetmap/src/icons/A.png", Aclick, 70, 21);
        //        }
    }
}

/*******************************************/
/*              DRAW NEW MAP            */
/*******************************************/

// build graphics for load map
void enter_new_map() {
    if (mapInfo.map) {
        mapInfo.search_type = false;
        mapInfo.directions = false;

        settextrotation(0);
        setfontsize(12);
        setcolor(WHITE);
        bool draw;
        int fontSize = 12;


        t_bound_box rect = t_bound_box(400, 10, 670, 60);
        fillrect(rect);
        setcolor(DARKGREY);

        // draw leading name
        drawtext_in(rect, "/cad2/ece297s/public/maps/...");

        setcolor(WHITE);
        rect = t_bound_box(400, 60, 670, 120);
        fillrect(rect);
        setcolor(DARKGREY);

        // draw go button
        draw_surface(load_png_from_file("/nfs/ug/homes-5/r/roscoeol/mapper_repo/mapper/libstreetmap/src/icons/go.png"), 630, 70);
        rect = t_bound_box(400, 60, 630, 120);

         // check if text draws, if it doesn't, decrease fontSize and try again until font size is too small
        draw = drawtext_in(rect, mapText);
        while (!draw && fontSize > 8) {
            fontSize--;
            setfontsize(fontSize);
            draw = drawtext_in(rect, mapText);
        }

        // draw placeholder text
        if (mapText.length() == 0) {
            drawtext_in(rect, "Enter map path...");
        }

        setlinewidth(4);
        setcolor(BLUE);
        drawline(400, 60, 400, 121);

    }
}


/*******************************************/
/*             DRAW HELP                   */
/*******************************************/

void help(void (*drawscreen_ptr) (void)) {
    
    // update the button name
    char old_button_name[200], new_button_name[200];

    set_coordinate_system(GL_SCREEN);
    if (!mapInfo.help) {
        sprintf(old_button_name, "Help");
        sprintf(new_button_name, "Help Off");
        change_button_text(old_button_name, new_button_name);

        mapInfo.help = true;
    } else {
        sprintf(old_button_name, "Help Off");
        sprintf(new_button_name, "Help");
        change_button_text(old_button_name, new_button_name);
        mapInfo.help = false;
    }
    set_coordinate_system(GL_WORLD);
    drawscreen();
}

/*******************************************/
/*            HIGHLIGHT PATH               */
/*******************************************/

void highlight_path() {
    print_directions();
    
    // draw starting pin A
    LatLon pinPosition = mapInfo.intersection_position[interA];
    draw_icons("/nfs/ug/homes-5/r/roscoeol/mapper_repo/mapper/libstreetmap/src/icons/A.png", pinPosition, 70, 21);

    // draw transparent thick red lines over the path
    for (unsigned i = 0; i < mapInfo.pathIDs.size(); i++) {
        draw_street_segment(mapInfo.pathIDs[i], t_color(255, 0, 0, 255 / 3), 7);
    }

    //I don't know why you have to draw this twice, but it fixed the weird error
    draw_icons("/nfs/ug/homes-5/r/roscoeol/mapper_repo/mapper/libstreetmap/src/icons/A.png", pinPosition, 70, 21);
    
    // draw ending pin B
    pinPosition = mapInfo.intersection_position[interB];
    draw_icons("/nfs/ug/homes-5/r/roscoeol/mapper_repo/mapper/libstreetmap/src/icons/B.png", pinPosition, 70, 21);

}



/*******************************************/
/*          PRINT DIRECTIONS            */
/*******************************************/
void print_directions() {
    
    // draw transparent directions panel
    settextrotation(0);
    setcolor(t_color(255, 255, 255, 180));
    setlinestyle(SOLID);
    setlinewidth(1);
    fillrect(0, 0, 400, 1000);
    bool draw;
    int fontSize = 10;

    setfontsize(15);
    setcolor(DARKGREY);
    t_bound_box rect;

    rect = t_bound_box(0, 20, 400, 90);
    draw = drawtext_in(rect, "DIRECTIONS");

    // print all the directions to the panel 
    setfontsize(10);
    for (unsigned i = 1; i < directions_to_print.size(); i++) {
        fontSize = 10;
        rect = t_bound_box(0, 70 + i * 20, 400, 130 + i * 20);
        
         // check if text draws, if it doesn't, decrease fontSize and try again until font size is too small
        draw = drawtext_in(rect, directions_to_print[i]);
        while (!draw && fontSize >= 8) {
            fontSize--;
            setfontsize(fontSize);
            draw = drawtext_in(rect, directions_to_print[i]);
        }

    }

    //draw the x to close the menu with
    draw_surface(load_png_from_file("/nfs/ug/homes-5/r/roscoeol/mapper_repo/mapper/libstreetmap/src/icons/x.png"), 330, 20);
}
