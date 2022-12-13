# Change Log

## [Unreleased](https://github.com/sakura-editor/sakura/tree/HEAD)

[Full Changelog](https://github.com/sakura-editor/sakura/compare/v2.4.2...HEAD)

### その他変更

- コンテキストメニュー情報の定義漏れを追加 [\#1876](https://github.com/sakura-editor/sakura/pull/1876) ([takeyamajp](https://github.com/takeyamajp))
- v2.4.2リリース後作業 [\#1875](https://github.com/sakura-editor/sakura/pull/1875) ([berryzplus](https://github.com/berryzplus))

## [v2.4.2](https://github.com/sakura-editor/sakura/tree/v2.4.2) (2022-12-04)

[Full Changelog](https://github.com/sakura-editor/sakura/compare/v2.4.1...v2.4.2)

### 仕様変更

- 「指定行へジャンプ」ダイアログで入力できる桁数を増やす [\#1829](https://github.com/sakura-editor/sakura/pull/1829) ([beru](https://github.com/beru))
- 表示スケールに合わせて拡大する処理を色々と追加 [\#1762](https://github.com/sakura-editor/sakura/pull/1762) ([beru](https://github.com/beru))
- アクティブタブの上部にトップバンドを描画する [\#1684](https://github.com/sakura-editor/sakura/pull/1684) ([suconbu](https://github.com/suconbu))
- レイアウト桁位置に対するロジック桁位置の進め方を改善する [\#1658](https://github.com/sakura-editor/sakura/pull/1658) ([ghost](https://github.com/ghost))
- フォルダ選択ダイアログを SHBrowseForFolder\(\) でなく IFileDialog を使用するように変更 [\#1609](https://github.com/sakura-editor/sakura/pull/1609) ([Ocelot1210](https://github.com/Ocelot1210))
- キャレット位置の文字情報をステータスバーに設定する際の再描画をまとめる [\#1601](https://github.com/sakura-editor/sakura/pull/1601) ([beru](https://github.com/beru))
- テストが異常終了する対策 [\#1560](https://github.com/sakura-editor/sakura/pull/1560) ([berryzplus](https://github.com/berryzplus))
- 設定値読み込みで発生した変換エラーを検出できるようにする [\#1548](https://github.com/sakura-editor/sakura/pull/1548) ([sanomari](https://github.com/sanomari))
- マウスホイールでのフォントサイズ変更をパーセンテージに基づいて行うようにする [\#1513](https://github.com/sakura-editor/sakura/pull/1513) ([suconbu](https://github.com/suconbu))
- GetDateTimeFormatの入出力をWCHAR\*からstd::wstring\(\_view\)に変更する [\#1489](https://github.com/sakura-editor/sakura/pull/1489) ([suconbu](https://github.com/suconbu))
- ファイル保存時の既定のファイル名を日時にする [\#1474](https://github.com/sakura-editor/sakura/pull/1474) ([suconbu](https://github.com/suconbu))
- （\#1457 補遺）カスタムメニュー画面でも追加・挿入後に機能リストを送る処理を入れたい [\#1459](https://github.com/sakura-editor/sakura/pull/1459) ([kengoide](https://github.com/kengoide))
- \[要望\]ツールバー編集画面においてボタン挿入・追加時に機能リストのフォーカスを1つ下にずらしてほしい [\#1457](https://github.com/sakura-editor/sakura/pull/1457) ([kengoide](https://github.com/kengoide))
- XML文書にタイプ別設定で選択した文字コードを使えるようにする [\#1428](https://github.com/sakura-editor/sakura/pull/1428) ([ghost](https://github.com/ghost))
- プロパティシートに対してシステムフォントを設定 [\#1425](https://github.com/sakura-editor/sakura/pull/1425) ([suconbu](https://github.com/suconbu))
- GREPダイアログ内の全コンボボックスに対してフォント設定で指定されたフォントを使用する [\#1400](https://github.com/sakura-editor/sakura/pull/1400) ([suconbu](https://github.com/suconbu))
- キーワードヘルプの説明文に"\n"を表示できるようにしたい。 [\#1399](https://github.com/sakura-editor/sakura/pull/1399) ([berryzplus](https://github.com/berryzplus))

### 機能追加

- uchardet.dll が存在したらそれを使って文字エンコーディングの検出が行われるように処理追加 [\#1726](https://github.com/sakura-editor/sakura/pull/1726) ([beru](https://github.com/beru))
- Cプリプロセッサの判定を強化 [\#1644](https://github.com/sakura-editor/sakura/pull/1644) ([beru](https://github.com/beru))
- デザインテンプレートを活用できるように改良する [\#1570](https://github.com/sakura-editor/sakura/pull/1570) ([berryzplus](https://github.com/berryzplus))
- ステータスバーにフォントサイズの倍率を表示する [\#1490](https://github.com/sakura-editor/sakura/pull/1490) ([suconbu](https://github.com/suconbu))
- コンボボックスに Ctrl+Backspace による単語削除機能を追加する [\#1470](https://github.com/sakura-editor/sakura/pull/1470) ([kengoide](https://github.com/kengoide))
- Installer file update \(ChineseTranslation\) [\#1469](https://github.com/sakura-editor/sakura/pull/1469) ([EldersJavas](https://github.com/EldersJavas))
- Grepダイアログ部品配置の改善 [\#1431](https://github.com/sakura-editor/sakura/pull/1431) ([suconbu](https://github.com/suconbu))
- ダイアログフォント美化 [\#1421](https://github.com/sakura-editor/sakura/pull/1421) ([suconbu](https://github.com/suconbu))
- アウトライン解析の選択行追従 [\#1398](https://github.com/sakura-editor/sakura/pull/1398) ([suconbu](https://github.com/suconbu))
- 画面先頭行を取得するマクロ関数 GetViewTop を追加 [\#1393](https://github.com/sakura-editor/sakura/pull/1393) ([beru](https://github.com/beru))

### バグ修正

- 無題シーケンス番号の保存に失敗する不具合の暫定対策 [\#1850](https://github.com/sakura-editor/sakura/pull/1850) ([sanomari](https://github.com/sanomari))
- トリプルクリック時に次の行の先頭にURLがあるとそれが選択されてしまう現象が起きないように対策 [\#1753](https://github.com/sakura-editor/sakura/pull/1753) ([beru](https://github.com/beru))
- 補完候補一覧ダイアログの不具合修正 [\#1750](https://github.com/sakura-editor/sakura/pull/1750) ([ghost](https://github.com/ghost))
- CNativeW::AllocStringBufferの失敗時にコピーを中断させる [\#1732](https://github.com/sakura-editor/sakura/pull/1732) ([berryzplus](https://github.com/berryzplus))
- プログレスバーの表示切り替え方法を変更して切り替えが正しく反映されるようにする [\#1727](https://github.com/sakura-editor/sakura/pull/1727) ([ghost](https://github.com/ghost))
- ツールバーの検索ボックスにフォーカスを当てていないにも関わらず文字列が選択状態として表示される現象が起きないように対策 [\#1724](https://github.com/sakura-editor/sakura/pull/1724) ([beru](https://github.com/beru))
- 外部コマンド実行でキャンセルした時のメッセージが異なる問題を修正 [\#1721](https://github.com/sakura-editor/sakura/pull/1721) ([beru](https://github.com/beru))
- Grep 置換ダイアログを表示すると，置換前 と 置換後の両方が選択状態になる現象が発生しないように修正 [\#1719](https://github.com/sakura-editor/sakura/pull/1719) ([beru](https://github.com/beru))
- ダイアログの位置が表示するたびにずれていく問題を修正 [\#1711](https://github.com/sakura-editor/sakura/pull/1711) ([beru](https://github.com/beru))
- ブラウザでURLを開く機能の改善 [\#1708](https://github.com/sakura-editor/sakura/pull/1708) ([sanomari](https://github.com/sanomari))
- 文字列をすべて置換したときに表示されるメッセージがおかしいのを対策する [\#1703](https://github.com/sakura-editor/sakura/pull/1703) ([berryzplus](https://github.com/berryzplus))
- マウスドラッグでのスクロール時に表示がおかしくなる問題の修正 [\#1699](https://github.com/sakura-editor/sakura/pull/1699) ([suconbu](https://github.com/suconbu))
- PatchUnicode-1051 現在のドキュメント内容をGrep\(2\) [\#1696](https://github.com/sakura-editor/sakura/pull/1696) ([dep5](https://github.com/dep5))
- 印刷ページ設定ダイアログのフォントが巨大化する問題を修正 [\#1682](https://github.com/sakura-editor/sakura/pull/1682) ([suconbu](https://github.com/suconbu))
- SonarScanで検出されたStaticVectorの範囲外アクセスを修正する  [\#1679](https://github.com/sakura-editor/sakura/pull/1679) ([berryzplus](https://github.com/berryzplus))
- 印刷ページ設定ダイアログの文字切れ等の修正 [\#1674](https://github.com/sakura-editor/sakura/pull/1674) ([suconbu](https://github.com/suconbu))
- タブ幅が半角単位8桁を超える場合でも下線が最後まで描画されるようにする [\#1673](https://github.com/sakura-editor/sakura/pull/1673) ([ghost](https://github.com/ghost))
- Grep/Grep置換ダイアログの余白調整/文言統一 [\#1672](https://github.com/sakura-editor/sakura/pull/1672) ([suconbu](https://github.com/suconbu))
- 先頭行でPageUp・末尾行でPageDownした後のカーソル移動で下線が残る不具合に対処 [\#1652](https://github.com/sakura-editor/sakura/pull/1652) ([beru](https://github.com/beru))
- キャレット位置の文字情報をステータスバーに設定する際の再描画をまとめた変更で UpdateWindow の呼び出しが必要なのに漏れていたのを修正 [\#1642](https://github.com/sakura-editor/sakura/pull/1642) ([beru](https://github.com/beru))
- CMemory::AppendRawData\(\)でAllocBuffer条件ミスを修正 [\#1638](https://github.com/sakura-editor/sakura/pull/1638) ([usagisita](https://github.com/usagisita))
- CMemory変更のデグレ対策 [\#1629](https://github.com/sakura-editor/sakura/pull/1629) ([berryzplus](https://github.com/berryzplus))
- UTF32BEの文字コード変換クラスが7bit ASCIIを取り込めない問題に対処する [\#1627](https://github.com/sakura-editor/sakura/pull/1627) ([berryzplus](https://github.com/berryzplus))
- タイプ別設定の「カーソル位置縦線」を有効時に、カーソルが先頭行にある状態で PageUp キーを押した後にカーソル左右移動するとカーソル位置縦線が残る問題を修正 [\#1619](https://github.com/sakura-editor/sakura/pull/1619) ([beru](https://github.com/beru))
- タイプ別設定のインポートでバージョンチェックが正しく働いていなかった [\#1613](https://github.com/sakura-editor/sakura/pull/1613) ([usagisita](https://github.com/usagisita))
- C++アウトラインでclass x final:base{}と:がfinalにくっついているとクラス名がfinalになる不具合の修正 [\#1612](https://github.com/sakura-editor/sakura/pull/1612) ([usagisita](https://github.com/usagisita))
- IsFileExists\(\)とIsDirectory\(\)でワイルドカードが通過する不具合を修正する [\#1608](https://github.com/sakura-editor/sakura/pull/1608) ([usagisita](https://github.com/usagisita))
- MinGW版CRTでwcsncat\_sのC++オーバーロードが未定義になっていることに対策する [\#1604](https://github.com/sakura-editor/sakura/pull/1604) ([berryzplus](https://github.com/berryzplus))
- Revert "アウトライン解析ダイアログボックスをドッキング時にスプリッターをドラッグしてリサイズするとウィンドウの描画がちらつく問題を解消" [\#1603](https://github.com/sakura-editor/sakura/pull/1603) ([beru](https://github.com/beru))
- ファイルダイアログとフォルダ選択ダイアログでのバッファオーバーランを修正 [\#1599](https://github.com/sakura-editor/sakura/pull/1599) ([usagisita](https://github.com/usagisita))
- バックアップで詳細$0～$9指定を使いMAX\_PATH近いパスを保存しようとすると落ちる不具合を修正 [\#1596](https://github.com/sakura-editor/sakura/pull/1596) ([usagisita](https://github.com/usagisita))
- SonarCloudにBugだと言われているGetExistPathWの範囲外アクセスを修正する [\#1594](https://github.com/sakura-editor/sakura/pull/1594) ([berryzplus](https://github.com/berryzplus))
- CDlgInput1で文字列長はNUL終端を含まない長さを指定するはずが1文字多いところがあるのを修正 [\#1589](https://github.com/sakura-editor/sakura/pull/1589) ([usagisita](https://github.com/usagisita))
- Grepダイアログのフォルダ名設定処理でのバッファオーバーラン修正 [\#1588](https://github.com/sakura-editor/sakura/pull/1588) ([usagisita](https://github.com/usagisita))
- SonarCloudで検出されたCDlgExec::SetDataのコーディングミスを修正する [\#1586](https://github.com/sakura-editor/sakura/pull/1586) ([berryzplus](https://github.com/berryzplus))
- 外部コマンド実行ダイアログのコマンド文字列バッファ長が1文字大きい不具合の対応 [\#1584](https://github.com/sakura-editor/sakura/pull/1584) ([usagisita](https://github.com/usagisita))
- このファイルのパス名とカーソル位置をコピーでのバッファオーバーランの不具合対応 [\#1583](https://github.com/sakura-editor/sakura/pull/1583) ([usagisita](https://github.com/usagisita))
- SonarScanで検出されたCNativeW::GetCharPrevのメモリオーバーフローを修正する（その２） [\#1579](https://github.com/sakura-editor/sakura/pull/1579) ([berryzplus](https://github.com/berryzplus))
- JIS\<=\>SJIS 変換の際、常に日本語ロケールを使用する [\#1578](https://github.com/sakura-editor/sakura/pull/1578) ([k-takata](https://github.com/k-takata))
- プロポーショナル版で禁則処理を使えるようにする [\#1543](https://github.com/sakura-editor/sakura/pull/1543) ([ghost](https://github.com/ghost))
- sakura.iniがない状態で起動した時にマウスホイールでフォントサイズが変更できない問題を修正 [\#1536](https://github.com/sakura-editor/sakura/pull/1536) ([suconbu](https://github.com/suconbu))
- Wnd\_GetText が一文字取りこぼす問題に対処 [\#1528](https://github.com/sakura-editor/sakura/pull/1528) ([kengoide](https://github.com/kengoide))
- SonarCloud解析で検出されたゼロ除算コードに対策する [\#1511](https://github.com/sakura-editor/sakura/pull/1511) ([berryzplus](https://github.com/berryzplus))
- コンボボックスのサブクラス化に関連するクラッシュバグの修正 [\#1481](https://github.com/sakura-editor/sakura/pull/1481) ([kengoide](https://github.com/kengoide))
- 「ファイル名の簡易表示」設定における更新ボタンの不具合修正 [\#1479](https://github.com/sakura-editor/sakura/pull/1479) ([ghost](https://github.com/ghost))
- サクラエディタのコマンドライン引数に超長い文字列を指定するとクラッシュする問題に対処する。\(Take2\) [\#1453](https://github.com/sakura-editor/sakura/pull/1453) ([berryzplus](https://github.com/berryzplus))
- Grep置換ダイアログに誤追加された「否該当行」を削除 [\#1448](https://github.com/sakura-editor/sakura/pull/1448) ([suconbu](https://github.com/suconbu))
- カスタムメニューの追加時に配列の範囲外を書き換えないようにする [\#1445](https://github.com/sakura-editor/sakura/pull/1445) ([ghost](https://github.com/ghost))
- デバッグ実行時に外部コマンド実行ダイアログを開くとwarning\_point関数で止まる問題を修正する [\#1440](https://github.com/sakura-editor/sakura/pull/1440) ([suconbu](https://github.com/suconbu))
- XML宣言を用いた文字コード判別処理を強化する [\#1422](https://github.com/sakura-editor/sakura/pull/1422) ([ghost](https://github.com/ghost))
- DIFF差分とウィンドウ一覧ダイアログでファイル名がMAX\_PATHぎりぎりだと落ちてしまう不具合の修正 [\#1413](https://github.com/sakura-editor/sakura/pull/1413) ([usagisita](https://github.com/usagisita))
- CImageListMgr::Create において読み込んだビットマップの形式を 32bit に変換する処理を追加 [\#1396](https://github.com/sakura-editor/sakura/pull/1396) ([beru](https://github.com/beru))
- 空行描画のオプションが誤っているのを修正する。 [\#1358](https://github.com/sakura-editor/sakura/pull/1358) ([berryzplus](https://github.com/berryzplus))
- 検索ボックスの表示位置を調整する [\#1345](https://github.com/sakura-editor/sakura/pull/1345) ([berryzplus](https://github.com/berryzplus))
- CSelectLangクラスの初期化が必ず行われるようにする [\#1341](https://github.com/sakura-editor/sakura/pull/1341) ([berryzplus](https://github.com/berryzplus))
- ツールバーの折り返しが機能しなくなる不具合を引き起こした変更をrevert [\#1314](https://github.com/sakura-editor/sakura/pull/1314) ([beru](https://github.com/beru))



### その他変更

- バージョンがv2.4.2.0になる件の対策 [\#1874](https://github.com/sakura-editor/sakura/pull/1874) ([berryzplus](https://github.com/berryzplus))
- マルチユーザー設定ファイルの不具合修正 [\#1865](https://github.com/sakura-editor/sakura/pull/1865) ([berryzplus](https://github.com/berryzplus))
- マルチユーザー設定ファイル\(sakura.exe.ini\)のテストケース修正 [\#1864](https://github.com/sakura-editor/sakura/pull/1864) ([berryzplus](https://github.com/berryzplus))
- CMakeLists.txtのコメントタイポ修正 [\#1862](https://github.com/sakura-editor/sakura/pull/1862) ([eltociear](https://github.com/eltociear))
- Code modernize C++17, refactor and minor optimize [\#1860](https://github.com/sakura-editor/sakura/pull/1860) ([GermanAizek](https://github.com/GermanAizek))
- resolve unavailable Oniguruma URL [\#1859](https://github.com/sakura-editor/sakura/pull/1859) ([dAu6jARL](https://github.com/dAu6jARL))
- Windows SDKのマクロ定数「NULL」を再定義する [\#1858](https://github.com/sakura-editor/sakura/pull/1858) ([sanomari](https://github.com/sanomari))
- CDllImp::DeinitDllをnoexceptとマークする [\#1857](https://github.com/sakura-editor/sakura/pull/1857) ([sanomari](https://github.com/sanomari))
- SonarQubeクライアントのバージョンを更新する [\#1856](https://github.com/sakura-editor/sakura/pull/1856) ([berryzplus](https://github.com/berryzplus))
- インクルード ShellAPI.h を shellapi.h に訂正 [\#1855](https://github.com/sakura-editor/sakura/pull/1855) ([sanomari](https://github.com/sanomari))
- fix 'Do not throw uncaught exceptions in a destructor.' [\#1854](https://github.com/sakura-editor/sakura/pull/1854) ([sanomari](https://github.com/sanomari))
- マウスドラッグ時のスクロール速度を制限する [\#1853](https://github.com/sakura-editor/sakura/pull/1853) ([dep5](https://github.com/dep5))
- GREPの「カレントフォルダーが初期値」に対応 [\#1852](https://github.com/sakura-editor/sakura/pull/1852) ([dep5](https://github.com/dep5))
- Visual Studio 2022でのCMakeの警告に対応 [\#1848](https://github.com/sakura-editor/sakura/pull/1848) ([dep5](https://github.com/dep5))
- ソースコード中の誤字訂正\(Pouse\) [\#1847](https://github.com/sakura-editor/sakura/pull/1847) ([dep5](https://github.com/dep5))
- 「割付」を「割当」に変更 [\#1846](https://github.com/sakura-editor/sakura/pull/1846) ([dep5](https://github.com/dep5))
- CClipboard の単体テストをさらに追加する [\#1843](https://github.com/sakura-editor/sakura/pull/1843) ([kengoide](https://github.com/kengoide))
- StdAfx.h に定義されている malloc と GlobalLock の型変換マクロを削除する [\#1842](https://github.com/sakura-editor/sakura/pull/1842) ([kengoide](https://github.com/kengoide))
- 「フォルダー」のカタカナ表記をOSに合わせる [\#1841](https://github.com/sakura-editor/sakura/pull/1841) ([sanomari](https://github.com/sanomari))
- 「プリンター」のカタカナ表記をOSに合わせる [\#1840](https://github.com/sakura-editor/sakura/pull/1840) ([sanomari](https://github.com/sanomari))
- 「ユーザー」のカタカナ表記を統一する [\#1839](https://github.com/sakura-editor/sakura/pull/1839) ([sanomari](https://github.com/sanomari))
- 「ヘッダー」または「フッター」のカタカナ表記を統一する [\#1838](https://github.com/sakura-editor/sakura/pull/1838) ([sanomari](https://github.com/sanomari))
- 「インターフェース」のカタカナ表記を統一する [\#1837](https://github.com/sakura-editor/sakura/pull/1837) ([sanomari](https://github.com/sanomari))
- 「インタープリタ」のカタカナ表記を統一する [\#1836](https://github.com/sakura-editor/sakura/pull/1836) ([sanomari](https://github.com/sanomari))
- 「マウスキャプチャー」の長音記号を削る [\#1835](https://github.com/sakura-editor/sakura/pull/1835) ([sanomari](https://github.com/sanomari))
- memory leakの誤記を修正する [\#1834](https://github.com/sakura-editor/sakura/pull/1834) ([sanomari](https://github.com/sanomari))
- トレイアイコンから「履歴とお気に入りの管理」ダイアログを開けるようにする [\#1833](https://github.com/sakura-editor/sakura/pull/1833) ([berryzplus](https://github.com/berryzplus))
- \_com\_raise\_errorの独自実装を除去してMinGWビルドのエラーを解消します [\#1831](https://github.com/sakura-editor/sakura/pull/1831) ([sanomari](https://github.com/sanomari))
- Revert change at \#1512 partially [\#1830](https://github.com/sakura-editor/sakura/pull/1830) ([sanomari](https://github.com/sanomari))
- DLL読み込み失敗のテストを追加する [\#1819](https://github.com/sakura-editor/sakura/pull/1819) ([berryzplus](https://github.com/berryzplus))
- MinGWビルドバッチの出力先フォルダ指定を訂正する [\#1818](https://github.com/sakura-editor/sakura/pull/1818) ([berryzplus](https://github.com/berryzplus))
- CPPAクラスの自動テストを追加する [\#1817](https://github.com/sakura-editor/sakura/pull/1817) ([berryzplus](https://github.com/berryzplus))
- SonarCloud静的解析でスタブDLLのソースを除外する [\#1816](https://github.com/sakura-editor/sakura/pull/1816) ([berryzplus](https://github.com/berryzplus))
- メッセージボックス関数の単体テスト向け独自拡張の不具合を修正したい [\#1813](https://github.com/sakura-editor/sakura/pull/1813) ([berryzplus](https://github.com/berryzplus))
- strprintfを改良してナロー文字も扱えるようにする [\#1810](https://github.com/sakura-editor/sakura/pull/1810) ([berryzplus](https://github.com/berryzplus))
- 複数のVisual Studioを入れたときの問題に対応 [\#1806](https://github.com/sakura-editor/sakura/pull/1806) ([dep5](https://github.com/dep5))
- ToolBarImageSplitterを手軽に実行できるようにする [\#1805](https://github.com/sakura-editor/sakura/pull/1805) ([berryzplus](https://github.com/berryzplus))
- 「このファイルのフォルダ名をコピー」の機能のアイコンを実装する [\#1803](https://github.com/sakura-editor/sakura/pull/1803) ([berryzplus](https://github.com/berryzplus))
- CShareDataのテストを追加する [\#1801](https://github.com/sakura-editor/sakura/pull/1801) ([berryzplus](https://github.com/berryzplus))
- CClipboard のテストにモックを導入する [\#1800](https://github.com/sakura-editor/sakura/pull/1800) ([kengoide](https://github.com/kengoide))
- SonarQubeの静的解析で検出されたCFontAutoDeleterのバグを修正する [\#1799](https://github.com/sakura-editor/sakura/pull/1799) ([berryzplus](https://github.com/berryzplus))
- tests1.exe の依存ライブラリに gmock.lib を追加する [\#1798](https://github.com/sakura-editor/sakura/pull/1798) ([kengoide](https://github.com/kengoide))
- SonarQubeの静的解析で検出されたFileNameSepExtのバグを修正する（その２） [\#1797](https://github.com/sakura-editor/sakura/pull/1797) ([berryzplus](https://github.com/berryzplus))
- CEditViewの誤記を訂正する [\#1794](https://github.com/sakura-editor/sakura/pull/1794) ([berryzplus](https://github.com/berryzplus))
- アーカイブ展開を必要な場合のみに限定する（バラコミットver） [\#1793](https://github.com/sakura-editor/sakura/pull/1793) ([berryzplus](https://github.com/berryzplus))
- Azure Pipelinesの除外条件を更新する [\#1791](https://github.com/sakura-editor/sakura/pull/1791) ([ghost](https://github.com/ghost))
- ワークフローを手動で実行できるようにする [\#1789](https://github.com/sakura-editor/sakura/pull/1789) ([ghost](https://github.com/ghost))
- ブランチ名に関わらずAZP/GHAが実行されるようにする [\#1788](https://github.com/sakura-editor/sakura/pull/1788) ([ghost](https://github.com/ghost))
- コピーライト表記の改定年を2022年に更新する [\#1786](https://github.com/sakura-editor/sakura/pull/1786) ([ghost](https://github.com/ghost))
- MSBuildの探索手順を変更する [\#1785](https://github.com/sakura-editor/sakura/pull/1785) ([ghost](https://github.com/ghost))
- GoogleTestを更新する [\#1779](https://github.com/sakura-editor/sakura/pull/1779) ([ghost](https://github.com/ghost))
- プロジェクト設定ファイル内のLinkタスクの記述位置を移動する [\#1778](https://github.com/sakura-editor/sakura/pull/1778) ([sanomari](https://github.com/sanomari))
- CIでVS2022を利用したビルドを行うようにする [\#1777](https://github.com/sakura-editor/sakura/pull/1777) ([ghost](https://github.com/ghost))
- プロジェクト設定内の重複した記述をまとめたい [\#1775](https://github.com/sakura-editor/sakura/pull/1775) ([berryzplus](https://github.com/berryzplus))
- ExtractCtagsのWin32ビルドの依存ターゲット指定をx64ビルドと合わせる [\#1774](https://github.com/sakura-editor/sakura/pull/1774) ([berryzplus](https://github.com/berryzplus))
- Azure PipelinesにおけるWindows Server 2016環境の利用を廃止する [\#1773](https://github.com/sakura-editor/sakura/pull/1773) ([ghost](https://github.com/ghost))
- MinGWビルドで使用するビルドツールを更新する [\#1771](https://github.com/sakura-editor/sakura/pull/1771) ([ghost](https://github.com/ghost))
- find-tools.batでNUM\_VSVERSIONのログ出力を抑制していたのを復活させる（その2） [\#1770](https://github.com/sakura-editor/sakura/pull/1770) ([berryzplus](https://github.com/berryzplus))
- コンパイルチェックで利用するPythonバージョンを変更する [\#1766](https://github.com/sakura-editor/sakura/pull/1766) ([ghost](https://github.com/ghost))
- Azure Pipelinesの不要な警告メッセージを削減する [\#1765](https://github.com/sakura-editor/sakura/pull/1765) ([ghost](https://github.com/ghost))
- Visual Studio 2022を使用したローカルビルドに対応する [\#1764](https://github.com/sakura-editor/sakura/pull/1764) ([ghost](https://github.com/ghost))
- Checkout batch files with CRLF [\#1761](https://github.com/sakura-editor/sakura/pull/1761) ([k-takata](https://github.com/k-takata))
- マルチvs構成向けの環境変数を削除する [\#1760](https://github.com/sakura-editor/sakura/pull/1760) ([berryzplus](https://github.com/berryzplus))
- 不要なバッチ変数を削除する [\#1758](https://github.com/sakura-editor/sakura/pull/1758) ([berryzplus](https://github.com/berryzplus))
- .NET Frameworkのターゲットを変更する [\#1756](https://github.com/sakura-editor/sakura/pull/1756) ([ghost](https://github.com/ghost))
- BSキーでカーソル前を削除時に画面が余計に再描画される事を防ぐ [\#1754](https://github.com/sakura-editor/sakura/pull/1754) ([beru](https://github.com/beru))
- GetEditWndの指摘修正 [\#1747](https://github.com/sakura-editor/sakura/pull/1747) ([sanomari](https://github.com/sanomari))
- sakura.vcxprojのビルド設定を「手作り仕様」に書き替える [\#1730](https://github.com/sakura-editor/sakura/pull/1730) ([sanomari](https://github.com/sanomari))
- windows10SDKをvs2019向けに更新する [\#1729](https://github.com/sakura-editor/sakura/pull/1729) ([sanomari](https://github.com/sanomari))
- CClipboard::SetText と CClipboard::GetText のテストを追加する [\#1707](https://github.com/sakura-editor/sakura/pull/1707) ([kengoide](https://github.com/kengoide))
- README.mdの誤字を修正 "Visaul -\> Visual" [\#1693](https://github.com/sakura-editor/sakura/pull/1693) ([toduq](https://github.com/toduq))
- .gitignoreにビルド構成の変更を反映する [\#1676](https://github.com/sakura-editor/sakura/pull/1676) ([berryzplus](https://github.com/berryzplus))
- UTF-8で登録されていたpowershellスクリプトを登録し直す [\#1675](https://github.com/sakura-editor/sakura/pull/1675) ([berryzplus](https://github.com/berryzplus))
- アウトライン解析を表示して閉じる操作がテストで実行されるようにする [\#1671](https://github.com/sakura-editor/sakura/pull/1671) ([berryzplus](https://github.com/berryzplus))
- tests1.exeにzipリソースを添付する [\#1670](https://github.com/sakura-editor/sakura/pull/1670) ([berryzplus](https://github.com/berryzplus))
- SonarScanにモジュールを検索を導入する [\#1668](https://github.com/sakura-editor/sakura/pull/1668) ([berryzplus](https://github.com/berryzplus))
- 英語DLLがMinGWビルドできなくなっていたのを修正 [\#1667](https://github.com/sakura-editor/sakura/pull/1667) ([berryzplus](https://github.com/berryzplus))
- MSVCプロジェクトにbregonigとctagsのzip展開を組み込む [\#1666](https://github.com/sakura-editor/sakura/pull/1666) ([berryzplus](https://github.com/berryzplus))
- MinGW Makefileにもbregonig.dllとctags.exeのzip展開を組み込む [\#1663](https://github.com/sakura-editor/sakura/pull/1663) ([berryzplus](https://github.com/berryzplus))
- MinGWビルドでリソースのout-of-sourceビルドができなくなっていたのを修正する [\#1662](https://github.com/sakura-editor/sakura/pull/1662) ([berryzplus](https://github.com/berryzplus))
- CSearchAgent::ReplaceData のテストを追加する [\#1661](https://github.com/sakura-editor/sakura/pull/1661) ([kengoide](https://github.com/kengoide))
- CDocLineMgr のテストを追加する [\#1657](https://github.com/sakura-editor/sakura/pull/1657) ([kengoide](https://github.com/kengoide))
- find-tools.bat のドキュメントを修正する [\#1656](https://github.com/sakura-editor/sakura/pull/1656) ([ghost](https://github.com/ghost))
- CDocLine のテストを追加する [\#1655](https://github.com/sakura-editor/sakura/pull/1655) ([kengoide](https://github.com/kengoide))
- 単体テストに関するドキュメントをサブディレクトリに移動する [\#1654](https://github.com/sakura-editor/sakura/pull/1654) ([ghost](https://github.com/ghost))
- CRunningTimerの使い勝手を改善 [\#1653](https://github.com/sakura-editor/sakura/pull/1653) ([suconbu](https://github.com/suconbu))
- アウトライン解析ウィンドウのツリービューの設定処理の高速化 [\#1648](https://github.com/sakura-editor/sakura/pull/1648) ([beru](https://github.com/beru))
- プロジェクトファイルに含まれていないソースファイルを追加する [\#1646](https://github.com/sakura-editor/sakura/pull/1646) ([ghost](https://github.com/ghost))
- CMemory::AppendRawDataのテストを追加 [\#1643](https://github.com/sakura-editor/sakura/pull/1643) ([beru](https://github.com/beru))
- Latin1の変換テストを増強する [\#1633](https://github.com/sakura-editor/sakura/pull/1633) ([berryzplus](https://github.com/berryzplus))
- CI/CDに関係するドキュメントを更新・整理する [\#1630](https://github.com/sakura-editor/sakura/pull/1630) ([ghost](https://github.com/ghost))
- .vsconfigファイルを更新する [\#1623](https://github.com/sakura-editor/sakura/pull/1623) ([ghost](https://github.com/ghost))
- 最新のWindows SDKでビルドできるようにする [\#1621](https://github.com/sakura-editor/sakura/pull/1621) ([dep5](https://github.com/dep5))
- メモリバッファクラスをx64対応できるように書き替える [\#1618](https://github.com/sakura-editor/sakura/pull/1618) ([berryzplus](https://github.com/berryzplus))
- メモリDCを利用しない場合はアンダーライン描画を行描画の直後に行う事でちらつきを抑える [\#1616](https://github.com/sakura-editor/sakura/pull/1616) ([beru](https://github.com/beru))
- CCodeBaseの変換テストを追加する [\#1614](https://github.com/sakura-editor/sakura/pull/1614) ([berryzplus](https://github.com/berryzplus))
- 単体テストコードの記述ミスを訂正する [\#1611](https://github.com/sakura-editor/sakura/pull/1611) ([berryzplus](https://github.com/berryzplus))
- ヘルプのJScriptサンプルのInsTextに\(\)がなかったのを修正 [\#1598](https://github.com/sakura-editor/sakura/pull/1598) ([usagisita](https://github.com/usagisita))
- TABバーのリソース文字列の英語dll対応漏れを修正する [\#1597](https://github.com/sakura-editor/sakura/pull/1597) ([usagisita](https://github.com/usagisita))
- アウトライン解析ダイアログボックスをドッキング時にスプリッターをドラッグしてリサイズするとウィンドウの描画がちらつく問題を解消 [\#1592](https://github.com/sakura-editor/sakura/pull/1592) ([beru](https://github.com/beru))
- CCodeBase::MIMEHeaderDecode のテストを追加 [\#1591](https://github.com/sakura-editor/sakura/pull/1591) ([kengoide](https://github.com/kengoide))
- WCODE::IsHankaku が文字幅キャッシュを参照しない問題を修正する [\#1590](https://github.com/sakura-editor/sakura/pull/1590) ([kengoide](https://github.com/kengoide))
- CDecode のテストを追加する [\#1580](https://github.com/sakura-editor/sakura/pull/1580) ([kengoide](https://github.com/kengoide))
- CConvert のテストを追加する [\#1577](https://github.com/sakura-editor/sakura/pull/1577) ([kengoide](https://github.com/kengoide))
- Copyright を 2021 にする [\#1549](https://github.com/sakura-editor/sakura/pull/1549) ([berryzplus](https://github.com/berryzplus))
- フォントサイズ相対指定の仕様変更をSetFontSizeの説明に反映する [\#1544](https://github.com/sakura-editor/sakura/pull/1544) ([suconbu](https://github.com/suconbu))
- HTMLヘルプの設定ファイルの文字コードを変換する [\#1539](https://github.com/sakura-editor/sakura/pull/1539) ([berryzplus](https://github.com/berryzplus))
- WinMainTestを改良して効率的なテストを実現する [\#1538](https://github.com/sakura-editor/sakura/pull/1538) ([berryzplus](https://github.com/berryzplus))
- COMエラー情報クラスを追加する [\#1533](https://github.com/sakura-editor/sakura/pull/1533) ([sanomari](https://github.com/sanomari))
- GetInidirOrExedirの振る舞いを元に戻す [\#1527](https://github.com/sakura-editor/sakura/pull/1527) ([sanomari](https://github.com/sanomari))
- stdafx.h から charcode.h と codechecker.h を除去する [\#1526](https://github.com/sakura-editor/sakura/pull/1526) ([kengoide](https://github.com/kengoide))
- 独自関数strprintfをC++風に使えるように拡張します。 [\#1522](https://github.com/sakura-editor/sakura/pull/1522) ([sanomari](https://github.com/sanomari))
- charset/charcode.\(h|cpp\)に対するテストを拡充する [\#1520](https://github.com/sakura-editor/sakura/pull/1520) ([kengoide](https://github.com/kengoide))
- SonarCloud解析の対象をReleaseからDebugに変更する [\#1515](https://github.com/sakura-editor/sakura/pull/1515) ([berryzplus](https://github.com/berryzplus))
- 文字幅キャッシュのテストを追加する [\#1514](https://github.com/sakura-editor/sakura/pull/1514) ([kengoide](https://github.com/kengoide))
- テストカバレッジを少し改善する [\#1508](https://github.com/sakura-editor/sakura/pull/1508) ([berryzplus](https://github.com/berryzplus))
- HTMLヘルプをSonarCloudの解析対象に含めたことにより増えた警告に対処する [\#1505](https://github.com/sakura-editor/sakura/pull/1505) ([berryzplus](https://github.com/berryzplus))
- Azure Pipelinesの除外パス指定が機能していなかったのを修正する [\#1500](https://github.com/sakura-editor/sakura/pull/1500) ([sanomari](https://github.com/sanomari))
- Azure PipelinesのPRビルドの除外パスが漏れていたのを追加する [\#1498](https://github.com/sakura-editor/sakura/pull/1498) ([sanomari](https://github.com/sanomari))
- CWordParseのテストを追加する [\#1488](https://github.com/sakura-editor/sakura/pull/1488) ([kengoide](https://github.com/kengoide))
- \(GHA\) 使用しているアクションをバージョンアップする [\#1467](https://github.com/sakura-editor/sakura/pull/1467) ([ghost](https://github.com/ghost))
- \#1449を訂正してADSを使えるにする [\#1455](https://github.com/sakura-editor/sakura/pull/1455) ([sanomari](https://github.com/sanomari))
- CodeFactorで検出されたps1の問題を修正するためlistzip.ps1を書き直す [\#1447](https://github.com/sakura-editor/sakura/pull/1447) ([berryzplus](https://github.com/berryzplus))
- googletestとcompiletestsを2回目のビルドに対応させる [\#1436](https://github.com/sakura-editor/sakura/pull/1436) ([sanomari](https://github.com/sanomari))
- プロパティシートのコントロール表示のちらつき防止 [\#1424](https://github.com/sakura-editor/sakura/pull/1424) ([beru](https://github.com/beru))
- テキスト幅計算に使用する文字間隔配列のコンテナを使いまわす事で負荷を削減 [\#1415](https://github.com/sakura-editor/sakura/pull/1415) ([beru](https://github.com/beru))
- CGraphics 関連の最適化 [\#1411](https://github.com/sakura-editor/sakura/pull/1411) ([beru](https://github.com/beru))
- 一度にExtTextOutで描画しての高速化 [\#1405](https://github.com/sakura-editor/sakura/pull/1405) ([usagisita](https://github.com/usagisita))
- サクラエディタの起動テストを追加する [\#1385](https://github.com/sakura-editor/sakura/pull/1385) ([berryzplus](https://github.com/berryzplus))
- MinGW: Fix SEGV [\#1382](https://github.com/sakura-editor/sakura/pull/1382) ([k-takata](https://github.com/k-takata))
- 英語リソースに英語の言語IDを使う [\#1376](https://github.com/sakura-editor/sakura/pull/1376) ([k-takata](https://github.com/k-takata))
- VS2019 16.7.1 でのコンパイルエラーを修正 \(std::back\_inserter が iterator で定義されているのでインクルード\) [\#1374](https://github.com/sakura-editor/sakura/pull/1374) ([m-tmatma](https://github.com/m-tmatma))
- MinGW: C++ の未定義動作によるクラッシュを修正する [\#1372](https://github.com/sakura-editor/sakura/pull/1372) ([k-takata](https://github.com/k-takata))
- iss ファイルでコメント感がもう少しわかりやすいようにする [\#1371](https://github.com/sakura-editor/sakura/pull/1371) ([m-tmatma](https://github.com/m-tmatma))
- 単体テストで起きたassert失敗を捕捉できるように、コンソールモードではメッセージボックスを表示しないようにしたい [\#1362](https://github.com/sakura-editor/sakura/pull/1362) ([berryzplus](https://github.com/berryzplus))
- テストケース一覧の出力が重複しているので削る。 [\#1356](https://github.com/sakura-editor/sakura/pull/1356) ([berryzplus](https://github.com/berryzplus))
- MinGWビルドで出ているinlineの警告に対処する。 [\#1355](https://github.com/sakura-editor/sakura/pull/1355) ([berryzplus](https://github.com/berryzplus))
- 英語OSでの初回起動時に設定値を変換する処理を挿入する [\#1350](https://github.com/sakura-editor/sakura/pull/1350) ([berryzplus](https://github.com/berryzplus))
- tools/find-tools.batにpython検出機能を追加する [\#1348](https://github.com/sakura-editor/sakura/pull/1348) ([berryzplus](https://github.com/berryzplus))
- マクロ専用の検索オプションがヘルプに記載されていないので追加する [\#1344](https://github.com/sakura-editor/sakura/pull/1344) ([berryzplus](https://github.com/berryzplus))
- ヘルプのリンクを修正 [\#1338](https://github.com/sakura-editor/sakura/pull/1338) ([berryzplus](https://github.com/berryzplus))
- MSVCのCMakeによる単体テストにリソースを埋め込む [\#1337](https://github.com/sakura-editor/sakura/pull/1337) ([berryzplus](https://github.com/berryzplus))
- HTMLヘルプの目次に「Windows 10 でのファイル拡張子関連付け」ページが追加されていないので追加 [\#1330](https://github.com/sakura-editor/sakura/pull/1330) ([beru](https://github.com/beru))
- ヘルプの「Windows 10 でのファイル拡張子関連付け」「Grep」「Grep置換」の画像がやたら大きくて見づらい問題に対処 [\#1328](https://github.com/sakura-editor/sakura/pull/1328) ([beru](https://github.com/beru))
- フォントの拡大縮小操作時にフォントサイズが変わらない場合は処理を行わないように判定追加 [\#1321](https://github.com/sakura-editor/sakura/pull/1321) ([beru](https://github.com/beru))
- PageUp, PageDown 時に描画する必要が無い場合は描画しないようにする判定を追加 [\#1320](https://github.com/sakura-editor/sakura/pull/1320) ([beru](https://github.com/beru))
- ツールバーの状態更新を必要な場合にのみ行うように変更 [\#1319](https://github.com/sakura-editor/sakura/pull/1319) ([beru](https://github.com/beru))

## [v2.4.1](https://github.com/sakura-editor/sakura/tree/v2.4.1) (2020-05-30)

[Full Changelog](https://github.com/sakura-editor/sakura/compare/v2.4.1-beta3...v2.4.1)

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
- 折り返しされた際、文字数カウントが合わない不具合を修正 [\#1241](https://github.com/sakura-editor/sakura/pull/1241) ([beru](https://github.com/beru))
- \_wcsdup\(\)ではfree\(\)を使うべきなはずなのでそうする [\#1239](https://github.com/sakura-editor/sakura/pull/1239) ([beru](https://github.com/beru))
- SJISエンコードのキーワードヘルプ辞書を設定するとき表示化けする問題への対処 [\#1238](https://github.com/sakura-editor/sakura/pull/1238) ([beru](https://github.com/beru))
- wstringをWCHAR\[N\]にコピーする処理でNUL終端が付かない不具合を修正 [\#1235](https://github.com/sakura-editor/sakura/pull/1235) ([berryzplus](https://github.com/berryzplus))



### その他変更

- installer のタイトルのアプリ名で 32bit/64bit の区別ができるようにする [\#1293](https://github.com/sakura-editor/sakura/pull/1293) ([m-tmatma](https://github.com/m-tmatma))
- Combo\_AddString の戻り値の型を int にする [\#1291](https://github.com/sakura-editor/sakura/pull/1291) ([m-tmatma](https://github.com/m-tmatma))
- CLayoutIntのテストを追加 [\#1290](https://github.com/sakura-editor/sakura/pull/1290) ([berryzplus](https://github.com/berryzplus))
- 単体テストで文字列リソースを利用できるようにする [\#1275](https://github.com/sakura-editor/sakura/pull/1275) ([berryzplus](https://github.com/berryzplus))
- 履歴コンボの履歴削除機能の発動条件を変更したい [\#1255](https://github.com/sakura-editor/sakura/pull/1255) ([berryzplus](https://github.com/berryzplus))
- ヘルプの動作環境OSの記述を更新 [\#1254](https://github.com/sakura-editor/sakura/pull/1254) ([beru](https://github.com/beru))
- GrepとGrep置換ダイアログの微調整 [\#1242](https://github.com/sakura-editor/sakura/pull/1242) ([7-rate](https://github.com/7-rate))
- TCHAR系マクロ \_stprintf の呼び出しがまだ残っている箇所を swprintf 関数の呼び出しに変更 [\#1240](https://github.com/sakura-editor/sakura/pull/1240) ([beru](https://github.com/beru))
- CNativeテストケースの期待値を修正する [\#1230](https://github.com/sakura-editor/sakura/pull/1230) ([berryzplus](https://github.com/berryzplus))
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

### 機能追加

- 除外ファイル・除外フォルダの指定をGrep置換でも使えるようにする [\#1210](https://github.com/sakura-editor/sakura/pull/1210) ([berryzplus](https://github.com/berryzplus))

### バグ修正

- キーワードを指定してタグジャンプができないバグを修正 [\#1208](https://github.com/sakura-editor/sakura/pull/1208) ([7-rate](https://github.com/7-rate))



### その他変更

- バージョン情報に表示するプロジェクトURLに付けるラベルキャプションを短くする [\#1215](https://github.com/sakura-editor/sakura/pull/1215) ([berryzplus](https://github.com/berryzplus))
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

- 最近使ったファイル挿入 と 最近使ったフォルダ挿入 を行う機能追加 [\#1063](https://github.com/sakura-editor/sakura/pull/1063) ([beru](https://github.com/beru))
- PlatformToolset 指定をプロパティーシートに分離して VS2017 および VS2019 で両対応できるようにする [\#866](https://github.com/sakura-editor/sakura/pull/866) ([m-tmatma](https://github.com/m-tmatma))
- 「同名のC/C++ヘッダ\(ソース\)を開く」機能が利用可能か調べる処理で拡張子の確認が行われるように記述追加 [\#812](https://github.com/sakura-editor/sakura/pull/812) ([beru](https://github.com/beru))

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
- 開いているファイルのフォルダのパスをクリップボードにコピーできるようにする [\#718](https://github.com/sakura-editor/sakura/pull/718) ([m-tmatma](https://github.com/m-tmatma))
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
- grep/grep 置換で除外ファイル、除外フォルダを指定できるようにする [\#403](https://github.com/sakura-editor/sakura/pull/403) ([m-tmatma](https://github.com/m-tmatma))
- 環境変数 SKIP\_CREATE\_GITHASH が 1 にセットしている場合、githash.h の生成をスキップする [\#319](https://github.com/sakura-editor/sakura/pull/319) ([m-tmatma](https://github.com/m-tmatma))
- res ファイルのハッシュを計算するスクリプト [\#317](https://github.com/sakura-editor/sakura/pull/317) ([m-tmatma](https://github.com/m-tmatma))
- python のアウトライン解析を実装 [\#314](https://github.com/sakura-editor/sakura/pull/314) ([m-tmatma](https://github.com/m-tmatma))
- エディタ設定：インデント設定と行末スペース削り [\#245](https://github.com/sakura-editor/sakura/pull/245) ([kobake](https://github.com/kobake))
- \[x64対応\] バージョン情報にPlatform情報を埋め込み \(32bit/64bit\) [\#179](https://github.com/sakura-editor/sakura/pull/179) ([kobake](https://github.com/kobake))

### バグ修正

- grep で 除外ファイル、除外フォルダが効かない のを修正 [\#758](https://github.com/sakura-editor/sakura/pull/758) ([m-tmatma](https://github.com/m-tmatma))
- grep で 除外ファイル、除外フォルダが効かない問題を修正するため、除外パターンを指定するコマンドラインを復活する [\#750](https://github.com/sakura-editor/sakura/pull/750) ([m-tmatma](https://github.com/m-tmatma))
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
- Revert "grep で 除外ファイル、除外フォルダが効かない問題を修正するため、除外パターンを指定するコマンドラインを復活する" [\#753](https://github.com/sakura-editor/sakura/pull/753) ([m-tmatma](https://github.com/m-tmatma))
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
- マルチユーザ設定を有効にする [\#689](https://github.com/sakura-editor/sakura/pull/689) ([berryzplus](https://github.com/berryzplus))
- ツール類のコンパイルオプションに/MPを付ける [\#688](https://github.com/sakura-editor/sakura/pull/688) ([berryzplus](https://github.com/berryzplus))
- 「このファイルのパス名をコピー」 をタブメニューにも追加する [\#666](https://github.com/sakura-editor/sakura/pull/666) ([m-tmatma](https://github.com/m-tmatma))
- 縦スクロール時に不必要にルーラーの再描画がされないように対策 [\#660](https://github.com/sakura-editor/sakura/pull/660) ([beru](https://github.com/beru))
- スクロールバーの再描画を常には行わないように変更 [\#657](https://github.com/sakura-editor/sakura/pull/657) ([beru](https://github.com/beru))
- 辞書Tipの描画改善、およびHighDPI対応 [\#647](https://github.com/sakura-editor/sakura/pull/647) ([berryzplus](https://github.com/berryzplus))
- フォントラベルのHighDPI対応 [\#645](https://github.com/sakura-editor/sakura/pull/645) ([berryzplus](https://github.com/berryzplus))
- 分割線ウィンドウクラスの処理で使用している各数値をDPI設定に応じて調整 [\#641](https://github.com/sakura-editor/sakura/pull/641) ([beru](https://github.com/beru))
- 独自拡張プロパティシートの「設定フォルダ」ボタンの位置調整処理でDPIを考慮 [\#638](https://github.com/sakura-editor/sakura/pull/638) ([beru](https://github.com/beru))
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
- bregonig のライセンスファイルをライセンス用のサブフォルダに配置する [\#455](https://github.com/sakura-editor/sakura/pull/455) ([m-tmatma](https://github.com/m-tmatma))
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
- MinGW向けMakefileからbtoolフォルダへの参照を取り除く [\#371](https://github.com/sakura-editor/sakura/pull/371) ([berryzplus](https://github.com/berryzplus))
- sakura\_lang\_en\_US.dll の MinGW 向け Makefile を更新する [\#369](https://github.com/sakura-editor/sakura/pull/369) ([berryzplus](https://github.com/berryzplus))
- HTML Help Workshop がVS2017で導入される条件 [\#366](https://github.com/sakura-editor/sakura/pull/366) ([KENCHjp](https://github.com/KENCHjp))
- MINGW\_HAS\_SECURE\_APIが無効か判定する式の修正 [\#363](https://github.com/sakura-editor/sakura/pull/363) ([berryzplus](https://github.com/berryzplus))
- ビルド関連のバッチファイルのエラー耐性を上げる [\#362](https://github.com/sakura-editor/sakura/pull/362) ([m-tmatma](https://github.com/m-tmatma))
- funccode.bat のエラー耐性を上げる [\#361](https://github.com/sakura-editor/sakura/pull/361) ([m-tmatma](https://github.com/m-tmatma))
- githash.bat のエラー耐性を上げる [\#360](https://github.com/sakura-editor/sakura/pull/360) ([m-tmatma](https://github.com/m-tmatma))
- インストーラの UI を英語対応にする [\#359](https://github.com/sakura-editor/sakura/pull/359) ([m-tmatma](https://github.com/m-tmatma))
- MinGWでサクラエディタをビルドできるようにメイクファイルを更新する [\#351](https://github.com/sakura-editor/sakura/pull/351) ([berryzplus](https://github.com/berryzplus))
- インストーラ関連のフォルダを無視ファイルに追加 [\#343](https://github.com/sakura-editor/sakura/pull/343) ([m-tmatma](https://github.com/m-tmatma))
- バッチファイルのコピーで /Y と /B をつける [\#342](https://github.com/sakura-editor/sakura/pull/342) ([m-tmatma](https://github.com/m-tmatma))
- ビルドに関するドキュメントを更新、SKIP\_CREATE\_GITHASH に関する説明を移動 [\#341](https://github.com/sakura-editor/sakura/pull/341) ([m-tmatma](https://github.com/m-tmatma))
- keyword ディレクトリ以下のファイルの説明を追加 [\#340](https://github.com/sakura-editor/sakura/pull/340) ([m-tmatma](https://github.com/m-tmatma))
- markdown のスペルミス修正 [\#339](https://github.com/sakura-editor/sakura/pull/339) ([m-tmatma](https://github.com/m-tmatma))
- インストーラ用のドキュメントを更新 [\#337](https://github.com/sakura-editor/sakura/pull/337) ([m-tmatma](https://github.com/m-tmatma))
- postBuild.bat の処理内容を更新 [\#336](https://github.com/sakura-editor/sakura/pull/336) ([m-tmatma](https://github.com/m-tmatma))
- bregonig.dll のファイル内容とタイムスタンプを比較する [\#335](https://github.com/sakura-editor/sakura/pull/335) ([m-tmatma](https://github.com/m-tmatma))
- インストーラの生成先のフォルダにプラットフォーム名を含める [\#333](https://github.com/sakura-editor/sakura/pull/333) ([m-tmatma](https://github.com/m-tmatma))
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