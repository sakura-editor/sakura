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
    - [CI でのビルド](#ci-でのビルド)
        - [azure pipelines の設定](#azure-pipelines-の設定)
            - [環境変数](#環境変数)
            - [スケジュール設定](#スケジュール設定)
            - [azure-pipelines.yml の設定](#azure-pipelinesyml-の設定)
                - [Job の設定](#job-の設定)
                - [Steps の設定](#steps-の設定)
        - [Appveyor の設定](#appveyor-の設定)
            - [Secure the GitHub Authentication token](#secure-the-github-authentication-token)

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

[SonarQube][SonarQube] は
[sonarsource][sonarsource] が提供する静的解析サービス。  


### SonarCloud

[SonarCloud][SonarCloud] は [SonarQube][SonarQube] のクラウド版。  
いつものごとく、オープンソースに対してはタダです。

サクラエディタのソースコード解析には 1時間半ほどかかるので、並列実行が可能な Azure Pipelines で夜間の定期タスクのみで解析を実施します。  
(Appveyor では実施しない。定期タスク以外でも実施しない。)

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

1. https://java.com/ja/download/manual.jsp から **64bit 版 の JAVA のランタイム** をダウンロードしてインストールする
1. `JAVA_HOME` の環境変数を設定する

	例

	`set JAVA_HOME=C:\Program Files\Java\jre1.8.0_211`

1. https://chocolatey.org/install#install-with-cmdexe を参考に chocolatey をインストールする。

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

## CI でのビルド

### azure pipelines の設定

#### 環境変数

1. [azure pipelines の sakura editor のプロジェクト] にアクセスします。
2. `Pipelines` を選ぶ
3. `Edit` ボタンを押す
4. `Run` の右隣りのアイコンをクリックする
5. `Variables` のボタンを押す
6. `Pipelines Variables` で `Add` を押す
7. `SONAR_QUBE_ORG` と  `SONAR_QUBE_PROJECT` を追加してそれぞれ値を設定する
8. `SONAR_QUBE_TOKEN` を追加して値を設定し、 鍵のアイコンをクリックする (重要) 
9. `Save & Queue` で `Save` を選ぶ

	![環境変数の設定画面](azure-SonarQube.png)

#### スケジュール設定

1. [azure pipelines の sakura editor のプロジェクト] にアクセスします。
2. `Pipelines` を選ぶ
3. `Edit` ボタンを押す
4. `Run` の右隣りのアイコンをクリックする
5. `Triggers` のボタンを押す
6. `Scheduled` の `Add` をクリックする
	![スケジュール設定1](azure-SonarQube-schedule1.png)
7. 実行させたいスケジュールを設定して、実行させる対象ブランチを設定する
	![スケジュール設定2](azure-SonarQube-schedule2.png)
8. 設定を保存する
	![スケジュール設定3](azure-SonarQube-schedule3.png)

#### azure-pipelines.yml の設定

##### Job の設定

```
- job: SonarQube
  timeoutInMinutes: 120
  variables:
    SONAR_QUBE: Yes
  condition:
    eq(variables['Build.Reason'], 'Schedule')
```

1. SonarQube の実行には時間がかかるので [Azure Pipelines の timeoutInMinutes] の設定を行いタイムアウト時間を延ばす
1. [Azure Pipelines の variables] の設定で `SONAR_QUBE` の環境変数を定義して、[build-sln.bat] に [SonarQube] を有効にしたビルドであると伝える
1. [Azure Pipelines の Predefined build variables] のうち `Build.Reason` を参照して、どういうトリガーでビルドが行われたかを yaml の中からあるいはビルド用のバッチファイル等から参照することができる。
	- `Build.Reason` を yaml の中で参照するとき `variables['Build.Reason']`
	- `Build.Reason` を バッチファイル の中で参照するとき `BUILD_REASON`
1. [Azure Pipelines の conditon] でビルドトリガーの条件を設定する。条件指定では and や or の条件を指定することができる。

	`Build.Reason` としてどういう値を設定できるかは [Azure Pipelines の variables] の `Build.Reason` に説明がある。


例: ビルドトリガーが定期実行のとき

```
  condition:
    eq(variables['Build.Reason'], 'Schedule')
```


例: ビルドトリガーが定期実行でも PullRequest でもないとき

```
  condition:
    and
    (
      ne(variables['Build.Reason'], 'Schedule'),
      ne(variables['Build.Reason'], 'PullRequest')
    )
```

##### Steps の設定


```
steps:
- script: choco install "msbuild-sonarqube-runner" -y
  displayName: install msbuild-sonarqube-runner

# Build solution with SonarQube
- script: build-sln.bat       $(BuildPlatform) $(Configuration)
  displayName: Build solution with SonarQube
  
  env:
    SONAR_QUBE_ORG: $(SONAR_QUBE_ORG)
    SONAR_QUBE_PROJECT: $(SONAR_QUBE_PROJECT)
    SONAR_QUBE_TOKEN: $(SONAR_QUBE_TOKEN)
```

1. `choco install "msbuild-sonarqube-runner" -y` を実行して `SonarScanner.MSBuild.exe` をインストールする
2. `script` で `build-sln.bat` を実行する
3. `env` で `環境変数` のところで設定した環境変数が有効になるように設定する。

	[Azure Pipelines の Secret Variable] の項目を参照

### Appveyor の設定

**appveyor で実施する場合、タイムアウトになってビルドに成功しないので現状で使用していない **

Appveyor のプロジェクトで Settings の Environment にアクセスして `Add variable` を押して環境変数を追加する。

|変数名|意味|注意|
|--|--|--|
|SONAR_QUBE_ORG|Sonar Qube のOrganization 識別子||
|SONAR_QUBE_PROJECT|Sonar Qube のプロジェクト識別子||
|SONAR_QUBE_TOKEN|Sonar Qube のアクセスキー (API キー)|追加するとき右の鍵マークを押して秘密の環境変数に設定する|

![環境変数の設定画面](appveyor-SonarQube.png)


#### Secure the GitHub Authentication token

SonarQube で使用するアクセストークンを暗号化するために使用する

[appveyor の Secure Variables]
