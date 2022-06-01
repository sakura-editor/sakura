# Change Log

## [Unreleased](https://github.com/sakura-editor/sakura/tree/HEAD)

[Full Changelog](https://github.com/sakura-editor/sakura/compare/v2.4.1...HEAD)

### その他変更

- ver 2.4.1 をリリースしたので ver 2.4.2 に上げる [\#1317](https://github.com/sakura-editor/sakura/pull/1317) ([m-tmatma](https://github.com/m-tmatma))
- Release/v2.4.1  [\#1316](https://github.com/sakura-editor/sakura/pull/1316) ([KENCHjp](https://github.com/KENCHjp))

## [v2.4.1](https://github.com/sakura-editor/sakura/tree/v2.4.1) (2020-05-30)

[Full Changelog](https://github.com/sakura-editor/sakura/compare/v2.4.1-beta3...v2.4.1)

### その他変更

- GitHub Actions の成果物を別々の ZIP として upload する [\#1285](https://github.com/sakura-editor/sakura/pull/1285) ([m-tmatma](https://github.com/m-tmatma))

## [v2.4.1-beta3](https://github.com/sakura-editor/sakura/tree/v2.4.1-beta3) (2020-05-20)

[Full Changelog](https://github.com/sakura-editor/sakura/compare/v2.4.1-beta2...v2.4.1-beta3)



### その他変更

- README.md の「変更履歴」に CHANGELOG.md へのリンクを追加 [\#1309](https://github.com/sakura-editor/sakura/pull/1309) ([takke](https://github.com/takke))
- \#1302 OSDNより転載： 「タイプ別設定」等ダイアログ内の「ヘルプ」ボタンが機能しない [\#1306](https://github.com/sakura-editor/sakura/pull/1306) ([m-tmatma](https://github.com/m-tmatma))
- MinGW のコンパイルエラーを修正する [\#1299](https://github.com/sakura-editor/sakura/pull/1299) ([k-takata](https://github.com/k-takata))
- コンパイルテストを導入する [\#1297](https://github.com/sakura-editor/sakura/pull/1297) ([berryzplus](https://github.com/berryzplus))

## [v2.4.1-beta2](https://github.com/sakura-editor/sakura/tree/v2.4.1-beta2) (2020-05-10)

[Full Changelog](https://github.com/sakura-editor/sakura/compare/v2.4.0...v2.4.1-beta2)

### 仕様変更


### バグ修正

- CImpExpRegex::Import の実装を見直し [\#1273](https://github.com/sakura-editor/sakura/pull/1273) ([beru](https://github.com/beru))
- ツールイメージのリサイズ機構にフォールバック機能を付ける [\#1261](https://github.com/sakura-editor/sakura/pull/1261) ([berryzplus](https://github.com/berryzplus))
- 正規表現キーワードのインポートで許容サイズを超える文字列を無駄にコピーしているのを修正する [\#1244](https://github.com/sakura-editor/sakura/pull/1244) ([berryzplus](https://github.com/berryzplus))



### その他変更

- installer のタイトルのアプリ名で 32bit/64bit の区別ができるようにする [\#1293](https://github.com/sakura-editor/sakura/pull/1293) ([m-tmatma](https://github.com/m-tmatma))
- Combo\_AddString の戻り値の型を int にする [\#1291](https://github.com/sakura-editor/sakura/pull/1291) ([m-tmatma](https://github.com/m-tmatma))
- CLayoutIntのテストを追加 [\#1290](https://github.com/sakura-editor/sakura/pull/1290) ([berryzplus](https://github.com/berryzplus))
- 単体テストで文字列リソースを利用できるようにする [\#1275](https://github.com/sakura-editor/sakura/pull/1275) ([berryzplus](https://github.com/berryzplus))
- CodeFactor の警告を修正 [\#1272](https://github.com/sakura-editor/sakura/pull/1272) ([m-tmatma](https://github.com/m-tmatma))
- GitHub Actions の yml のCodeFactor の警告 [\#1270](https://github.com/sakura-editor/sakura/pull/1270) ([m-tmatma](https://github.com/m-tmatma))
- azure pipeline の表示名のタイポ修正 [\#1258](https://github.com/sakura-editor/sakura/pull/1258) ([m-tmatma](https://github.com/m-tmatma))
- googletest を無視リストに追加 [\#1256](https://github.com/sakura-editor/sakura/pull/1256) ([m-tmatma](https://github.com/m-tmatma))
- 履歴コンボの履歴削除機能の発動条件を変更したい [\#1255](https://github.com/sakura-editor/sakura/pull/1255) ([berryzplus](https://github.com/berryzplus))
- ヘルプの動作環境OSの記述を更新 [\#1254](https://github.com/sakura-editor/sakura/pull/1254) ([beru](https://github.com/beru))
- GrepとGrep置換ダイアログの微調整 [\#1242](https://github.com/sakura-editor/sakura/pull/1242) ([7-rate](https://github.com/7-rate))
- 折り返しされた際、文字数カウントが合わない不具合を修正 [\#1241](https://github.com/sakura-editor/sakura/pull/1241) ([beru](https://github.com/beru))
- TCHAR系マクロ \_stprintf の呼び出しがまだ残っている箇所を swprintf 関数の呼び出しに変更 [\#1240](https://github.com/sakura-editor/sakura/pull/1240) ([beru](https://github.com/beru))
- \_wcsdup\(\)ではfree\(\)を使うべきなはずなのでそうする [\#1239](https://github.com/sakura-editor/sakura/pull/1239) ([beru](https://github.com/beru))
- SJISエンコードのキーワードヘルプ辞書を設定するとき表示化けする問題への対処 [\#1238](https://github.com/sakura-editor/sakura/pull/1238) ([beru](https://github.com/beru))
- wstringをWCHAR\[N\]にコピーする処理でNUL終端が付かない不具合を修正 [\#1235](https://github.com/sakura-editor/sakura/pull/1235) ([berryzplus](https://github.com/berryzplus))
- 開発バージョンを ver 2.4.1 に変更 [\#1231](https://github.com/sakura-editor/sakura/pull/1231) ([m-tmatma](https://github.com/m-tmatma))
- CNativeテストケースの期待値を修正する [\#1230](https://github.com/sakura-editor/sakura/pull/1230) ([berryzplus](https://github.com/berryzplus))
- UNICODE一本化対応で見落としていたA版専用処理を削りたい [\#1229](https://github.com/sakura-editor/sakura/pull/1229) ([berryzplus](https://github.com/berryzplus))
- コピー代入のメソッド名を書き間違っているのを訂正 [\#1228](https://github.com/sakura-editor/sakura/pull/1228) ([berryzplus](https://github.com/berryzplus))
- googletestのビルドをバッチスクリプトで行うように変更したい [\#1227](https://github.com/sakura-editor/sakura/pull/1227) ([berryzplus](https://github.com/berryzplus))
- Grep置換ダイアログの「置換後」の実装を置換ダイアログと合わせる [\#1224](https://github.com/sakura-editor/sakura/pull/1224) ([berryzplus](https://github.com/berryzplus))

## [v2.4.0](https://github.com/sakura-editor/sakura/tree/v2.4.0) (2020-04-18)

[Full Changelog](https://github.com/sakura-editor/sakura/compare/v2.4.0-beta5...v2.4.0)

### その他変更

- 一括フォーマット実施に向けての微修正 [\#1222](https://github.com/sakura-editor/sakura/pull/1222) ([berryzplus](https://github.com/berryzplus))
- beta5 リリース用のヘルプをマージ [\#1218](https://github.com/sakura-editor/sakura/pull/1218) ([m-tmatma](https://github.com/m-tmatma))

## [v2.4.0-beta5](https://github.com/sakura-editor/sakura/tree/v2.4.0-beta5) (2020-03-21)

[Full Changelog](https://github.com/sakura-editor/sakura/compare/v2.4.0-beta4...v2.4.0-beta5)

### 仕様変更

- 設定画面の数値入力用のエディット コントロールにフォーカス時にIMEを使わないように設定 [\#1212](https://github.com/sakura-editor/sakura/pull/1212) ([beru](https://github.com/beru))



### その他変更

- バージョン情報に表示するプロジェクトURLに付けるラベルキャプションを短くする [\#1215](https://github.com/sakura-editor/sakura/pull/1215) ([berryzplus](https://github.com/berryzplus))
- 除外ファイル・除外フォルダーの指定をGrep置換でも使えるようにする [\#1210](https://github.com/sakura-editor/sakura/pull/1210) ([berryzplus](https://github.com/berryzplus))
- キーワードを指定してタグジャンプができないバグを修正 [\#1208](https://github.com/sakura-editor/sakura/pull/1208) ([7-rate](https://github.com/7-rate))
- マクロMakeStringBufferW0を廃止する [\#1203](https://github.com/sakura-editor/sakura/pull/1203) ([berryzplus](https://github.com/berryzplus))
- ヘルプのコピーライトの年とソフト名変更。 [\#1202](https://github.com/sakura-editor/sakura/pull/1202) ([KENCHjp](https://github.com/KENCHjp))
- C\#ツールの.vsconfigを追加する [\#1199](https://github.com/sakura-editor/sakura/pull/1199) ([berryzplus](https://github.com/berryzplus))

## [v2.4.0-beta4](https://github.com/sakura-editor/sakura/tree/v2.4.0-beta4) (2020-02-15)

[Full Changelog](https://github.com/sakura-editor/sakura/compare/v2.4.0-beta3...v2.4.0-beta4)

### 機能追加

- batch ファイルのキーワードに setlocal/endlocal を追加 [\#1171](https://github.com/sakura-editor/sakura/pull/1171) ([m-tmatma](https://github.com/m-tmatma))

### バグ修正

- C++ファイルのアウトライン解析がトグルで閉じない不具合の修正 [\#1176](https://github.com/sakura-editor/sakura/pull/1176) ([berryzplus](https://github.com/berryzplus))
- 検索条件の文字列をエスケープする処理を関数化する変更のうち、CNativeW::AppendStringF を使うように変更したのをやっぱ止める [\#1135](https://github.com/sakura-editor/sakura/pull/1135) ([beru](https://github.com/beru))



### その他変更

- ChmSourceConverterのターゲットバージョンを下げる [\#1195](https://github.com/sakura-editor/sakura/pull/1195) ([berryzplus](https://github.com/berryzplus))
- ログの整形、変数の定義位置の移動 \(\#1183 を分割してレビューしやすくする\) [\#1186](https://github.com/sakura-editor/sakura/pull/1186) ([m-tmatma](https://github.com/m-tmatma))
- Appveyor 専用の環境変数を汎用的なものに定義しなおす \(\#1183 を分割した PR\) [\#1185](https://github.com/sakura-editor/sakura/pull/1185) ([m-tmatma](https://github.com/m-tmatma))
- Visual Studio 2017/2019 の環境構築方法に関する説明を更新 [\#1165](https://github.com/sakura-editor/sakura/pull/1165) ([m-tmatma](https://github.com/m-tmatma))
- Visual Studio のインストールオプションの設定ファイルを追加する [\#1162](https://github.com/sakura-editor/sakura/pull/1162) ([m-tmatma](https://github.com/m-tmatma))
- インストーラに同梱する bregonig.dll を Ver.4.20 にする [\#1158](https://github.com/sakura-editor/sakura/pull/1158) ([m-tmatma](https://github.com/m-tmatma))
- Universal Ctags の Windows のバイナリ \(2020-01-12/feffe43a\) に差し替え [\#1156](https://github.com/sakura-editor/sakura/pull/1156) ([m-tmatma](https://github.com/m-tmatma))
- CDataProfileの「abortします」コメントを復活する [\#1155](https://github.com/sakura-editor/sakura/pull/1155) ([berryzplus](https://github.com/berryzplus))
- Copyright を 2020 にする [\#1154](https://github.com/sakura-editor/sakura/pull/1154) ([m-tmatma](https://github.com/m-tmatma))
- 検索条件の文字列をエスケープする処理を関数化する [\#1132](https://github.com/sakura-editor/sakura/pull/1132) ([berryzplus](https://github.com/berryzplus))

## [v2.4.0-beta3](https://github.com/sakura-editor/sakura/tree/v2.4.0-beta3) (2019-12-22)

[Full Changelog](https://github.com/sakura-editor/sakura/compare/v2.4.0-beta2...v2.4.0-beta3)

### 機能追加

- ICU4Cによる文字コード検出機能を追加する [\#1104](https://github.com/sakura-editor/sakura/pull/1104) ([berryzplus](https://github.com/berryzplus))
- C/C++の強調キーワード更新 [\#1102](https://github.com/sakura-editor/sakura/pull/1102) ([beru](https://github.com/beru))
- 「カーソル行をウィンドウ上部へ」「カーソル行をウィンドウ下部へ」機能を追加 [\#1101](https://github.com/sakura-editor/sakura/pull/1101) ([7-rate](https://github.com/7-rate))
- ドラッグ操作でタブの並び替え、最後のタブより右の位置にドラッグした場合に最後のタブの位置にする [\#1099](https://github.com/sakura-editor/sakura/pull/1099) ([beru](https://github.com/beru))

### バグ修正

- 「ツールバーを表示」などに関する問題に対処する [\#1113](https://github.com/sakura-editor/sakura/pull/1113) ([berryzplus](https://github.com/berryzplus))



### その他変更

- ヘルプ中のスタイルシートでセミコロンの指定の誤りによる Code Factor の警告を修正する [\#1111](https://github.com/sakura-editor/sakura/pull/1111) ([m-tmatma](https://github.com/m-tmatma))

## [v2.4.0-beta2](https://github.com/sakura-editor/sakura/tree/v2.4.0-beta2) (2019-11-23)

[Full Changelog](https://github.com/sakura-editor/sakura/compare/v2.4.0-beta...v2.4.0-beta2)

### バグ修正

- CNativeW::SetString に NULL を指定した場合に wcslen に NULL を渡して落ちてしまう不具合を修正 [\#1087](https://github.com/sakura-editor/sakura/pull/1087) ([beru](https://github.com/beru))
- MYDEVMODEの等価比較演算子の隠れバグを修正する [\#1079](https://github.com/sakura-editor/sakura/pull/1079) ([berryzplus](https://github.com/berryzplus))



### その他変更

- PNG の最適化 [\#1085](https://github.com/sakura-editor/sakura/pull/1085) ([Rukoto](https://github.com/Rukoto))
- installer: Enable solid compression [\#1080](https://github.com/sakura-editor/sakura/pull/1080) ([k-takata](https://github.com/k-takata))

## [v2.4.0-beta](https://github.com/sakura-editor/sakura/tree/v2.4.0-beta) (2019-10-12)

[Full Changelog](https://github.com/sakura-editor/sakura/compare/v2.4.0-alpha1...v2.4.0-beta)

### 仕様変更

- プロジェクトのビルドにWindows10 SDKを使う [\#1044](https://github.com/sakura-editor/sakura/pull/1044) ([berryzplus](https://github.com/berryzplus))

### 機能追加

- 最近使ったファイル挿入 と 最近使ったフォルダー挿入 を行う機能追加 [\#1063](https://github.com/sakura-editor/sakura/pull/1063) ([beru](https://github.com/beru))
- PlatformToolset 指定をプロパティーシートに分離して VS2017 および VS2019 で両対応できるようにする [\#866](https://github.com/sakura-editor/sakura/pull/866) ([m-tmatma](https://github.com/m-tmatma))
- 「同名のC/C++ヘッダー\(ソース\)を開く」機能が利用可能か調べる処理で拡張子の確認が行われるように記述追加 [\#812](https://github.com/sakura-editor/sakura/pull/812) ([beru](https://github.com/beru))

### バグ修正

- 確保済みメモリサイズの更新タイミングを訂正する [\#1061](https://github.com/sakura-editor/sakura/pull/1061) ([berryzplus](https://github.com/berryzplus))
- 文字コードの指定ダイアログのCPチェックの仕様を修正する [\#1059](https://github.com/sakura-editor/sakura/pull/1059) ([berryzplus](https://github.com/berryzplus))
- 正規表現キーワードの一致判定が0文字マッチをマッチとみなさないように変更する [\#1030](https://github.com/sakura-editor/sakura/pull/1030) ([berryzplus](https://github.com/berryzplus))
- シンボリックリンク自体のタイムスタンプではなくターゲットのタイムスタンプを取得するように修正 [\#1011](https://github.com/sakura-editor/sakura/pull/1011) ([beru](https://github.com/beru))
- \#780 でのCNativeW の初期状態での仕様を取り消して、 \#948 の不具合を修正する [\#949](https://github.com/sakura-editor/sakura/pull/949) ([m-tmatma](https://github.com/m-tmatma))
- Vistaスタイルのファイルダイアログ使用時に名前を付けて保存の文字コードセット, 改行コード, BOM指定が有効に動作しない問題を修正 [\#874](https://github.com/sakura-editor/sakura/pull/874) ([beru](https://github.com/beru))
- Vistaスタイルのファイルダイアログ使用時に新規ファイルの保存が行えない問題を修正 [\#867](https://github.com/sakura-editor/sakura/pull/867) ([beru](https://github.com/beru))
- Vistaスタイルのファイルダイアログでカスタマイズ部分が表示されない場合がある問題の修正 [\#845](https://github.com/sakura-editor/sakura/pull/845) ([beru](https://github.com/beru))



### その他変更

- ルーラー描画の高速化 [\#1067](https://github.com/sakura-editor/sakura/pull/1067) ([beru](https://github.com/beru))
- レイアウト情報の作成処理をわずかに高速化 [\#1056](https://github.com/sakura-editor/sakura/pull/1056) ([beru](https://github.com/beru))
- ファイル保存の副次処理調整 [\#1054](https://github.com/sakura-editor/sakura/pull/1054) ([beru](https://github.com/beru))
- レイアウト処理の負荷を下げる為、コメントが色分け表示対象外になる機会を増やす [\#1052](https://github.com/sakura-editor/sakura/pull/1052) ([beru](https://github.com/beru))
- インストーラー同梱用のキーワードファイルをUTF-8テキストに変換する [\#1022](https://github.com/sakura-editor/sakura/pull/1022) ([berryzplus](https://github.com/berryzplus))
- フォントサイズ変更時に不要な処理の呼び出しを行わないように判定追加 [\#1021](https://github.com/sakura-editor/sakura/pull/1021) ([beru](https://github.com/beru))
- CEditView::CreateOrUpdateCompatibleBitmap において画面バッファを作成する条件を変更 [\#1015](https://github.com/sakura-editor/sakura/pull/1015) ([beru](https://github.com/beru))
- CImageListMgr::MyDitherBlt で DIB を使う事で最適化 [\#997](https://github.com/sakura-editor/sakura/pull/997) ([beru](https://github.com/beru))
- CViewSelect::PrintSelectionInfoMsg の高速化、及び行のEOL長を取得する処理の高速化 [\#993](https://github.com/sakura-editor/sakura/pull/993) ([beru](https://github.com/beru))
- ファイル読み込み時のステータスバー上のプログレスバー更新やメッセージ配送処理の呼び出しは時間経過を契機に行う方式に変更 [\#992](https://github.com/sakura-editor/sakura/pull/992) ([beru](https://github.com/beru))
- 「すべて置換」処理の高速化 [\#987](https://github.com/sakura-editor/sakura/pull/987) ([beru](https://github.com/beru))
- メモリプールを使う事でメモリ確保と解放を高速化 [\#985](https://github.com/sakura-editor/sakura/pull/985) ([beru](https://github.com/beru))
- DISABLED テストのサンプルを追加する [\#943](https://github.com/sakura-editor/sakura/pull/943) ([m-tmatma](https://github.com/m-tmatma))
- HTML の Help 中の JavaScript の文字コードを UTF-8 に修正 [\#939](https://github.com/sakura-editor/sakura/pull/939) ([m-tmatma](https://github.com/m-tmatma))
- md5 ファイルを git の無視リストに追加する [\#923](https://github.com/sakura-editor/sakura/pull/923) ([m-tmatma](https://github.com/m-tmatma))
- PR でビルドに関係ないドキュメント等の更新の場合に CI ビルドが走らないようにする [\#919](https://github.com/sakura-editor/sakura/pull/919) ([m-tmatma](https://github.com/m-tmatma))
- README に star history を追加 [\#917](https://github.com/sakura-editor/sakura/pull/917) ([m-tmatma](https://github.com/m-tmatma))
- VS2017 用の msbuild.exe の探索処理で時間がかかる問題を修正する [\#915](https://github.com/sakura-editor/sakura/pull/915) ([m-tmatma](https://github.com/m-tmatma))
- Issue Template のバグ報告用のものを修正 [\#914](https://github.com/sakura-editor/sakura/pull/914) ([m-tmatma](https://github.com/m-tmatma))
- \#877 で導入した単体テストで DISABLED テストを実装 [\#905](https://github.com/sakura-editor/sakura/pull/905) ([m-tmatma](https://github.com/m-tmatma))
- NOMINMAXの定義をifndefで括る [\#904](https://github.com/sakura-editor/sakura/pull/904) ([berryzplus](https://github.com/berryzplus))
- \#endifに続く文字列をコメントアウトする [\#902](https://github.com/sakura-editor/sakura/pull/902) ([berryzplus](https://github.com/berryzplus))
- gmockのターゲットを参照するのをやめる [\#901](https://github.com/sakura-editor/sakura/pull/901) ([berryzplus](https://github.com/berryzplus))
- Inno Setup での OS の対応バージョンを Windows 7 以降に変更 [\#895](https://github.com/sakura-editor/sakura/pull/895) ([m-tmatma](https://github.com/m-tmatma))
- テストのビルドにパッケージを使ってビルド時間を短縮する\(MinGW版\) [\#894](https://github.com/sakura-editor/sakura/pull/894) ([berryzplus](https://github.com/berryzplus))
- MsBuild探索方法を見直す [\#892](https://github.com/sakura-editor/sakura/pull/892) ([berryzplus](https://github.com/berryzplus))
- SonarQube に関するドキュメントを更新 [\#882](https://github.com/sakura-editor/sakura/pull/882) ([m-tmatma](https://github.com/m-tmatma))
- 構造体比較にmemcmpを使うのをやめる [\#877](https://github.com/sakura-editor/sakura/pull/877) ([berryzplus](https://github.com/berryzplus))
- APPVEYOR\_REPO\_TAG\_NAME を使うのをやめて `git describe --tags --contains` を使うようにする [\#876](https://github.com/sakura-editor/sakura/pull/876) ([m-tmatma](https://github.com/m-tmatma))
- アセンブラ出力の指定方式を変更する [\#871](https://github.com/sakura-editor/sakura/pull/871) ([berryzplus](https://github.com/berryzplus))
- README.md の \#866 の PR に対するリンクを修正 [\#870](https://github.com/sakura-editor/sakura/pull/870) ([m-tmatma](https://github.com/m-tmatma))
- azure pipelines =\> Azure Pipelines に変更 [\#869](https://github.com/sakura-editor/sakura/pull/869) ([m-tmatma](https://github.com/m-tmatma))
- ビルドで生成されるファイルをクリーン対象に含める [\#868](https://github.com/sakura-editor/sakura/pull/868) ([berryzplus](https://github.com/berryzplus))
- マクロのリンクの誤りを修正 [\#864](https://github.com/sakura-editor/sakura/pull/864) ([m-tmatma](https://github.com/m-tmatma))
- postBuild.bat から失われた build-installer.bat が依存する処理を復元する。 [\#860](https://github.com/sakura-editor/sakura/pull/860) ([ds14050](https://github.com/ds14050))
- クリーン時に成果物が削除されるようにする [\#858](https://github.com/sakura-editor/sakura/pull/858) ([berryzplus](https://github.com/berryzplus))
- 不要な CCaret::ShowCaretPosInfo の呼び出しを無くす [\#854](https://github.com/sakura-editor/sakura/pull/854) ([beru](https://github.com/beru))
- 1文字の比較で wcsncmp 関数を使うのは止める [\#853](https://github.com/sakura-editor/sakura/pull/853) ([beru](https://github.com/beru))
- 英単語の綴り間違い修正 [\#849](https://github.com/sakura-editor/sakura/pull/849) ([beru](https://github.com/beru))
- インストーラの内部の文章の句点を「，．」から「、。」に修正する [\#842](https://github.com/sakura-editor/sakura/pull/842) ([berryzplus](https://github.com/berryzplus))
- タグジャンプにてタグ検索が二分探索可能な場合は二分探索を行うようにした [\#836](https://github.com/sakura-editor/sakura/pull/836) ([7-rate](https://github.com/7-rate))
- Add tests of IsMailAddress. / IsMailAddress のテストを追加します。 [\#823](https://github.com/sakura-editor/sakura/pull/823) ([ds14050](https://github.com/ds14050))

## [v2.4.0-alpha1](https://github.com/sakura-editor/sakura/tree/v2.4.0-alpha1) (2019-03-27)

[Full Changelog](https://github.com/sakura-editor/sakura/compare/v2.3.2.0...v2.4.0-alpha1)

### 仕様変更

- UTF8 BOMなし を文字コードのデフォルトにする [\#604](https://github.com/sakura-editor/sakura/pull/604) ([m-tmatma](https://github.com/m-tmatma))
- アウトライン解析の更新アイコンを変更する [\#558](https://github.com/sakura-editor/sakura/pull/558) ([berryzplus](https://github.com/berryzplus))
- Universal ctags \(2018-09-16 の daily build\) を組み込む [\#454](https://github.com/sakura-editor/sakura/pull/454) ([m-tmatma](https://github.com/m-tmatma))
- バージョンダイアログの "Share Ver" 表記を削除する [\#449](https://github.com/sakura-editor/sakura/pull/449) ([m-tmatma](https://github.com/m-tmatma))
- テキストの折り返し方法のデフォルトを 「折り返さない」に変更 [\#267](https://github.com/sakura-editor/sakura/pull/267) ([m-tmatma](https://github.com/m-tmatma))
- 「ファイルの排他制御モード」のデフォルトを 「排他制御しない」に変更する [\#263](https://github.com/sakura-editor/sakura/pull/263) ([m-tmatma](https://github.com/m-tmatma))
- 「常駐しない」をデフォルトに変更 [\#32](https://github.com/sakura-editor/sakura/pull/32) ([berryzplus](https://github.com/berryzplus))

### 機能追加

- Window テキストをCNativeT で取得/設定するユーティリティ関数を追加 [\#776](https://github.com/sakura-editor/sakura/pull/776) ([m-tmatma](https://github.com/m-tmatma))
- 開いているファイルのフォルダーのパスをクリップボードにコピーできるようにする [\#718](https://github.com/sakura-editor/sakura/pull/718) ([m-tmatma](https://github.com/m-tmatma))
- Vistaスタイルのファイルダイアログを使えるようにする [\#716](https://github.com/sakura-editor/sakura/pull/716) ([beru](https://github.com/beru))
- 背景画像表示の不透明度を設定出来るように変更 [\#704](https://github.com/sakura-editor/sakura/pull/704) ([ds14050](https://github.com/ds14050))
- Windows Imaging Component を使って背景画像を読み込み、透過描画対応 [\#683](https://github.com/sakura-editor/sakura/pull/683) ([beru](https://github.com/beru))
- コマンドアイコンの追加 [\#680](https://github.com/sakura-editor/sakura/pull/680) ([berryzplus](https://github.com/berryzplus))
- 「ファイルのプロパティ」ダイアログのエディットコントロールにTabStopを付ける [\#670](https://github.com/sakura-editor/sakura/pull/670) ([beru](https://github.com/beru))
- HighDPI環境でも正しくアイコンを表示できるようにする [\#631](https://github.com/sakura-editor/sakura/pull/631) ([berryzplus](https://github.com/berryzplus))
- `ファイルの場所を powershell で開く` に対応する [\#623](https://github.com/sakura-editor/sakura/pull/623) ([m-tmatma](https://github.com/m-tmatma))
- 管理者としてコマンドプロンプトを開くメニュー項目を追加 [\#618](https://github.com/sakura-editor/sakura/pull/618) ([m-tmatma](https://github.com/m-tmatma))
- コマンドプロンプトを開くためのメニューを実装 [\#603](https://github.com/sakura-editor/sakura/pull/603) ([m-tmatma](https://github.com/m-tmatma))
- 「ファイルの場所をエクスプローラーで開く」追加 [\#549](https://github.com/sakura-editor/sakura/pull/549) ([berryzplus](https://github.com/berryzplus))
- ターゲットwindowsをwindows7に上げる [\#548](https://github.com/sakura-editor/sakura/pull/548) ([berryzplus](https://github.com/berryzplus))
- ツールバーの表示ON/OFFを行うとちらつく現象への対策 [\#533](https://github.com/sakura-editor/sakura/pull/533) ([beru](https://github.com/beru))
- ツールバーを非表示から表示に切り替える際にアイコンの状態を設定する処理を呼び出し [\#464](https://github.com/sakura-editor/sakura/pull/464) ([beru](https://github.com/beru))
- メールアドレスの色替え判定を高速化すると同時にRFCに準拠させる [\#421](https://github.com/sakura-editor/sakura/pull/421) ([berryzplus](https://github.com/berryzplus))
- 『SAKURAで Grep』をエクスプローラに追加する [\#411](https://github.com/sakura-editor/sakura/pull/411) ([m-tmatma](https://github.com/m-tmatma))
- grep/grep 置換で除外ファイル、除外フォルダーを指定できるようにする [\#403](https://github.com/sakura-editor/sakura/pull/403) ([m-tmatma](https://github.com/m-tmatma))
- 環境変数 SKIP\_CREATE\_GITHASH が 1 にセットしている場合、githash.h の生成をスキップする [\#319](https://github.com/sakura-editor/sakura/pull/319) ([m-tmatma](https://github.com/m-tmatma))
- res ファイルのハッシュを計算するスクリプト [\#317](https://github.com/sakura-editor/sakura/pull/317) ([m-tmatma](https://github.com/m-tmatma))
- python のアウトライン解析を実装 [\#314](https://github.com/sakura-editor/sakura/pull/314) ([m-tmatma](https://github.com/m-tmatma))
- エディタ設定：インデント設定と行末スペース削り [\#245](https://github.com/sakura-editor/sakura/pull/245) ([kobake](https://github.com/kobake))
- \[x64対応\] バージョン情報にPlatform情報を埋め込み \(32bit/64bit\) [\#179](https://github.com/sakura-editor/sakura/pull/179) ([kobake](https://github.com/kobake))

### バグ修正

- grep で 除外ファイル、除外フォルダーが効かない のを修正 [\#758](https://github.com/sakura-editor/sakura/pull/758) ([m-tmatma](https://github.com/m-tmatma))
- grep で 除外ファイル、除外フォルダーが効かない問題を修正するため、除外パターンを指定するコマンドラインを復活する [\#750](https://github.com/sakura-editor/sakura/pull/750) ([m-tmatma](https://github.com/m-tmatma))
- ウィンドウ一覧画面を開いた後にウィンドウサイズを大きくしてから閉じて開き直すとウィンドウサイズにコントロールが追従していない問題を修正 [\#726](https://github.com/sakura-editor/sakura/pull/726) ([beru](https://github.com/beru))
- マウスクリックによるキャレット移動が出来なくなる不具合の解消 [\#574](https://github.com/sakura-editor/sakura/pull/574) ([beru](https://github.com/beru))
- Fix \#539 「Texのファイルで落ちます。」 [\#553](https://github.com/sakura-editor/sakura/pull/553) ([ds14050](https://github.com/ds14050))
- 英語windowsでメッセージがおかしい件に対処する [\#534](https://github.com/sakura-editor/sakura/pull/534) ([berryzplus](https://github.com/berryzplus))
- エントリをProfileへ書き込む処理（CProfile::SetProfileDataImp）に入れてしまった不具合の修正 [\#294](https://github.com/sakura-editor/sakura/pull/294) ([beru](https://github.com/beru))
- CGrepAgent::DoGrep のローカル変数初期化漏れ修正 [\#236](https://github.com/sakura-editor/sakura/pull/236) ([yoshinrt](https://github.com/yoshinrt))
- Git情報がなくてもビルドできるようにする \(Close \#148\) [\#191](https://github.com/sakura-editor/sakura/pull/191) ([kobake](https://github.com/kobake))
- sakura\_lang\_en\_US.dll のバージョンにGitHash が含まれない [\#159](https://github.com/sakura-editor/sakura/pull/159) ([m-tmatma](https://github.com/m-tmatma))
- git情報が存在しないとき、バージョンダイアログにバージョン番号が表示されない＆文字化けする問題を修正 [\#150](https://github.com/sakura-editor/sakura/pull/150) ([kobake](https://github.com/kobake))
- SVN Revision 利用箇所を削除し、代わりに GitHash を用いる [\#147](https://github.com/sakura-editor/sakura/pull/147) ([kobake](https://github.com/kobake))
- タイプ別一覧ダイアログのインポート機能バグ修正 [\#137](https://github.com/sakura-editor/sakura/pull/137) ([berryzplus](https://github.com/berryzplus))



### その他変更

- PR 421 によって導入されたけど revert された IsMailAddress の単体テストを復活する [\#808](https://github.com/sakura-editor/sakura/pull/808) ([m-tmatma](https://github.com/m-tmatma))
- githash.batでgitを探すようにする [\#794](https://github.com/sakura-editor/sakura/pull/794) ([berryzplus](https://github.com/berryzplus))
- \#398 に対する回避策 \(特定のファイルで描画が遅くなる\) [\#792](https://github.com/sakura-editor/sakura/pull/792) ([berryzplus](https://github.com/berryzplus))
- 文字化け対策コメントを除去 [\#791](https://github.com/sakura-editor/sakura/pull/791) ([berryzplus](https://github.com/berryzplus))
- vs2017ビルド時のビルドログを整理する [\#790](https://github.com/sakura-editor/sakura/pull/790) ([berryzplus](https://github.com/berryzplus))
- Fix \#781 issue「googletest でテストに失敗しても ビルド結果が失敗にならない」 [\#784](https://github.com/sakura-editor/sakura/pull/784) ([ds14050](https://github.com/ds14050))
- Fix: missing compile option of test1.exe on MinGW platform. [\#783](https://github.com/sakura-editor/sakura/pull/783) ([ds14050](https://github.com/ds14050))
- バッファが空の状態で CNativeW::Clear を呼び出したときに落ちる不具合修正 [\#780](https://github.com/sakura-editor/sakura/pull/780) ([m-tmatma](https://github.com/m-tmatma))
- CNative::Clear の実装を改善 [\#777](https://github.com/sakura-editor/sakura/pull/777) ([m-tmatma](https://github.com/m-tmatma))
-  chm のビルド前に chm ファイルを削除する  [\#774](https://github.com/sakura-editor/sakura/pull/774) ([m-tmatma](https://github.com/m-tmatma))
- ツールバーの "Grep" と "アウトライン解析" の間にセパレータを入れる [\#773](https://github.com/sakura-editor/sakura/pull/773) ([m-tmatma](https://github.com/m-tmatma))
- ツールバー検索ボックスの垂直位置を調整する [\#771](https://github.com/sakura-editor/sakura/pull/771) ([berryzplus](https://github.com/berryzplus))
- 背景塗りつぶしにPatBltを使う Part2 [\#766](https://github.com/sakura-editor/sakura/pull/766) ([berryzplus](https://github.com/berryzplus))
- 背景塗りつぶしにPatBltを使う Part1 [\#765](https://github.com/sakura-editor/sakura/pull/765) ([berryzplus](https://github.com/berryzplus))
- WinMain のコメントを修正する [\#764](https://github.com/sakura-editor/sakura/pull/764) ([m-tmatma](https://github.com/m-tmatma))
- Copyright を 2019 にする [\#761](https://github.com/sakura-editor/sakura/pull/761) ([m-tmatma](https://github.com/m-tmatma))
- issue template を追加する [\#757](https://github.com/sakura-editor/sakura/pull/757) ([m-tmatma](https://github.com/m-tmatma))
- プラグインを列挙して呼び出す同じような記述があちらこちらにあったので、CJackManager::InvokePlugins を追加して使用 [\#755](https://github.com/sakura-editor/sakura/pull/755) ([beru](https://github.com/beru))
- Revert "grep で 除外ファイル、除外フォルダーが効かない問題を修正するため、除外パターンを指定するコマンドラインを復活する" [\#753](https://github.com/sakura-editor/sakura/pull/753) ([m-tmatma](https://github.com/m-tmatma))
- 新規インストール時に言語設定に英語を適用する処理方法の変更 [\#749](https://github.com/sakura-editor/sakura/pull/749) ([beru](https://github.com/beru))
- doxygen と cppcheck のインストールを install セクションで行う [\#742](https://github.com/sakura-editor/sakura/pull/742) ([m-tmatma](https://github.com/m-tmatma))
- 開いているファイル名をクリップボードにコピーする機能をデフォルトでメニューに追加する [\#741](https://github.com/sakura-editor/sakura/pull/741) ([m-tmatma](https://github.com/m-tmatma))
- ヘルプのバージョンを 2.4.0 に update \(\#713 に追従\) [\#739](https://github.com/sakura-editor/sakura/pull/739) ([m-tmatma](https://github.com/m-tmatma))
- patchunicode 1006 と 1047 の取り込み [\#738](https://github.com/sakura-editor/sakura/pull/738) ([beru](https://github.com/beru))
- add CONTRIBUTING.md [\#734](https://github.com/sakura-editor/sakura/pull/734) ([KageShiron](https://github.com/KageShiron))
- \(ビルドログの解析スクリプト\) 予約語の type と重複する変数名を使わないようにする [\#727](https://github.com/sakura-editor/sakura/pull/727) ([m-tmatma](https://github.com/m-tmatma))
- 冗長な空行を削除するスクリプトのバグを修正 [\#725](https://github.com/sakura-editor/sakura/pull/725) ([m-tmatma](https://github.com/m-tmatma))
- HTML Help のプロジェクトで alias の指定間違いを修正 [\#717](https://github.com/sakura-editor/sakura/pull/717) ([m-tmatma](https://github.com/m-tmatma))
- ダイアログのコントロールの見出し文字の見切れを修正 [\#711](https://github.com/sakura-editor/sakura/pull/711) ([beru](https://github.com/beru))
- Grep に関するメニューにショートカットを指定する [\#705](https://github.com/sakura-editor/sakura/pull/705) ([m-tmatma](https://github.com/m-tmatma))
- `README.md` で `タスクトレイのメニュー項目をデバッグする方法` に変更 [\#700](https://github.com/sakura-editor/sakura/pull/700) ([m-tmatma](https://github.com/m-tmatma))
- CHANGELOG.md の生成方法や取得方法を簡単に参照できるように markdown に説明を追加する [\#696](https://github.com/sakura-editor/sakura/pull/696) ([m-tmatma](https://github.com/m-tmatma))
- サイズボックスウィンドウをその都度作り直すのではなく表示・非表示を切り替えて使いまわす [\#695](https://github.com/sakura-editor/sakura/pull/695) ([beru](https://github.com/beru))
- \#include \<locale.h\> 追加 [\#694](https://github.com/sakura-editor/sakura/pull/694) ([beru](https://github.com/beru))
- CodeFactor の badge を追加 [\#693](https://github.com/sakura-editor/sakura/pull/693) ([m-tmatma](https://github.com/m-tmatma))
- WinMainを整理したい [\#692](https://github.com/sakura-editor/sakura/pull/692) ([berryzplus](https://github.com/berryzplus))
- 8bit256色ツールアイコンを取り込む [\#690](https://github.com/sakura-editor/sakura/pull/690) ([berryzplus](https://github.com/berryzplus))
- マルチユーザー設定を有効にする [\#689](https://github.com/sakura-editor/sakura/pull/689) ([berryzplus](https://github.com/berryzplus))
- ツール類のコンパイルオプションに/MPを付ける [\#688](https://github.com/sakura-editor/sakura/pull/688) ([berryzplus](https://github.com/berryzplus))
- 「このファイルのパス名をコピー」 をタブメニューにも追加する [\#666](https://github.com/sakura-editor/sakura/pull/666) ([m-tmatma](https://github.com/m-tmatma))
- 縦スクロール時に不必要にルーラーの再描画がされないように対策 [\#660](https://github.com/sakura-editor/sakura/pull/660) ([beru](https://github.com/beru))
- スクロールバーの再描画を常には行わないように変更 [\#657](https://github.com/sakura-editor/sakura/pull/657) ([beru](https://github.com/beru))
- 辞書Tipの描画改善、およびHighDPI対応 [\#647](https://github.com/sakura-editor/sakura/pull/647) ([berryzplus](https://github.com/berryzplus))
- フォントラベルのHighDPI対応 [\#645](https://github.com/sakura-editor/sakura/pull/645) ([berryzplus](https://github.com/berryzplus))
- 分割線ウィンドウクラスの処理で使用している各数値をDPI設定に応じて調整 [\#641](https://github.com/sakura-editor/sakura/pull/641) ([beru](https://github.com/beru))
- 独自拡張プロパティシートの「設定フォルダー」ボタンの位置調整処理でDPIを考慮 [\#638](https://github.com/sakura-editor/sakura/pull/638) ([beru](https://github.com/beru))
- `ファイルの場所を コマンドプロンプトを開く` で 管理者ではないときに 32bit アプリから 64bit OS上で起動したときに 32bit で起動してしまうのを修正 [\#627](https://github.com/sakura-editor/sakura/pull/627) ([m-tmatma](https://github.com/m-tmatma))
- 設定データ読み込み処理において言語設定切り替え後にMRUエントリが無い場合は新規インストール後とみなし false を返すように変更 [\#620](https://github.com/sakura-editor/sakura/pull/620) ([beru](https://github.com/beru))
- reST 用の拡張子 \(.rst\) を関連付け用の設定に追加 [\#612](https://github.com/sakura-editor/sakura/pull/612) ([m-tmatma](https://github.com/m-tmatma))
- PR \#599 の不備を修正 [\#609](https://github.com/sakura-editor/sakura/pull/609) ([m-tmatma](https://github.com/m-tmatma))
- HTML Help でビルド方法に関する説明で build.md を参照するように変更 [\#606](https://github.com/sakura-editor/sakura/pull/606) ([m-tmatma](https://github.com/m-tmatma))
- HTML Help で ビルドに必要な条件を更新 [\#605](https://github.com/sakura-editor/sakura/pull/605) ([m-tmatma](https://github.com/m-tmatma))
- Windows 10 でユーザーが手動でサクラエディタに対して関連付けを行えるようにする \(part2\) [\#602](https://github.com/sakura-editor/sakura/pull/602) ([m-tmatma](https://github.com/m-tmatma))
- Windows 10 でのファイルの関連付けに関する説明を HTML Help に追加 [\#599](https://github.com/sakura-editor/sakura/pull/599) ([m-tmatma](https://github.com/m-tmatma))
- Windows 10 でユーザーが手動でサクラエディタに対して関連付けを行えるようにする [\#596](https://github.com/sakura-editor/sakura/pull/596) ([m-tmatma](https://github.com/m-tmatma))
- テストコードで\_swprintf\_pの利用をやめる [\#593](https://github.com/sakura-editor/sakura/pull/593) ([berryzplus](https://github.com/berryzplus))
- Build and run tests on MinGW environment. [\#591](https://github.com/sakura-editor/sakura/pull/591) ([ds14050](https://github.com/ds14050))
- URLリンクの改善 [\#566](https://github.com/sakura-editor/sakura/pull/566) ([berryzplus](https://github.com/berryzplus))
- Fix \#509 「Ctrl+左ボタンダウンからの左ボタンドラッグによる単語選択がちょっとおかしい」 [\#552](https://github.com/sakura-editor/sakura/pull/552) ([ds14050](https://github.com/ds14050))
- ドロップダウンメニューに履歴の管理を追加 [\#551](https://github.com/sakura-editor/sakura/pull/551) ([berryzplus](https://github.com/berryzplus))
- doxygen コメントの @file をつける [\#535](https://github.com/sakura-editor/sakura/pull/535) ([m-tmatma](https://github.com/m-tmatma))
- CEditView::Create において SystemParametersInfo 呼び出しが失敗した場合の対策を追加 [\#527](https://github.com/sakura-editor/sakura/pull/527) ([beru](https://github.com/beru))
- カーソル表示制御の効率化 [\#526](https://github.com/sakura-editor/sakura/pull/526) ([beru](https://github.com/beru))
- キーボードの文字の入力の表示の間隔設定をミリ秒に変換してからタイマーの間隔に使用するように修正 [\#523](https://github.com/sakura-editor/sakura/pull/523) ([beru](https://github.com/beru))
- 共通設定のツールバー画面のリスト表示の項目の高さが表示スケール 100% の場合に不十分な問題を修正 [\#520](https://github.com/sakura-editor/sakura/pull/520) ([beru](https://github.com/beru))
- Visual Studio Install options を修正\(\#6 の反映など\) [\#518](https://github.com/sakura-editor/sakura/pull/518) ([takke](https://github.com/takke))
- HighDPI対応 アプリケーションのアイコン表示 [\#517](https://github.com/sakura-editor/sakura/pull/517) ([beru](https://github.com/beru))
- 共通設定のツールバー画面のリスト表示の項目の高さをDPIに合わせて調整 [\#513](https://github.com/sakura-editor/sakura/pull/513) ([beru](https://github.com/beru))
- メニュー項目の高さ計算でマージン分をDPIに合わせて調整 [\#512](https://github.com/sakura-editor/sakura/pull/512) ([beru](https://github.com/beru))
- parse-buildlog.py での不要なログを削除 [\#507](https://github.com/sakura-editor/sakura/pull/507) ([m-tmatma](https://github.com/m-tmatma))
- ルーラー上の数字フォントの幅が自動的に指定されるように LOGFONT 構造体の lfWidth メンバの値を 0 に変更 [\#505](https://github.com/sakura-editor/sakura/pull/505) ([beru](https://github.com/beru))
- タイプ別設定画面のカラータブの色指定リストの背景色と前景色の色見本矩形の描画処理をDPIに合わせるように変更 [\#499](https://github.com/sakura-editor/sakura/pull/499) ([beru](https://github.com/beru))
- インストーラのメニュー項目の表記を修正 [\#491](https://github.com/sakura-editor/sakura/pull/491) ([m-tmatma](https://github.com/m-tmatma))
- インストーラのビルドログをファイルに出力する [\#483](https://github.com/sakura-editor/sakura/pull/483) ([m-tmatma](https://github.com/m-tmatma))
- The zlib/libpng License の badge を追加 [\#473](https://github.com/sakura-editor/sakura/pull/473) ([m-tmatma](https://github.com/m-tmatma))
- LICENSEファイルを追加する [\#470](https://github.com/sakura-editor/sakura/pull/470) ([berryzplus](https://github.com/berryzplus))
- IME編集エリアの位置変更を行う ImmSetCompositionWindow の呼び出しは IMEが開いている時だけに限定 [\#460](https://github.com/sakura-editor/sakura/pull/460) ([beru](https://github.com/beru))
- ツールバーの状態更新を必要な場合にのみ行うように変更 [\#456](https://github.com/sakura-editor/sakura/pull/456) ([beru](https://github.com/beru))
- bregonig のライセンスファイルをライセンス用のサブフォルダーに配置する [\#455](https://github.com/sakura-editor/sakura/pull/455) ([m-tmatma](https://github.com/m-tmatma))
- SetScrollInfo の呼び出しを抑制 [\#453](https://github.com/sakura-editor/sakura/pull/453) ([beru](https://github.com/beru))
- ステータスバーのテキスト設定、既に同じ値が設定されている場合は再設定しないようにする [\#452](https://github.com/sakura-editor/sakura/pull/452) ([beru](https://github.com/beru))
- タイプ別設定のカラーで強調キーワードの共通設定を押すと表示されるダイアログの高さ調整、DPIスケーリング [\#443](https://github.com/sakura-editor/sakura/pull/443) ([beru](https://github.com/beru))
- ctestを有効にする [\#441](https://github.com/sakura-editor/sakura/pull/441) ([berryzplus](https://github.com/berryzplus))
- Pull Request を簡単にローカルに取得するためのバッチファイルを追加 \(take2\) [\#440](https://github.com/sakura-editor/sakura/pull/440) ([m-tmatma](https://github.com/m-tmatma))
- README にカンバンリンクを追加 [\#436](https://github.com/sakura-editor/sakura/pull/436) ([kobake](https://github.com/kobake))
- 単体テスト向けの CMakeLists.txt で Visual Studio のスタートアッププロジェクトの設定 [\#435](https://github.com/sakura-editor/sakura/pull/435) ([beru](https://github.com/beru))
- exeに高DPI対応のmanifestが埋め込まれるようにする [\#428](https://github.com/sakura-editor/sakura/pull/428) ([berryzplus](https://github.com/berryzplus))
- string\_ex2.h での CNativeA/CNativeW の先行宣言漏れを修正 [\#423](https://github.com/sakura-editor/sakura/pull/423) ([m-tmatma](https://github.com/m-tmatma))
- 単体テストのプロジェクト設定で sakura editor 本体のソースコードのインクルードディレクトリの指定を追加する [\#422](https://github.com/sakura-editor/sakura/pull/422) ([m-tmatma](https://github.com/m-tmatma))
- CGrepAgent::SetGrepResult 出力形式がノーマルの場合の処理の高速化 [\#418](https://github.com/sakura-editor/sakura/pull/418) ([beru](https://github.com/beru))
- タスクトレイのメニュー項目のデバッグ方法 に関するドキュメントを追加 [\#415](https://github.com/sakura-editor/sakura/pull/415) ([m-tmatma](https://github.com/m-tmatma))
- ファイルを開く、で \_MAX\_PATH \(260\) より長いパスのファイルの場合はエラーメッセージを出して処理を中断 [\#409](https://github.com/sakura-editor/sakura/pull/409) ([beru](https://github.com/beru))
- CSearchKeywordManager のコメントの間違いを修正 [\#408](https://github.com/sakura-editor/sakura/pull/408) ([m-tmatma](https://github.com/m-tmatma))
- MinGW Makefileをデバッグ/リリース両対応にする [\#407](https://github.com/sakura-editor/sakura/pull/407) ([berryzplus](https://github.com/berryzplus))
- コメントの修正 [\#405](https://github.com/sakura-editor/sakura/pull/405) ([beru](https://github.com/beru))
- ソースコード中のオンラインヘルプのアドレス更新 [\#392](https://github.com/sakura-editor/sakura/pull/392) ([beru](https://github.com/beru))
- ビルド時に 7zip が利用可能でない場合 7zip をインストールするように促すメッセージ出力する [\#391](https://github.com/sakura-editor/sakura/pull/391) ([m-tmatma](https://github.com/m-tmatma))
- \#357 build.mdの「HTML ヘルプのビルドに必要なもの」　レビュー結果反映　 [\#387](https://github.com/sakura-editor/sakura/pull/387) ([KENCHjp](https://github.com/KENCHjp))
- deleteのオーバーロードがGCCに怒られる対応 [\#386](https://github.com/sakura-editor/sakura/pull/386) ([berryzplus](https://github.com/berryzplus))
- HTML Help 中のリンクを GitHub のものに変える [\#383](https://github.com/sakura-editor/sakura/pull/383) ([m-tmatma](https://github.com/m-tmatma))
- githash.batも exit /b 0 にする [\#374](https://github.com/sakura-editor/sakura/pull/374) ([berryzplus](https://github.com/berryzplus))
- MinGW向けMakefileからbtoolフォルダーへの参照を取り除く [\#371](https://github.com/sakura-editor/sakura/pull/371) ([berryzplus](https://github.com/berryzplus))
- sakura\_lang\_en\_US.dll の MinGW 向け Makefile を更新する [\#369](https://github.com/sakura-editor/sakura/pull/369) ([berryzplus](https://github.com/berryzplus))
- HTML Help Workshop がVS2017で導入される条件 [\#366](https://github.com/sakura-editor/sakura/pull/366) ([KENCHjp](https://github.com/KENCHjp))
- MINGW\_HAS\_SECURE\_APIが無効か判定する式の修正 [\#363](https://github.com/sakura-editor/sakura/pull/363) ([berryzplus](https://github.com/berryzplus))
- ビルド関連のバッチファイルのエラー耐性を上げる [\#362](https://github.com/sakura-editor/sakura/pull/362) ([m-tmatma](https://github.com/m-tmatma))
- funccode.bat のエラー耐性を上げる [\#361](https://github.com/sakura-editor/sakura/pull/361) ([m-tmatma](https://github.com/m-tmatma))
- githash.bat のエラー耐性を上げる [\#360](https://github.com/sakura-editor/sakura/pull/360) ([m-tmatma](https://github.com/m-tmatma))
- インストーラの UI を英語対応にする [\#359](https://github.com/sakura-editor/sakura/pull/359) ([m-tmatma](https://github.com/m-tmatma))
- MinGWでサクラエディタをビルドできるようにメイクファイルを更新する [\#351](https://github.com/sakura-editor/sakura/pull/351) ([berryzplus](https://github.com/berryzplus))
- インストーラ関連のフォルダーを無視ファイルに追加 [\#343](https://github.com/sakura-editor/sakura/pull/343) ([m-tmatma](https://github.com/m-tmatma))
- バッチファイルのコピーで /Y と /B をつける [\#342](https://github.com/sakura-editor/sakura/pull/342) ([m-tmatma](https://github.com/m-tmatma))
- ビルドに関するドキュメントを更新、SKIP\_CREATE\_GITHASH に関する説明を移動 [\#341](https://github.com/sakura-editor/sakura/pull/341) ([m-tmatma](https://github.com/m-tmatma))
- keyword ディレクトリ以下のファイルの説明を追加 [\#340](https://github.com/sakura-editor/sakura/pull/340) ([m-tmatma](https://github.com/m-tmatma))
- markdown のスペルミス修正 [\#339](https://github.com/sakura-editor/sakura/pull/339) ([m-tmatma](https://github.com/m-tmatma))
- インストーラ用のドキュメントを更新 [\#337](https://github.com/sakura-editor/sakura/pull/337) ([m-tmatma](https://github.com/m-tmatma))
- postBuild.bat の処理内容を更新 [\#336](https://github.com/sakura-editor/sakura/pull/336) ([m-tmatma](https://github.com/m-tmatma))
- bregonig.dll のファイル内容とタイムスタンプを比較する [\#335](https://github.com/sakura-editor/sakura/pull/335) ([m-tmatma](https://github.com/m-tmatma))
- インストーラの生成先のフォルダーにプラットフォーム名を含める [\#333](https://github.com/sakura-editor/sakura/pull/333) ([m-tmatma](https://github.com/m-tmatma))
- \#329 によるデグレを修正 \(bron412.zip の bregonig.dll が使われなくなる\) [\#332](https://github.com/sakura-editor/sakura/pull/332) ([m-tmatma](https://github.com/m-tmatma))
- 大きすぎるファイル \(2GBくらいを超えるもの\) を読み込もうとしたときに正しくエラーメッセージを出す [\#330](https://github.com/sakura-editor/sakura/pull/330) ([kobake](https://github.com/kobake))
- bregonig.dll を sakura.exe と同じ場所に自動配置 [\#329](https://github.com/sakura-editor/sakura/pull/329) ([kobake](https://github.com/kobake))
- 右クリックで表示される「SAKURAで開く\(&E\)」メニューにアイコンを設定する [\#327](https://github.com/sakura-editor/sakura/pull/327) ([KENCHjp](https://github.com/KENCHjp))
- rc ファイルも .editorconfig の対象にする [\#325](https://github.com/sakura-editor/sakura/pull/325) ([m-tmatma](https://github.com/m-tmatma))
- 英語表示したときの著作権表示でプログラム本体とリソースを別に表示する [\#315](https://github.com/sakura-editor/sakura/pull/315) ([m-tmatma](https://github.com/m-tmatma))
- 変換先バッファ確保の new で例外処理を行わないように std::nothrow 指定追加 [\#287](https://github.com/sakura-editor/sakura/pull/287) ([beru](https://github.com/beru))
- markdown に TOC を追加 [\#255](https://github.com/sakura-editor/sakura/pull/255) ([m-tmatma](https://github.com/m-tmatma))
- copyright を 2018 に変更 [\#232](https://github.com/sakura-editor/sakura/pull/232) ([m-tmatma](https://github.com/m-tmatma))
- grep実行中はテキスト置換処理の進捗ダイアログ表示を抑制 [\#211](https://github.com/sakura-editor/sakura/pull/211) ([beru](https://github.com/beru))
- サポートリンク等の情報を GitHub ベースのものに変更する [\#201](https://github.com/sakura-editor/sakura/pull/201) ([m-tmatma](https://github.com/m-tmatma))
- 毎ビルドほぼすべての .cpp がリコンパイルされるのを修正 [\#193](https://github.com/sakura-editor/sakura/pull/193) ([yoshinrt](https://github.com/yoshinrt))
- \[x64\] x64 版でバージョン情報にアルファ版の表示を行う \(再作成\) [\#182](https://github.com/sakura-editor/sakura/pull/182) ([m-tmatma](https://github.com/m-tmatma))
- Installer のx64 対応 [\#180](https://github.com/sakura-editor/sakura/pull/180) ([m-tmatma](https://github.com/m-tmatma))
- \[x64対応\] master の修正を x64 ブランチにマージする [\#161](https://github.com/sakura-editor/sakura/pull/161) ([m-tmatma](https://github.com/m-tmatma))
- preBuild: gitrev.h 生成時に git と .git の存在を明示的にチェック [\#146](https://github.com/sakura-editor/sakura/pull/146) ([kobake](https://github.com/kobake))
- Windows10メニュー対応 [\#130](https://github.com/sakura-editor/sakura/pull/130) ([KENCHjp](https://github.com/KENCHjp))
- ファイル読み込みとgrepの高速化 [\#125](https://github.com/sakura-editor/sakura/pull/125) ([beru](https://github.com/beru))
- プロパティシートのコントロールの有効/無効制御を早めに実施 [\#122](https://github.com/sakura-editor/sakura/pull/122) ([beru](https://github.com/beru))
- x64 に対して master をマージ（定期的にやっていく） [\#118](https://github.com/sakura-editor/sakura/pull/118) ([kobake](https://github.com/kobake))
- 複数プロセッサによるコンパイルを x64 でも有効にする [\#117](https://github.com/sakura-editor/sakura/pull/117) ([m-tmatma](https://github.com/m-tmatma))
- \[x64対応\] master の修正を x64 ブランチにマージする [\#113](https://github.com/sakura-editor/sakura/pull/113) ([m-tmatma](https://github.com/m-tmatma))
- clone\_folder 指定を削除する [\#99](https://github.com/sakura-editor/sakura/pull/99) ([m-tmatma](https://github.com/m-tmatma))
- x64 に対して master をマージ（定期的にやっていく） [\#97](https://github.com/sakura-editor/sakura/pull/97) ([kobake](https://github.com/kobake))
- \[x64対応\] c4477 の警告を修正 [\#89](https://github.com/sakura-editor/sakura/pull/89) ([m-tmatma](https://github.com/m-tmatma))
- x64 作業集約場所：2018-06-27 をもって master に統合する [\#86](https://github.com/sakura-editor/sakura/pull/86) ([kobake](https://github.com/kobake))
- grep 処理速度改善 [\#77](https://github.com/sakura-editor/sakura/pull/77) ([beru](https://github.com/beru))
- HeaderMake出力メッセージの英語化 [\#57](https://github.com/sakura-editor/sakura/pull/57) ([kobake](https://github.com/kobake))
- MakefileMake 出力メッセージを英語に変更 [\#44](https://github.com/sakura-editor/sakura/pull/44) ([kobake](https://github.com/kobake))
- \#26: web site のリンクを変更 [\#27](https://github.com/sakura-editor/sakura/pull/27) ([m-tmatma](https://github.com/m-tmatma))
- バージョン情報で git の commit hash を表示する [\#19](https://github.com/sakura-editor/sakura/pull/19) ([m-tmatma](https://github.com/m-tmatma))
- Create README.md [\#9](https://github.com/sakura-editor/sakura/pull/9) ([kobake](https://github.com/kobake))

## [v2.3.2.0](https://github.com/sakura-editor/sakura/tree/v2.3.2.0) (2017-05-02)

[Full Changelog](https://github.com/sakura-editor/sakura/compare/v2.3.1.0...v2.3.2.0)

## [v2.3.1.0](https://github.com/sakura-editor/sakura/tree/v2.3.1.0) (2016-08-13)

[Full Changelog](https://github.com/sakura-editor/sakura/compare/v2.3.0.0...v2.3.1.0)

## [v2.3.0.0](https://github.com/sakura-editor/sakura/tree/v2.3.0.0) (2015-10-12)

[Full Changelog](https://github.com/sakura-editor/sakura/compare/v2.2.0.1...v2.3.0.0)

## [v2.2.0.1](https://github.com/sakura-editor/sakura/tree/v2.2.0.1) (2015-03-01)

[Full Changelog](https://github.com/sakura-editor/sakura/compare/v2.2.0.0...v2.2.0.1)

## [v2.2.0.0](https://github.com/sakura-editor/sakura/tree/v2.2.0.0) (2015-02-22)

[Full Changelog](https://github.com/sakura-editor/sakura/compare/c1357c0cf2b5dcee5458d2e48f2b0b3130643db9...v2.2.0.0)



\* *This Change Log was automatically generated by [github_changelog_generator](https://github.com/skywinder/Github-Changelog-Generator)*