#include <stdio.h>
#include <cmath>
#include <random>
#include <vector>

#include <iostream>
#include <riscv-vector.h>
void FLOAT2INT(vfloat32m4_t& f, vfloat32m4_t& rf, vint32m4_t& fint, int _vl)
{
    vfloat32m4_t tmp = vfadd_vf_f32m4(f, 12582912.0f, _vl);
    rf = vfsub_vf_f32m4(tmp, 12582912.0f, _vl);
    fint = vreinterpret_v_f32m4_i32m4(tmp);

    // fint = vand_vx_i32m4(fint, 0xFFFF, _vl);
    //d = d & 0x00000000ffffffff; - это наверное уже есть
 //d = d | ((d&0x0000000080000000)*0x00000001fffffffe);
    fint = vand_vx_i32m4(fint, 0x0000ffff, _vl);
    vint32m4_t finttmp = vand_vx_i32m4(fint, 0x00008000, _vl);
    finttmp = vmul_vx_i32m4(finttmp, 0x0001ffff, _vl);
    fint = vor_vv_i32m4(fint, finttmp, _vl);


}



vfloat32m4_t my_exp0(vfloat32m4_t x, int vl) {
    vfloat32m4_t y, kf;
    vint32m4_t ki;


    float Log2 = (float)0x1.62e43p-1;
    float Log2h = (float)0xb.17200p-4;
    float Log2l = (float)0x1.7f7d1cf8p-20;
    float InvLog2 = (float)0x1.715476p0;

    // Here should be the tests for exceptional cases
    vfloat32m4_t x_mult_InvLog2 = vfmul_vf_f32m4(x, InvLog2, vl);
    FLOAT2INT(x_mult_InvLog2, kf, ki, vl);
    int* kmas = new int[16];
    vse_v_i32m4(kmas, ki, vl);
    for (int i = 0; i < vl; i++)
    {
        std::cout << "k" << i << "=" << kmas[i] << "";
    }
    ki = vle_v_i32m4(kmas, vl);

    //y = (x - kf*Log2h) - kf*Log2l;
    vfloat32m4_t kf_mult_Log2h = vfmul_vf_f32m4(kf, Log2h, vl);
    vfloat32m4_t kf_mult_Log2l = vfmul_vf_f32m4(kf, Log2l, vl);
    vfloat32m4_t  x_sub_kflog2h = vfsub_vv_f32m4(x, kf_mult_Log2h, vl);
    y = vfsub_vv_f32m4(x_sub_kflog2h, kf_mult_Log2l, vl);

    vfloat32m4_t result;
    result = vfmul_vf_f32m4(y, (float)0x1.6850e4p-10, vl);
    result = vfadd_vf_f32m4(result, (float)0x1.123bccp-7, vl);
    result = vfmul_vv_f32m4(y, result, vl);
    result = vfadd_vf_f32m4(result, (float)0x1.555b98p-5, vl);
    result = vfmul_vv_f32m4(y, result, vl);
    result = vfadd_vf_f32m4(result, (float)0x1.55548ep-3, vl);
    result = vfmul_vv_f32m4(y, result, vl);
    result = vfadd_vf_f32m4(result, (float)0x1.fffff8p-2, vl);
    result = vfmul_vv_f32m4(y, result, vl);
    result = vfadd_vf_f32m4(result, (float)0x1p0, vl);
    result = vfmul_vv_f32m4(y, result, vl);
    result = vfadd_vf_f32m4(result, (float)0x1p0, vl);

    /* r.f = (float)0x1p0 + y * ((float)0x1p0
                        + y * ((float)0x1.fffff8p-2
                        + y * ((float)0x1.55548ep-3
                        + y * ((float)0x1.555b98p-5
                        + y * ((float)0x1.123bccp-7
                        + y *  (float)0x1.6850e4p-10)))));*/

                        //  r.i16[HI] += k << 7; //Exponent update
    vint32m4_t resint = vreinterpret_v_f32m4_i32m4(result);
    ki = vsll_vx_i32m4(ki, 23, vl);
    resint = vadd_vv_i32m4(resint, ki, vl);
    vfloat32m4_t exp_vect = vreinterpret_v_i32m4_f32m4(resint);

    return exp_vect;
    //return result;
}
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