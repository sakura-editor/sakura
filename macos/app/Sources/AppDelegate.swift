// Copyright (C) 2026, Sakura Editor Organization
//
// SPDX-License-Identifier: Zlib

import AppKit

final class AppDelegate: NSObject, NSApplicationDelegate {
    private var windowControllers: [EditorWindowController] = []

    func applicationDidFinishLaunching(_ notification: Notification) {
        NSApp.mainMenu = MainMenuBuilder.build()
        newEditorWindow(nil)
        NSApp.activate(ignoringOtherApps: true)
    }

    // 原版在最后一个编辑窗口关闭时退出（不常驻任务托盘），这里保持一致。
    func applicationShouldTerminateAfterLastWindowClosed(_ sender: NSApplication) -> Bool {
        true
    }

    @objc func newEditorWindow(_ sender: Any?) {
        let controller = EditorWindowController()
        windowControllers.append(controller)
        controller.onClose = { [weak self, weak controller] in
            self?.windowControllers.removeAll { $0 === controller }
        }
        controller.showWindow(nil)
    }
}
