# sphinx

<!-- TOC -->

- [sphinx](#sphinx)
    - [概要](#概要)
        - [インストール方法](#インストール方法)
        - [sphinx 用のプロジェクト作成方法](#sphinx-用のプロジェクト作成方法)
        - [HTML Help 用のファイルの作成](#html-help-用のファイルの作成)
        - [HTML Help 用のファイルのコンパイル](#html-help-用のファイルのコンパイル)

<!-- /TOC -->

## 概要

[sphinx](http://www.sphinx-doc.org/en/master/) は reStructuredText のマークダウン形式で
記述したドキュメントから各種形式(PDF, Html Help など)のドキュメントを生成できるドキュメントシステムです。

もともと Python のドキュメントの作成のために作成されましたが、Python 以外のプロジェクトでも使用されています。

サクラエディタでも [sphinx](http://www.sphinx-doc.org/en/master/) を使用してドキュメントを
作成できるか検討中です。(実験段階)

### インストール方法

以下コマンドでインストールするができます。

```
pip install sphinx
```

### sphinx 用のプロジェクト作成方法

以下は Python 2.7 を使う場合のプロジェクト作成方法です。

```
C:\Python27\Scripts\sphinx-quickstart.exe
```

→ 対話形式で質問に答えます。

### HTML Help 用のファイルの作成

```
make htmlhelp
```

### HTML Help 用のファイルのコンパイル

HTML Help 用のコンパイラをインストールしている状態で以下コマンドを実行します。

```
"C:\Program Files (x86)\HTML Help Workshop\hhc.exe" build\htmlhelp\SAKURAEditordoc.chm
```


