# azure pipelines でのビルド

<!-- TOC -->

- [azure pipelines でのビルド](#azure-pipelines-でのビルド)
    - [Azure Pipelines の利用開始方法](#azure-pipelines-の利用開始方法)
        - [単にサクラエディタ用のプロジェクトに読み取りアクセスする場合](#単にサクラエディタ用のプロジェクトに読み取りアクセスする場合)
        - [Azure Pipelines にアカウントがない場合](#azure-pipelines-にアカウントがない場合)
        - [Azure Pipelines にアカウントがある場合](#azure-pipelines-にアカウントがある場合)
    - [参考情報](#参考情報)
    - [参考サイト](#参考サイト)
    - [Azure Pipelines の設定ファイルの構成](#azure-pipelines-の設定ファイルの構成)
    - [Azure Pipelines のJOB の構成](#azure-pipelines-のjob-の構成)

<!-- /TOC -->

## Azure Pipelines の利用開始方法

Azure Pipelines では Microsoft アカウントでログインします。
public リポジトリに読み取りアクセスするだけではアカウントもログインも不要です。

### 単にサクラエディタ用のプロジェクトに読み取りアクセスする場合

https://dev.azure.com/sakuraeditor/sakura にアクセスして、`Pipelines` のリンクをクリックします。

### Azure Pipelines にアカウントがない場合

https://azure.microsoft.com/ja-jp/services/devops/pipelines/ にアクセスして `Pipelines の使用を無料で開始する` のリンクをクリックします。


### Azure Pipelines にアカウントがある場合

https://azure.microsoft.com/ja-jp/services/devops/pipelines/ にアクセスして `Azure DevOps にサイン` のリンクをクリックします。

## 参考情報

- [Wiki: azure pipelinesメモ](https://github.com/sakura-editor/sakura/wiki/azure-pipelines%E3%83%A1%E3%83%A2)

## 参考サイト

- [What is Azure Pipelines?](https://docs.microsoft.com/en-us/azure/devops/pipelines/get-started/what-is-azure-pipelines?toc=/azure/devops/pipelines/toc.json&bc=/azure/devops/boards/pipelines/breadcrumb/toc.json&view=azure-devops)
- [Key concepts for new Azure Pipelines users](https://docs.microsoft.com/en-us/azure/devops/pipelines/get-started/key-pipelines-concepts?toc=/azure/devops/pipelines/toc.json&bc=/azure/devops/boards/pipelines/breadcrumb/toc.json&view=azure-devops)
- [Sign up for Azure Pipelines](https://docs.microsoft.com/en-us/azure/devops/pipelines/get-started/what-is-azure-pipelines?toc=/azure/devops/pipelines/toc.json&bc=/azure/devops/boards/pipelines/breadcrumb/toc.json&view=azure-devops)
- [YAML schema reference](https://docs.microsoft.com/en-us/azure/devops/pipelines/yaml-schema?view=azure-devops&tabs=schema)
- [Predefined build variables](https://docs.microsoft.com/en-us/azure/devops/pipelines/build/variables?view=azure-devops&tabs=yaml)
- [Job and step templates](https://docs.microsoft.com/en-us/azure/devops/pipelines/process/templates?view=azure-devops)
- [Build pipeline triggers](https://docs.microsoft.com/en-us/azure/devops/pipelines/build/triggers?view=azure-devops&tabs=yaml)


## Azure Pipelines の設定ファイルの構成

- [azure-pipelines.yml](azure-pipelines.yml) 最上位の設定ファイル
  - [ci/azure-pipelines/template.steps.build-unittest.yml](ci/azure-pipelines/template.steps.build-unittest.yml) sakura editor 本体のビルド、HTML ヘルプのビルド、インストーラのビルド、単体テストのビルド、実行
  - [ci/azure-pipelines/template.steps.checkEncoding.yml](ci/azure-pipelines/template.steps.checkEncoding.yml) 文字コードの確認
  - [ci/azure-pipelines/template.steps.cppcheck.yml](ci/azure-pipelines/template.steps.cppcheck.yml) cppcheck の実行
  - [ci/azure-pipelines/template.steps.doxygen.yml](ci/azure-pipelines/template.steps.doxygen.yml) doxygen の実行

## Azure Pipelines のJOB の構成

| JOB 名 | 説明 | steps を定義する template |
----|----|----
|windows              | サクラエディタのビルドを行う | [ci/azure-pipelines/template.steps.build-unittest.yml](ci/azure-pipelines/template.steps.build-unittest.yml) |
|cppcheck             | cppcheck を行う              | [ci/azure-pipelines/template.steps.cppcheck.yml](ci/azure-pipelines/template.steps.cppcheck.yml)             |
|doxygen              | doxygen  を行う              | [ci/azure-pipelines/template.steps.doxygen.yml](ci/azure-pipelines/template.steps.doxygen.yml)               |
|checkEncoding        | 文字コードのチェックを行う   | [ci/azure-pipelines/template.steps.checkEncoding.yml](ci/azure-pipelines/template.steps.checkEncoding.yml)   |
