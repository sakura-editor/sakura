#!/usr/bin/env python
# -*- coding: utf-8 -*-
import time

from appium.webdriver.common.appiumby import AppiumBy
from appium.webdriver.webdriver import WebDriver
from appium.webdriver.webelement import WebElement
from helper import wait_for_context_menu
from helper import wait_for_dialog
from helper import cancel_element
from helper import emulate_click
from selenium.webdriver.common.by import By
from selenium.webdriver.common.keys import Keys
from selenium.webdriver.support import expected_conditions as EC
from selenium.webdriver.support.ui import WebDriverWait
from typing import Final

def test_show_grep_dialog(desktop):
    # 左クリックでポップアップメニューを表示
    emulate_click('click')

    # メニュー項目F_GREP_DIALOGをクリック
    F_GREP_DIALOG: Final = '30910'
    context_menu = wait_for_context_menu(desktop,  F_GREP_DIALOG)
    context_menu.click()
    WebDriverWait(desktop, 15).until_not(EC.visibility_of_element_located((By.ID, context_menu.id)))

    # Grepダイアログが表示されるまで待つ
    grep_dialog = wait_for_dialog(desktop, 'Grep')

    # キャンセルで閉じる
    cancel_element(desktop, grep_dialog)

def test_show_favorite_dialog(desktop):
    # 左クリックでポップアップメニューを表示
    emulate_click('click')

    # メニュー項目F_FAVORITEをクリック
    F_FAVORITE: Final = '31113'
    context_menu = wait_for_context_menu(desktop,  F_FAVORITE)
    context_menu.click()
    WebDriverWait(desktop, 15).until_not(EC.visibility_of_element_located((By.ID, context_menu.id)))

    # 「履歴とお気に入りの管理」ダイアログが表示されるまで待つ
    favorite_dialog = wait_for_dialog(desktop, '履歴とお気に入りの管理')

    # キャンセルで閉じる
    cancel_element(desktop, favorite_dialog)
