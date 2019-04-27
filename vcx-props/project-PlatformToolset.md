# Visual Studio の 各バージョンの PlatformToolset に関して

<!-- TOC -->

- [Visual Studio の 各バージョンの PlatformToolset に関して](#visual-studio-の-各バージョンの-platformtoolset-に関して)
    - [PlatformToolset の対応関係](#platformtoolset-の対応関係)
    - [異なる Visual Studio のバージョンで開いたときの動作](#異なる-visual-studio-のバージョンで開いたときの動作)
    - [解決策](#解決策)
        - [参考情報](#参考情報)
    - [コマンドラインで PlatformToolset の設定を上書きする方法](#コマンドラインで-platformtoolset-の設定を上書きする方法)
        - [PlatformToolset の指定に関する参考リンク](#platformtoolset-の指定に関する参考リンク)

<!-- /TOC -->

## PlatformToolset の対応関係

Visual Studio の各バージョンごとにデフォルトの PlatformToolset (コンパイルエンジンのバージョン)があります。

|Visual Studio のバージョン|PlatformToolset|
|--|--|
|Visual Studio 2017|v141|
|Visaul Studio 2019|v142|

## 異なる Visual Studio のバージョンで開いたときの動作

Visual Studio 2017 で作成したソリューション/プロジェクトを Visual Studio 2019 で開くと Visaul Studio 2019 の標準の v142 に変換するか
確認するダイアログが出ます。一度ソリューションを開くとユーザーの選択がローカルに保存されるので同じソリューションを再度開いても再度確認される
ことはありません。

しかしながら別のフォルダにソースコードを clone した場合などには再度ユーザーに確認するダイアログが出るので煩雑です。

## 解決策

1.  `PlatformToolset` の設定を[外部のファイル](vcxcompat.props) にまとめます
2.  そのファイルでは、 `Condition` の指定で開かれた Visaul Studio のバージョンによって、 `デフォルトで 使用する PlatformToolset` を指定します。
3. 各プロジェクトファイルで上記ファイルを [Import Project](https://docs.microsoft.com/ja-jp/visualstudio/msbuild/import-element-msbuild?view=vs-2019) でインクルードします。

### 参考情報

https://qiita.com/yumetodo/items/a8324efaf83c9c08d168


## コマンドラインで PlatformToolset の設定を上書きする方法

上記方法によって、デフォルトで使用する `PlatformToolset` を変更しますが、
`/p:PlatformToolset=v141` あるいは `/p:PlatformToolset=v142` のようなパラメータを `msbuild.exe` の引数に
追加してやれば、デフォルトの設定を上書きして好きな `PlatformToolset` でコンパイルすることができます。

### PlatformToolset の指定に関する参考リンク

https://docs.microsoft.com/en-us/cpp/build/reference/msbuild-visual-cpp-overview?view=vs-2019#platformtoolset-property
