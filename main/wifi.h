#ifndef WIFI_H
#define WIFI_H

#include <stdbool.h>

enum class WIFI_STATE
{
    Ready = 0,
    NoAccessPointOrHidden,
    NoAccessPoint,
    HasAccessPoint,
    InvalidPassword,
    CorrectPassword,
    ServerIsUnaccessible,
    Disconnected
};

void wifi_initialize();
void wifi_start_scan();

#endif //WIFI_H