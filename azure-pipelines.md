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
    - [Azure Pipelines の template ファイルの命名規則](#azure-pipelines-の-template-ファイルの命名規則)
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
  - [ci/azure-pipelines/template.job.build-unittest.yml](ci/azure-pipelines/template.job.build-unittest.yml) sakura editor 本体のビルド、HTML ヘルプのビルド、インストーラのビルド、単体テストのビルド、実行
    - [ci/azure-pipelines/template.steps.install-python-modules.yml](ci/azure-pipelines/template.steps.install-python-modules.yml) 必要な python モジュールのインストール
  - [ci/azure-pipelines/template.job.SonarQube.yml](ci/azure-pipelines/template.job.SonarQube.yml) SonarQube での解析
  - [ci/azure-pipelines/template.job.checkEncoding.yml](ci/azure-pipelines/template.job.checkEncoding.yml) 文字コードの確認
  - [ci/azure-pipelines/template.job.cppcheck.yml](ci/azure-pipelines/template.job.cppcheck.yml) cppcheck の実行
  - [ci/azure-pipelines/template.job.doxygen.yml](ci/azure-pipelines/template.job.doxygen.yml) doxygen の実行
    - [ci/azure-pipelines/template.steps.install-python-modules.yml](ci/azure-pipelines/template.steps.install-python-modules.yml) 必要な python モジュールのインストール

## Azure Pipelines の template ファイルの命名規則

| JOB 名 | 説明 | job を定義する template |
----|----|----
|template.job.***.yml   | job を定義する template   | [Job reuse](https://docs.microsoft.com/en-us/azure/devops/pipelines/process/templates?view=azure-devops#job-reuse) |
|template.steps.***.yml | steps を定義する template | [Step re-use](https://docs.microsoft.com/en-us/azure/devops/pipelines/process/templates?view=azure-devops#step-re-use) |

## Azure Pipelines のJOB の構成

| JOB 名 | 説明 | job を定義する template |
----|----|----
|VS2017               | サクラエディタのビルドを行う | [ci/azure-pipelines/template.job.build-unittest.yml](ci/azure-pipelines/template.job.build-unittest.yml) |
|SonarQube            | SonarQube を行う             | [ci/azure-pipelines/template.job.SonarQube.yml](ci/azure-pipelines/template.job.SonarQube.yml)           |
|cppcheck             | cppcheck を行う              | [ci/azure-pipelines/template.job.cppcheck.yml](ci/azure-pipelines/template.job.cppcheck.yml)             |
|doxygen              | doxygen  を行う              | [ci/azure-pipelines/template.job.doxygen.yml](ci/azure-pipelines/template.job.doxygen.yml)               |
|checkEncoding        | 文字コードのチェックを行う   | [ci/azure-pipelines/template.job.checkEncoding.yml](ci/azure-pipelines/template.job.checkEncoding.yml)   |
|script_check         | python のコンパイルのチェックを行う   | [ci/azure-pipelines/template.job.python-check.yml](ci/azure-pipelines/template.job.python-check.yml)   |
