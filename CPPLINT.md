# CppLint

<!-- TOC -->

- [CppLint](#cpplint)
- [CodeFactor のスタイルチェック](#codefactor-のスタイルチェック)
- [CodeFactor のデフォルトのCPPLINT.CFG](#codefactor-のデフォルトのcpplintcfg)
- [cpplint のインストール](#cpplint-のインストール)
- [cpplint の実行](#cpplint-の実行)
    - [cpplint のインストール場所の例](#cpplint-のインストール場所の例)

<!-- /TOC -->


# CodeFactor のスタイルチェック

- CodeFactor では [cpplint](https://pypi.org/project/cpplint/) でチェックを行います。
   - https://support.codefactor.io/i24-analysis-tools-open-source
- [cpplint](https://pypi.org/project/cpplint/) はカレントディレクトリに [CPPLINT.CFG](https://github.com/google/styleguide/blob/6271f3f473ceb3a7fef99388a3040903b1a145f1/cpplint/cpplint.py#L156-L197) というファイルを配置することでフィルタリング条件を指定できます。


# CodeFactor のデフォルトのCPPLINT.CFG

CodeFactor のデフォルトの `CPPLINT.CFG` は https://github.com/codefactor-io/default-configs/blob/master/CPPLINT.cfg です。

# cpplint のインストール

```
pip install cpplint
```


# cpplint の実行


```
cd /d <sakura editor ソースのトップ>
python <path-to-cpplint>\cpplint.py sakura_core
```

## cpplint のインストール場所の例

`C:\Python27\Lib\site-packages\cpplint.py`
