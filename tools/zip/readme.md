<!-- TOC -->

- [ZIP 関連の圧縮、解凍、中身の確認用のツール](#zip-関連の圧縮解凍中身の確認用のツール)
    - [参考サイト](#参考サイト)
    - [使い方](#使い方)
        - [圧縮](#圧縮)
        - [解凍](#解凍)
        - [ZIP ファイルの内容確認](#zip-ファイルの内容確認)

<!-- /TOC -->

# ZIP 関連の圧縮、解凍、中身の確認用のツール

7z.exe に依存を排除するために powershell により実現する。

## 参考サイト

- https://qiita.com/alchemist/items/e6706cd425f8f5e5032e
- https://cheshire-wara.com/powershell/ps-cmdlets/item-file/expand-archive/
- https://cheshire-wara.com/powershell/ps-cmdlets/item-file/compress-archive/
- https://dobon.net/vb/dotnet/file/zipfile.html#entries
- http://tech.guitarrapc.com/entry/2013/10/07/083837
- http://tech.guitarrapc.com/entry/2013/10/07/083837
- https://msdn.microsoft.com/ja-jp/library/system.io.compression.ziparchiveentry(v=vs.110).aspx

## 使い方

### 圧縮

```
zip.bat <圧縮先Zipファイル> <圧縮するフォルダパス>
```

例

```
zip.bat bron412-2.zip temp
```



### 解凍

```
unzip.bat <解凍するZipファイル> <展開先フォルダパス>
```

例

```
unzip.bat bron412.zip temp
```


### ZIP ファイルの内容確認

```
listzip.bat <内容確認するZipファイル>
```

例

```
listzip.bat bron412.zip
```
