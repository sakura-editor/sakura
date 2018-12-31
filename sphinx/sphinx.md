# sphinx

<!-- TOC -->

- [sphinx](#sphinx)
    - [概要](#概要)
        - [インストール方法](#インストール方法)
        - [sphinx 用のプロジェクト作成方法](#sphinx-用のプロジェクト作成方法)
            - [プロジェクト作成時にしていたパラメータ](#プロジェクト作成時にしていたパラメータ)
        - [HTML Help 用のファイルの作成](#html-help-用のファイルの作成)
        - [HTML Help 用のファイルのコンパイル](#html-help-用のファイルのコンパイル)
    - [目次の作り方](#目次の作り方)
        - [やり方](#やり方)
            - [ドキュメント構造](#ドキュメント構造)
            - [実際のマークアップ](#実際のマークアップ)
        - [参考サイト](#参考サイト)
        - [既知の課題](#既知の課題)

<!-- /TOC -->

## 概要

[sphinx](http://www.sphinx-doc.org/en/master/) は [reStructuredText](https://quick-restructuredtext.readthedocs.io/en/latest/) のマークダウン形式で
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

#### プロジェクト作成時にしていたパラメータ

```
Welcome to the Sphinx 1.8.3 quickstart utility.

Please enter values for the following settings (just press Enter to
accept a default value, if one is given in brackets).

Selected root path: .

You have two options for placing the build directory for Sphinx output.
Either, you use a directory "_build" within the root path, or you separate
"source" and "build" directories within the root path.
> Separate source and build directories (y/n) [n]: y

Inside the root directory, two more directories will be created; "_templates"
for custom HTML templates and "_static" for custom stylesheets and other static
files. You can enter another prefix (such as ".") to replace the underscore.
> Name prefix for templates and static dir [_]:

The project name will occur in several places in the built documentation.
> Project name: SAKURA Editor
> Author name(s): SAKURA Editor Organization
> Project release []:

If the documents are to be written in a language other than English,
you can select a language here by its language code. Sphinx will then
translate text that it generates into that language.

For a list of supported codes, see
http://sphinx-doc.org/config.html#confval-language.
> Project language [en]: jp

The file name suffix for source files. Commonly, this is either ".txt"
or ".rst".  Only files with this suffix are considered documents.
> Source file suffix [.rst]:

One document is special in that it is considered the top node of the
"contents tree", that is, it is the root of the hierarchical structure
of the documents. Normally, this is "index", but if your "index"
document is a custom template, you can also set this to another filename.
> Name of your master document (without suffix) [index]:
Indicate which of the following Sphinx extensions should be enabled:
> autodoc: automatically insert docstrings from modules (y/n) [n]: y
> doctest: automatically test code snippets in doctest blocks (y/n) [n]: y
> intersphinx: link between Sphinx documentation of different projects (y/n) [n]: y
> todo: write "todo" entries that can be shown or hidden on build (y/n) [n]: y
> coverage: checks for documentation coverage (y/n) [n]: y
> imgmath: include math, rendered as PNG or SVG images (y/n) [n]: y
> mathjax: include math, rendered in the browser by MathJax (y/n) [n]: y
> ifconfig: conditional inclusion of content based on config values (y/n) [n]: y
> viewcode: include links to the source code of documented Python objects (y/n) [n]: y
> githubpages: create .nojekyll file to publish the document on GitHub pages (y/n) [n]: y
Note: imgmath and mathjax cannot be enabled at the same time. imgmath has been deselected.

A Makefile and a Windows command file can be generated for you so that you
only have to run e.g. `make html' instead of invoking sphinx-build
directly.
> Create Makefile? (y/n) [y]: y
> Create Windows command file? (y/n) [y]: y

Creating file .\source\conf.py.
Creating file .\source\index.rst.
Creating file .\Makefile.
Creating file .\make.bat.

Finished: An initial directory structure has been created.

You should now populate your master file .\source\index.rst and create other documentation
source files. Use the Makefile to build the docs, like so:
   make builder
where "builder" is one of the supported builders, e.g. html, latex or linkcheck.
```

### HTML Help 用のファイルの作成

```
make htmlhelp
```

### HTML Help 用のファイルのコンパイル

HTML Help 用のコンパイラをインストールしている状態で以下コマンドを実行します。

```
"C:\Program Files (x86)\HTML Help Workshop\hhc.exe" build\htmlhelp\SAKURAEditordoc.chm
```

## 目次の作り方

### やり方

1. `index.rst` から `toctree` で目次の項目にしたい rst ファイルを指定する。
2. その項目から更に小項目にしたい rst ファイルを指定する。

#### ドキュメント構造

- [sakura\source\index.rst](sakura/source/index.rst)
    - [sakura\source\sample-chaptor\index.rst](sakura/source/sample-chaptor/index.rst)
        - [sakura\source\sample-chaptor\index.rst](sakura/source/sample-chaptor/index.rst)

#### 実際のマークアップ

[sakura\source\index.rst](sakura/source/index.rst)

```
.. toctree::

    sample-chaptor/index
```

[sakura\source\sample-chaptor\index.rst](sakura/source/sample-chaptor/index.rst)

```
sample
=========================================

.. toctree::

    sample1
```

[sakura\source\sample-chaptor\index.rst](sakura/source/sample-chaptor/index.rst)

```
sample1
=========================================

サンプル1

テスト・テスト
```

### 参考サイト

https://water2litter.net/pisco/doc/rst_syntax_toctree.html

### 既知の課題

HTML Help 出力するときに目次が文字化けする
