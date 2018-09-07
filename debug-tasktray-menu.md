
# タスクトレイのメニュー項目のデバッグ方法

<!-- TOC -->

- [タスクトレイのメニュー項目のデバッグ方法](#タスクトレイのメニュー項目のデバッグ方法)
    - [手順](#手順)
        - [**左クリック** した場合の該当コード](#左クリック-した場合の該当コード)
        - [**右クリック** した場合の該当コード](#右クリック-した場合の該当コード)

<!-- /TOC -->

## 手順

1. Visual Studio から デバッグなしで sakura.exe を起動します。
2. ウィンドウを最小化します。
3. `無題` というウィンドウがない方のプロセスにアタッチします。
4. タスクトレイでサクラエディタのアイコンを **左クリック** または **右クリック** します。(どちらを行うかによってメニュー項目は異なります)
5. CControlTray::DispatchEvent の `WM_LBUTTONUP` に制御が来ます。
6. テストしたい項目を選びます。
7. CControlTray::DispatchEvent の `CreatePopUpMenu_L` または `CreatePopUpMenu_R` が制御を返します。

### **左クリック** した場合の該当コード

以下の部分です。

https://github.com/sakura-editor/sakura/blob/4aa227c4aec130e1faa6a0f7da47b23fa0ca0e31/sakura_core/_main/CControlTray.cpp#L859-L875

例えば grep を行う場合は以下の部分です。

https://github.com/sakura-editor/sakura/blob/4aa227c4aec130e1faa6a0f7da47b23fa0ca0e31/sakura_core/_main/CControlTray.cpp#L910-L913

### **右クリック** した場合の該当コード

以下の部分です。

https://github.com/sakura-editor/sakura/blob/4aa227c4aec130e1faa6a0f7da47b23fa0ca0e31/sakura_core/_main/CControlTray.cpp#L766-L775
