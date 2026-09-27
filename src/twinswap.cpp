// FATAL FRAME II: Crimson Butterfly REMAKE — Twin Swap
// Created by MixedNuts - https://github.com/MixedNuts-Dev/fatal-frame2-remake-twin-swap
// Licensed under the MIT License. See LICENSE for details.
//
// 操作キャラ（澪）と同行キャラ（繭）の見た目を ini の指定で入れ替える。
// ゲームのルートに xinput1_4.dll として置くと自動的にロードされ、
// XInput 本来の機能は System32 の実体へ転送する。
//
// 仕組み:
//   キャラの枠（衣装ごとに高精細・軽量の 2 つ）は、2 つの kidsobjdb（0x2082ad97 /
//   0x97485e9b）の中でモデル定義を参照している。この参照を書き換えると、モデルと
//   それにぶら下がる専用設定（材質・骨・揺れもの）が丸ごと一緒に移る。
//
//   澪のモデルを同行キャラ（繭）に付けると、顔・歯・目まわりが描画されない。描画は
//   grp のグループ単位で表示が決まり、澪のモデルは顔を専用グループ 5526a88f に置いて
//   いるが、繭のキャラはこのグループを表示しないため。繭のモデルと同じく、顔の LOD
//   エントリを常時表示のグループ 0 の範囲へ移し（g1m）、グループ 0 を広げて顔の
//   グループを空にする（grp）。どちらもサイズは変わらない。
//
//   改変したファイルは Mod 専用の fdata にまとめ、root.rdb / root.rdx をそれを指す
//   ように書き換えたものと一緒に、Mods\twinswap\cache に作る。ゲームがこの 3 つを
//   開くときだけ、CreateFileW のフックでキャッシュへ差し替える。元にするのは
//   fdata_package にある現在の root.rdb / root.rdx なので、Yumia ツールで入れた
//   他の Mod（スカート丈など）はそのまま残る。ゲームのファイルは一切変更しない。
//
// フックは exe ではなく kernel32 の輸入テーブル（kernel32!CreateFileW が飛ぶ先）に
// 掛ける。Native 120FPS Option のローダが exe の輸入テーブルの CreateFileW を
// 起動直後に何度も掛け直すので、同じ場所に掛けると互いを「元の関数」として保存し
// 合って無限再帰になる。kernel32 側なら、そちらのフックの先で呼ばれるので干渉しない。
//
// ログは英語で書く（利用者が自分で状況を判断できるように）。コメントは日本語。

#include <windows.h>
#include <cstdio>
#include <cstdint>
#include <cstdarg>
#include <cstring>
#include <string>
#include <vector>
#include <unordered_map>

#include "inflate.hpp"

