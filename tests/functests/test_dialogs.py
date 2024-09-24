#!/usr/bin/env python
# -*- coding: utf-8 -*-
import os
import time

from appium.webdriver.common.appiumby import AppiumBy
from appium.webdriver.webdriver import WebDriver
from appium.webdriver.webelement import WebElement
from helper import set_foreground_window
from helper import wait_for_context_menu
from helper import wait_for_dialog
from helper import cancel_element
from helper import emulate_click
from selenium.webdriver import ActionChains
from selenium.webdriver.common.by import By
from selenium.webdriver.common.keys import Keys
from selenium.webdriver.support import expected_conditions as EC
from selenium.webdriver.support.ui import WebDriverWait
from typing import Final

def test_show_type_prop(driver, wnd):
    # wndをフォアグラウンドウインドウに設定
    set_foreground_window(wnd)

    wnd.send_keys(Keys.CONTROL, '5')
    time.sleep(1)

    # 「タイプ別設定」ダイアログが表示されるまで待つ
    type_prop = wait_for_dialog(wnd, 'タイプ別設定')

    # キャンセルで閉じる
    cancel_element(wnd, type_prop)

def test_show_common_prop(driver, wnd):
    # wndをフォアグラウンドウインドウに設定
    set_foreground_window(wnd)

    wnd.send_keys(Keys.CONTROL, '6')
    time.sleep(1)

    # 「共通設定」ダイアログが表示されるまで待つ
    common_prop = wait_for_dialog(wnd, '共通設定')

    # キャンセルで閉じる
    cancel_element(wnd, common_prop)
