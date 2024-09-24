#!/usr/bin/env python
# -*- coding: utf-8 -*-
import ctypes
import os
import time
import subprocess

from appium.webdriver.common.appiumby import AppiumBy
from appium.webdriver.webelement import WebElement
from selenium.webdriver.common.by import By
from selenium.webdriver.common.keys import Keys
from selenium.webdriver.support import expected_conditions as EC
from selenium.webdriver.support.ui import WebDriverWait
from typing import Final

appium_host: Final = '127.0.0.1'
appium_port: Final = 4723
appium_url: Final = f'http://{appium_host}:{appium_port}'
text_editor_window_classname: Final = 'TextEditorWindowM64WP_DEBUG179'

# Windows API 関数の定義
user32 = ctypes.windll.user32

def set_foreground_window(wnd):
    hwnd = wnd.get_property("handle")
    user32.SetForegroundWindow(hwnd)

def get_win_handle(win: WebElement) -> str:
    handle_value = win.get_attribute('NativeWindowHandle')
    win_handle = format(int(handle_value), 'x') 
    return win_handle

def wait_for_context_menu(desktop: WebElement, func_code) -> WebElement:
    context_menu = WebDriverWait(desktop, 120).until(EC.visibility_of_element_located((By.CLASS_NAME, '#32768'))) # ContextMenu
    menu_item = WebDriverWait(context_menu, 120).until(EC.visibility_of_element_located((AppiumBy.ACCESSIBILITY_ID, str(func_code))))
    time.sleep(1)
    return menu_item

def wait_for_dialog(desktop: WebElement, title) -> WebElement:
    dialog = WebDriverWait(desktop, 120).until(EC.presence_of_element_located((By.XPATH, f'//Window[@ClassName="#32770"][@Name="{title}"]')))
    time.sleep(1)
    return dialog

def cancel_element(desktop: WebElement, dialog: WebElement):
    dialog.send_keys(Keys.ESCAPE)
    WebDriverWait(desktop, 120).until_not(EC.visibility_of_element_located((By.ID, dialog.id)))
    time.sleep(1)

def emulate_click(action: str, count: int = 1):
    # TrayClickEmulator.exe を実行
    tray_click_emulator_path = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', 'build', 'vcpkg_installed', 'x64-windows-static', 'tools', 'click-emu', 'TrayClickEmulator.exe'))
    subprocess.run([tray_click_emulator_path, '--action', action, '--count', str(count)])
