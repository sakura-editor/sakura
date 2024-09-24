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

def test_select_menu_by_key(driver, wnd):
    # wndをフォアグラウンドウインドウに設定
    set_foreground_window(wnd)

    wnd.send_keys(Keys.ALT, 'h')
    time.sleep(1)
    wnd.send_keys('a')

    # 「バージョン情報」ダイアログが表示されるまで待つ
    about_dialog = wait_for_dialog(wnd, 'バージョン情報')

    # キャンセルで閉じる
    cancel_element(wnd, about_dialog)
