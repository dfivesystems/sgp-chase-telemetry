#ifndef NMEAUTILS_H
#define NMEAUTILS_H

double nmeaPositionToDecimal(const std::string& nmeaCoordinate, const std::string& direction) {
    double degrees = 0.0;

    if (nmeaCoordinate.size() > 2) {
        // For latitude, the format is DDMM.MMMM
        // For longitude, the format is DDDMM.MMMM

        // Determine the number of degree digits based on the length of the string
        const size_t degreeDigits = (direction == "E" || direction == "W") ? 3 : 2;

        // Extract degrees
        degrees = std::stod(nmeaCoordinate.substr(0, degreeDigits));
        // Extract minutes
        const double minutes = std::stod(nmeaCoordinate.substr(degreeDigits));

        // Convert to decimal degrees
        degrees += (minutes / 60.0);

        // Adjust for the hemisphere (N/S for latitude, E/W for longitude)
        if (direction == "S" || direction == "W") {
            degrees = -degrees;
        }
    }

    return degrees;
}

#endif //NMEAUTILS_H
