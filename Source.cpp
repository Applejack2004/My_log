#include <stdio.h>
#include <cmath>
#include <random>
#include <vector>

#include <iostream>
#include <riscv-vector.h>

vfloat32m4_t my_log0(vfloat32m4_t x, int vl)
{

    //fast_log abs(rel) : avgError = 2.85911e-06(3.32628e-08), MSE = 4.67298e-06(5.31012e-08), maxError = 1.52588e-05(1.7611e-07)
    float s_log_C0 = -19.645704f;
    float s_log_C1 = 0.767002f;
    float s_log_C2 = 0.3717479f;
    float s_log_C3 = 5.2653985f;
    float s_log_2 = 0.6931472f;
    float s_log_C4 = -(1.0f + s_log_C0) * (1.0f + s_log_C1) / ((1.0f + s_log_C2) * (1.0f + s_log_C3)); //ensures that log(1) == 0


    // int e = static_cast<int>(ux - 0x3f800000) >> 23;
    vint32m4_t intx = vreinterpret_v_f32m4_i32m4(x);
    vint32m4_t exp = vsub_vx_i32m4(intx, 0x3f800000, vl);
    exp = vsra_vx_i32m4(exp, 23, vl);
    // ux |= 0x3f800000;
    // ux &= 0x3fffffff; // 1 <= x < 2  after replacing the exponent field
    intx = vor_vx_i32m4(intx, 0x3f800000, vl);
    intx = vand_vx_i32m4(intx, 0x3fffffff, vl);

    // x = reinterpret_cast<float&>(ux);
    vfloat32m4_t y = vreinterpret_v_i32m4_f32m4(intx);

    //float a = (x + s_log_C0) * (x + s_log_C1);
    vfloat32m4_t a = vfadd_vf_f32m4(y, s_log_C0, vl);
    vfloat32m4_t tmp = vfadd_vf_f32m4(y, s_log_C1, vl);
    a = vfmul_vv_f32m4(a, tmp, vl);
    // float b = (x + s_log_C2) * (x + s_log_C3);

    vfloat32m4_t b = vfadd_vf_f32m4(y, s_log_C2, vl);
    vfloat32m4_t tmp2 = vfadd_vf_f32m4(y, s_log_C3, vl);
    b = vfmul_vv_f32m4(b, tmp2, vl);
    //float c = (float(e) + s_log_C4);

    vfloat32m4_t e = vfcvt_f_x_v_f32m4(exp, vl);
    vfloat32m4_t c = vfadd_vf_f32m4(e, s_log_C4, vl);

    // float d = a / b;
    vfloat32m4_t d = vfdiv_vv_f32m4(a, b, vl);


    //(c + d)* s_log_2;
    vfloat32m4_t c_plus_d = vfadd_vv_f32m4(c, d, vl);
    vfloat32m4_t result = vfmul_vf_f32m4(c_plus_d, s_log_2, vl);
    return result;
}

int main()
{
    std::default_random_engine rd(0);//генератор случайных чисел
    std::uniform_real_distribution<float> dist1(-5.0f, 10.0f);
    float* S0 = new float[16];
    float* my_log = new float[16];
    float* logarithm = new float[16];

    for (int i = 0; i < 16; i++)
    {

        S0[i] = dist1(rd);
    }

    vfloat32m4_t so = vle_v_f32m4(S0, 16);
    vfloat32m4_t log_val = my_log0(S0, 16);
    vse_v_f32m4(my_log, log_val, 16);

    for (int i = 0; i < 16; i++)
    {
        logarithm[i] = std::log(S0[i]);
    }
    float* diff = new float[16];
    float max_diff = 0.0f;
    for (int i = 0; i < 16; i++)
    {
        diff[i] = abs(my_log[i] - logarithm[i]);
        if (diff[i] > max_diff) max_diff = diff[i];
        printf("x=%.12ef\nmy_erf=%.12ef\nerf=%.12ef\ndiff=%.12ef max_diff=%.12ef\n",
            S0[i], my_log[i], logarithm[i], diff[i], max_diff);
    }






    return 0;
}