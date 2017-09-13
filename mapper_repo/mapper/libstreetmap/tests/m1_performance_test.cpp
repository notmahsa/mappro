//#include <algorithm>
//#include <unittest++/UnitTest++.h>
//#include "unit_test_util.h"
//#include "m1.h"
//#include "StreetsDatabaseAPI.h"
//
//using ece297test::relative_error;
//
//
//struct MapFixture {
//
//    MapFixture() {
//        rng = std::minstd_rand(0);
//        rand_intersection = std::uniform_int_distribution<unsigned>(0, getNumberOfIntersections() - 1);
//        rand_street = std::uniform_int_distribution<unsigned>(1, getNumberOfStreets() - 1);
//
//    }
//
//    std::minstd_rand rng;
//    std::uniform_int_distribution<unsigned> rand_intersection;
//    std::uniform_int_distribution<unsigned> rand_street;
//
//};
//
//SUITE(street_queries_perf) {
//
//
//    TEST_FIXTURE(MapFixture, intersection_ids_from_street_names_perf) {
//        //Verify Functionality
//        std::vector<unsigned> expected;
//        std::vector<unsigned> actual;
//
//        expected = {63023, 63024};
//        actual = find_intersection_ids_from_street_names("Anderson Crescent", "Farmstead Lane");
//        std::sort(actual.begin(), actual.end());
//        CHECK_EQUAL(expected, actual);
//
//        expected = {};
//        actual = find_intersection_ids_from_street_names("Birchlea Avenue", "Donnamora Crescent");
//        std::sort(actual.begin(), actual.end());
//        CHECK_EQUAL(expected, actual);
//
//        expected = {};
//        actual = find_intersection_ids_from_street_names("Bonnacord Drive", "Bradstock Road");
//        std::sort(actual.begin(), actual.end());
//        CHECK_EQUAL(expected, actual);
//
//        expected = {76854};
//        actual = find_intersection_ids_from_street_names("Brickstone Mews", "Curran Place");
//        std::sort(actual.begin(), actual.end());
//        CHECK_EQUAL(expected, actual);
//
//        expected = {};
//        actual = find_intersection_ids_from_street_names("Denton Avenue", "Squire Drive");
//        std::sort(actual.begin(), actual.end());
//        CHECK_EQUAL(expected, actual);
//
//        expected = {49625};
//        actual = find_intersection_ids_from_street_names("Dumfrey Road", "Kenmark Boulevard");
//        std::sort(actual.begin(), actual.end());
//        CHECK_EQUAL(expected, actual);
//
//        expected = {54017};
//        actual = find_intersection_ids_from_street_names("Dunera Avenue", "Payzac Avenue");
//        std::sort(actual.begin(), actual.end());
//        CHECK_EQUAL(expected, actual);
//
//        expected = {24972};
//        actual = find_intersection_ids_from_street_names("Father D'souza Drive", "Mavis Road");
//        std::sort(actual.begin(), actual.end());
//        CHECK_EQUAL(expected, actual);
//
//        expected = {0};
//        actual = find_intersection_ids_from_street_names("Highway 401 Eastbound Collectors", "Ramp to Highway 404 / Don Valley Parkway");
//        std::sort(actual.begin(), actual.end());
//        CHECK_EQUAL(expected, actual);
//
//        expected = {};
//        actual = find_intersection_ids_from_street_names("Howard Street", "Tillings Road");
//        std::sort(actual.begin(), actual.end());
//        CHECK_EQUAL(expected, actual);
//
//        expected = {49834, 49835};
//        actual = find_intersection_ids_from_street_names("Kidbrooke Crescent", "Montvale Drive");
//        std::sort(actual.begin(), actual.end());
//        CHECK_EQUAL(expected, actual);
//
//        expected = {49834, 49835};
//        actual = find_intersection_ids_from_street_names("Montvale Drive", "Kidbrooke Crescent");
//        std::sort(actual.begin(), actual.end());
//        CHECK_EQUAL(expected, actual);
//
//        expected = {};
//        actual = find_intersection_ids_from_street_names("Naomi Crescent", "Bridgman Avenue");
//        std::sort(actual.begin(), actual.end());
//        CHECK_EQUAL(expected, actual);
//
//        expected = {74379};
//        actual = find_intersection_ids_from_street_names("National Pine Drive", "Deepsprings Crescent");
//        std::sort(actual.begin(), actual.end());
//        CHECK_EQUAL(expected, actual);
//
//        expected = {};
//        actual = find_intersection_ids_from_street_names("Parkham Crescent", "Camor Court");
//        std::sort(actual.begin(), actual.end());
//        CHECK_EQUAL(expected, actual);
//
//        expected = {};
//        actual = find_intersection_ids_from_street_names("Pendulum Circle", "Satchell Boulevard");
//        std::sort(actual.begin(), actual.end());
//        CHECK_EQUAL(expected, actual);
//
//        expected = {};
//        actual = find_intersection_ids_from_street_names("Rambert Crescent", "Hartwell Road");
//        std::sort(actual.begin(), actual.end());
//        CHECK_EQUAL(expected, actual);
//
//        expected = {};
//        actual = find_intersection_ids_from_street_names("Ramp to Highway 407 E.T.R. Westbound", "Kingsdale Avenue");
//        std::sort(actual.begin(), actual.end());
//        CHECK_EQUAL(expected, actual);
//
//        expected = {62777};
//        actual = find_intersection_ids_from_street_names("Reith Way", "Porterfield Crescent");
//        std::sort(actual.begin(), actual.end());
//        CHECK_EQUAL(expected, actual);
//
//        expected = {27621};
//        actual = find_intersection_ids_from_street_names("Saffron Crescent", "Rymer Road");
//        std::sort(actual.begin(), actual.end());
//        CHECK_EQUAL(expected, actual);
//
//        expected = {};
//        actual = find_intersection_ids_from_street_names("Seminoff Street", "Terrydale Drive");
//        std::sort(actual.begin(), actual.end());
//        CHECK_EQUAL(expected, actual);
//
//        expected = {8311};
//        actual = find_intersection_ids_from_street_names("Shaftesbury Street", "Acton Avenue");
//        std::sort(actual.begin(), actual.end());
//        CHECK_EQUAL(expected, actual);
//
//        expected = {};
//        actual = find_intersection_ids_from_street_names("South Parade Court", "Gorton Road");
//        std::sort(actual.begin(), actual.end());
//        CHECK_EQUAL(expected, actual);
//
//        expected = {60625, 60626};
//        actual = find_intersection_ids_from_street_names("Stargell Crescent", "Raymerville Drive");
//        std::sort(actual.begin(), actual.end());
//        CHECK_EQUAL(expected, actual);
//
//        expected = {79369, 79403};
//        actual = find_intersection_ids_from_street_names("Strong Gate", "O'Shea Crescent");
//        std::sort(actual.begin(), actual.end());
//        CHECK_EQUAL(expected, actual);
//
//        expected = {};
//        actual = find_intersection_ids_from_street_names("Tennyson Avenue", "Queensborough Crescent");
//        std::sort(actual.begin(), actual.end());
//        CHECK_EQUAL(expected, actual);
//
//        expected = {};
//        actual = find_intersection_ids_from_street_names("To [107] Queen Street west", "Eagleview Drive");
//        std::sort(actual.begin(), actual.end());
//        CHECK_EQUAL(expected, actual);
//
//        expected = {};
//        actual = find_intersection_ids_from_street_names("Tomel Court", "Jim Baird Mews");
//        std::sort(actual.begin(), actual.end());
//        CHECK_EQUAL(expected, actual);
//
//        expected = {77972};
//        actual = find_intersection_ids_from_street_names("Trentin Road", "Thorndale Drive");
//        std::sort(actual.begin(), actual.end());
//        CHECK_EQUAL(expected, actual);
//
//        expected = {};
//        actual = find_intersection_ids_from_street_names("Zaph Avenue", "Jenvic Gardens");
//        std::sort(actual.begin(), actual.end());
//        CHECK_EQUAL(expected, actual);
//
//        //Generate random inputs
//        std::vector<std::string> street_names_1;
//        std::vector<std::string> street_names_2;
//        for (size_t i = 0; i < 900000; i++) {
//            unsigned street_id1 = rand_street(rng);
//            unsigned street_id2 = rand_street(rng);
//            street_names_1.push_back(getStreetName(street_id1));
//            street_names_2.push_back(getStreetName(street_id2));
//        }
//        {
//            //Timed Test
//            ECE297_TIME_CONSTRAINT(676);
//            std::vector<unsigned> result;
//            for (size_t i = 0; i < 900000; i++) {
//                result = find_intersection_ids_from_street_names(street_names_1[i], street_names_2[i]);
//            }
//        }
//    } //intersection_ids_from_street_names_perf
//
//} //street_queries_perf}