# 与上游同步的流程

Mac 版在 `macos` 分支开发，原版代码（`sakura_core/`、`src/test/`）和上游 [sakura-editor/sakura](https://github.com/sakura-editor/sakura) 共用。为了让 Mac 版能编译，第 15 步把原版代码里的宽字符写法机械地换成了在两个平台上都能用的写法（约 1 万行，Windows 上展开后与原来完全相同，见 [phase1-char-type.md](phase1-char-type.md)）。上游新提交的代码仍然用 `L"…"`、`wchar_t`、`std::wstring` 等写法，所以每次合并后都要再替换一次。

> **当前状态**：2026-11-09 按项目负责人的要求，把上游 `master` 到 `4fe60275c` 为止的全部更新合并进了 `macos` 分支（见下面“合并记录”）。`macos` 分支不自动运行 Windows 构建。之后上游的新更新按下面的流程合并；最晚在计划第 50 步“核对对照表”时再合并一次，让 Mac 版与最新的 Windows 版一致。

## 步骤

1. **更新 `master`**：把上游的 `master` 拉到 fork 的 `master`（GitHub 网页上的 “Sync fork”，或下面的命令）。`master` 不放 Mac 版的修改。

   ```sh
   git remote add upstream https://github.com/sakura-editor/sakura.git   # 第一次
   git fetch upstream master
   git checkout master && git merge --ff-only upstream/master && git push origin master
   ```

2. **合并到 `macos`**：

   ```sh
   git checkout macos
   git merge master
   ```

   有冲突时，冲突几乎都是“上游改了某行，Mac 版把同一行的 `L"…"` 换成了 `LTEXT("…")`”。按上游的内容解决，然后在第 3 步重新替换即可。

3. **重新替换字符类型**：

   ```sh
   python3 macos/tools/convert_wchar.py sakura_core
   python3 macos/tools/convert_wchar.py $(grep -o 'src/test/cpp/tests1/[^ ]*\.cpp' macos/core/tests/tests.cmake)
   ```

   第二行只替换 Mac 版已经在用的测试文件（列在 `macos/core/tests/tests.cmake` 里）。其他测试文件在加入 Mac 版时再替换。`--check` 只统计、不写回。

4. **检查工具替换不了的写法**：`std::wregex`、`std::to_wstring`、`std::wostringstream`、`path.wstring()`、`std::format(L"…")`。

   ```sh
   grep -rnE 'std::w(regex|smatch|cmatch|ostringstream|stringstream)|std::to_wstring|\.wstring\(\)|std::format\(' sakura_core
   ```

   新出现的、在 Mac 版已编译的文件里的，要改成 `sakura::format`、`sakura::path_to_wstring` 等（见 phase1-char-type.md 的“留到后续步骤”）；在还没编入 Mac 版的文件里的，记到那个表里。

5. **确认**：推送到 `macos` 后，GitHub Actions 自动跑 macOS 的构建与测试。Windows（MSVC、MinGW）、Cppcheck、Doxygen 目前不自动运行，需要确认 Windows 版时在 Actions 页面手动运行（`gh workflow run build-sakura.yml --ref macos` 等）。本地可以先用 `cmake -S macos/core -B build/mac-core && cmake --build build/mac-core && ctest --test-dir build/mac-core` 确认 Mac 版。

6. **代码页转换表**（偶尔）：`build-macos.yml` 的 `codepage-dump-windows` 任务每次都在最新的 Windows 上重新导出转换表，与仓库里的 `macos/platform/data/codepages.bin` 不同时会出警告。出现警告时，用那次的导出结果替换仓库里的文件（见 [phase1-charset.md](phase1-charset.md)）。

## 合并记录

| 日期 | 合并到的上游提交 | 冲突 | 第 3 步替换 | 确认 |
|---|---|---|---|---|
| 2026-11-09 | `4fe60275c`（上一次的共同祖先 `52a7f1859` 之后的 21 个提交，其中 13 个是普通提交） | 无 | 3 个文件 5 处 `std::wstring_view`（`CDropTarget.cpp`、`CDropTarget.h`、`CEditView_Mouse.cpp`） | 第 4 步的检查没有新出现的写法；Mac 核心库构建通过，595 个测试全部通过 |

注意：仓库是浅克隆时 `git merge-base` 找不到真正的共同祖先，合并前先用 `git fetch --deepen=300 origin macos` 和 `git fetch --deepen=300 upstream master` 补全历史。

## 减少分叉

字符类型的替换在 Windows 上不改变编译结果，如果上游愿意接受，以后上游的新代码就会直接用这些写法，合并成本会大幅降低。是否询问上游，项目负责人决定暂不联系（2026-11-09），在计划第 53 步之前与名字、图标的事一并联系。在此之前按上面的流程跟进上游。
