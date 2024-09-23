#!/usr/bin/env python
# -*- coding: utf-8 -*-
import ctypes
import os
import time

from appium.webdriver.common.appiumby import AppiumBy
from appium.webdriver.webdriver import WebDriver
from appium.webdriver.webelement import WebElement
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

def test_show_type_list(root_driver, desktop):
    emulate_click('context')

    # メニュー項目F_TYPE_LISTをクリック
    F_TYPE_LIST: Final = '31110'
    context_menu = wait_for_context_menu(desktop,  F_TYPE_LIST)
    context_menu.click()
    WebDriverWait(desktop, 15).until_not(EC.visibility_of_element_located((By.ID, context_menu.id)))

    # 「タイプ別設定一覧」ダイアログが表示されるまで待つ
    type_list = wait_for_dialog(desktop, 'タイプ別設定一覧')

    # 設定変更(S)を選択
    type_list.send_keys(Keys.ALT, 's')
    time.sleep(1)

    # 「タイプ別設定」ダイアログが表示されるまで待つ
    type_prop = wait_for_dialog(desktop, 'タイプ別設定')

    # キャンセルで閉じる
    cancel_element(desktop, type_prop)

def test_show_common_prop(root_driver, desktop):
    emulate_click('context')

    # メニュー項目F_OPTIONをクリック
    F_OPTION: Final = '31112'
    context_menu = wait_for_context_menu(desktop,  F_OPTION)
    context_menu.click()
    WebDriverWait(desktop, 15).until_not(EC.visibility_of_element_located((By.ID, context_menu.id)))

    # 「共通設定」ダイアログが表示されるまで待つ
    common_prop = wait_for_dialog(desktop, '共通設定')

    # キャンセルで閉じる
    cancel_element(desktop, common_prop)

def test_show_about_dialog(root_driver, desktop):
    emulate_click('context')

    # メニュー項目F_ABOUTをクリック
    F_ABOUT: Final = '31455'
    context_menu = wait_for_context_menu(desktop,  F_ABOUT)
    context_menu.click()
    WebDriverWait(desktop, 15).until_not(EC.visibility_of_element_located((By.ID, context_menu.id)))

    # 「バージョン情報」ダイアログが表示されるまで待つ
    about_dialog = wait_for_dialog(desktop, 'バージョン情報')

    # キャンセルで閉じる
    cancel_element(desktop, about_dialog)
