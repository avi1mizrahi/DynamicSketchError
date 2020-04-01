//
// Created by Avi on 29/03/2020.
//

#include <iostream>
#include <boost/icl/interval_map.hpp>
#include "CountMinSketch.h"
#include "SketchRing.h"

struct C {
    C(int i = 0) : i(i) {}

    bool operator==(const C& rhs) const {
        return i == rhs.i;
    }

    bool operator!=(const C& rhs) const {
        return !(rhs == *this);
    }

    friend std::ostream& operator<<(std::ostream& os, const C& c) {
        os << "i: " << c.i;
        return os;
    }

    int i;
};

int main() {

    using namespace boost::icl;
    using namespace std;
    interval_map<int, C> im;

    im.set(make_pair(interval<int>::type(0, 99), C(1)));
    cout << im << endl;

    im.set(make_pair(interval<int>::type(0, 50), C(11)));
    cout << im << endl;

    im.set(make_pair(interval<int>::type(25, 75), C(111)));
    cout << im << endl;

    im.set(make_pair(interval<int>::type(60, 90), C(1111)));
    cout << im << endl;

    cout << im.find(26)->second << endl;
    cout << im.find(26)->first << endl;

    im.set(make_pair(interval<int>::type(16, 25), C(50)));
    cout << im << endl;

    im.erase(interval<int>::type(19, 21));
    cout << im << endl;

    im.set(make_pair(interval<int>::type(19, 21), C(50)));
    cout << im << endl;


    CountMinSketch cm(10, 34, 1);

    std::cout << "hello" << std::endl;
    std::cout << cm.size() << std::endl;

    cm.update(3, 5);
    cm.update(6, 5);
    cm.update(3, 5);
    cm.update(1, 5);
    cm.update(6, -5);

    std::cout << cm.size() << std::endl;

    std::cout << "new cm" << std::endl;

    cm = CountMinSketch(5, 5, 1);
    const vector<int> values{1, 223, 325, 326, 544, 555, 556};

    for (unsigned i : values) {
        cm.update(i, 1024 - i);
    }

    for (unsigned i : values) {
        printf("%4d: %4d %4d\n", i, 1024 - i, cm.estimate(i));
    }

    const auto& hh = cm.heavyHitters(.5);
    for (auto [item, freq] : hh) {
        printf("ITEM: %5d   Freq: %5d\n", item, freq);
    }

//    return 0;
    SketchRing sketchRing(CountMinSketch(5, 5, 1));

    cout << sketchRing.size() << endl;
    cout << "+++++++++++++++++++++" << endl;


    for (unsigned i : values) {
        sketchRing.update(i, 1024 - i);
    }

    for (unsigned i : values) {
        printf("%4d: %4d %4d\n", i, 1024 - i, sketchRing.estimate(i));
    }

    sketchRing.resize(6);
    cout << "RESIZE" << endl;


    for (unsigned i : values) {
        printf("%4d: %4d %4d\n", i, 1024 - i, sketchRing.estimate(i));
    }

    const vector<int> values2{111, 222, 333};

    for (unsigned i : values2) {
        sketchRing.update(i, i);
    }
    for (unsigned i : values2) {
        printf("%4d: %4d %4d\n", i, i, sketchRing.estimate(i));
    }


    return 0;
}