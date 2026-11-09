# 与上游同步的流程

Mac 版在 `macos` 分支开发，原版代码（`sakura_core/`、`src/test/`）和上游 [sakura-editor/sakura](https://github.com/sakura-editor/sakura) 共用。为了让 Mac 版能编译，第 15 步把原版代码里的宽字符写法机械地换成了在两个平台上都能用的写法（约 1 万行，Windows 上展开后与原来完全相同，见 [phase1-char-type.md](phase1-char-type.md)）。上游新提交的代码仍然用 `L"…"`、`wchar_t`、`std::wstring` 等写法，所以每次合并后都要再替换一次。

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

5. **确认**：推送到 `macos` 后，GitHub Actions 会自动跑 Windows（MSVC、MinGW）、Cppcheck、Doxygen 和 macOS 的构建与测试，全部通过后才算合并完成。本地可以先用 `cmake -S macos/core -B build/mac-core && cmake --build build/mac-core && ctest --test-dir build/mac-core` 确认 Mac 版。

6. **代码页转换表**（偶尔）：`build-macos.yml` 的 `codepage-dump-windows` 任务每次都在最新的 Windows 上重新导出转换表，与仓库里的 `macos/platform/data/codepages.bin` 不同时会出警告。出现警告时，用那次的导出结果替换仓库里的文件（见 [phase1-charset.md](phase1-charset.md)）。

## 试合并的结果（2026-11-09）

合并上游 `master` 当时最新的 5 个合并提交（到 `4fe60275c`）：没有冲突；合并后第 3 步替换了 3 个文件里的 5 处 `std::wstring_view`。

## 减少分叉

字符类型的替换在 Windows 上不改变编译结果，如果上游愿意接受，以后上游的新代码就会直接用这些写法，合并成本会大幅降低。这件事准备通过上游仓库的 Issue 询问（草稿经项目负责人确认后再发出）。
