# cpplint.py

<!-- TOC -->

- [cpplint.py](#cpplintpy)
- [CodeFactor のスタイルチェック](#codefactor-のスタイルチェック)
- [CodeFactor のデフォルトのCPPLINT.CFG](#codefactor-のデフォルトのcpplintcfg)
- [cpplint.py のインストール](#cpplintpy-のインストール)
- [cpplint.py の実行](#cpplintpy-の実行)
    - [cpplint.py のインストール場所の例](#cpplintpy-のインストール場所の例)
- [参考サイト](#参考サイト)

<!-- /TOC -->


# CodeFactor のスタイルチェック

- CodeFactor では [cpplint](https://pypi.org/project/cpplint/) でチェックを行います。
   - https://support.codefactor.io/i24-analysis-tools-open-source
- [cpplint](https://pypi.org/project/cpplint/) はカレントディレクトリに [CPPLINT.CFG](https://github.com/google/styleguide/blob/6271f3f473ceb3a7fef99388a3040903b1a145f1/cpplint/cpplint.py#L156-L197) というファイルを配置することでフィルタリング条件を指定できます。


# CodeFactor のデフォルトのCPPLINT.CFG

CodeFactor のデフォルトの `CPPLINT.CFG` は https://github.com/codefactor-io/default-configs/blob/master/CPPLINT.cfg です。

# cpplint.py のインストール

```
pip install cpplint
```


# cpplint.py の実行


```
cd /d <sakura editor ソースのトップ>
python <path-to-cpplint>\cpplint.py --recursive sakura_core
```

## cpplint.py のインストール場所の例

`C:\Python27\Lib\site-packages\cpplint.py`

# 参考サイト

- [Analysis Tools (Open Source) at support.codefactor.io](https://support.codefactor.io/i24-analysis-tools-open-source)
- [cpplint  (pip)](https://pypi.org/project/cpplint/)
- [CPPLINT.cfg](https://github.com/google/styleguide/blob/6271f3f473ceb3a7fef99388a3040903b1a145f1/cpplint/cpplint.py#L156-L197)
- [default CPPLINT.cfg  at support.codefactor.io](https://github.com/codefactor-io/default-configs/blob/master/CPPLINT.cfg)
- [Visual Studioでcpplintする](https://qiita.com/odanado/items/cd364521a048c662a888)
