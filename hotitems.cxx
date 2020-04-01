/********************************************************************
Approximate frequent items in a data stream
G. Cormode 2002, 2003

Last modified: 2003-12

*********************************************************************/
extern "C" {
#include "massdalsketches/prng.h"
#include "massdalsketches/massdal.h"

/******************************************************************/

#include "massdalsketches/cgt.h"
#include "massdalsketches/lossycount.h"
#include "massdalsketches/frequent.h"
#include "massdalsketches/ccfc.h"
#include "massdalsketches/countmin.h"

/******************************************************************/

#include "SketchRingC.h"
}

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <string>
using std::to_string;
using std::string;

float zipfpar, phi;
int   range;
int* exact;
int       d;
int       n     = 1000, lgn = 10;
int       width = 10, depth = 2, gran = 1;
int       netpos;
int       quartiles[4], max;
long long sumsq;

/******************************************************************/

void CheckArguments(int argc, char** argv) {

    /*****************************************************************/
    /* Examine the command line arguments.  Pick up the file to read */
    /* from the first argument, and also look for other parameters   */
    /*****************************************************************/

    int failed = 0;

    if (argc > 1)
        range = atoi(argv[1]);
    else range = 623456;

    if (argc > 2)
        zipfpar = atof(argv[2]);
    else
        zipfpar = 1.1;

    if (argc > 3)
        phi = atof(argv[3]);
    else phi = 0.01;

    d = (int) ceil(1.0 / phi);

    if (argc > 4)
        width = atoi(argv[4]);
    else width = 512;

    if (argc > 5)
        depth = atoi(argv[5]);
    else
        depth = 5;

    if (argc > 6)
        gran = atoi(argv[6]);
    else gran = 1;

    if (zipfpar < 0.0) failed = 1;
    if (range <= 0) failed    = 1;
    if (gran <= 0) failed     = 1;
    if (phi <= 0.0) failed    = 1;
    if (width <= 0) failed    = 1;
    if (depth <= 0) failed    = 1;

    if (failed == 1) {
        printf("%s range zipfpar phi width depth gran\n", argv[0]);
        printf("range = number of items to process \n");
        printf("zipfpar = parameter of zipf dbn. 0.0 = uniform. 3+ = skewed\n");
        printf("phi = threshold for hot items.  Default = 0.01\n");
        printf("width = width of sketch to keep. Default = 512\n");
        printf("depth = depth of sketch to keep. Default = 5\n");
        printf("gran = granularity to process at (in bits).  Default =1\n");
        exit(1);
    }
}


/******************************************************************/

int* CreateStream(int length) {
    long  a, b;
    float zet;
    int   i;
    long  value;
    int      * stream;
    prng_type* prng;

    n      = 1048575;
    exact  = (int*) calloc(n + 1, sizeof(int));
    stream = (int*) calloc(length + 1, sizeof(int));

    prng = prng_Init(44545, 2);
    a    = (long long) (prng_int(prng) % MOD);
    b    = (long long) (prng_int(prng) % MOD);

    netpos = length;

    zet = zeta(length, zipfpar);

    for (i = 1; i <= length; i++) {
        value = hash31(a, b, ((int) floor(fastzipf(zipfpar, n, zet, prng)))) % n;
        exact[value]++;
        stream[i] = value;
    }

    prng_Destroy(prng);
    lgn = 20;

    return (stream);
}

/******************************************************************/

void CheckOutput(const char* title,
                 const unsigned int resultlist[],
                 int thresh,
                 int hh,
                 int upt,
                 int space) {

    int          i;
    unsigned int correct = 0;
    unsigned int claimed = 0;
    unsigned int last    = 0;
    float        recall, precision;

    for (i = 1; i <= resultlist[0]; i++) {
        if (resultlist[i] != last) {
            correct += int(exact[resultlist[i]] >= thresh);
            last = resultlist[i];
        }
    }

    claimed = resultlist[0];

    if (hh == 0)
        printf("%s\t--\t--\t%d\t%d\n", title, space, upt);
    else {
        recall    = 100.0f * ((float) correct) / ((float) hh);
        precision = 100.0f * ((float) correct) / ((float) claimed);
        printf("%s\t%1.2f\t%1.2f\t%d\t%d\n", title, recall, precision, space, upt);
    }
}

/******************************************************************/

