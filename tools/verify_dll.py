# DLL の生成物を、参照実装（twinswap_ref.py）の出力とバイト単位で比べる。
#
#   python verify_dll.py <テスト用のゲームフォルダ>
#
# テスト用のゲームフォルダには fdata_package（root.rdb / root.rdx と fdata）、harness.exe、
# Mods\twinswap を置いておく。dist\xinput1_4.dll をコピーし、ini の 4 通りの組み合わせで
# harness.exe に root.rdb / root.rdx / Mod の fdata を開かせて、キャッシュを比べる。
import sys, os, shutil, subprocess, hashlib
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import twinswap_ref as R

DIST = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'dist', 'xinput1_4.dll')
INI = """[Swap]
Main=%s
Sub=%s
[General]
Enabled=1
Log=1
"""


def sha(b):
    return hashlib.sha256(b).hexdigest()[:16]


def main():
    game = sys.argv[1]
    pkg = os.path.join(game, 'fdata_package')
    mod = os.path.join(game, 'Mods', 'twinswap')
    shutil.copy2(DIST, os.path.join(game, 'xinput1_4.dll'))
    rdb = open(os.path.join(pkg, 'root.rdb'), 'rb').read()
    rdx = open(os.path.join(pkg, 'root.rdx'), 'rb').read()
    ok_all = True
    for main_, sub in (('mayu', 'mio'), ('mio', 'mio'), ('mayu', 'mayu')):
        open(os.path.join(mod, 'twinswap.ini'), 'w').write(INI % (main_, sub))
        shutil.rmtree(os.path.join(mod, 'cache'), ignore_errors=True)
        if os.path.exists(os.path.join(mod, 'twinswap.log')):
            os.remove(os.path.join(mod, 'twinswap.log'))
        subprocess.run([os.path.join(game, 'harness.exe')], cwd=game, check=True, capture_output=True)
        want = dict(zip(('root.rdb', 'root.rdx', '0x%08x.fdata' % R.FDATA_HASH), R.build(pkg, rdb, rdx, main_, sub)))
        for name, b in want.items():
            p = os.path.join(mod, 'cache', name)
            got = open(p, 'rb').read() if os.path.exists(p) else b''
            same = got == b
            ok_all &= same
            print('%-5s %-5s %-18s %s  dll=%s ref=%s' % (main_, sub, name, 'OK ' if same else 'NG ', sha(got), sha(b)))
        log = open(os.path.join(mod, 'twinswap.log'), encoding='utf-8-sig').read()
        if '[NG]' in log:
            ok_all = False
            print(log)
    print('ALL OK' if ok_all else 'MISMATCH')


if __name__ == '__main__':
    main()
