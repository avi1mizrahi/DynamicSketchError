#ifndef SKETCH_RING_SKETCHRINGC_H
#define SKETCH_RING_SKETCHRINGC_H

#endif //SKETCH_RING_SKETCHRINGC_H

struct SketchRing;
typedef struct SketchRing SK_type;

SK_type* SK_init(int width, int depth, int n, int domainMax);

void SK_destroy(SK_type*);

int SK_size(const SK_type*);

void SK_update(SK_type*, unsigned int item, int diff);

int SK_estimate(const SK_type*, unsigned int item);

void SK_resize(SK_type*, int n);

//typedef struct {
//    int size;
//    int* items;
//    int* freq;
//} HH;
//
//void HH_free(HH* hh);
//
//HH SK_heavyHitters(const SK_type*, int threshold);

int* SK_heavyHitters(const SK_type*, int threshold);
