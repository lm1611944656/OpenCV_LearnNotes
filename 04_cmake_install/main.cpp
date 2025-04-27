#include <iostream>
#include "yolo.h"
#include "utils.h"


int main(int argc, char **argv){

    CYOLO *m_yolo = new CYOLO();

    CUtils *m_utils = new CUtils();

    std::cout << "输出检测" << std::endl;

    delete m_utils;
    delete m_yolo;
    return 0;
}