#ifndef SKETCH_RING_SKETCHRING_H
#define SKETCH_RING_SKETCHRING_H


#include <vector>
#include <boost/icl/interval_map.hpp>
#include "CountMinSketch.h"


struct SketchRing {
  public:
    typedef int                                    Point;
    typedef boost::icl::interval_map<Point, Point> IntervalMap;

    explicit SketchRing(int width, int depth, int n = 10, Point domainMax = 1 << 12);

    [[nodiscard]] int size() const;

    void update(unsigned int item, int diff);

    [[nodiscard]] int estimate(unsigned int item) const;

    void resize(int n);

    [[nodiscard]] std::vector<std::pair<int, int> > heavyHitters(int threshold) const;

  private:

    void shrink();

    void expand();

    IntervalMap                     buckets;
    std::map<Point, CountMinSketch> cms;
    int                             domainMax;
};


#endif //SKETCH_RING_SKETCHRING_H
