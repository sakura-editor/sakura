# Copyright (C) 2026, Sakura Editor Organization
# SPDX-License-Identifier: Zlib
import json, re, csv, sys, collections
rows = json.load(open(sys.argv[1])); out = sys.argv[2]
D, G, X = '直接移植', '需要改造', '沙盒下不可行'
M, P, O = 'MVP', '追平', '可选'
SEC_ZH = {'ファイル操作系':'文件操作','編集系':'编辑','カーソル移動系':'光标移动','選択系':'选择','矩形選択系':'矩形选择','クリップボード系':'剪贴板','挿入系':'插入','変換系':'转换','検索系':'查找','モード切り替え系':'模式切换','設定系':'设置','マクロ系':'宏','ウィンドウ系':'窗口','支援':'辅助','カスタムメニュー':'自定义菜单','間接呼び出し':'宏专用命令','マクロ関数':'宏函数'}
SEC_DEF = {'ファイル操作系':(D,M),'編集系':(D,M),'カーソル移動系':(D,M),'選択系':(D,M),'矩形選択系':(D,M),'クリップボード系':(D,M),'挿入系':(D,P),'変換系':(D,P),'検索系':(D,P),'モード切り替え系':(D,M),'設定系':(G,P),'マクロ系':(G,P),'ウィンドウ系':(G,P),'支援':(G,P),'カスタムメニュー':(G,P),'間接呼び出し':(D,P),'マクロ関数':(D,P)}
ZH = {'F_FILESAVEAS':'另存为（宏用）','F_FILESAVE_QUIET':'保存（限定动作）','F_ADDTAIL_W':'在末尾追加文本','F_CTRL_CODE':'输入控制码','F_TAGJUMP_LIST':'标签跳转列表','F_CHGMOD_EOL_NEL':'换行符类型(NEL)','F_CHGMOD_EOL_PS':'换行符类型(PS)','F_CHGMOD_EOL_LS':'换行符类型(LS)',
'F_TYPE_SCREEN':'文件类型设置「屏幕」','F_TYPE_COLOR':'文件类型设置「颜色」','F_TYPE_WINDOW':'文件类型设置「窗口」','F_TYPE_HELPER':'文件类型设置「辅助」','F_TYPE_REGEX_KEYWORD':'文件类型设置「正则关键字」','F_TYPE_KEYHELP':'文件类型设置「关键字帮助」',
'F_OPTION_GENERAL':'通用设置「常规」','F_OPTION_WINDOW':'通用设置「窗口」','F_OPTION_EDIT':'通用设置「编辑」','F_OPTION_FILE':'通用设置「文件」','F_OPTION_BACKUP':'通用设置「备份」','F_OPTION_FORMAT':'通用设置「格式」','F_OPTION_GREP':'通用设置「Grep」','F_OPTION_KEYBIND':'通用设置「快捷键」','F_OPTION_CUSTMENU':'通用设置「自定义菜单」','F_OPTION_TOOLBAR':'通用设置「工具栏」','F_OPTION_KEYWORD':'通用设置「强调关键字」','F_OPTION_HELPER':'通用设置「辅助」','F_OPTION_MACRO':'通用设置「宏」','F_OPTION_FNAME':'通用设置「文件名显示」','F_OPTION_TAB':'通用设置「标签页」','F_OPTION_STATUSBAR':'通用设置「状态栏」','F_OPTION_PLUGIN':'通用设置「插件」','F_OPTION_MAINMENU':'通用设置「主菜单」',
'F_WINMAXIMIZE':'最大化编辑窗口','F_WINMINIMIZE':'最小化编辑窗口','F_WINRESTORE':'还原编辑窗口',
'F_SET_QUOTESTRING':'设置引用符','F_TRACEOUT':'输出到宏输出窗口','F_PUTFILE':'临时输出当前文件','F_INSFILE':'在光标处插入文件','F_INSBOXTEXT':'插入矩形文本','F_MOVECURSOR':'移动光标','F_MOVECURSORLAYOUT':'移动光标（按布局）','F_STATUSMSG':'状态栏消息','F_MSGBEEP':'提示音','F_CHANGETYPE':'临时应用文件类型设置','F_FILEOPEN2':'打开2（宏用）','F_COMMITUNDOBUFFER':'提交撤销块','F_ADDREFUNDOBUFFER':'撤销块引用+1','F_SETUNDOBUFFER':'释放撤销块','F_APPENDUNDOBUFFERCURSOR':'撤销块记录光标位置','F_CLIPBOARDEMPTY':'清空剪贴板','F_SETVIEWTOP':'设置视图首行','F_SETVIEWLEFT':'设置视图左端列',
'F_GETFILENAME':'获取当前文件名','F_GETSAVEFILENAME':'获取保存时文件名','F_GETSELECTED':'获取选中文本','F_EXPANDPARAMETER':'展开命令行参数','F_GETLINESTR':'获取指定行文本','F_GETLINECOUNT':'获取行数','F_CHGTABWIDTH':'获取/设置 Tab 宽度','F_ISTEXTSELECTED':'是否有选中文本','F_GETSELLINEFROM':'获取选择起始行','F_GETSELCOLUMNFROM':'获取选择起始列','F_GETSELLINETO':'获取选择结束行','F_GETSELCOLUMNTO':'获取选择结束列','F_ISINSMODE':'获取插入/覆盖模式','F_GETCHARCODE':'获取字符编码','F_GETLINECODE':'获取换行符','F_ISPOSSIBLEUNDO':'能否撤销','F_ISPOSSIBLEREDO':'能否重做','F_CHGWRAPCOLUMN':'获取/设置折行列','F_ISCURTYPEEXT':'扩展名是否属于当前类型','F_ISSAMETYPEEXT':'两个扩展名是否同类型','F_INPUTBOX':'文本输入对话框','F_MESSAGEBOX':'消息框','F_ERRORMSG':'错误消息框','F_WARNMSG':'警告消息框','F_INFOMSG':'信息消息框','F_OKCANCELBOX':'确认框（确定/取消）','F_YESNOBOX':'确认框（是/否）','F_COMPAREVERSION':'比较版本号','F_MACROSLEEP':'暂停指定毫秒','F_FILEOPENDIALOG':'打开文件对话框','F_FILESAVEDIALOG':'保存文件对话框','F_FOLDERDIALOG':'选择文件夹对话框','F_GETCLIPBOARD':'获取剪贴板文本','F_SETCLIPBOARD':'设置剪贴板文本','F_LAYOUTTOLOGICLINENUM':'获取逻辑行号','F_LOGICTOLAYOUTLINENUM':'获取布局行号','F_LINECOLUMNTOINDEX':'获取逻辑列号','F_LINEINDEXTOCOLUMN':'获取布局列号','F_GETCOOKIE':'获取 Cookie','F_GETCOOKIEDEFAULT':'获取 Cookie（带默认值）','F_SETCOOKIE':'设置 Cookie','F_DELETECOOKIE':'删除 Cookie','F_GETCOOKIENAMES':'获取 Cookie 名称','F_SETDRAWSWITCH':'设置重绘开关','F_GETDRAWSWITCH':'获取重绘开关','F_ISSHOWNSTATUS':'状态栏是否显示','F_GETSTRWIDTH':'获取字符串宽度','F_GETSTRLAYOUTLENGTH':'获取字符串布局宽度','F_GETDEFAULTCHARLENGTH':'获取默认字符宽度','F_ISINCLUDECLIPBOARDFORMAT':'剪贴板是否含指定格式','F_GETCLIPBOARDBYFORMAT':'按格式获取剪贴板','F_SETCLIPBOARDBYFORMAT':'按格式设置剪贴板','F_GETLINEATTRIBUTE':'获取行属性','F_ISTEXTSELECTINGLOCK':'获取选择锁定状态','F_GETVIEWLINES':'获取视图行数','F_GETVIEWCOLUMNS':'获取视图列数','F_CREATEMENU':'创建菜单','F_GETVIEWTOP':'获取视图首行行号','F_CHGINSSPACE':'获取/设置插入空格'}
OV = {}
def ov(names, cat, pri, note=''):
    for n in names.split(): OV[n] = (cat, pri, note)