int RunExact(int thresh) {
    int i, hh, sum, j;
    StartTheClock();
    hh  = 0;
    sum = -1;

    j      = 0;
    sumsq  = 0;
    for (i = 0; i < n; i++) {
        sum += exact[i];
        sumsq += (long long) exact[i] * (long long) exact[i];
        if (exact[i] > 0) max = i;
        while (sum >= 0) {
            quartiles[j] = i;
            j++;
            sum -= netpos / 4;
        }
        if (exact[i] >= thresh) {
            hh++;
        }
    }
    printf("Exact used %lu bytes, took %ld ms.  Zipfparam = %f\n",
           n * sizeof(int), StopTheClock(), zipfpar);
    printf("There were %d items above threshold of %d, for phi=%f, n=%d\n",
           hh, thresh, phi, range);
    return (hh);
}


/******************************************************************/

int main(int argc, char** argv) {
    int i, uptime, outtime;
    int thresh, hh;
    unsigned int* uilist;
    int         * stream;

    CGT_type * cgt;
    LC_type  * lc;
    freq_type* freq;
    CCFC_type* ccfc;
    CMH_type * cmh;
    SK_type  * sk;

    CheckArguments(argc, argv);

    printf("____________________________________________________________\n");
    printf("%s compiled at %s, %s\n", __FILE__, __TIME__, __DATE__);

    if (d <= 0) exit(1);

    stream = CreateStream(range);

    thresh = floor(phi * netpos);
    if (thresh == 0) thresh = 1;

    printf("\nTesting finding frequent Items...\n\n");

    hh = RunExact(thresh);

    printf("\nMethod\tRecall\tPrecis\tSpace\tUpd/ms\n");

    sk = SK_init(width, depth, 16, n + 1);
    StartTheClock();
    for (i = 1; i <= range; i++)
        if (stream[i] > 0)
            SK_update(sk, stream[i], 1);
        else
            SK_update(sk, -stream[i], -1);
    uptime = StopTheClock();
    StartTheClock();
    uilist  = (unsigned int*) SK_heavyHitters(sk, thresh);
    outtime = StopTheClock();
    CheckOutput((string("SK") + to_string(16)).c_str(),
                uilist,
                thresh,
                hh,
                uptime,
                SK_size(sk));
    free(uilist);
    SK_destroy(sk);

//    for (int size = 2; size <= 32; size += 2) {
//        sk = SK_init(width, depth, size, n+1);
//        StartTheClock();
//        for (i = 1; i <= range; i++)
//            if (stream[i] > 0)
//                SK_update(sk, stream[i], 1);
//            else
//                SK_update(sk, -stream[i], -1);
//        uptime = StopTheClock();
//        StartTheClock();
//        uilist  = (unsigned int*) SK_heavyHitters(sk, thresh);
//        outtime = StopTheClock();
//        CheckOutput((string("SK") + to_string(size)).c_str(),
//                    uilist,
//                    thresh,
//                    hh,
//                    uptime,
//                    SK_size(sk));
//        free(uilist);
//        SK_destroy(sk);
//    }

//    for (int size = 2; size <= 32; size += 2) {
//        sk = SK_init(8*width / size, depth, size, n+1);
//        StartTheClock();
//        for (i = 1; i <= range; i++)
//            if (stream[i] > 0)
//                SK_update(sk, stream[i], 1);
//            else
//                SK_update(sk, -stream[i], -1);
//        uptime = StopTheClock();
//        StartTheClock();
//        uilist  = (unsigned int*) SK_heavyHitters(sk, thresh);
//        outtime = StopTheClock();
//        CheckOutput((string("SK") + to_string(size)).c_str(),
//                    uilist,
//                    thresh,
//                    hh,
//                    uptime,
//                    SK_size(sk));
//        free(uilist);
//        SK_destroy(sk);
//    }

//    SK_type  * sk0;
//    SK_type  * sk1;
//    for (int reductions = 1; reductions < 16; ++reductions) {
//        printf("%d\n", reductions);
//
//        sk = SK_init(width/4, depth, 32, n+1);
//        sk0 = SK_init(width/4, depth, 32, n+1);
//        sk1 = SK_init(width/4, depth, 32-reductions, n+1);
//        StartTheClock();
//        for (i = 1; i <= range; i++) {
//            if (stream[i] > 0) {
//                SK_update(sk0, stream[i], 1);
//                SK_update(sk, stream[i], 1);
//                SK_update(sk1, stream[i], 1);
//            } else {
//                SK_update(sk0, -stream[i], -1);
//                SK_update(sk, -stream[i], -1);
//                SK_update(sk1, -stream[i], -1);
//            }
//
//            if (i % (range/2) == 0 && i != range) {
////                printf(">>>>>>>%d\n", i);
//                SK_resize(sk, 32-reductions);
//            }
//        }
//
//        uptime = StopTheClock();
//        uilist  = (unsigned int*) SK_heavyHitters(sk0, thresh);
//        CheckOutput("SK0", uilist, thresh, hh, uptime, SK_size(sk0));
//        uilist  = (unsigned int*) SK_heavyHitters(sk1, thresh);
//        CheckOutput("SK1", uilist, thresh, hh, uptime, SK_size(sk1));
//        uilist  = (unsigned int*) SK_heavyHitters(sk, thresh);
//        CheckOutput("SK", uilist, thresh, hh, uptime, SK_size(sk));
//        free(uilist);
//        SK_destroy(sk);
//    }




//////////////////////////////////

    cmh = CMH_Init(width, depth, lgn, gran);
    StartTheClock();
    for (i = 1; i <= range; i++)
        if (stream[i] > 0)
            CMH_Update(cmh, stream[i], 1);
        else
            CMH_Update(cmh, -stream[i], -1);
    uptime = StopTheClock();
    StartTheClock();
    uilist  = (unsigned int*) CMH_FindHH(cmh, thresh);
    outtime = StopTheClock();
    CheckOutput("CM", uilist, thresh, hh, uptime, CMH_Size(cmh));
    free(uilist);

    ccfc = CCFC_Init(width, depth, lgn, gran);
    StartTheClock();
    for (i = 1; i <= range; i++)
        if (stream[i] > 0)
            CCFC_Update(ccfc, stream[i], 1);
        else
            CCFC_Update(ccfc, -stream[i], -1);
    uptime = StopTheClock();
    StartTheClock();
    uilist  = CCFC_Output(ccfc, thresh);
    outtime = StopTheClock();
    CheckOutput("CCFC", uilist, thresh, hh, uptime, CCFC_Size(ccfc));
    free(uilist);


    cgt = CGT_Init(width, depth, lgn, gran);
    StartTheClock();
    for (i = 1; i <= range; i++)
        if (stream[i] > 0)
            CGT_Update(cgt, stream[i], 1);
        else
            CGT_Update(cgt, -stream[i], -1);
    uptime = StopTheClock();
    StartTheClock();
    uilist  = CGT_Output(cgt, thresh);
    outtime = StopTheClock();
    CheckOutput("CGT", uilist, thresh, hh, uptime, CGT_Size(cgt));
    free(uilist);


    lc = LC_Init(phi);
    StartTheClock();
    for (i = 1; i <= range; i++)
        LC_Update(lc, stream[i]);
    uptime = StopTheClock();
    StartTheClock();
    uilist  = LC_Output(lc, thresh);
    outtime = StopTheClock();
    CheckOutput("LC", uilist, thresh, hh, uptime, LC_Size(lc));
    free(uilist);

    freq = Freq_Init(phi);
    StartTheClock();
    for (i = 1; i <= range; i++)
        Freq_Update(freq, stream[i]);
    uptime = StopTheClock();
    StartTheClock();
    uilist  = Freq_Output(freq, thresh);
    outtime = StopTheClock();
    CheckOutput("Freq", uilist, thresh, hh, uptime, Freq_Size(freq));
    free(uilist);

    printf("\nTesting finding Quantiles\n\n");
    printf("Approximate minimum is %d [should be %d]\n",
           CMH_Quantile(cmh, 0.0), quartiles[0]);
    printf("Approximate 1-quartile is %d [should be %d]\n",
           CMH_Quantile(cmh, 0.25), quartiles[1]);
    printf("Approximate median is %d [should be %d]\n",
           CMH_Quantile(cmh, 0.5), quartiles[2]);
    printf("Approximate 3-quartile is %d [should be %d]\n",
           CMH_Quantile(cmh, 0.75), quartiles[3]);
    printf("Approximate maximum is %d [should be %d]\n",
           CMH_Quantile(cmh, 0.999), max);

    CMH_Destroy(cmh);
    CCFC_Destroy(ccfc);
    CGT_Destroy(cgt);
    LC_Destroy(lc);
    Freq_Destroy(freq);

    printf("\n");
    /* Done! */
    return 0;
}

