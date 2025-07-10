#include <iostream>


int main(int argc, char **argv)
{
    int iter = 0;

    double D[6][3] = {{0.0}};

    while(iter < 6){
        D[2][2] += 2.2;
        iter++;
    }
    return 0;
}
