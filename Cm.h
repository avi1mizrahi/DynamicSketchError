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

class CM {
  public:
    CM(int width, int depth, int seed) : cm(CM_Init(width, depth, seed)) {
        if (!cm) throw std::bad_alloc();
    }

    CM(const CM& other) : cm(CM_Copy(other.cm)) {
        if (!cm) throw std::bad_alloc();
    }

    ~CM() {
        CM_Destroy(cm);
    }

    int size() {
        return CM_Size(cm);
    }

    void update(unsigned int item, int diff) {
        return CM_Update(cm, item, diff);
    }

    int pointEst(unsigned int query) {
        return CM_PointEst(cm, query);
    }

    int pointMed(unsigned int query) {
        return CM_PointMed(cm, query);
    }

    int residue(const std::vector<unsigned>& q) {
        return CM_Residue(cm, const_cast<unsigned*>(q.data()), q.size());
    }

    static int innerProd(const CM& cm1, const CM& cm2) {
        return CM_InnerProd(cm1.cm, cm2.cm);
    }

  private:
    CM_type* cm;
};


#endif //DYNAMIC_SKETCH_ERROR_CM_H
