#ifndef SKETCH_RING_SKETCH_H
#define SKETCH_RING_SKETCH_H


class Sketch {
  public:
    virtual int size() = 0;

    virtual void update(unsigned int item, int diff) = 0;

    virtual int estimate(unsigned int query) = 0;

    virtual ~Sketch() = default;
};

#endif //SKETCH_RING_SKETCH_H
