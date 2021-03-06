﻿#include <array>
#include <iostream>
#include "common.h"
#include "GrayscaleFilter.h"

using namespace std;
using namespace ragii::image;

namespace
{
    constexpr float WEIGHT_B = 0.298912f;
    constexpr float WEIGHT_G = 0.586611f;
    constexpr float WEIGHT_R = 0.114478f;
}  // namespace

void GrayscaleFilter::apply()
{
    int w = m_Params.width;
    int h = m_Params.height;
    int d = m_Params.bitCount / 8;
    uint8_t* img = m_Params.image;

    if (d != 4) {
        cout << "depth " << d << " not supported." << endl;
        return;
    }

    // 係数
    __m128 weight_b = _mm_set_ps1(WEIGHT_B);
    __m128 weight_g = _mm_set_ps1(WEIGHT_G);
    __m128 weight_r = _mm_set_ps1(WEIGHT_R);

    // 配置先
    __m128i mapping_b = _mm_setr_epi8(0, -1, -1, -1, 4, -1, -1, -1, 8, -1, -1, -1, 12, -1, -1, -1);
    __m128i mapping_g = _mm_setr_epi8(1, -1, -1, -1, 5, -1, -1, -1, 9, -1, -1, -1, 13, -1, -1, -1);
    __m128i mapping_r = _mm_setr_epi8(2, -1, -1, -1, 6, -1, -1, -1, 10, -1, -1, -1, 14, -1, -1, -1);
    __m128i mapping_result = _mm_setr_epi8(0, 0, 0, -1, 0, 0, 0, -1, 0, 0, 0, -1, 0, 0, 0, -1);

    __m128i src;
    __m128i shuffle_b, shuffle_g, shuffle_r, shuffle_result;
    __m128 s_b, s_g, s_r;
    __m128 m_b, m_g, m_r;
    __m128 s_gray;
    __m128i i_gray;

    for (int i = 0; i < w * h; i += 4) {
        // TODO:
        // グレースケールにはなったけど・・・なぜがモザイクが掛かったようになってしまった。直す。

        // 4px (4bytes/px * 4) ロード
        src = _mm_load_si128(reinterpret_cast<__m128i*>(img));

        // 並べ替え
        shuffle_b = _mm_shuffle_epi8(src, mapping_b);
        shuffle_g = _mm_shuffle_epi8(src, mapping_g);
        shuffle_r = _mm_shuffle_epi8(src, mapping_r);

        // float化(係数の型に合わせる)
        s_b = _mm_cvtepi32_ps(shuffle_b);
        s_g = _mm_cvtepi32_ps(shuffle_g);
        s_r = _mm_cvtepi32_ps(shuffle_r);

        // 乗算
        m_b = _mm_mul_ps(s_b, weight_b);
        m_g = _mm_mul_ps(s_g, weight_g);
        m_r = _mm_mul_ps(s_r, weight_r);

        // 加算
        s_gray = _mm_add_ps(_mm_add_ps(m_b, m_g), m_r);

        // uint8化(格納する型に合わせる)
        i_gray = _mm_cvtps_epi32(s_gray);

        // 並べ替え
        shuffle_result = _mm_shuffle_epi8(i_gray, mapping_result);

        // 格納
        _mm_store_si128(reinterpret_cast<__m128i*>(img), shuffle_result);

        img += 16;
    }
}
