#pragma once

/**
 * @file geo.h
 * Geographic utilities: coordinates and distance computation.
 *
 * Provides data structures for representing stop coordinates and road
 * distances, as well as a function for computing the geographic (haversine)
 * distance between two points on Earth.
 */

#include <cmath>
#include <string>

/**
 * Utility namespace for the application.
 */
namespace utils
{
    /**
     * Describes the road distance to a neighboring stop.
     */
    struct Distance {
        /**
         * Name of the destination stop.
         */        std::string stop;
        /**
         * Actual road distance in meters.
         */        double value;
    };

    /**
     * Geographic coordinates of a point.
     */
    struct Coordinates {
        /**
         * Latitude in degrees.
         */        double lat;
        /**
         * Longitude in degrees.
         */        double lng;

        /**
         * Equality comparison for coordinates.
         * @param other Coordinates to compare against.
         * @return true if both latitude and longitude match exactly.
         */
        bool operator==(const Coordinates& other) const {
            return lat == other.lat && lng == other.lng;
        }

        /**
         * Inequality comparison for coordinates.
         * @param other Coordinates to compare against.
         * @return true if coordinates differ.
         */
        bool operator!=(const Coordinates& other) const {
            return !(*this == other);
        }
    };

    /**
     * Computes the great-circle distance between two points using the haversine formula.
     *
     * @param from Starting point coordinates.
     * @param to   Destination point coordinates.
     * @return Distance in meters. Returns 0.0 if the two points are identical.
     */
    inline double ComputeDistance(Coordinates from, Coordinates to) {
        using namespace std;
        if (from == to) {
            return 0;
        }
        static const double dr = 3.1415926535 / 180.;
        static const int kEarthRadius = 6371000;
        return acos(sin(from.lat * dr) * sin(to.lat * dr)
                    + cos(from.lat * dr) * cos(to.lat * dr) * cos(abs(from.lng - to.lng) * dr))
            * kEarthRadius;
    }
}