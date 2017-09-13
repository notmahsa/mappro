#include "m1.h"
#include "m2.h"
#include "m3.h"
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include "MapParser.h"
#include "algorithm"
#include <algorithm>
#include <vector>
#include <math.h>
#include <string>
#include <unordered_map>
#include <deque>
#include <iostream>
using namespace std;

extern mapData mapInfo;

#include "m4.h"

unsigned closestDeliveryByDistance(unsigned current, vector<DeliveryInfo> deliveries,
        vector<bool>&pickupVisited, vector<bool>&dropoffVisited) {

    LatLon currentPos, nextPos;
    double tempDistance, minDistance = INF;
    unsigned nextDelivery;
    bool pickUp;
    currentPos = mapInfo.intersection_position[current];

    for (unsigned i = 0; i < deliveries.size(); i++) {
        if (pickupVisited[i] && !dropoffVisited[i]) {
            nextPos = mapInfo.intersection_position[deliveries[i].dropOff];
            tempDistance = find_distance_between_two_points(currentPos, nextPos);
            if (tempDistance < minDistance) {
                nextDelivery = i;
                minDistance = tempDistance;
                pickUp = false;
            }
        } else if (!pickupVisited[i]) {
            nextPos = mapInfo.intersection_position[deliveries[i].pickUp];
            tempDistance = find_distance_between_two_points(currentPos, nextPos);
            if (tempDistance < minDistance) {
                minDistance = tempDistance;
                nextDelivery = i;
                pickUp = true;
            }
        }
    }
    if (pickUp) {
        pickupVisited[nextDelivery] = true;
        nextDelivery = deliveries[nextDelivery].pickUp;
    } else {
        dropoffVisited[nextDelivery] = true;
        nextDelivery = deliveries[nextDelivery].dropOff;
    }

    return nextDelivery;
}

std::vector<unsigned> traveling_courier(const std::vector<DeliveryInfo>& deliveries,
        const std::vector<unsigned>& depots,
       const float turn_penalty) {

    vector <unsigned> pathIntersections; //vector this function will return
    vector <unsigned> pathSegments, tempPath;
    int count_deliveries = deliveries.size()*2 + 2;
    pathIntersections.resize(count_deliveries);
    pathSegments.resize(count_deliveries - 1);

    vector <bool>pickupVisited(deliveries.size(), false);
    vector <bool>dropoffVisited(deliveries.size(), false);

    unsigned start_depot = depots[0];
    pathIntersections[0] = start_depot;
    unsigned current;


    for (unsigned i = 0; i < count_deliveries - 1; i++) {
        current = closestDeliveryByDistance(pathIntersections[i],
                deliveries, pickupVisited, dropoffVisited);

        pathIntersections[i + 1] = current;

        tempPath = find_path_between_intersections(pathIntersections[i], pathIntersections[i + 1], turn_penalty);

        pathSegments.insert(pathSegments.end(), tempPath.begin(), tempPath.end());

        count_deliveries--;
    }

    pathIntersections[count_deliveries - 1] = start_depot;
    tempPath = find_path_between_intersections(pathIntersections[count_deliveries - 2], pathIntersections[count_deliveries - 1], turn_penalty);
    pathSegments.insert(pathSegments.end(), tempPath.begin(), tempPath.end());
    return pathSegments;
}

