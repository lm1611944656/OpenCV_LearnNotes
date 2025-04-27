#ifndef __UTILS_H__
#define __UTILS_H__

#include <iostream>


class CUtils{
public:
    explicit CUtils(void);
    ~CUtils();

public:
    uint16_t float32ToFloat16(float &value);

    float float16ToFloat32(uint16_t &value);

};

#endif // __UTILS_H__