ov('F_ACTIVATE_SQLPLUS F_PLSQL_COMPILE_ON_SQLPLUS', X, O, '依赖 Windows 版 SQL*Plus 的窗口消息')
ov('F_OPEN_COMMAND_PROMPT F_OPEN_COMMAND_PROMPT_AS_ADMIN F_OPEN_POWERSHELL F_OPEN_POWERSHELL_AS_ADMIN', X, O, 'Windows 专有；沙盒内不能启动终端，可在 App Store 外版本改为“在终端中打开”')
ov('F_EXECMD_DIALOG F_EXECMD', X, O, '沙盒禁止执行任意外部程序；可在 App Store 外版本提供')
ov('F_EXTHTMLHELP', X, O, '依赖 Windows HTML Help（CHM）')
ov('F_OPTION_PLUGIN', X, O, 'DLL 插件无法加载；脚本插件可考虑以后用 JS 实现')
ov('F_TAGS_MAKE', G, O, '原版调用 ctags.exe；需内置 ctags，否则沙盒下不可行')
ov('F_FILEOPEN F_FILESAVEAS_DIALOG F_FILEOPEN_DROPDOWN F_FILECLOSE_OPEN', G, M, '改用系统文件面板，配合沙盒权限')
ov('F_FILENEW_NEWWINDOW', G, M, '改为单进程多窗口')
ov('F_PRINT F_PRINT_PREVIEW F_PRINT_PAGESETUP', G, P, '改用 NSPrintOperation')
ov('F_OPEN_HfromtoC', G, P, '沙盒下需要该文件夹的访问权限')
ov('F_BROWSE', G, P, '改用系统默认浏览器打开')
ov('F_PROPERTY_FILE', G, P, '')
ov('F_PROFILEMGR', G, O, '原版按进程切换配置；需重新设计')
ov('F_EXITALLEDITORS F_EXITALL', G, P, '单进程后语义变为退出 App')
ov('F_OPEN_FOLDER_IN_EXPLORER', G, P, '改为“在访达中显示”')
ov('F_VIEWMODE', D, P)
ov('F_IME_CHAR', G, M, '依赖输入法接口 NSTextInputClient')
ov('F_RECONVERT', G, P, '输入法重新转换，需 NSTextInputClient 支持')
ov('F_COPY_COLOR_HTML F_COPY_COLOR_HTML_LINENUMBER', G, P, '改用 Mac 剪贴板的 HTML 格式')
ov('F_COPYLINES F_COPYLINESASPASSAGE F_COPYLINESWITHLINENUMBER F_COPYPATH F_COPYTAG F_COPYFNAME F_COPYDIRPATH F_CREATEKEYBINDLIST F_COPY_ADDCRLF F_COPY_CRLF F_ADDTAIL_W', D, P)
ov('F_CTRL_CODE_DIALOG', G, P, '对话框重写')
ov('F_SEARCH_DIALOG F_REPLACE_DIALOG F_JUMP_DIALOG', G, M, '对话框重写')
ov('F_SEARCH_NEXT F_SEARCH_PREV F_REPLACE F_REPLACE_ALL F_SEARCH_CLEARMARK F_JUMP_SRCHSTARTPOS F_JUMP', D, M)
ov('F_GREP_DIALOG F_GREP F_GREP_REPLACE_DLG F_GREP_REPLACE', G, P, '只能搜索用户授权过的文件夹')
ov('F_OUTLINE F_OUTLINE_TOGGLE F_BOOKMARK_VIEW F_SEARCH_BOX', G, P, '窗口或面板重写')
ov('F_TAGJUMP F_TAGJUMPBACK F_TAGJUMP_LIST F_DIRECT_TAGJUMP F_TAGJUMP_CLOSE F_TAGJUMP_KEYWORD', G, P, '读取 tags 文件需要文件夹权限')
ov('F_COMPARE', G, P, '原版与另一进程的窗口比较，改为单进程内比较')
ov('F_DIFF_DIALOG F_DIFF', G, P, '原版调用 diff.exe，需内置差异算法')
ov('F_ISEARCH_MIGEMO_NEXT F_ISEARCH_MIGEMO_PREV', G, O, '需随 App 打包 Migemo 及词典')
ov('F_FILETREE', G, P, '只能显示用户授权过的文件夹')
ov('F_CHG_CHARSET', G, M, '对话框重写')
ov('F_FONT', G, M, '改用系统字体面板')
ov('F_SETFONTSIZE F_SETFONTSIZEUP F_SETFONTSIZEDOWN F_WRAPWINDOWWIDTH F_TMPWRAPNOWRAP F_TMPWRAPSETTING F_TMPWRAPWINDOW', D, M)
ov('F_SELECT_COUNT_MODE', D, P)
ov('F_SHOWFUNCKEY', G, O, '功能键栏在 Mac 上用处不大')
ov('F_RECKEYMACRO F_EXECKEYMACRO', D, P)
ov('F_SAVEKEYMACRO F_LOADKEYMACRO', G, P, '改用系统文件面板')
ov('F_EXECEXTMACRO', G, P, '只支持键盘宏和 JS 宏；PPA、VBScript、Python 宏不可行')
ov('F_CASCADE F_TILE_V F_TILE_H F_MAXIMIZE_V F_MAXIMIZE_H F_MINIMIZE_ALL', G, O, 'Mac 窗口管理习惯不同')
ov('F_REDRAW', D, P)
ov('F_BIND_WINDOW', G, P, '改用 Mac 原生标签页')
ov('F_HOKAN', D, P)
ov('F_EXTHELP1', G, O, '改为用默认程序打开外部帮助')
ov('F_ABOUT', G, M, '必须显示 zlib 许可证和原作者')
ov('F_INPUTBOX F_MESSAGEBOX F_ERRORMSG F_WARNMSG F_INFOMSG F_OKCANCELBOX F_YESNOBOX F_FILEOPENDIALOG F_FILESAVEDIALOG F_FOLDERDIALOG F_CREATEMENU', G, P, '对话框和菜单改用 AppKit')
ov('F_GETCLIPBOARDBYFORMAT F_SETCLIPBOARDBYFORMAT F_ISINCLUDECLIPBOARDFORMAT', G, P, 'Windows 剪贴板格式需映射到 Mac')
ov('F_BASE64DECODE F_UUDECODE', G, O, '保存时改用系统文件面板')
ov('F_PUTFILE F_INSFILE F_FILEOPEN2', G, P, '沙盒下只能访问已授权的文件')
ov('F_WordDeleteToStart F_WordDeleteToEnd F_WordCut F_WordDelete F_LineCutToStart F_LineCutToEnd F_LineDeleteToStart F_LineDeleteToEnd F_CUT_LINE F_DELETE_LINE F_DUPLICATELINE F_INDENT_SPACE F_UNINDENT_SPACE F_LTRIM F_RTRIM F_SORT_ASC F_SORT_DESC F_MERGE', D, P)
skip = {'F_CUSTMENU_BASE','F_USERMACRO_0'}
out_rows = []
for r in rows:
    if r['name'] in skip: continue
    cat, pri = SEC_DEF[r['sec']]; note = ''
    if r['name'] in OV: cat, pri, note = OV[r['name']]
    ja = re.sub(r'\s+(//.*|\d{4}[./].*|\(?[A-Z][a-z]{2}\. \d.*)$', '', r['ja']).strip()
    zh = r['zh'] or ZH.get(r['name'], '')
    m = re.match(r'F_CUSTMENU_(\d+)$', r['name'])
    if m: zh = zh or f'自定义菜单{m.group(1)}'
    out_rows.append(['命令', SEC_ZH[r['sec']], r['name'], r['code'], ja, zh, cat, pri, note])
