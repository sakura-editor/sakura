
■ sakura V2 インストーラ
インストーラ作成スクリプト: sakura.iss

インストーラの作成には Inno Setup 5 が必要です。
Unicode 版 Inno Setup なら、英語版 Windows へのインストールでも
日本語フォント環境さえ整っていれば日本語部分は文字化けしないはず。
（Control Panel [Regional and Language Options] の設定は不要）
＃ただし、Inno Setup 5.4.3 (u) では不具合があるため？か、
＃カスタムページだけは文字が SJIS もどきに化ける模様。

■ 使い方

フォルダ配置は以下の通り。

    ./
        sakura.iss
        readme.txt  この文書
        set_time.js  SVNからチェックアウト後、キーワードファイルの更新日時を復元する
    ./instmaterials/
        インストーラで表示する画像ファイルが入っています。 
    ./sakura/
        エディタ本体、ヘルプなど、インストーラに含めるファイルを格納する。
        対象ファイルは、インストーラ中に記述してある。
        ここにリリースするバージョンの
            ・sakura.exe
            ・sakura.chm
            ・bregonig.dll
            ・bsd_license.txt（上の bregonig.dll に添付のもの）
        を含める。
    ./sakura/keyword
        キーワードファイル。このフォルダの中にある物は無条件でインストーラに含められる。 

20行目付近にある

  ; エディタのバージョンに応じて書き換える場所
  OutputBaseFilename=sinst2-0-4-0
  VersionInfoVersion=2.0.4.0

の部分をリリースするバージョンに書き換え、sakura.iss を開いてコンパイルする。

できあがったファイルは trunk2/Output/ に格納される。


■ 仕様

Ver 1.6.6.0（ANSI版）のインストーラからの変更点
・関連付けツールは本体取り込みに伴って削除
・"Editor with Shortcut" タイプは "Editor Only" タイプと同じなので削除
・OS バージョン制限（Windows 2000 以上）を追加
・”「送る」に追加”を追加
・keyword以下のファイルを再帰的に読み込む設定を追加

Ver 1.6.6.0 のインストーラ仕様:
http://sourceforge.net/apps/trac/sakura-editor/wiki/Installer