namespace {

constexpr char     kVersion[]  = "1.0.0";
constexpr char     kCacheTag[] = "twinswap-cache-v1";   // 生成ロジックを変えたら上げる
constexpr uint32_t kFdataHash  = 0xFFFE7510;

HMODULE      g_real = nullptr;
std::wstring g_gameDir;
std::wstring g_modDir;
std::wstring g_cacheDir;

bool g_enabled = true;
bool g_log     = true;
bool g_mainMayu = true;   // 操作キャラの見た目
bool g_subMio   = true;   // 同行キャラの見た目

// ---- ログ ---------------------------------------------------------------

void Log(const char* fmt, ...)
{
    if (!g_log || g_modDir.empty()) return;
    const std::wstring path = g_modDir + L"twinswap.log";
    const bool isNew = (GetFileAttributesW(path.c_str()) == INVALID_FILE_ATTRIBUTES);
    FILE* f = nullptr;
    if (_wfopen_s(&f, path.c_str(), L"a") != 0 || !f) return;
    if (isNew) fwrite("\xEF\xBB\xBF", 1, 3, f);

    SYSTEMTIME st{};
    GetLocalTime(&st);
    fprintf(f, "[%02d:%02d:%02d] ", st.wHour, st.wMinute, st.wSecond);
    va_list ap;
    va_start(ap, fmt);
    vfprintf(f, fmt, ap);
    va_end(ap);
    fputc('\n', f);
    fclose(f);
}

// ---- CreateFileW のフック -----------------------------------------------

using PFN_CreateFileW = HANDLE(WINAPI*)(LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES,
                                        DWORD, DWORD, HANDLE);
PFN_CreateFileW  g_origCreateFileW = nullptr;
CRITICAL_SECTION g_lock{};

// Mod 自身のファイル操作は、フックを通らない本来の関数で行う
HANDLE OpenRead(const std::wstring& path)
{
    return g_origCreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr,
                             OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
}

// ---- ファイル入出力 -----------------------------------------------------

bool ReadAt(HANDLE h, uint64_t off, void* dst, size_t n)
{
    LARGE_INTEGER li{};
    li.QuadPart = static_cast<LONGLONG>(off);
    if (!SetFilePointerEx(h, li, nullptr, FILE_BEGIN)) return false;
    auto p = static_cast<uint8_t*>(dst);
    size_t done = 0;
    while (done < n)
    {
        const size_t left = n - done;
        const DWORD want = static_cast<DWORD>(left > (16u << 20) ? (16u << 20) : left);
        DWORD got = 0;
        if (!ReadFile(h, p + done, want, &got, nullptr) || got == 0) return false;
        done += got;
    }
    return true;
}

bool ReadWholeFile(const std::wstring& path, std::vector<uint8_t>& out)
{
    HANDLE h = OpenRead(path);
    if (h == INVALID_HANDLE_VALUE) return false;
    LARGE_INTEGER sz{};
    bool ok = GetFileSizeEx(h, &sz) && sz.QuadPart > 0 && sz.QuadPart < (1LL << 30);
    if (ok)
    {
        out.resize(static_cast<size_t>(sz.QuadPart));
        ok = ReadAt(h, 0, out.data(), out.size());
    }
    CloseHandle(h);
    return ok;
}

bool WriteWholeFile(const std::wstring& path, const std::vector<uint8_t>& data)
{
    const std::wstring tmp = path + L".tmp";
    HANDLE h = g_origCreateFileW(tmp.c_str(), GENERIC_WRITE, 0, nullptr,
                                 CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) return false;
    size_t done = 0;
    while (done < data.size())
    {
        const size_t left = data.size() - done;
        const DWORD want = static_cast<DWORD>(left > (16u << 20) ? (16u << 20) : left);
        DWORD put = 0;
        if (!WriteFile(h, data.data() + done, want, &put, nullptr) || put == 0) break;
        done += put;
    }
    CloseHandle(h);
    if (done != data.size() ||
        !MoveFileExW(tmp.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING))
    {
        DeleteFileW(tmp.c_str());
        return false;
    }
    return true;
}

// 目印用。サイズと更新日時が変われば、Yumia ツールで Mod が入れ替わったとみなす
std::string Stamp(const std::wstring& path)
{
    WIN32_FILE_ATTRIBUTE_DATA fa{};
    if (!GetFileAttributesExW(path.c_str(), GetFileExInfoStandard, &fa)) return "missing";
    char buf[64];
    sprintf_s(buf, "%lu:%lu:%lu:%lu", fa.nFileSizeHigh, fa.nFileSizeLow,
              fa.ftLastWriteTime.dwHighDateTime, fa.ftLastWriteTime.dwLowDateTime);
    return buf;
}

template <class T> T Rd(const uint8_t* p) { T v; memcpy(&v, p, sizeof(v)); return v; }
template <class T> void Wr(uint8_t* p, T v) { memcpy(p, &v, sizeof(v)); }

// ---- 対応表 -------------------------------------------------------------

const uint32_t kDbs[] = {0x2082AD97, 0x97485E9B};   // キャラの枠を定義する DB

// 衣装メニューの順に 1 対 1。{高精細, 軽量} のモデル定義。
// 澪の 8 着目（SILENT HILL f のネイビーセーラー）は相手がいないので入れ替えない
const uint32_t kMioDefs[7][2] = {
    {0x511668A8, 0x36546A72}, {0xD946A887, 0xBE84AA51}, {0x6176E866, 0x46B4EA30},
    {0xE9A72845, 0xCEE52A0F}, {0x71D76824, 0x571569EE}, {0xFA07A803, 0xDF45A9CD},
    {0x8237E7E2, 0x6775E9AC}};
const uint32_t kMayuDefs[7][2] = {
    {0x49118300, 0x0B93BA76}, {0xC6E93F01, 0x896B7677}, {0x44C0FB02, 0x07433278},
    {0xC298B703, 0x851AEE79}, {0x40707304, 0x02F2AA7A}, {0xBE482F05, 0x80CA667B},
    {0x3C1FEB06, 0xFEA2227C}};

// 上の澪のモデル定義が使う {g1m, grp}
const uint32_t kMioModels[14][2] = {
    {0xCADE596E, 0xD1CDEAEC}, {0xB50F91E4, 0xBBFF2362}, {0xD7774EEF, 0xDE66E06D},
    {0xC1A88765, 0xC89818E3}, {0xE4104470, 0xEAFFD5EE}, {0xCE417CE6, 0xD5310E64},
    {0xF0A939F1, 0xF798CB6F}, {0xDADA7267, 0xE1CA03E5}, {0xFD422F72, 0x0431C0F0},
    {0xE77367E8, 0xEE62F966}, {0x09DB24F3, 0x10CAB671}, {0xF40C5D69, 0xFAFBEEE7},
    {0x16741A74, 0x1D63ABF2}, {0x00A552EA, 0x0794E468}};

constexpr uint32_t kFaceGroup = 0x5526A88F;

// ---- rdb / rdx / fdata --------------------------------------------------

struct RdbEntry { size_t off, size; };

// rdb のエントリ: 0x30 バイトの見出し（+0x08 長さ, +0x10 末尾の長さ, +0x18 ファイルサイズ,
// +0x24 ハッシュ, +0x2C フラグ）、付属データ、末尾（fdata の番号と位置）
bool IndexRdb(const std::vector<uint8_t>& rdb, std::unordered_map<uint32_t, RdbEntry>& out)
{
    size_t o = 0x20;
    while (o + 0x30 <= rdb.size())
    {
        if (memcmp(&rdb[o], "IDRK", 4) != 0) return false;
        const uint64_t size = Rd<uint64_t>(&rdb[o + 8]);
        if (size < 0x30 || o + size > rdb.size()) return false;
        out[Rd<uint32_t>(&rdb[o + 0x24])] = {o, static_cast<size_t>(size)};
        o += static_cast<size_t>((size + 3) & ~3ull);
    }
    return o == rdb.size();
}

bool Location(const std::vector<uint8_t>& rdb, const RdbEntry& e, int16_t& idx, uint64_t& off)
{
    const uint64_t ssz = Rd<uint64_t>(&rdb[e.off + 0x10]);
    const uint8_t* f = &rdb[e.off + e.size - ssz];
    if (ssz == 13)
    {
        off = Rd<uint32_t>(f + 2);
        idx = Rd<int16_t>(f + 10);
    }
    else if (ssz == 0x11)
    {
        off = Rd<uint32_t>(f + 6) + (static_cast<uint64_t>(Rd<uint32_t>(f + 2) & 0xFF) << 32);
        idx = Rd<int16_t>(f + 14);
    }
    else return false;
    return true;
}

struct Source {
    std::vector<uint8_t> rdb, rdx;
    std::unordered_map<uint32_t, RdbEntry> ents;
    std::unordered_map<int16_t, uint32_t> fdatas;   // rdx: 番号 → fdata のハッシュ
};

struct File {
    uint32_t hash = 0;
    uint32_t type = 0, tkid = 0;
    std::vector<uint8_t> extra;    // fdata 側のエントリの付属データ（そのまま引き継ぐ）
    std::vector<uint8_t> data;
};

bool ReadEntry(const Source& src, uint32_t hash, File& out)
{
    auto it = src.ents.find(hash);
    if (it == src.ents.end()) { Log("[NG] 0x%08X is not in root.rdb", hash); return false; }
    int16_t idx = 0;
    uint64_t off = 0;
    if (!Location(src.rdb, it->second, idx, off)) { Log("[NG] Unknown rdb entry for 0x%08X", hash); return false; }
    auto fd = src.fdatas.find(idx);
    if (fd == src.fdatas.end()) { Log("[NG] fdata #%d of 0x%08X is not in root.rdx", idx, hash); return false; }

    wchar_t name[32];
    swprintf_s(name, L"0x%08x.fdata", fd->second);
    const std::wstring path = g_gameDir + L"fdata_package\\" + name;
    HANDLE h = OpenRead(path);
    if (h == INVALID_HANDLE_VALUE) { Log("[NG] Cannot open %ls", name); return false; }

    bool ok = false;
    uint8_t head[0x30];
    do
    {
        if (!ReadAt(h, off, head, sizeof(head)) || memcmp(head, "IDRK0000", 8) != 0) break;
        const uint64_t esize = Rd<uint64_t>(head + 8), csize = Rd<uint64_t>(head + 0x10),
                       usize = Rd<uint64_t>(head + 0x18);
        const uint32_t flags = Rd<uint32_t>(head + 0x2C);
        if (Rd<uint32_t>(head + 0x24) != hash || esize < 0x30 + csize || usize > (1u << 30) ||
            csize > (1u << 30))
            break;
        out.hash = hash;
        out.type = Rd<uint32_t>(head + 0x20);
        out.tkid = Rd<uint32_t>(head + 0x28);
        out.extra.resize(static_cast<size_t>(esize - csize - 0x30));
        if (!out.extra.empty() && !ReadAt(h, off + 0x30, out.extra.data(), out.extra.size())) break;
        const uint64_t body = off + 0x30 + out.extra.size();

        out.data.clear();
        if (csize == usize)
        {
            out.data.resize(static_cast<size_t>(usize));
            ok = ReadAt(h, body, out.data.data(), out.data.size());
            break;
        }
        // 圧縮: チャンクごとに {u16 長さ, u64 ?} か {u32 長さ}（flags & 0x100000）＋ zlib
        std::vector<uint8_t> z(static_cast<size_t>(csize));
        if (!ReadAt(h, body, z.data(), z.size())) break;
        out.data.reserve(static_cast<size_t>(usize));
        size_t p = 0;
        bool good = true;
        while (out.data.size() < usize)
        {
            size_t zsize = 0;
            if (flags & 0x100000)
            {
                if (p + 4 > z.size()) { good = false; break; }
                zsize = Rd<uint32_t>(&z[p]);
                p += 4;
            }
            else
            {
                if (p + 10 > z.size()) { good = false; break; }
                zsize = Rd<uint16_t>(&z[p]);
                p += 10;
            }
            if (p + zsize > z.size() ||
                !inflate::Zlib(&z[p], zsize, out.data, static_cast<size_t>(usize)))
            {
                good = false;
                break;
            }
            p += zsize;
        }
        ok = good && out.data.size() == usize;
    } while (false);
    CloseHandle(h);
    if (!ok) Log("[NG] Cannot read 0x%08X from %ls", hash, name);
    return ok;
}

// ---- 改変 ---------------------------------------------------------------

bool FindUnique(const std::vector<uint8_t>& d, uint32_t v, size_t& pos)
{
    int n = 0;
    for (size_t i = 0; i + 4 <= d.size(); ++i)
        if (Rd<uint32_t>(&d[i]) == v) { pos = i; ++n; }
    return n == 1;
}

// 澪の枠は Main の見た目、繭の枠は Sub の見た目のモデル定義を指すようにする
bool AssignRefs(std::vector<uint8_t>& db, uint32_t hash)
{
    size_t mioPos[7][2], mayuPos[7][2];
    for (int i = 0; i < 7; ++i)
        for (int k = 0; k < 2; ++k)
            if (!FindUnique(db, kMioDefs[i][k], mioPos[i][k]) ||
                !FindUnique(db, kMayuDefs[i][k], mayuPos[i][k]))
            {
                Log("[NG] DB 0x%08X: costume %d is not found exactly once", hash, i + 1);
                return false;
            }
    for (int i = 0; i < 7; ++i)
        for (int k = 0; k < 2; ++k)
        {
            Wr<uint32_t>(&db[mioPos[i][k]], g_mainMayu ? kMayuDefs[i][k] : kMioDefs[i][k]);
            Wr<uint32_t>(&db[mayuPos[i][k]], g_subMio ? kMioDefs[i][k] : kMayuDefs[i][k]);
        }
    return true;
}

// g1m の G1MG チャンクの中から、指定の種類のセクションの先頭を探す
bool G1mgSection(const std::vector<uint8_t>& d, uint32_t type, size_t& out)
{
    if (d.size() < 0x10) return false;
    size_t o = Rd<uint32_t>(&d[0xC]);
    for (;;)
    {
        if (o + 0x30 > d.size()) return false;
        if (memcmp(&d[o], "GM1G", 4) == 0) break;   // "G1MG" が逆順で入っている
        const uint32_t len = Rd<uint32_t>(&d[o + 8]);
        if (len == 0) return false;
        o += len;
    }
    size_t p = o + 0x30;
    const uint32_t n = Rd<uint32_t>(&d[o + 0x2C]);
    for (uint32_t i = 0; i < n; ++i)
    {
        if (p + 12 > d.size()) return false;
        const uint32_t t = Rd<uint32_t>(&d[p]), len = Rd<uint32_t>(&d[p + 4]);
        if (t == type) { out = p; return true; }
        if (len == 0) return false;
        p += len;
    }
    return false;
}

// 顔のグループ（grp の最後、5526a88f）の LOD エントリを、グループ 0 の直後へ移す。
// grp はグループ 0 を広げ、顔のグループは名前を残したまま空にする（名前から番号を
// 引く処理が、見つからない名前でどう振る舞うか分からないので、名前は消さない）
bool FaceFix(File& g1m, File& grp)
{
    auto& gd = grp.data;
    if (gd.size() < 64 || gd.size() % 32) return false;
    const size_t groups = gd.size() / 32;
    uint8_t* last = &gd[(groups - 1) * 32];
    if (Rd<uint32_t>(last) != kFaceGroup) { Log("[NG] grp 0x%08X: face group is not last", grp.hash); return false; }

    std::vector<uint32_t> nent(groups);
    size_t total = 0;
    for (size_t i = 0; i < groups; ++i) total += nent[i] = Rd<uint32_t>(&gd[i * 32 + 0x14]);

    auto& d = g1m.data;
    size_t sec = 0;
    if (!G1mgSection(d, 0x10009, sec)) { Log("[NG] g1m 0x%08X: no LOD section", g1m.hash); return false; }
    std::vector<std::pair<size_t, size_t>> ents;   // {位置, 長さ}
    size_t q = sec + 0x30;
    while (q + 28 <= d.size() && d[q] == '@')
    {
        const size_t len = 28 + 4 * static_cast<size_t>(Rd<uint32_t>(&d[q + 24]));
        if (q + len > d.size()) return false;
        ents.push_back({q, len});
        q += len;
    }
    if (ents.size() < total)
    {
        Log("[NG] g1m 0x%08X: %zu LOD entries, grp expects %zu", g1m.hash, ents.size(), total);
        return false;
    }
    const size_t g0 = nent[0], face0 = total - nent[groups - 1];
    std::vector<size_t> order;
    for (size_t i = 0; i < g0; ++i) order.push_back(i);
    for (size_t i = face0; i < total; ++i) order.push_back(i);
    for (size_t i = g0; i < face0; ++i) order.push_back(i);
    for (size_t i = total; i < ents.size(); ++i) order.push_back(i);

    std::vector<uint8_t> blob;
    for (size_t i : order) blob.insert(blob.end(), d.begin() + ents[i].first,
                                       d.begin() + ents[i].first + ents[i].second);
    memcpy(&d[ents[0].first], blob.data(), blob.size());   // エントリは連続しているので長さは同じ

    Wr<uint32_t>(&gd[0x08], Rd<uint32_t>(&gd[0x08]) + Rd<uint32_t>(last + 0x08));
    Wr<uint32_t>(&gd[0x14], Rd<uint32_t>(&gd[0x14]) + Rd<uint32_t>(last + 0x14));
    Wr<uint32_t>(last + 0x08, 0);
    Wr<uint32_t>(last + 0x14, 0);
    return true;
}

// ---- 生成 ---------------------------------------------------------------

bool Generate()
{
    const std::wstring pkg = g_gameDir + L"fdata_package\\";
    Source src;
    if (!ReadWholeFile(pkg + L"root.rdb", src.rdb) || !ReadWholeFile(pkg + L"root.rdx", src.rdx) ||
        src.rdx.size() % 8)
    {
        Log("[NG] Cannot read root.rdb / root.rdx");
        return false;
    }
    if (!IndexRdb(src.rdb, src.ents)) { Log("[NG] root.rdb has an unknown layout"); return false; }
    int16_t marker = 0;
    for (size_t i = 0; i < src.rdx.size(); i += 8)
    {
        const int16_t m = Rd<int16_t>(&src.rdx[i]);
        const uint32_t h = Rd<uint32_t>(&src.rdx[i + 4]);
        if (h == kFdataHash) { Log("[NG] root.rdx already lists this mod's fdata"); return false; }
        src.fdatas[m] = h;
        if (m > marker) marker = m;
    }
    ++marker;

    std::vector<File> files;
    for (uint32_t h : kDbs)
    {
        File f;
        if (!ReadEntry(src, h, f) || !AssignRefs(f.data, h)) return false;
        files.push_back(std::move(f));
    }
    int fixed = 0;
    if (g_subMio)
    {
        for (auto& m : kMioModels)
        {
            File g1m, grp;
            if (!ReadEntry(src, m[0], g1m) || !ReadEntry(src, m[1], grp)) return false;
            if (!FaceFix(g1m, grp))
            {
                Log("[NG] Could not fix the face of model 0x%08X; Mio's face may be missing in"
                    " that costume when she is the companion", m[0]);
                continue;
            }
            files.push_back(std::move(g1m));
            files.push_back(std::move(grp));
            ++fixed;
        }
    }

    // fdata: "PDRK0000", u32 0x10, u32 全体サイズ、その後に 16 バイト境界でエントリ
    std::vector<uint8_t> fdata(0x10);
    memcpy(fdata.data(), "PDRK0000", 8);
    Wr<uint32_t>(&fdata[8], 0x10);
    struct Where { uint32_t off, esize, usize; };
    std::unordered_map<uint32_t, Where> where;
    for (auto& f : files)
    {
        const size_t off = fdata.size();
        const size_t esize = 0x30 + f.extra.size() + f.data.size();
        fdata.resize(off + 0x30);
        uint8_t* e = &fdata[off];
        memcpy(e, "IDRK0000", 8);
        Wr<uint64_t>(e + 0x08, esize);
        Wr<uint64_t>(e + 0x10, f.data.size());
        Wr<uint64_t>(e + 0x18, f.data.size());
        Wr<uint32_t>(e + 0x20, f.type);
        Wr<uint32_t>(e + 0x24, f.hash);
        Wr<uint32_t>(e + 0x28, f.tkid);
        Wr<uint32_t>(e + 0x2C, 0);
        fdata.insert(fdata.end(), f.extra.begin(), f.extra.end());
        fdata.insert(fdata.end(), f.data.begin(), f.data.end());
        fdata.resize((fdata.size() + 15) & ~static_cast<size_t>(15));
        where[f.hash] = {static_cast<uint32_t>(off), static_cast<uint32_t>(esize),
                         static_cast<uint32_t>(f.data.size())};
    }
    Wr<uint32_t>(&fdata[0xC], static_cast<uint32_t>(fdata.size()));

    // rdb: エントリの長さは変えず、サイズ・フラグ・末尾（位置と fdata 番号）だけ書き換える
    std::vector<uint8_t> rdb = src.rdb;
    for (auto& [h, w] : where)
    {
        const RdbEntry& e = src.ents[h];
        const uint64_t ssz = Rd<uint64_t>(&rdb[e.off + 0x10]);
        Wr<uint64_t>(&rdb[e.off + 0x18], w.usize);
        Wr<uint32_t>(&rdb[e.off + 0x2C], 0x20000);   // 非圧縮（Yumia ツールと同じ値）
        uint8_t* f = &rdb[e.off + e.size - ssz];
        if (ssz == 13)
        {
            Wr<uint32_t>(f + 2, w.off);
            Wr<uint32_t>(f + 6, w.esize);
            Wr<int16_t>(f + 10, marker);
        }
        else
        {
            Wr<uint32_t>(f + 2, 0);
            Wr<uint32_t>(f + 6, w.off);
            Wr<uint32_t>(f + 10, w.esize);
            Wr<int16_t>(f + 14, marker);
        }
    }

    // rdx: 末尾に {番号, -1, fdata のハッシュ} を足す
    std::vector<uint8_t> rdx = src.rdx;
    rdx.resize(rdx.size() + 8);
    Wr<int16_t>(&rdx[rdx.size() - 8], marker);
    Wr<int16_t>(&rdx[rdx.size() - 6], -1);
    Wr<uint32_t>(&rdx[rdx.size() - 4], kFdataHash);

    wchar_t name[32];
    swprintf_s(name, L"0x%08x.fdata", kFdataHash);
    CreateDirectoryW(g_cacheDir.c_str(), nullptr);
    if (!WriteWholeFile(g_cacheDir + name, fdata) || !WriteWholeFile(g_cacheDir + L"root.rdx", rdx) ||
        !WriteWholeFile(g_cacheDir + L"root.rdb", rdb))
    {
        Log("[NG] Cannot write to %ls", g_cacheDir.c_str());
        return false;
    }
    Log("[OK] Generated the swap data (%zu files, %d Mio models fixed, %zu bytes)",
        files.size(), fixed, fdata.size());
    return true;
}

std::string TagText()
{
    const std::wstring pkg = g_gameDir + L"fdata_package\\";
    return std::string(kCacheTag) + " main=" + (g_mainMayu ? "mayu" : "mio") +
           " sub=" + (g_subMio ? "mio" : "mayu") + " rdb=" + Stamp(pkg + L"root.rdb") +
           " rdx=" + Stamp(pkg + L"root.rdx");
}

// 生成済みで元のファイルも設定も変わっていなければ、そのまま使う
bool EnsureCache()
{
    const std::wstring tagPath = g_cacheDir + L"cache.tag";
    const std::string want = TagText();
    {
        std::vector<uint8_t> have;
        wchar_t name[32];
        swprintf_s(name, L"0x%08x.fdata", kFdataHash);
        if (ReadWholeFile(tagPath, have) && std::string(have.begin(), have.end()) == want &&
            GetFileAttributesW((g_cacheDir + name).c_str()) != INVALID_FILE_ATTRIBUTES &&
            GetFileAttributesW((g_cacheDir + L"root.rdb").c_str()) != INVALID_FILE_ATTRIBUTES &&
            GetFileAttributesW((g_cacheDir + L"root.rdx").c_str()) != INVALID_FILE_ATTRIBUTES)
        {
            Log("[OK] Using the cached swap data");
            return true;
        }
    }
    DeleteFileW(tagPath.c_str());
    const DWORD t0 = GetTickCount();
    if (!Generate()) return false;
    WriteWholeFile(tagPath, std::vector<uint8_t>(want.begin(), want.end()));
    Log("     (took %lu ms)", GetTickCount() - t0);
    return true;
}

LONG g_state = 0;   // 0=未実行 / 1=成功 / -1=失敗

bool EndsWithNoCase(const wchar_t* s, const wchar_t* suffix)
{
    const size_t ls = wcslen(s), lt = wcslen(suffix);
    if (ls < lt) return false;
    for (size_t i = 0; i < lt; ++i)
    {
        wchar_t a = s[ls - lt + i], b = suffix[i];
        if (a == L'/') a = L'\\';
        if (towlower(a) != towlower(b)) return false;
    }
    return true;
}

HANDLE WINAPI MyCreateFileW(LPCWSTR name, DWORD access, DWORD share, LPSECURITY_ATTRIBUTES sa,
                            DWORD disp, DWORD flags, HANDLE tmpl)
{
    if (name && (access & GENERIC_READ) && !(access & GENERIC_WRITE))
    {
        const wchar_t* target = nullptr;
        wchar_t fdataName[48];
        swprintf_s(fdataName, L"fdata_package\\0x%08x.fdata", kFdataHash);
        if (EndsWithNoCase(name, L"fdata_package\\root.rdb")) target = L"root.rdb";
        else if (EndsWithNoCase(name, L"fdata_package\\root.rdx")) target = L"root.rdx";
        else if (EndsWithNoCase(name, fdataName)) target = fdataName + 14;

        if (target)
        {
            EnterCriticalSection(&g_lock);
            if (g_state == 0) g_state = EnsureCache() ? 1 : -1;
            const bool ok = (g_state == 1);
            LeaveCriticalSection(&g_lock);
            if (ok)
            {
                const std::wstring dst = g_cacheDir + target;
                HANDLE h = g_origCreateFileW(dst.c_str(), access, share, sa, disp, flags, tmpl);
                if (h != INVALID_HANDLE_VALUE) return h;
                Log("[NG] Cannot open the cached %ls; the game uses its own file", target);
            }
        }
    }
    return g_origCreateFileW(name, access, share, sa, disp, flags, tmpl);
}

// kernel32!CreateFileW は「jmp [kernel32 の輸入テーブル]」だけの中継なので、
// その輸入テーブルの枠を差し替える
bool InstallHook()
{
    HMODULE k32 = GetModuleHandleW(L"kernel32.dll");
    auto stub = reinterpret_cast<const uint8_t*>(GetProcAddress(k32, "CreateFileW"));
    if (!stub) return false;
    PVOID* slot = nullptr;
    if (stub[0] == 0xFF && stub[1] == 0x25)
        slot = reinterpret_cast<PVOID*>(const_cast<uint8_t*>(stub) + 6 + Rd<int32_t>(stub + 2));
    else if (stub[0] == 0x48 && stub[1] == 0xFF && stub[2] == 0x25)   // rex.w jmp
        slot = reinterpret_cast<PVOID*>(const_cast<uint8_t*>(stub) + 7 + Rd<int32_t>(stub + 3));
    if (!slot)
    {
        Log("[NG] kernel32!CreateFileW has an unexpected form (%02X %02X %02X); swap disabled",
            stub[0], stub[1], stub[2]);
        return false;
    }
    DWORD old = 0;
    if (!VirtualProtect(slot, sizeof(PVOID), PAGE_READWRITE, &old)) return false;
    g_origCreateFileW = reinterpret_cast<PFN_CreateFileW>(*slot);
    *slot = reinterpret_cast<PVOID>(&MyCreateFileW);
    DWORD tmp = 0;
    VirtualProtect(slot, sizeof(PVOID), old, &tmp);
    return true;
}

// ---- 設定 ---------------------------------------------------------------

// 項目が無ければ既定値（入れ替え）を使う。値が mio / mayu のどちらでもなければ、
// 打ち間違いで意図しない入れ替えが起きないよう、そのキャラ本来の見た目のままにする
bool ReadLook(const std::wstring& ini, const wchar_t* key, const wchar_t* def, bool& isOther,
              const wchar_t* other, const wchar_t* own)
{
    wchar_t buf[32]{};
    GetPrivateProfileStringW(L"Swap", key, def, buf, 32, ini.c_str());
    // 前後の空白とコメントを落とす
    wchar_t* s = buf;
    while (*s == L' ' || *s == L'\t') ++s;
    wchar_t* e = s;
    while (*e && *e != L' ' && *e != L'\t' && *e != L';' && *e != L'#') ++e;
    *e = 0;
    if (_wcsicmp(s, L"mio") == 0 || _wcsicmp(s, L"mayu") == 0)
    {
        isOther = _wcsicmp(s, other) == 0;
        return true;
    }
    Log("[NG] [Swap] %ls=%ls is not mio or mayu; keeping the original look (%ls)", key, s, own);
    isOther = false;
    return false;
}

void LoadConfig()
{
    const std::wstring ini = g_modDir + L"twinswap.ini";
    g_enabled = GetPrivateProfileIntW(L"General", L"Enabled", 1, ini.c_str()) != 0;
    g_log     = GetPrivateProfileIntW(L"General", L"Log", 1, ini.c_str()) != 0;
    ReadLook(ini, L"Main", L"mayu", g_mainMayu, L"mayu", L"mio");
    ReadLook(ini, L"Sub", L"mio", g_subMio, L"mio", L"mayu");
}

// ---- XInput の転送 ------------------------------------------------------

void LoadRealXInput()
{
    if (g_real) return;
    wchar_t path[MAX_PATH]{};
    GetSystemDirectoryW(path, MAX_PATH);
    wcscat_s(path, L"\\xinput1_4.dll");
    g_real = LoadLibraryW(path);
}

FARPROC RealProc(const char* name)
{
    LoadRealXInput();
    return g_real ? GetProcAddress(g_real, name) : nullptr;
}

} // namespace

