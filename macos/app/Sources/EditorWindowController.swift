// Copyright (C) 2026, Sakura Editor Organization
//
// SPDX-License-Identifier: Zlib

import AppKit

/// 编辑窗口的空壳。
///
/// 文本区域目前只是占位视图；阶段 2 会换成移植自原版的自绘文本视图
/// （Core Text 绘制 + NSTextInputClient 输入法支持）。
final class EditorWindowController: NSWindowController, NSWindowDelegate {
    var onClose: (() -> Void)?

    convenience init() {
        let window = NSWindow(
            contentRect: NSRect(x: 0, y: 0, width: 800, height: 600),
            styleMask: [.titled, .closable, .miniaturizable, .resizable],
            backing: .buffered,
            defer: false
        )
        window.title = "(無題)"
        window.contentView = PlaceholderEditorView()
        window.cascadeTopLeft(from: NSPoint(x: 120, y: 120))
        self.init(window: window)
        window.delegate = self
    }

    func windowWillClose(_ notification: Notification) {
        onClose?()
    }
}

/// 占位视图：白底，左侧留出行号区域，只显示一行 [EOF]。
final class PlaceholderEditorView: NSView {
    override var isFlipped: Bool { true }

    override func draw(_ dirtyRect: NSRect) {
        NSColor.textBackgroundColor.setFill()
        bounds.fill()

        let gutter = NSRect(x: 0, y: 0, width: 48, height: bounds.height)
        NSColor.windowBackgroundColor.setFill()
        gutter.fill()

        let font = NSFont.monospacedSystemFont(ofSize: 13, weight: .regular)
        let attributes: [NSAttributedString.Key: Any] = [
            .font: font,
            .foregroundColor: NSColor.secondaryLabelColor,
        ]
        ("1" as NSString).draw(at: NSPoint(x: 28, y: 6), withAttributes: attributes)
        ("[EOF]" as NSString).draw(at: NSPoint(x: 56, y: 6), withAttributes: attributes)
    }
}
