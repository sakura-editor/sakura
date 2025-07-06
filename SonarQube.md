# SonarQube

<!-- TOC -->

- [SonarQube](#sonarqube)
  - [SonarQube および SonarCloud](#sonarqube-および-sonarcloud)
    - [SonarQube](#sonarqube-1)
    - [SonarCloud](#sonarcloud)
  - [SonarQube のローカルでの使用方法](#sonarqube-のローカルでの使用方法)
    - [SonarQube のアカウント設定](#sonarqube-のアカウント設定)
    - [プロジェクトの作成](#プロジェクトの作成)
    - [Access Token の作成](#access-token-の作成)
    - [ローカルで実行する場合の環境設定 (事前準備)](#ローカルで実行する場合の環境設定-事前準備)
    - [解析手順の流れ (一般論)](#解析手順の流れ-一般論)
    - [解析手順の流れ (サクラエディタ)](#解析手順の流れ-サクラエディタ)
  - [サクラエディタを SonarQube でビルドする手順](#サクラエディタを-sonarqube-でビルドする手順)
  - [SonarQube に関する情報](#sonarqube-に関する情報)
    - [SonarQube の使用方法に関するサイト](#sonarqube-の使用方法に関するサイト)
    - [SonarScanner の使用方法](#sonarscanner-の使用方法)

<!-- /TOC -->

<!-- 以下は Markdownの参照形式によるリンク の定義です。 -->
<!-- 参照 https://hail2u.net/blog/coding/markdown-reference-style-links.html -->

[sonarsource]: https://www.sonarsource.com/ "SonarSource"
[SonarQube]: https://www.sonarsource.com/products/sonarqube/ "SonarQube"
[SonarCloud]: https://sonarcloud.io/about "SonarCloud"
[SonarScanner for MSBuild (SonarScanner.MSBuild.exe)]: https://docs.sonarqube.org/display/SCAN/Analyzing+with+SonarQube+Scanner+for+MSBuild "SonarScanner for MSBuild (SonarScanner.MSBuild.exe)"
[chocolatey]: https://chocolatey.org/ "chocolatey"
[build-wrapper-win-x86.zip]: https://sonarcloud.io/static/cpp/build-wrapper-win-x86.zip "https://sonarcloud.io/static/cpp/build-wrapper-win-x86.zip"
[Git for Windows]: https://gitforwindows.org/ "Git for Windows"
[curl]: https://curl.haxx.se/download.html "curl"
[build-sln.bat]: build-sln.bat "build-sln.bat"
[build-sonar-qube-env.bat]: build-sonar-qube-env.bat "build-sonar-qube-env.bat"
[build-sonar-qube-start.bat]: build-sonar-qube-start.bat "build-sonar-qube-start.bat"
[build-sonar-qube-finish.bat]: build-sonar-qube-finish.bat "build-sonar-qube-finish.bat"
[azure pipelines の sakura editor のプロジェクト]: https://dev.azure.com/sakuraeditor/sakura "https://dev.azure.com/sakuraeditor/sakura"
[Azure Pipelines の Secret Variable]: https://docs.microsoft.com/en-us/azure/devops/pipelines/process/variables?view=azure-devops&tabs=yaml%2Cbatch&viewFallbackFrom=vsts#secret-variables
[Azure Pipelines の variables]: https://docs.microsoft.com/en-us/azure/devops/pipelines/process/variables?view=azure-devops&tabs=yaml%2Cbatch
[Azure Pipelines の timeoutInMinutes]: https://docs.microsoft.com/ja-jp/azure/devops/pipelines/process/phases?view=azure-devops&tabs=yaml#timeouts
[Azure Pipelines の Predefined build variables]: https://docs.microsoft.com/ja-jp/azure/devops/pipelines/build/variables?view=azure-devops&tabs=yaml
[Azure Pipelines の conditon]: https://docs.microsoft.com/ja-jp/azure/devops/pipelines/process/conditions?view=azure-devops&viewFallbackFrom=vsts&tabs=yaml
[appveyor の Secure Variables]: https://www.appveyor.com/docs/build-configuration/#secure-variables


<!-- Markdownの参照形式によるリンク の定義終わり -->

## SonarQube および SonarCloud 

### SonarQube

[SonarQube][SonarQube] は [sonarsource][sonarsource] が提供する静的解析サービス。

### SonarCloud

[SonarCloud][SonarCloud] は [SonarQube][SonarQube] のクラウド版。  
いつものごとく、オープンソースに対してはタダです。

サクラエディタのソースコード解析には 1時間半ほどかかるので、並列実行が可能な Azure Pipelines における夜間の定期タスクでのみ解析を実施します。

## SonarQube のローカルでの使用方法

### SonarQube のアカウント設定

https://sonarcloud.io/sessions/new にアクセスして GitHub アカウントでログインします。

### プロジェクトの作成

https://sonarcloud.io/projects/create にアクセスしてプロジェクトを作成します。

- Organization 名をメモしておきます。
- Project 名をメモしておきます。
- Access Token をメモしておきます。 
**この情報はパスワードと同じ意味を持つので漏れないように注意します。**

### Access Token の作成

https://sonarcloud.io/account/security/ のページでいつでも Access Token の作成、Revoke ができます。

ヘルプ: https://sonarcloud.io/documentation/user-guide/user-token/

### ローカルで実行する場合の環境設定 (事前準備)

1. chocolatey をインストールする  
  https://chocolatey.org/install#install-with-cmdexe を参考にインストールする。  
  ※powershellコンソールを「管理者として実行」して、サイトにあるスクリプトをコピペ実行するだけです。

2. SonarScanner実行環境として Java 11 をインストールする
    1. JDKを使う場合 https://chocolatey.org/packages/openjdk11
    2. JREを使う場合 https://chocolatey.org/packages/openjdk11jre
    3. Oracleに開発者登録している場合 Oracle JDK/JRE で代替してもよいです。
    4. 参考情報
        - 普通の [Java Runtime Envirionment (jre8)](https://java.com/ja/download/manual.jsp) は使えません。  
          SonarSource の方針により Java8 を使った静的解析はできなくなりました。
        - [Prerequisites and Overview (Supported Platforms)](https://docs.sonarqube.org/latest/requirements/requirements/#header-3)
        - [SonarQubeの Java 11 対応状況](https://qiita.com/hayao_k/items/2cd81161f8dffd3a178b)

3. `JAVA_HOME` の環境変数を設定する
  ※コマンドプロンプトで `set J` して `JAVA_HOME` が表示されない場合のみ

	例

	`set JAVA_HOME=C:\Program Files\Java\jdk11.0.9`

### 解析手順の流れ (一般論)

1. [chocolatey] で [SonarScanner for MSBuild (SonarScanner.MSBuild.exe)][SonarScanner for MSBuild (SonarScanner.MSBuild.exe)] をダウンロードする
1. curl で [build-wrapper-win-x86.zip] をダウンロードして解凍する
1. `C:\ProgramData\chocolatey\bin\SonarScanner.MSBuild.exe begin` を呼ぶ。
1. `build-wrapper-win-x86.zip` の中の `build-wrapper-win-x86-64.exe` を使って msbuild.exe を起動する。
1. `C:\ProgramData\chocolatey\bin\SonarScanner.MSBuild.exe end` を呼ぶ。

メモ: [curl] は [Git for Windows] をインストールすると `C:\Program Files\Git\mingw64\bin\curl.exe` にインストールされて、自動的にパスも通されます。

### 解析手順の流れ (サクラエディタ)

1. [chocolatey] で [SonarScanner for MSBuild (SonarScanner.MSBuild.exe)] をダウンロードする
    1. `choco install "msbuild-sonarqube-runner" -y`
1. [build-sln.bat] でソリューションをビルドする
    1. [build-sonar-qube-start.bat] で SonarQube の準備を行う。
        1. [build-sonar-qube-env.bat] を呼び出し必要な環境変数の設定を行う。
        1. curl で [build-wrapper-win-x86.zip] をダウンロードして解凍する。
        1. `C:\ProgramData\chocolatey\bin\SonarScanner.MSBuild.exe begin` を呼んで SonarQube の解析を開始する。
    1. [build-wrapper-win-x86.zip] の中の `build-wrapper-win-x86-64.exe` 経由で `msbuild.exe` を起動する。
    1. [build-sonar-qube-finish.bat] で SonarQube の解析結果を [SonarCloud] のサーバーに結果を送る。
        1. `C:\ProgramData\chocolatey\bin\SonarScanner.MSBuild.exe end` を呼ぶ。

## サクラエディタを SonarQube でビルドする手順

`SONAR_QUBE_TOKEN` の値は SonarQube のサイトにログインして確認できます。

`SONAR_QUBE_ORG` と  `SONAR_QUBE_PROJECT` と `SONAR_QUBE_TOKEN` の値はそれぞれ自分が作成したものに読み替えてください。

```
set SONAR_QUBE=Yes
set SONAR_QUBE_ORG=sakura-editor
set SONAR_QUBE_PROJECT=sakura-editor_sakura
set SONAR_QUBE_TOKEN=xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
build-sln.bat Win32 Release
```

## SonarQube に関する情報

### SonarQube の使用方法に関するサイト

- https://www.appveyor.com/blog/2016/12/23/sonarqube/
- https://docs.sonarqube.org/7.4/analysis/analysis-parameters/

### SonarScanner の使用方法

https://docs.sonarqube.org/display/SCAN/Analyzing+with+SonarQube+Scanner+for+MSBuild#AnalyzingwithSonarQubeScannerforMSBuild-Usage