// XInput の関数はどれも整数・ポインタの引数を 8 個以下しか取らず、浮動小数点の
// 引数も無い。x64 の呼び出し規約では 8 個をそのまま受け渡せば元の関数と同じに
// 振る舞う。番号だけの関数（100〜）も同じ方法で番号から引いて転送する。
using Fwd8 = uintptr_t(WINAPI*)(uintptr_t, uintptr_t, uintptr_t, uintptr_t,
                                uintptr_t, uintptr_t, uintptr_t, uintptr_t);

#define FORWARD(export_name, lookup)                                                       \
    extern "C" uintptr_t WINAPI export_name(uintptr_t a, uintptr_t b, uintptr_t c,         \
                                            uintptr_t d, uintptr_t e, uintptr_t f,         \
                                            uintptr_t g, uintptr_t h)                      \
    {                                                                                      \
        static Fwd8 fn = reinterpret_cast<Fwd8>(RealProc(lookup));                         \
        return fn ? fn(a, b, c, d, e, f, g, h) : ERROR_DEVICE_NOT_CONNECTED;               \
    }

FORWARD(Proxy_XInputGetState, "XInputGetState")
FORWARD(Proxy_XInputSetState, "XInputSetState")
FORWARD(Proxy_XInputGetCapabilities, "XInputGetCapabilities")
FORWARD(Proxy_XInputEnable, "XInputEnable")
FORWARD(Proxy_XInputGetBatteryInformation, "XInputGetBatteryInformation")
FORWARD(Proxy_XInputGetKeystroke, "XInputGetKeystroke")
FORWARD(Proxy_XInputGetAudioDeviceIds, "XInputGetAudioDeviceIds")
FORWARD(Proxy_Ordinal100, MAKEINTRESOURCEA(100))
FORWARD(Proxy_Ordinal101, MAKEINTRESOURCEA(101))
FORWARD(Proxy_Ordinal102, MAKEINTRESOURCEA(102))
FORWARD(Proxy_Ordinal103, MAKEINTRESOURCEA(103))
FORWARD(Proxy_Ordinal104, MAKEINTRESOURCEA(104))
FORWARD(Proxy_Ordinal108, MAKEINTRESOURCEA(108))
FORWARD(Proxy_Ordinal109, MAKEINTRESOURCEA(109))

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hModule);
        InitializeCriticalSection(&g_lock);

        wchar_t exe[MAX_PATH]{};
        GetModuleFileNameW(nullptr, exe, MAX_PATH);
        g_gameDir.assign(exe);
        g_gameDir.resize(g_gameDir.find_last_of(L'\\') + 1);
        g_modDir   = g_gameDir + L"Mods\\twinswap\\";
        g_cacheDir = g_modDir + L"cache\\";

        LoadConfig();
        Log("Twin Swap %s  Main=%s Sub=%s", kVersion, g_mainMayu ? "mayu" : "mio",
            g_subMio ? "mio" : "mayu");

        // 見た目が元のままなら何もしない
        if (!g_enabled || (!g_mainMayu && !g_subMio))
            Log("[OK] Nothing to swap (disabled or Main=mio / Sub=mayu)");
        else if (GetFileAttributesW(g_modDir.c_str()) == INVALID_FILE_ATTRIBUTES)
            ;   // Mods\twinswap が無い（DLL だけ残っている）
        else if (InstallHook())
            Log("[OK] File hook installed");
        else
            Log("[NG] Could not install the file hook; the game runs unmodified");
    }
    return TRUE;
}
