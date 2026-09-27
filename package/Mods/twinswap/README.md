# TwinSwap

**FATAL FRAME II: Crimson Butterfly REMAKE** 用の Mod です。
A mod for FATAL FRAME / PROJECT ZERO II: Crimson Butterfly REMAKE.

Created by MixedNuts

---

# 日本語

## これは何か

操作キャラ（澪）と同行キャラ（繭）の**見た目を入れ替えます**。
繭を操作して澪を連れて歩く、二人とも繭、二人とも澪、の 3 通りを設定ファイルで選べます。

入れ替わるのはモデル（顔・髪・体・衣装）だけです。動き・声・字幕・ストーリーは
元のままです（例えば、繭の足を引きずる歩き方は、澪の姿になっても同行キャラに残ります）。

## 動作環境

- FATAL FRAME II: Crimson Butterfly REMAKE（Steam 版）

ゲームのファイルは一切変更しないため、Steam のファイル整合性チェックに
引っかかることはありません。

## 同梱ファイル

| ファイル | 役割 |
|---|---|
| `xinput1_4.dll` | 本体 |
| `Mods\twinswap\twinswap.ini` | 設定ファイル |
| `Mods\twinswap\README.md` | このファイル |

このうち **`xinput1_4.dll` と `Mods` フォルダの 2 つ**をコピーします。
起動すると `Mods\twinswap\` の中に、ログ `twinswap.log` と、入れ替え用のデータを
置く `cache` フォルダが作られます。

### 他の Mod との併用

- **Native 120FPS Option / Mouse Wheel Camera Speed とは干渉しません。**
  あちらは `dinput8.dll` / `version.dll`、こちらは `xinput1_4.dll` を使うので、
  どの組み合わせでも、どの順番で入れても動作します。`Mods` フォルダは中身が合流するだけです
- **Yumia fdata tools で入れる Mod（衣装の改変など）と併用できます。** その時点の
  Mod 込みのデータを元に入れ替え用のデータを作るので、後から Mod を入れ直しても、
  次の起動で自動的に作り直されます
- ゲームのルートに **別の `xinput1_4.dll` が既にある場合は、上書きしないでください。**
  他のツールが同じ名前を使っている可能性があります

## 導入方法

1. ゲームを終了します

2. 同梱の `xinput1_4.dll` と `Mods` フォルダを、ゲームのルートディレクトリ
   （`FatalFrameII.exe` と同じ場所）にそのままコピーします

   ```
   ...\steamapps\common\FatalFrameII\FatalFrameII.exe
   ...\steamapps\common\FatalFrameII\xinput1_4.dll     ← 追加
   ...\steamapps\common\FatalFrameII\Mods\twinswap\    ← 追加
   ```

   ゲームフォルダの開き方：Steam ライブラリでタイトルを右クリック →
   **管理** → **ローカルファイルを閲覧**

3. ゲームを起動し、セーブデータをロードしてください

## 削除方法

`xinput1_4.dll` と `Mods\twinswap` フォルダを削除するだけです。
ゲームのファイルは一切変更していないため、完全に元に戻ります。

一時的に無効化したい場合は、`twinswap.ini` の `Enabled` を `0` にしてください。

## 設定ファイル

`twinswap.ini` で次の項目を変更できます。変更はゲームの再起動後に反映されます。

| 項目 | 意味 |
|---|---|
| `Main` | 操作キャラ（本編の澪）の見た目。`mio` か `mayu`。既定値 `mayu` |
| `Sub` | 同行キャラ（本編の繭）の見た目。`mio` か `mayu`。既定値 `mio` |
| `Enabled` | `1` = 有効 / `0` = 無効 |
| `Log` | `1` = ログを出力 / `0` = 出力しない |

| `Main` | `Sub` | 結果 |
|---|---|---|
| `mayu` | `mio` | 姉妹を入れ替える（既定） |
| `mayu` | `mayu` | 二人とも繭 |
| `mio` | `mio` | 二人とも澪 |
| `mio` | `mayu` | 元のまま |

`mio` / `mayu` 以外の値を書いた場合は、そのキャラ本来の見た目のままになります。

## 衣装の対応

衣装は、衣装メニューの並び順で 1 対 1 に対応させています。例えば、澪に 2 着目の
衣装を着せると、操作キャラは繭の 2 着目の衣装の姿になります。

- **SILENT HILL f とのコラボ衣装・アイテムは対象外です。** 澪の 8 着目
  （ネイビーセーラー）は繭側に対になる衣装が無いため入れ替えず、選ぶと澪の姿のままになります
- アクセサリーは入れ替えの対象外です。入れ替えた姿にも、そのまま反映されます

### ゲーム内での動作確認の範囲

ゲーム内で実際に確認できているのは、**初期衣装**と**和風ゴシックドレス**
（澪の左翅・繭の右翅）の 2 組だけです。`Main` / `Sub` の 3 通りの組み合わせ、
TAB メニュー、衣装画面、フォトモードを確認しています。

それ以外の衣装（2〜6 着目）は、同じ手順で入れ替えデータを作っていますが、
ゲーム内ではまだ確認していません。表示がおかしい衣装があれば、
GitHub の Issue で教えてください。

## 注意事項

- **衣装画面のプレビューも、入れ替えた後の姿で表示されます。** 衣装の名前と
  プレビューの姿が一致しないのは、この Mod の仕様です
- ゲーム中のムービーは動画ファイルなので、入れ替わりません
- 入れ替え用のデータ（`Mods\twinswap\cache`、最大で約 90 MB）は、初回の起動時と、
  設定や Yumia fdata tools の Mod を変えた後の起動時に作り直されます。
  それ以外の起動では、作ったものをそのまま使います
- ゲームのアップデート後に動かなくなることがあります。その場合、Mod は何もせず
  ゲームは素の状態で動きます。ログに理由が記録されます
- セーブデータには何も書き込みません。Mod を外すと、元の姿に戻ります
- ウイルス対策ソフトが誤検知することがあります。ゲームがファイルを開く処理に
  割り込む仕組みのためです。この Mod はネットワーク通信を一切行わず、
  ファイルを書き込むのも自分のフォルダの中だけです

## 免責事項

**この Mod は無保証で提供されます。使用によって生じたいかなる損害についても、
作者は一切の責任を負いません。** セーブデータの破損・消失、ゲームの動作不良、
その他の不具合を含みます。自己責任でご使用ください。

**導入前に、必ずセーブデータのバックアップを取ってください。**

```
%LOCALAPPDATA%\KoeiTecmo\FatalFrameII\Savedata\
```

## うまく動かないとき

1. `xinput1_4.dll` がゲームのルート（`FatalFrameII.exe` と同じ場所）にあるか。
   **`Mods` フォルダの中ではありません**
2. `Mods\twinswap\` の中に `twinswap.ini` があるか。
   フォルダ名を変更していないか
3. `Mods\twinswap\` に `twinswap.log` が生成されているか。
   生成されていなければ `xinput1_4.dll` が読み込まれていません

ログに次の行が出ていれば正常に適用されています（ログは英語で出力されます）。

```
[OK] File hook installed
[OK] Generated the swap data (...)
```

2 回目以降の起動では、2 行目が `[OK] Using the cached swap data` になります。
不具合を報告するときは、GitHub の Issue で `twinswap.log` を添付してください。

https://github.com/MixedNuts-Dev/fatal-frame2-remake-twin-swap/issues

## 仕組み

ゲームのファイルは変更しません。

キャラの衣装は、ゲームのデータベース（kidsobjdb）の中で「どのモデルを使うか」を
参照しています。Mod はこの参照を、設定に合わせて相手のキャラのモデルに書き換えます。
モデルに付随する材質・骨・揺れものの設定も、一緒に移ります。

澪のモデルは、顔・歯・目まわりを専用の表示グループに置いていて、同行キャラ（繭）は
このグループを表示しません。澪のモデルをそのまま同行キャラに付けると、目玉以外の
顔が消えます。Mod は繭のモデルと同じ構造になるよう、顔を常時表示のグループへ移します
（ファイルのサイズは変わりません）。

書き換えたデータは Mod 専用のデータファイルにまとめ、それを指すように書き換えた
索引ファイル（root.rdb / root.rdx）と一緒に `Mods\twinswap\cache` に作ります。
ゲームが索引ファイルを開くときだけ、キャッシュの方を渡します。元にするのは
その時点の索引ファイルなので、Yumia fdata tools で入れた Mod はそのまま残ります。

---

# English

## What this does

**Swaps the looks of the player character (Mio) and the companion (Mayu).**
In the config file you can choose between playing as Mayu with Mio at your side,
two Mayus, or two Mios.

Only the models (face, hair, body and costume) change. Animations, voices, subtitles
and the story stay as they are. For example, Mayu's limp remains on the companion
even when she looks like Mio.

## Requirements

- FATAL FRAME II: Crimson Butterfly REMAKE (Steam)

No game files are modified, so this will not trip Steam's file integrity verification.

## What's included

| File | Role |
|---|---|
| `xinput1_4.dll` | the mod itself |
| `Mods\twinswap\twinswap.ini` | configuration |
| `Mods\twinswap\README.md` | this file |

You copy two things: **`xinput1_4.dll` and the `Mods` folder.**
When the game runs, a log (`twinswap.log`) and a `cache` folder holding the swap
data are created in `Mods\twinswap\`.

### Using it with other mods

- **It does not interfere with Native 120FPS Option or Mouse Wheel Camera Speed.**
  Those use `dinput8.dll` / `version.dll` and this mod uses `xinput1_4.dll`, so any
  combination works, installed in any order. The `Mods` folders simply merge
- **It works together with mods installed with Yumia fdata tools** (costume edits
  and so on). The swap data is built from the game data as it currently is,
  including those mods, so if you reinstall them it is rebuilt on the next launch
- If the game folder **already contains a different `xinput1_4.dll`, do not
  overwrite it.** Another tool may be using the same name

## Installation

1. Close the game.

2. Copy `xinput1_4.dll` and the `Mods` folder into the game's root directory
   (the folder containing `FatalFrameII.exe`).

   ```
   ...\steamapps\common\FatalFrameII\FatalFrameII.exe
   ...\steamapps\common\FatalFrameII\xinput1_4.dll     <- added
   ...\steamapps\common\FatalFrameII\Mods\twinswap\    <- added
   ```

   To open the game folder: right-click the title in your Steam library →
   **Manage** → **Browse local files**

3. Launch the game and load a save.

## Uninstallation

Delete `xinput1_4.dll` and the `Mods\twinswap` folder. No game files are modified,
so removal restores the original state completely.

To disable temporarily, set `Enabled` to `0` in `twinswap.ini`.

## Configuration

`twinswap.ini` exposes the following. Changes take effect after restarting the game.

| Key | Meaning |
|---|---|
| `Main` | Look of the player character (Mio in the story). `mio` or `mayu`. Default `mayu` |
| `Sub` | Look of the companion (Mayu in the story). `mio` or `mayu`. Default `mio` |
| `Enabled` | `1` = on / `0` = off |
| `Log` | `1` = write a log file / `0` = no log |

| `Main` | `Sub` | Result |
|---|---|---|
| `mayu` | `mio` | the twins swapped (default) |
| `mayu` | `mayu` | two Mayus |
| `mio` | `mio` | two Mios |
| `mio` | `mayu` | vanilla |

Any value other than `mio` / `mayu` leaves that character's original look.

## Costume pairing

Costumes are paired one-to-one in costume menu order. For example, when Mio wears
her 2nd costume, the player character appears as Mayu in Mayu's 2nd costume.

- **The SILENT HILL f collaboration costume and items are out of scope.** Mio's
  8th costume (the navy sailor outfit) has no counterpart on Mayu's side and is not
  swapped; with it selected, the player character keeps Mio's look
- Accessories are not part of the swap. They show up on the swapped look as usual

### What has been checked in game

Only two pairs have actually been checked in game: **the default costumes** and
**the 7th costumes** (the Japanese-style gothic dresses; left wing for Mio, right
wing for Mayu). All three `Main` / `Sub` combinations, the TAB menu, the costume
menu and photo mode were checked with them.

The other costumes (2nd to 6th) go through exactly the same process, but have not
been checked in game yet. If one of them looks wrong, please let me know in a
GitHub Issue.

## Notes

- **The preview in the costume menu also shows the swapped look.** The costume name
  and the preview not matching is expected with this mod
- The game's cutscenes are video files, so they are not swapped
- The swap data (`Mods\twinswap\cache`, up to about 90 MB) is built on the first
  launch, and again after you change the settings or the mods installed with
  Yumia fdata tools. Other launches reuse it
- A game update may break this mod. In that case the mod does nothing and the game
  runs unmodified; the reason is written to the log
- Nothing is written to your save data. Removing the mod restores the original looks
- Antivirus software may flag this mod because it intercepts the game opening its
  files. It performs no network activity, and the only files it writes are inside
  its own folder

## Disclaimer

**This mod is provided as-is, without any warranty. The author accepts no liability
for any damage arising from its use,** including but not limited to corruption or
loss of save data, game malfunction, or any other problem. Use it at your own risk.

**Always back up your save data before installing.**

```
%LOCALAPPDATA%\KoeiTecmo\FatalFrameII\Savedata\
```

## If it doesn't work

1. Is `xinput1_4.dll` in the game's root folder (next to `FatalFrameII.exe`)?
   **It does not go inside the `Mods` folder**
2. Is `twinswap.ini` present in `Mods\twinswap\`?
   Has the folder name been changed?
3. Has `twinswap.log` been created in `Mods\twinswap\`?
   If not, `xinput1_4.dll` is not being loaded

If the log contains lines like these, the mod is working:

```
[OK] File hook installed
[OK] Generated the swap data (...)
```

On later launches the second line reads `[OK] Using the cached swap data`.
When reporting a problem, please open a GitHub Issue and attach `twinswap.log`.

https://github.com/MixedNuts-Dev/fatal-frame2-remake-twin-swap/issues

## How it works

No game files are modified.

Each costume refers to the model it uses from inside the game's database
(kidsobjdb). The mod rewrites those references to point at the other twin's models,
according to the settings. The material, skeleton and cloth settings attached to
each model move along with it.

Mio's models keep the face, teeth and eye area in a display group of their own,
which the companion (Mayu) never shows. Put on the companion as they are, Mio's
models lose everything of the face but the eyeballs. The mod moves the face into
the always-visible group, the same layout Mayu's models use (file sizes do not
change).

The rewritten data goes into a data file of the mod's own, created in
`Mods\twinswap\cache` together with copies of the index files (root.rdb /
root.rdx) rewritten to point at it. Only when the game opens the index files does
the mod hand it the cached ones. They are built from the index files as they are
at that moment, so mods installed with Yumia fdata tools stay in place.

---

## License

MIT License — Copyright (c) 2026 MixedNuts

本ソフトウェアは MIT ライセンスで提供されます。再配布・改変は自由ですが、
著作権表示とライセンス文を必ず残してください。

This software is provided under the MIT License. You are free to redistribute and
modify it, but the copyright notice and the license text must be retained.

https://github.com/MixedNuts-Dev/fatal-frame2-remake-twin-swap
