# azure pipelines でのビルド (工事中)

<!-- TOC -->

- [azure pipelines でのビルド (工事中)](#azure-pipelines-でのビルド-工事中)
    - [Azure Pipelines の利用開始方法](#azure-pipelines-の利用開始方法)
        - [単にサクラエディタ用のプロジェクトに読み取りアクセスする場合](#単にサクラエディタ用のプロジェクトに読み取りアクセスする場合)
        - [Azure Pipelines にアカウントがない場合](#azure-pipelines-にアカウントがない場合)
        - [Azure Pipelines にアカウントがある場合](#azure-pipelines-にアカウントがある場合)
    - [参考サイト](#参考サイト)

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


## 参考サイト

- [What is Azure Pipelines?](https://docs.microsoft.com/en-us/azure/devops/pipelines/get-started/what-is-azure-pipelines?toc=/azure/devops/pipelines/toc.json&bc=/azure/devops/boards/pipelines/breadcrumb/toc.json&view=azure-devops)
- [Key concepts for new Azure Pipelines users](https://docs.microsoft.com/en-us/azure/devops/pipelines/get-started/key-pipelines-concepts?toc=/azure/devops/pipelines/toc.json&bc=/azure/devops/boards/pipelines/breadcrumb/toc.json&view=azure-devops)
- [Sign up for Azure Pipelines](https://docs.microsoft.com/en-us/azure/devops/pipelines/get-started/what-is-azure-pipelines?toc=/azure/devops/pipelines/toc.json&bc=/azure/devops/boards/pipelines/breadcrumb/toc.json&view=azure-devops)
- [YAML schema reference](https://docs.microsoft.com/en-us/azure/devops/pipelines/yaml-schema?view=azure-devops&tabs=schema)
- [Predefined build variables](https://docs.microsoft.com/en-us/azure/devops/pipelines/build/variables?view=azure-devops&tabs=yaml)
- [Job and step templates](https://docs.microsoft.com/en-us/azure/devops/pipelines/process/templates?view=azure-devops)
- [Build pipeline triggers](https://docs.microsoft.com/en-us/azure/devops/pipelines/build/triggers?view=azure-devops&tabs=yaml)
