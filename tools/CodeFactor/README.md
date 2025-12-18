# CodeFactor について

<!-- TOC -->

- [CodeFactor について](#codefactor-について)
  - [概要](#概要)
  - [CPPLINT.CFG について](#cpplintcfg-について)
  - [cpplint のローカル実行](#cpplint-のローカル実行)
    - [インストール](#インストール)
    - [実行方法](#実行方法)
  - [参考資料](#参考資料)

<!-- /TOC -->

## 概要

[CodeFactor][CodeFactor] は、コードの品質を自動的にチェックするWebサービスです。

このプロジェクトでは、CodeFactorを使ってC++コードのスタイルチェックを行っています。CodeFactorは内部的に [cpplint][cpplint pip] を使用してC++コードを解析します。

- 参考: [CodeFactor - Analysis Tools (Open Source)][CodeFactor Analysis Tools]

## CPPLINT.CFG について

リポジトリ直下にある `CPPLINT.CFG` は、CodeFactorがcpplintを実行する際の設定ファイルです。このファイルでチェックのフィルタリング条件を指定できます。

- 設定ファイルの仕様: [CPPLINT.CFG 仕様][CPPLINT.cfg spec]
- CodeFactorのデフォルト設定: [default CPPLINT.cfg][default CPPLINT.cfg]

## cpplint のローカル実行

cpplintはPythonモジュールとして提供されているため、ローカル環境でも実行できます。

### インストール

```pwsh
pip install cpplint
```

インストール後、cpplintは以下のような場所に配置されます:
- 例: `C:\Python27\Lib\site-packages\cpplint.py`

### 実行方法

**モジュールとして実行 (推奨):**
```pwsh
cd /d <sakura editor ソースのトップ>
cpplint --recursive sakura_core
```

**スクリプトとして実行:**
```pwsh
cd /d <sakura editor ソースのトップ>
python <path-to-cpplint>\cpplint.py --recursive sakura_core
```

## 参考資料

- [CodeFactor][CodeFactor]
- [CodeFactor - Analysis Tools (Open Source)][CodeFactor Analysis Tools]
- [cpplint (pip)][cpplint pip]
- [CPPLINT.cfg 仕様][CPPLINT.cfg spec]
- [CodeFactor デフォルト CPPLINT.cfg][default CPPLINT.cfg]
- [Visual Studioでcpplintする][VS cpplint]

<!-- リンク定義 -->
[CodeFactor]: https://www.codefactor.io/
[CodeFactor Analysis Tools]: https://support.codefactor.io/i24-analysis-tools-open-source
[cpplint pip]: https://pypi.org/project/cpplint/
[CPPLINT.cfg spec]: https://github.com/google/styleguide/blob/6271f3f473ceb3a7fef99388a3040903b1a145f1/cpplint/cpplint.py#L156-L197
[default CPPLINT.cfg]: https://github.com/codefactor-io/default-configs/blob/master/CPPLINT.cfg
[VS cpplint]: https://qiita.com/odanado/items/cd364521a048c662a888
