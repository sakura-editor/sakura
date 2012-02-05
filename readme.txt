
■インストーラ
インストーラ作成スクリプト: sakura2.iss
インストーラの作成には Inno Setup 5 が必要です。
Unicode 版 Inno Setup なら、英語版 Windows へのインストールでも日本語部分も文字化けしません。

■使い方

フォルダ配置は以下の通り。

    trunk2/
        sakura2.iss
    trunk2/instmaterials/
        インストーラで表示する画像ファイルが入っています。 
    trunk2/sakura/
        エディタ本体、ヘルプなど、インストーラに含めるファイルを格納する。
        対象ファイルは、インストーラ中に記述してある。
        ここにリリースするバージョンの
            ・sakura.exe
            ・sakura.chm
            ・bregonig.dll
            ・bsd_license.txt（上の bregonig.dll に添付のもの）
        を含める。
    trunk2/sakura/Keyword
        キーワードファイル。このフォルダの中にある物は無条件でインストーラに含められる。 

20行目付近にある

  ; エディタのバージョンに応じて書き換える場所
  OutputBaseFilename=sinst2-0-4-0
  VersionInfoVersion=2.0.4.0

の部分をリリースするバージョンに書き換え、sakura2.iss を開いてコンパイルする。

できあがったファイルは trunk2/Output/ に格納される。


■仕様

Ver 1.6.6.0（ANSI版）のインストーラからの変更点
・関連付けツールは本体取り込みに伴って削除
・ショートカットの作業用フォルダに "%HOMEDRIVE%%HOMEPATH%" を設定
・"Editor with Shortcut" タイプは "Editor Only" タイプと同じなので削除

Ver 1.6.6.0 のインストーラ仕様:
http://sourceforge.net/apps/trac/sakura-editor/wiki/Installer