EXTRA = [
 ('编码','字符编码自动识别与转换（SJIS、JIS、EUC、UTF-7/8/16、CESU-8、Latin1 等）',D,M,'用 uchardet 静态库'),
 ('文件','打开/保存时保持 BOM 与换行符',D,M,''),
 ('文件','最近打开的文件和文件夹',G,M,'用安全书签保存访问权限'),
 ('文件','自动保存',D,P,''),
 ('文件','保存时备份',G,P,'备份文件夹需要用户授权'),
 ('文件','外部修改后自动重新载入',G,P,'改用 macOS 文件系统事件'),
 ('文件','文件独占锁',G,O,'Windows 文件共享模式与 macOS 不同'),
 ('界面','语法高亮与文件类型设置',D,M,''),
 ('界面','行号、折返、Tab/全角空格/换行符标记',G,M,'排版逻辑移植，绘制改用 Core Text'),
 ('界面','工具栏、状态栏、标签栏、缩略图',G,P,''),
 ('界面','日、英、中三种界面语言',G,P,'从 .rc 和语言包转换'),
 ('架构','多进程 + 共享内存（窗口间同步设置）',G,M,'改为单进程多窗口'),
 ('架构','任务托盘常驻进程',X,O,'Mac 无任务托盘；可改为菜单栏图标，非必需'),
 ('架构','命令行参数（-X、-Y、-CODE 等）',G,O,'App Store 版通过 open 命令传参受限'),
 ('设置','sakura.ini 读写与导入',G,M,'格式保持兼容'),
 ('设置','键位设置的导入导出',G,P,''),
 ('宏','键盘宏（.mac）',D,P,''),
 ('宏','JScript 宏（.js）',G,P,'改用 JavaScriptCore 实现 Editor 对象'),
 ('宏','VBScript 宏（.vbs）',X,O,'依赖 Windows 脚本宿主'),
 ('宏','PPA 宏（.ppa）',X,O,'依赖 Windows 专有 DLL'),
 ('宏','Python 宏',X,O,'依赖外部 Python DLL'),
 ('插件','插件（DLL / WSH 插件）',X,O,'可考虑以后提供 JS 插件'),
 ('外部模块','正则表达式（bregonig.dll）',G,M,'改为静态链接 Onigmo'),
 ('外部模块','Migemo 罗马字搜索',G,O,'需打包词典'),
 ('辅助','关键字帮助、补全词典',D,P,''),
 ('辅助','HTML 帮助（CHM）',G,P,'改写为内置 HTML 帮助'),
]
for a,b,c,d,e in EXTRA: out_rows.append(['非命令功能', a, '', '', '', b, c, d, e])
hdr = ['种类','分组','命令名','编号','日文名（原版）','中文名','分类','优先级','说明']
with open(out + '/feature-matrix.csv', 'w', encoding='utf-8-sig', newline='') as f:
    w = csv.writer(f); w.writerow(hdr); w.writerows(out_rows)
c = collections.Counter((r[6]) for r in out_rows); p = collections.Counter(r[7] for r in out_rows)
print(len(out_rows), c, p, sum(1 for r in out_rows if not r[5]))
