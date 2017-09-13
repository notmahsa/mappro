#include <unittest++/UnitTest++.h>
#include "m1.h"
#include "unit_test_util.h"
#include "StreetsDatabaseAPI.h"
#include "unit_test_util.h"
#include <algorithm>

using ece297test::relative_error;
using namespace std;

struct MapFixture {

    MapFixture() {
        //Load the map
        load_map("/cad2/ece297s/public/maps/toronto_canada.streets.bin");

        rng = std::minstd_rand(0);
        rand_intersection = std::uniform_int_distribution<unsigned>(0, getNumberOfIntersections() - 1);
        rand_street = std::uniform_int_distribution<unsigned>(1, getNumberOfStreets() - 1);

    }

    ~MapFixture() {
        //Clean-up
        close_map();
    }
    
    std::minstd_rand rng;
    std::uniform_int_distribution<unsigned> rand_intersection;
    std::uniform_int_distribution<unsigned> rand_street;

};


SUITE(street_queries) {

    //Add a unit test here to test the functionality of "find_street_ids_from_name"
    // ...

    //    TEST(find_street_ids_from_name){
    //    std::vector<unsigned> expected;
    //    std::vector<unsigned> actual;
    //    
    //    actual  = find_street_ids_from_name("Highway 401 Eastbound Express");
    //    expected = {2};
    //    CHECK_EQUAL(expected, actual);

    TEST_FIXTURE(MapFixture, find_street_ids_from_name) {
        //UNITTEST_TIME_CONSTRAINT(0.00075); //250 ns per test
        for (unsigned i = 0; i < getNumberOfStreets(); i++) {
            string name = getStreetName(i);
            vector <unsigned> vec = find_street_ids_from_name(name);

            for (unsigned j = 0; j < vec.size(); j++) {
                string newName = getStreetName(vec.at(j));
                CHECK_EQUAL(name, newName);
            }
            vec.clear();
        }

        //Verify Functionality
        std::vector<unsigned> expected;
        std::vector<unsigned> actual;

        expected = {12480};
        actual = find_street_ids_from_name("Willcocks Crescent");
        std::sort(actual.begin(), actual.end());
        CHECK_EQUAL(expected, actual);

        expected = {12504};
        actual = find_street_ids_from_name("Ridgecrest Drive");
        std::sort(actual.begin(), actual.end());
        CHECK_EQUAL(expected, actual);

        expected = {12521};
        actual = find_street_ids_from_name("Brantford Drive");
        std::sort(actual.begin(), actual.end());
        CHECK_EQUAL(expected, actual);

        expected = {12986};
        actual = find_street_ids_from_name("Kittiwake Avenue");
        std::sort(actual.begin(), actual.end());
        CHECK_EQUAL(expected, actual);

        expected = {13000};
        actual = find_street_ids_from_name("Auld Croft Road");
        std::sort(actual.begin(), actual.end());
        CHECK_EQUAL(expected, actual);

        expected = {13096};
        actual = find_street_ids_from_name("Milvan Drive");
        std::sort(actual.begin(), actual.end());
        CHECK_EQUAL(expected, actual);

        expected = {13247};
        actual = find_street_ids_from_name("Bartel Drive");
        std::sort(actual.begin(), actual.end());
        CHECK_EQUAL(expected, actual);

        expected = {13259};
        actual = find_street_ids_from_name("Firgrove Crescent");
        std::sort(actual.begin(), actual.end());
        CHECK_EQUAL(expected, actual);

        expected = {254, 16537};
        actual = find_street_ids_from_name("Bay Street");
        std::sort(actual.begin(), actual.end());
        CHECK_EQUAL(expected, actual);

        expected = {35};
        actual = find_street_ids_from_name("Yonge Street");
        std::sort(actual.begin(), actual.end());
        CHECK_EQUAL(expected, actual);

        expected = {};
        actual = find_street_ids_from_name("Go team 053!");
        std::sort(actual.begin(), actual.end());
        CHECK_EQUAL(expected, actual);

        //Generate random inputs
        std::vector<std::string> street_names_1;
        for (size_t i = 0; i < 1000000; i++) {
            unsigned street_id1 = rand_street(rng);
            street_names_1.push_back(getStreetName(street_id1));
     
        }
        {
            //Timed Test
            ECE297_TIME_CONSTRAINT(250);
            std::vector<unsigned> result;
            for (size_t i = 0; i < 1000000; i++) {
                result = find_street_ids_from_name(street_names_1[i]);
            }
        }
    } //intersection_ids_from_street_names_perf

}