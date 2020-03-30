//
// Created by Avi on 29/03/2020.
//

#ifndef DYNAMIC_SKETCH_ERROR_CM_H
#define DYNAMIC_SKETCH_ERROR_CM_H

extern "C" {
#include "massdalsketches/countmin.h"
}

#include <new>
#include <vector>
#include <boost/current_function.hpp>
#include "Sketch.h"


#if 0
#define PRINT_DBG(...) do { printf("====%s====\n", BOOST_CURRENT_FUNCTION); } while(0)
#else
#define PRINT_DBG() do {} while(0)
#endif

class CountMinSketch : public Sketch {
  public:
    CountMinSketch(int width, int depth, int seed) : cm(CM_Init(width, depth, seed)) {
        PRINT_DBG();
        if (!cm) throw std::bad_alloc();
    }

    CountMinSketch(const CountMinSketch& other) : cm(CM_Copy(other.cm)) {
        PRINT_DBG();
        if (!cm) throw std::bad_alloc();
    }

    CountMinSketch(CountMinSketch&& other) noexcept : cm(other.cm) {
        PRINT_DBG();
        other.cm = nullptr;
    }

    CountMinSketch& operator=(const CountMinSketch& other) {
        PRINT_DBG();
        *this = CountMinSketch(other);
        return *this;
    }

    CountMinSketch& operator=(CountMinSketch&& other) noexcept {
        PRINT_DBG();
        std::swap(cm, other.cm);
        return *this;
    }

    ~CountMinSketch() override {
        PRINT_DBG();
        CM_Destroy(cm);
    }

    int size() override {
        PRINT_DBG();
        return CM_Size(cm);
    }

    void update(unsigned int item, int diff) override {
        PRINT_DBG();
        return CM_Update(cm, item, diff);
    }

    int estimate(unsigned int query) override {
        PRINT_DBG();
        return CM_PointEst(cm, query);
    }

    int pointMed(unsigned int query) {
        PRINT_DBG();
        return CM_PointMed(cm, query);
    }

    int residue(const std::vector<unsigned>& q) {
        PRINT_DBG();
        return CM_Residue(cm, const_cast<unsigned*>(q.data()), q.size());
    }

    static int innerProd(const CountMinSketch& cm1, const CountMinSketch& cm2) {
        PRINT_DBG();
        return CM_InnerProd(cm1.cm, cm2.cm);
    }

  private:
    CM_type* cm;
};


#endif //DYNAMIC_SKETCH_ERROR_CM_H
