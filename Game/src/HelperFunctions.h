//
// Created by Zekai on 2/21/2023.
//

#ifndef CS4150_HELPERFUNCTIONS_H
#define CS4150_HELPERFUNCTIONS_H

#include "Vec2.h"

float euclidDistance(Vec2 v1, Vec2 v2) {
    return sqrt((v1.x - v2.x) * (v1.x - v2.x) + (v1.y - v2.y) * (v1.y - v2.y));
}


#endif //CS4150_HELPERFUNCTIONS_H
