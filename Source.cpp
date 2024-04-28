#include <stdio.h>
#include <iostream>
#include <fstream>
#include <random>
//fast_log abs(rel) : avgError = 2.85911e-06(3.32628e-08), MSE = 4.67298e-06(5.31012e-08), maxError = 1.52588e-05(1.7611e-07)
const float s_log_C0 = -19.645704f;
const float s_log_C1 = 0.767002f;
const float s_log_C2 = 0.3717479f;
const float s_log_C3 = 5.2653985f;
const float s_log_C4 = -(1.0f + s_log_C0) * (1.0f + s_log_C1) / ((1.0f + s_log_C2) * (1.0f + s_log_C3)); //ensures that log(1) == 0
const float s_log_2 = 0.6931472f;

// assumes x > 0 and that it's not a subnormal.
// Results for 0 or negative x won't be -Infinity or NaN
 float my_log0(float x)
{
    int xint = reinterpret_cast<int&>(x);
    int e = (xint - 0x3f800000) >> 23; //e = exponent part can be negative
    xint |= 0x3f800000;
   xint &= 0x3fffffff; // 1 <= x < 2  after replacing the exponent field
    x = reinterpret_cast<float&>(xint);
    float a = (x + s_log_C0) * (x + s_log_C1);
    float b = (x + s_log_C2) * (x + s_log_C3);
    float c = (float(e) + s_log_C4);
    float d = a / b;
    return (c + d) * s_log_2;
}

int main()
{

    std::default_random_engine rd(0);//ãåíåðàòîð ñëó÷àéíûõ ÷èñåë
    std::uniform_real_distribution<float> dist1(-1.5f, 1.5f);
    float* S0 = new float[16];
    float* my_log = new float[16];
    float* logarithm = new float[16];
   
    for (int i = 0; i < 16; i++)
    {

        S0[i] = dist1(rd);
    }

    for (int i = 0; i < 16; i++)
    {
        logarithm[i] = std::log(S0[i]);
        my_log[i] = my_log0(S0[i]);

    }
    float* diff = new float[16];
    float max_diff = 0.0;
    for (int i = 0; i < 16; i++)
    {
        diff[i] = abs(my_log[i] - logarithm[i]);
        if (diff[i] > max_diff)
        {
            max_diff = diff[i];
        }
        printf("x=%.7ef\nmy_log=%.12ef\nlog=%.7ef\ndiff=%.7ef\nmax_diff=%.7ef\n",
            S0[i], my_log[i], logarithm[i], diff[i], max_diff);
    }



    delete[] S0;
    delete[] my_log;
    delete[] logarithm;
    delete[] diff;


}