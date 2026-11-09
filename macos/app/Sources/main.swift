// Copyright (C) 2026, Sakura Editor Organization
//
// SPDX-License-Identifier: Zlib

import AppKit

// 不使用 Storyboard / Nib，菜单和窗口都在代码中构建，
// 以便日后按原版的 MainMenu.ini 动态生成菜单。
let app = NSApplication.shared
let delegate = AppDelegate()
app.delegate = delegate
app.setActivationPolicy(.regular)
app.run()
