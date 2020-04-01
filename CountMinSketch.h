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
#include <unordered_set>
#include <boost/current_function.hpp>


#if 0
#define PRINT_DBG(...) do { printf("====%s====\n", BOOST_CURRENT_FUNCTION); } while(0)
#else
#define PRINT_DBG() do {} while(0)
#endif


class CountMinSketch {
  public:
    CountMinSketch(int width, int depth, int seed) : cm(CM_Init(width, depth, 1)) {
        PRINT_DBG();
        if (!cm) throw std::bad_alloc();
    }

    CountMinSketch(const CountMinSketch& other)
            : cm(CM_Copy(other.cm)), allItems(other.allItems) {
        PRINT_DBG();
        if (!cm) throw std::bad_alloc();
    }

    CountMinSketch(CountMinSketch&& other) noexcept : cm(other.cm), allItems(other.allItems) {
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
        std::swap(allItems, other.allItems);
        return *this;
    }

    ~CountMinSketch() {
        PRINT_DBG();
        CM_Destroy(cm);
    }

    [[nodiscard]] unsigned size() const {
        PRINT_DBG();
        return CM_Size(cm);
    }

    void update(unsigned int item, int diff) {
        PRINT_DBG();
        allItems.insert(item);
        return CM_Update(cm, item, diff);
    }

    [[nodiscard]] unsigned load() const { return allItems.size(); }

    [[nodiscard]] int estimate(unsigned int item) const {
        PRINT_DBG();
        return CM_PointEst(cm, item);
    }

    [[nodiscard]] std::vector<std::pair<int, int> > heavyHitters(double threshold) const {
        std::vector<std::pair<int, int> > data;
        data.reserve(allItems.size());

        for (auto item : allItems) {
            data.emplace_back(item, estimate(item));
        }

        auto nth_index = unsigned(data.size() * threshold);
        std::nth_element(data.begin(),
                         data.begin() + nth_index,
                         data.end(),
                         [](const auto& p1, const auto& p2) { return p1.second < p2.second; });

        return decltype(data)(data.begin() + nth_index, data.end());
    }

    [[nodiscard]] std::vector<std::pair<int, int> > heavyHitters(int threshold) const {
        std::vector<std::pair<int, int> > data;

        for (auto item : allItems) {
            auto est = estimate(item);
            if (est >= threshold)
                data.emplace_back(item, est);
        }

        return data;
    }

  private:
    CM_type* cm;

    std::unordered_set<int> allItems;
};


#endif //DYNAMIC_SKETCH_ERROR_CM_H
