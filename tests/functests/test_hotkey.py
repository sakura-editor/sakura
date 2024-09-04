#!/usr/bin/env python
# -*- coding: utf-8 -*-
import time

from appium.webdriver.common.appiumby import AppiumBy
from appium.webdriver.webdriver import WebDriver
from appium.webdriver.webelement import WebElement
from helper import wait_for_context_menu
from helper import wait_for_dialog
from helper import cancel_element
from selenium.webdriver.common.by import By
from selenium.webdriver.common.keys import Keys
from selenium.webdriver.support import expected_conditions as EC
from selenium.webdriver.support.ui import WebDriverWait
from typing import Final

def test_show_and_cancel(desktop: WebElement):
    # ホットキーでポップアップメニューを表示
    desktop.send_keys(Keys.CONTROL + Keys.ALT, 'z')

    context_menu = WebDriverWait(desktop, 120).until(EC.visibility_of_element_located((By.CLASS_NAME, '#32768'))) # ContextMenu
    time.sleep(1)

    # ESCキーを押下してポップアップメニューを閉じる
    cancel_element(desktop, context_menu)

def test_show_grep_dialog(desktop: WebElement):
    # ホットキーでポップアップメニューを表示
    desktop.send_keys(Keys.CONTROL + Keys.ALT, 'z')

    # メニュー項目F_GREP_DIALOGをクリック
    F_GREP_DIALOG: Final = '30910'
    context_menu = wait_for_context_menu(desktop,  F_GREP_DIALOG)
    context_menu.click()
    WebDriverWait(desktop, 120).until_not(EC.visibility_of_element_located((By.ID, context_menu.id)))

    # Grepダイアログが表示されるまで待つ
    grep_dialog = wait_for_dialog(desktop, 'Grep')

    # キャンセルで閉じる
    cancel_element(desktop, grep_dialog)

def test_show_favorite_dialog(desktop: WebElement):
    # ホットキーでポップアップメニューを表示
    desktop.send_keys(Keys.CONTROL + Keys.ALT, 'z')

    # メニュー項目F_FAVORITEをクリック
    F_FAVORITE: Final = '31113'
    context_menu = wait_for_context_menu(desktop,  F_FAVORITE)
    context_menu.click()
    WebDriverWait(desktop, 120).until_not(EC.visibility_of_element_located((By.ID, context_menu.id)))

    # 「履歴とお気に入りの管理」ダイアログが表示されるまで待つ
    favorite_dialog = wait_for_dialog(desktop, '履歴とお気に入りの管理')

    # キャンセルで閉じる
    cancel_element(desktop, favorite_dialog)
