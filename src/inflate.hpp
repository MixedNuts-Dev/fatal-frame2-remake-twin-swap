// FATAL FRAME II: Crimson Butterfly REMAKE — TwinSwap
// Created by MixedNuts - https://github.com/MixedNutsJP/fatal-frame2-remake-twin-swap
// Licensed under the MIT License. See LICENSE for details.
//
// zlib 形式（RFC 1950 / 1951）の展開だけを行う最小実装。
// fdata の圧縮チャンクを読むためだけに使うので、圧縮側は持たない。
// 出力の長さは呼び出し側が知っている（rdb に展開後のサイズがある）ので、
// 範囲外の書き込みと読み込みはすべて失敗として扱う。

#pragma once
#include <cstdint>
#include <cstddef>
#include <vector>

namespace inflate {

namespace detail {

struct Bits {
    const uint8_t* p;
    size_t         n;
    size_t         pos = 0;
    uint32_t       buf = 0;
    int            cnt = 0;
    bool           bad = false;

    int Get(int need)
    {
        while (cnt < need)
        {
            if (pos >= n) { bad = true; return 0; }
            buf |= static_cast<uint32_t>(p[pos++]) << cnt;
            cnt += 8;
        }
        const int v = static_cast<int>(buf & ((1u << need) - 1));
        buf >>= need;
        cnt -= need;
        return v;
    }
};

// 正準ハフマン符号の表（符号長ごとの個数と、符号順に並べた記号）
struct Huffman {
    uint16_t count[16]{};
    uint16_t symbol[320]{};

    bool Build(const uint8_t* lengths, int n)
    {
        for (auto& c : count) c = 0;
        for (int i = 0; i < n; ++i) count[lengths[i]]++;
        if (count[0] == n) return true;          // 符号が 1 つも無い（距離符号で起こりうる）
        int left = 1;
        for (int len = 1; len < 16; ++len)
        {
            left <<= 1;
            left -= count[len];
            if (left < 0) return false;          // 符号が多すぎる
        }
        uint16_t offs[16]{};
        for (int len = 1; len < 15; ++len) offs[len + 1] = offs[len] + count[len];
        for (int i = 0; i < n; ++i)
            if (lengths[i]) symbol[offs[lengths[i]]++] = static_cast<uint16_t>(i);
        return true;
    }

    int Decode(Bits& b) const
    {
        int code = 0, first = 0, index = 0;
        for (int len = 1; len < 16; ++len)
        {
            code |= b.Get(1);
            if (b.bad) return -1;
            const int c = count[len];
            if (code - c < first) return symbol[index + (code - first)];
            index += c;
            first += c;
            first <<= 1;
            code <<= 1;
        }
        return -1;
    }
};

constexpr uint16_t kLenBase[29] = {3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
                                   35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};
constexpr uint8_t  kLenExtra[29] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
                                    3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};
constexpr uint16_t kDistBase[30] = {1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
                                    257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
                                    8193, 12289, 16385, 24577};
constexpr uint8_t  kDistExtra[30] = {0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
                                     7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13};

inline bool Codes(Bits& b, std::vector<uint8_t>& out, size_t limit,
                  const Huffman& lit, const Huffman& dist)
{
    for (;;)
    {
        int sym = lit.Decode(b);
        if (sym < 0) return false;
        if (sym < 256)
        {
            if (out.size() >= limit) return false;
            out.push_back(static_cast<uint8_t>(sym));
            continue;
        }
        if (sym == 256) return true;
        sym -= 257;
        if (sym >= 29) return false;
        const size_t len = kLenBase[sym] + b.Get(kLenExtra[sym]);
        const int ds = dist.Decode(b);
        if (ds < 0 || ds >= 30) return false;
        const size_t d = kDistBase[ds] + b.Get(kDistExtra[ds]);
        if (b.bad || d > out.size() || out.size() + len > limit) return false;
        const size_t from = out.size() - d;
        for (size_t i = 0; i < len; ++i) out.push_back(out[from + i]);   // 重なりありのコピー
    }
}

} // namespace detail

// zlib ストリーム 1 つを展開して out の末尾に足す。out の長さは limit を超えない。
inline bool Zlib(const uint8_t* src, size_t n, std::vector<uint8_t>& out, size_t limit)
{
    using namespace detail;
    if (n < 2 || (src[0] & 0x0F) != 8 || ((src[0] << 8) | src[1]) % 31 != 0 || (src[1] & 0x20))
        return false;
    Bits b{src + 2, n - 2};

    int last = 0;
    do
    {
        last = b.Get(1);
        const int type = b.Get(2);
        if (b.bad) return false;
        if (type == 0)                                   // 無圧縮ブロック
        {
            b.buf = 0;
            b.cnt = 0;
            if (b.pos + 4 > b.n) return false;
            const size_t len = b.p[b.pos] | (b.p[b.pos + 1] << 8);
            const size_t nlen = b.p[b.pos + 2] | (b.p[b.pos + 3] << 8);
            b.pos += 4;
            if (len != (~nlen & 0xFFFF) || b.pos + len > b.n || out.size() + len > limit) return false;
            out.insert(out.end(), b.p + b.pos, b.p + b.pos + len);
            b.pos += len;
        }
        else if (type == 1)                              // 固定ハフマン
        {
            static Huffman lit, dist;
            static bool ready = [] {
                uint8_t l[288];
                int i = 0;
                for (; i < 144; ++i) l[i] = 8;
                for (; i < 256; ++i) l[i] = 9;
                for (; i < 280; ++i) l[i] = 7;
                for (; i < 288; ++i) l[i] = 8;
                lit.Build(l, 288);
                uint8_t d[30];
                for (auto& x : d) x = 5;
                dist.Build(d, 30);
                return true;
            }();
            (void)ready;
            if (!Codes(b, out, limit, lit, dist)) return false;
        }
        else if (type == 2)                              // 動的ハフマン
        {
            static constexpr uint8_t order[19] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
            const int nlen = b.Get(5) + 257, ndist = b.Get(5) + 1, ncode = b.Get(4) + 4;
            if (b.bad || nlen > 286 || ndist > 30) return false;
            uint8_t lengths[320]{};
            for (int i = 0; i < ncode; ++i) lengths[order[i]] = static_cast<uint8_t>(b.Get(3));
            Huffman lencode;
            if (!lencode.Build(lengths, 19)) return false;
            int idx = 0;
            while (idx < nlen + ndist)
            {
                int sym = lencode.Decode(b);
                if (sym < 0) return false;
                if (sym < 16) { lengths[idx++] = static_cast<uint8_t>(sym); continue; }
                uint8_t v = 0;
                int rep = 0;
                if (sym == 16)
                {
                    if (idx == 0) return false;
                    v = lengths[idx - 1];
                    rep = 3 + b.Get(2);
                }
                else if (sym == 17) rep = 3 + b.Get(3);
                else rep = 11 + b.Get(7);
                if (b.bad || idx + rep > nlen + ndist) return false;
                while (rep--) lengths[idx++] = v;
            }
            if (lengths[256] == 0) return false;
            Huffman lit, dist;
            if (!lit.Build(lengths, nlen) || !dist.Build(lengths + nlen, ndist)) return false;
            if (!Codes(b, out, limit, lit, dist)) return false;
        }
        else return false;
    } while (!last);
    return !b.bad;
}

} // namespace inflate
