#include <iostream>
#include <numeric>
#include <boost/assign.hpp>
#include "SketchRing.h"


using namespace std;
using namespace boost;

unsigned hashf(int value) {
    unsigned temp = ((value % 3451) + 1) * (value ^ 0x238bc1) + (1 + value * 13);
    return temp ^ std::hash<int>{}(value);
}

int SketchRing::size() const {
    return cms.begin()->second.size() * cms.size();
}

void SketchRing::update(unsigned int item, int diff) {
    auto hash   = hashf(item) % domainMax;
    auto bucket = buckets.find(hash)->second;
//    cout << item << " bucket:" << bucket << endl;
    cms.at(bucket).update(item, diff);
}

int SketchRing::estimate(unsigned int item) const {
    auto hash = hashf(item) % domainMax;
    return cms.at(buckets.find(hash)->second).estimate(item);
}

SketchRing::SketchRing(int width, int depth, int n,
                       Point domainMax) : buckets(make_pair(0, 0)), domainMax(domainMax) {
    auto interval = double(domainMax) / n;

    for (Point high = interval, low = 1; low < domainMax; low = high, high += interval) {

//        std::cout << "I:" << high << ":" << buckets.iterative_size() << std::endl;
        cms.emplace(piecewise_construct,
                    forward_as_tuple(low),
                    forward_as_tuple(width, depth, 1));
        buckets.insert(make_pair(icl::interval<Point>::type(low, high), low));
    }
}

auto erase(SketchRing::IntervalMap& buckets, SketchRing::IntervalMap::const_iterator it) {
    bool       isFirst  = it == buckets.begin();
    const auto erasedIv = it->first;

    auto prevBucket = isFirst ? buckets.rbegin()->second : (--it)->second;

    buckets.set(make_pair(erasedIv, prevBucket));

    return prevBucket;
}

void SketchRing::shrink() {
    // find least loaded sketch
    unsigned  minLoad = 0;
    auto      minIter = cms.begin();
    for (auto it      = cms.begin(); it != cms.end(); ++it) {
        auto load = it->second.load();
        if (load > minLoad) {
            minLoad = load;
            minIter = it;
        }
    }

    // now we have the min
    const auto& sketch = minIter->second;
    const auto location = minIter->first;
    const auto& hh = sketch.heavyHitters(.6);

    auto toAdd = erase(buckets, buckets.find(location));

    for (auto[hitter, freq]: hh)
        cms.at(toAdd).update(hitter, freq);

    cms.erase(minIter);
}

void SketchRing::expand() {
    throw std::bad_function_call();
    // find most loaded sketch
    unsigned  maxLoad = 0;
    auto      maxIter = cms.begin();
    for (auto it      = cms.begin(); it != cms.end(); ++it) {
        auto load = it->second.load();
        if (load > maxLoad) {
            maxLoad = load;
            maxIter = it;
        }
    }

    // now we have the max
    const auto& sketch = maxIter->second;
    const auto location = maxIter->first;
    const auto& hh = sketch.heavyHitters(.4);

    auto toAdd = erase(buckets, buckets.find(location));

    for (auto[hitter, freq]: hh)
        cms.at(toAdd).update(hitter, freq);

    cms.erase(maxIter);

    throw std::exception();
}

void SketchRing::resize(int n) {
    if (n <= 2) throw out_of_range("can't shrink below 2");

    while (cms.size() > n) {
        shrink();
    }
    while (cms.size() < n) {
        expand();
    }
}

std::vector<std::pair<int, int> > SketchRing::heavyHitters(int threshold) const {
    std::vector<std::pair<int, int> > res;

    for (const auto&[p, cm] : cms) {
        const auto& hh = cm.heavyHitters(threshold);
        res.insert(res.end(), hh.begin(), hh.end());
    }

    return res;
}


extern "C" {
#include "SketchRingC.h"

SK_type* SK_init(int width, int depth, int n, int domainMax) {
    return new SketchRing(width, depth, n, domainMax);
}

void SK_destroy(SK_type* sr) {
    delete sr;
}

int SK_size(const SK_type* sr) {
    return sr->size();
}

void SK_update(SK_type* sr, unsigned int item, int diff) {
    sr->update(item, diff);
}

int SK_estimate(const SK_type* sr, unsigned int item) {
    return sr->estimate(item);
}

void SK_resize(SK_type* sr, int n) {
    sr->resize(n);
}

//void HH_free(HH* hh) {
//    free(hh->freq);
//    free(hh->items);
//    *hh = {0, 0, 0};
//}
//
//HH SK_heavyHitters(const SK_type* sk, int threshold) {
//    HH res;
//    const auto& hh = sk->heavyHitters(threshold);
//
//    res.size = hh.size();
//    res.items = static_cast<int*>(malloc(sizeof(int) * res.size));
//    res.freq = static_cast<int*>(malloc(sizeof(int) * res.size));
//
//    for (int i = 0; i < res.size; ++i) {
//        res.items[i] = hh[i].first;
//        res.freq[i] = hh[i].second;
//    }
//
//    return res;
//}
int* SK_heavyHitters(const SK_type* sk, int threshold) {
    const auto& hh = sk->heavyHitters(threshold);
    unsigned size = hh.size();
    int* res = static_cast<int*>(calloc(size+1, (sizeof(int))));

    res[0] = size;

    for (int i = 1; i <= res[0]; ++i) {
        res[i] = hh[i-1].first;
    }

    return res;
}

}