//
// Created by Ico on 04/12/2021.
//

#ifndef SCD30_FEEDS_H
#define SCD30_FEEDS_H

#define AIO_FINGERPRINT     "59 3C 48 0A B1 8B 39 4E 0D 58 50 47 9A 13 55 60 CC A0 1D AF"

#define REFRESH_FEED        15000
#define BASE_FEED           AIO_USERNAME "/feeds/"

#define FEED_CO2            BASE_FEED "scd30.co2"
#define FEED_TEMPERATURE    BASE_FEED "scd30.temperature"
#define FEED_HUMIDITY       BASE_FEED "scd30.relative-humidity"


#endif //SCD30_FEEDS_H