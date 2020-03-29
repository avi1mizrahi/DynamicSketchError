//
// Created by Avi on 29/03/2020.
//

#include <iostream>
#include "Cm.h"

int main() {
    CM cm(10, 34, 1);

    std::cout << "hello" << std::endl;
    std::cout << cm.size() << std::endl;

    cm.update(3, 5);
    cm.update(6, 5);
    cm.update(3, 5);
    cm.update(1, 5);
    cm.update(6, -5);

    std::cout << cm.size() << std::endl;

    CM cm2(cm);

    std::cout << cm2.size() << std::endl;

    cm2.update(3, 5);
    cm2.update(6, 5);
    cm2.update(2, 5);
    cm2.update(2, 5);
    cm2.update(1, -5);

    std::cout << CM::innerProd(cm, cm2) << std::endl;

    return 0;
}