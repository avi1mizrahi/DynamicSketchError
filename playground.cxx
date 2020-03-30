//
// Created by Avi on 29/03/2020.
//

#include <iostream>
#include "CountMinSketch.h"

int main() {

    CountMinSketch cm(10, 34, 1);

    std::cout << "hello" << std::endl;
    std::cout << cm.size() << std::endl;

    cm.update(3, 5);
    cm.update(6, 5);
    cm.update(3, 5);
    cm.update(1, 5);
    cm.update(6, -5);

    std::cout << cm.size() << std::endl;

    CountMinSketch cm2(cm);

    std::cout << cm2.size() << std::endl;

    cm2.update(3, 5);
    cm2.update(6, 5);
    cm2.update(2, 5);
    cm2.update(2, 5);
    cm2.update(1, -5);

    std::cout << CountMinSketch::innerProd(cm, cm2) << std::endl;

    std::cout << "new cm" << std::endl;

    cm = CountMinSketch(5, 5, 1);

    for (unsigned i : {1, 2, 3, 4, 5}) {
        cm.update(i, 10 - i);
    }

    for (unsigned i : {1, 2, 3, 4, 5}) {
        printf("%4d: %4d %4d\n", i, 10 - i, cm.estimate(i));
    }

    return 0;
}