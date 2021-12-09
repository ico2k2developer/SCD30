//
// Created by Ico on 04/12/2021.
//

#ifndef SCD30_FEEDS_H
#define SCD30_FEEDS_H

#define AIO_FINGERPRINT     "59 3C 48 0A B1 8B 39 4E 0D 58 50 47 9A 13 55 60 CC A0 1D AF"

#define REFRESH_FEED        15000
#define BASE_FEED           AIO_USERNAME "/feeds/scd30."

#define FEED_CO2                    BASE_FEED "co2"
#define FEED_TEMPERATURE            BASE_FEED "temperature"
#define FEED_HUMIDITY               BASE_FEED "relative-humidity"
#define FEED_LOG                    BASE_FEED "log"
#define FEED_CALIBRATION_VALUE      BASE_FEED "calibration"
#define FEED_CALIBRATION_CONFIRM    BASE_FEED "commit-calibration"


#endif //SCD30_FEEDS_H