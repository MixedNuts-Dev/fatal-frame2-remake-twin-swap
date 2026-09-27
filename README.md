# TwinSwap

**FATAL FRAME II: Crimson Butterfly REMAKE**（零 〜紅い蝶〜 REMAKE / Steam AppID 3920610）用の Mod です。
A mod for FATAL FRAME / PROJECT ZERO II: Crimson Butterfly REMAKE (Steam, AppID 3920610).

操作キャラ（澪）と同行キャラ（繭）の**見た目を入れ替えます**。繭を操作して澪を連れて歩く、
二人とも繭、二人とも澪、の 3 通りを設定ファイルで選べます。
入れ替わるのはモデル（顔・髪・体・衣装）だけで、動き・声・字幕・ストーリーは元のままです。

**Swaps the looks of the player character (Mio) and the companion (Mayu).** Play as Mayu
with Mio at your side, or make both twins Mayu or both Mio, chosen in a config file.
Only the models (face, hair, body and costume) change; animations, voices, subtitles
and the story stay as they are.

## 導入 / Installation

**ビルドは不要です。** [Releases](../../releases) または
[Nexus Mods](https://www.nexusmods.com/fatalframe2crimsonbutterflyremake/mods/25)
から配布物をダウンロードし、中身の `xinput1_4.dll` と `Mods` フォルダを、ゲームの
ルート（`FatalFrameII.exe` と同じ場所）にそのままコピーするだけです。

**No build required.** Download the archive from [Releases](../../releases) or
[Nexus Mods](https://www.nexusmods.com/fatalframe2crimsonbutterflyremake/mods/25) and
copy `xinput1_4.dll` and the `Mods` folder into the game's root directory (the folder
containing `FatalFrameII.exe`).

```
FatalFrameII/
  FatalFrameII.exe
  xinput1_4.dll                      <- added
  Mods/twinswap/twinswap.ini         <- added
  Mods/twinswap/README.md
  Mods/twinswap/twinswap.log         <- 起動時に生成 / generated at launch
  Mods/twinswap/cache/               <- 起動時に生成 / generated at launch
```

削除は 2 つを消すだけです。ゲームのファイルは一切変更しません。
To uninstall, just delete them. No game files are modified.

## 設定 / Configuration

`Mods\twinswap\twinswap.ini`

| 項目 / Key | 意味 / Meaning |
|---|---|
| `Main` | 操作キャラ（本編の澪）の見た目。`mio` / `mayu`、既定 `mayu` / Look of the player character (Mio in the story). Default `mayu` |
| `Sub` | 同行キャラ（本編の繭）の見た目。`mio` / `mayu`、既定 `mio` / Look of the companion (Mayu in the story). Default `mio` |
| `Enabled` | `1` = 有効 on / `0` = 無効 off |
| `Log` | `1` = ログを出力 write a log / `0` = 出力しない no log |

| `Main` | `Sub` | 結果 / Result |
|---|---|---|
| `mayu` | `mio` | 姉妹を入れ替える（既定） / the twins swapped (default) |
| `mayu` | `mayu` | 二人とも繭 / two Mayus |
| `mio` | `mio` | 二人とも澪 / two Mios |
| `mio` | `mayu` | 元のまま / vanilla |

## 衣装と動作確認の範囲 / Costumes and what has been checked

衣装は衣装メニューの並び順で 1 対 1 に対応させています（澪の n 着目 ↔ 繭の n 着目）。
Costumes are paired one-to-one in costume menu order (Mio's nth ↔ Mayu's nth).

- **SILENT HILL f とのコラボ衣装・アイテムは対象外です。** 澪の 8 着目は繭側に対が無いため
  入れ替えません。
  **The SILENT HILL f collaboration costume and items are out of scope.** Mio's 8th
  costume has no counterpart on Mayu's side and is not swapped.
- ゲーム内で確認できているのは、**初期衣装**と**和風ゴシックドレス（7 着目）**の 2 組だけです。
  2〜6 着目は同じ手順で入れ替えデータを作っていますが、ゲーム内では未確認です。
  Only **the default costumes** and **the 7th costumes (gothic dresses)** have been
  checked in game. The 2nd to 6th go through the same process but are unchecked.
- アクセサリーは対象外で、入れ替えた姿にもそのまま反映されます。
  Accessories are not swapped; they show up on the swapped look as usual.
- 衣装画面のプレビューも入れ替えた後の姿になります。
  The costume menu preview also shows the swapped look.

## 他の Mod との併用 / Using it with other mods

- **Native 120FPS Option（`dinput8.dll`）/ Mouse Wheel Camera Speed（`version.dll`）とは
  干渉しません。** どの組み合わせでも、どの順番で入れても動作します。
  **No interference with Native 120FPS Option (`dinput8.dll`) or Mouse Wheel Camera
  Speed (`version.dll`)**, in any combination or order.
- **Yumia fdata tools で入れる Mod と併用できます。** その時点の Mod 込みのデータを元に
  入れ替え用のデータを作り、Mod が変わると次の起動で作り直します。
  **Works with mods installed with Yumia fdata tools.** The swap data is built from the
  current game data including those mods, and rebuilt on the next launch when they change.

## ビルド / Build

Visual Studio 2022 の C++ ツールセットが必要です。外部ライブラリは使っていません
（zlib の展開も自前です）。
Requires the Visual Studio 2022 C++ toolset. No external libraries are used (the zlib
decoder is written from scratch).

```
build.bat
```

`dist\` に配布用の一式が出力されます。 / The distributable set is written to `dist\`.

`tools\` は開発用です。 / `tools\` is for development:

| ファイル / File | 用途 / Purpose |
|---|---|
| `twinswap_ref.py` | DLL と同じ処理の Python 版（参照実装） / A Python version of what the DLL does (reference implementation) |
| `verify_dll.py` | DLL の生成物を参照実装とバイト単位で比べる / Compares the DLL's output with the reference byte for byte |

どちらも開発者の環境のパスを直書きしています。 / Both contain paths from the author's machine.

## 仕組み / How it works

### 見た目の入れ替え / Swapping the looks

衣装表（`COSTUME_DATA_ECB`）の各行は、高精細・軽量の 2 つの「枠」を持ちます。枠は 2 つの
kidsobjdb（`0x2082AD97` / `0x97485E9B`）の中で、CharacterEditor DB のモデル定義を参照しています。
Mod はこの参照（4 バイト）を書き換えます。澪の枠は `Main`、繭の枠は `Sub` の見た目の
モデル定義を指すようにします。モデル定義には g1m・材質・骨・揺れもの・表示グループの
プリセットがぶら下がっているので、全部が一緒に移ります。

Each row of the costume table (`COSTUME_DATA_ECB`) has two slots, high detail and low
detail. The slots live in two kidsobjdb files (`0x2082AD97` / `0x97485E9B`) and refer to
model definitions in the CharacterEditor DB. The mod rewrites these 4-byte references:
Mio's slots point at the model definitions of the `Main` look, Mayu's at the `Sub` look.
Everything hanging off a model definition (g1m, materials, skeleton, cloth, display
group presets) moves with it.

索引ファイル（root.rdb）でファイルの行き先を付け替える方法は、骨や揺れものの設定が
キャラ側と食い違い、フォトモードでフリーズしたので使っていません。

Redirecting files in the index (root.rdb) was tried first, but it leaves the skeleton and
cloth settings out of step with the character and freezes photo mode, so it is not used.

### 顔の修正 / The face fix

描画は grp の「グループ」単位で表示・非表示が決まり、キャラはどのグループを表示するかを
ビットで渡します。澪のモデルは顔・歯・目まわりを専用のグループ `5526A88F`（grp の最後）に
置いていますが、同行キャラ（繭）はこのグループを表示しません。そのまま付けると、
目玉以外の顔が消えます。繭のモデルは顔が常時表示のグループ 0 に入っているので、逆方向は
問題ありません。

Visibility is decided per grp "group", and each character passes a bit mask of the groups
to show. Mio's models keep the face, teeth and eye area in a group of their own
(`5526A88F`, the last one in the grp), which the companion (Mayu) never shows, so put on
the companion as-is everything of the face but the eyeballs disappears. Mayu's models
keep the face in group 0, which is always shown, so the other direction is fine.

描画アイテムは g1m の LOD エントリの順に作られ、grp の各グループは「LOD エントリ何個・
アイテム何個」の連続した範囲です。Mod は澪のモデルを繭と同じ構造にします。

Draw items are built in the order of the g1m's LOD entries, and each grp group is a
consecutive run of "so many LOD entries, so many items". The mod gives Mio's models the
same layout as Mayu's:

- g1m: `5526A88F` の LOD エントリをグループ 0 の範囲の直後へ移す
  g1m: move the `5526A88F` LOD entries right after group 0's range
- grp: グループ 0 をその分広げ、`5526A88F` は名前を残したまま空（0 エントリ・0 アイテム）にする。
  名前から番号を引く処理があるため、名前は消さない
  grp: widen group 0 by the same amount and leave `5526A88F` empty (0 entries, 0 items)
  under its name, since the game looks groups up by name

どちらもファイルのサイズは変わりません。`Sub=mio` のとき、澪の 7 着 × 2 モデルに適用します。
Neither changes the file size. It is applied to Mio's 7 costumes × 2 models when `Sub=mio`.

### ファイルの差し替え / Serving the data

改変したファイルは Mod 専用の fdata（`0xFFFE7510`）にまとめ、それを指すように書き換えた
root.rdb / root.rdx と一緒に `Mods\twinswap\cache` に作ります。ゲームが
`fdata_package\root.rdb` / `root.rdx` / `0xfffe7510.fdata` を開くときだけ、`CreateFileW` の
フックでキャッシュの方を開かせます。元にするのは `fdata_package` の現在の root.rdb / rdx
なので、Yumia fdata tools で入れた Mod はそのまま残ります。rdb のエントリは長さを変えず、
サイズ・フラグ・fdata の位置だけを書き換えます。root.rdb / rdx のサイズと更新日時、
設定が前回と同じなら、キャッシュをそのまま使います。

The modified files go into a mod-only fdata (`0xFFFE7510`), created in
`Mods\twinswap\cache` together with a root.rdb / root.rdx rewritten to point at it. Only
when the game opens `fdata_package\root.rdb` / `root.rdx` / `0xfffe7510.fdata` does a
`CreateFileW` hook open the cached file instead. They are built from the current
root.rdb / rdx in `fdata_package`, so mods installed with Yumia fdata tools stay in
place. rdb entries keep their length; only the size, flags and fdata location change.
If the size and timestamp of root.rdb / rdx and the settings are unchanged, the cache
is reused.

フックは exe ではなく **kernel32 の輸入テーブル**（`kernel32!CreateFileW` の中継先）に
掛けます。Native 120FPS Option のローダーは、exe の輸入テーブルの `CreateFileW` を起動直後に
繰り返し掛け直すので、同じ場所に掛けると互いを「元の関数」として保存し合い、
無限再帰になります。kernel32 側なら、あちらのフックの先で呼ばれるので干渉しません。

The hook goes into **kernel32's import table** (where `kernel32!CreateFileW` jumps), not
the exe's. Native 120FPS Option's loader keeps re-hooking the exe's `CreateFileW` import
right after launch; hooking the same slot would make each save the other as "the
original" and recurse forever. On the kernel32 side it is simply called from behind that
hook.

ローダーの名前が `xinput1_4.dll` なのは、`dinput8.dll` と `version.dll` が既に使われている
ためです。XInput の関数（番号だけのものも含む）は System32 の実体へ転送します。

The DLL is named `xinput1_4.dll` because `dinput8.dll` and `version.dll` are already taken.
All XInput exports (including the ordinal-only ones) are forwarded to the System32 DLL.

## 免責事項 / Disclaimer

**この Mod は無保証で提供されます。使用によって生じたいかなる損害についても、
作者は一切の責任を負いません。** セーブデータの破損・消失、ゲームの動作不良、
その他の不具合を含みます。自己責任でご使用ください。

**This mod is provided as-is, without any warranty. The author accepts no liability
for any damage arising from its use,** including but not limited to corruption or loss
of save data, game malfunction, or any other problem. Use it at your own risk.

**導入前に、必ずセーブデータのバックアップを取ってください。**
**Always back up your save data before installing.**

```
%LOCALAPPDATA%\KoeiTecmo\FatalFrameII\Savedata\
```

## 不具合の報告 / Reporting issues

不具合を見つけた場合は、GitHub の Issue でご報告ください。その際、**必ず
`Mods\twinswap\twinswap.log` を添付してください。**

If you run into a problem, please open a GitHub Issue. **Be sure to attach
`Mods\twinswap\twinswap.log`.**

---

## クレジット / Credits

**Created by MixedNuts**

アーカイブ形式（rdb / rdx / fdata）は、eArmada8 氏の
[Yumia fdata tools](https://github.com/eArmada8/yumia_fdata_tools) を読んで理解しました。
The archive format (rdb / rdx / fdata) was understood from eArmada8's
[Yumia fdata tools](https://github.com/eArmada8/yumia_fdata_tools).

## ライセンス / License

MIT License — 詳細は [LICENSE](LICENSE) を参照してください。
See [LICENSE](LICENSE) for details.

再配布・改変は自由ですが、**著作権表示とライセンス文を必ず残してください。**
MIT ライセンスの条件です。

You are free to redistribute and modify this, but **the copyright notice and the
license text must be retained** — that is a condition of the MIT License.
