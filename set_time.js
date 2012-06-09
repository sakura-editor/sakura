//
// keywordファイル時刻設定スクリプト
//
// 使い方：
//   コマンドプロンプトを開き、trunk2ディレクトリで cscript set_time.js を実行
//   またはファイルを右クリックして「開く」
// 注意：
//   SVNからチェックアウトする際、ファイルの更新日時が最後にコミットした日時に
//   なるようにすること（TortoiseSVNの設定）
//
var sh = new ActiveXObject('Shell.Application');

// SVN初回登録日より後のファイルはSVNの時刻を使用する
//var limit = new Date('2012/06/03 17:00');

var message = '';

function touch(path, date) {
	var d = sh.NameSpace(path.replace(/\\[^\\]+$/, ""));
	var fn = path.replace(/.*\\/, "")
	var f = d.ParseName(fn);
//	if (f.ModifyDate > limit) {
//		message += (fn + ' 日付が新しいため変更しません\n');
//	} else {
		f.ModifyDate = date;
//	}
}

// ファイル名とタイムスタンプのマップ
var dates = {
	'ABAP\\ABAP4.kwd': '2003/03/10 23:29:10',
	'ActionScript\\ActionScript.kwd': '2007/10/19 21:42:08',
	'AutoHotKey\\AHK.col': '2005/07/22 16:12:06',
	'AutoHotKey\\AHK.khp': '2005/07/22 16:43:20',
	'AutoHotKey\\AHK.rkw': '2005/07/22 16:12:28',
	'AviSynth\\AviSynth.col': '2006/05/10 23:29:08',
	'AviSynth\\AviSynth.kwd': '2007/10/19 22:03:16',
	'AviSynth\\AviSynth_Others.kwd': '2006/05/10 23:28:28',
	'AviSynth\\AviSynth_Plugins.kwd': '2006/05/10 23:51:04',
	'AviSynth\\AviSynth_Properties.kwd': '2006/03/04 01:07:04',
	'AviSynth\\AviSynth_Script.kwd': '2006/03/04 01:07:42',
	'AWK\\awk.kwd': '1999/11/26 11:18:32',
	'Batch\\bat.khp': '2003/08/02 23:09:54',
	'Batch\\bat.rl': '2003/08/02 23:09:16',
	'Batch\\batch.kwd': '2001/11/15 09:08:56',
	'Batch\\bat_win2k.khp': '2003/08/07 16:46:46',
	'C#\\csharp.kwd': '2010/03/06 12:02:00',
	'C#\\csharp-context.kwd': '2012/06/09 14:27:14',
	'C++\\cpp.kwd': '2003/10/13 01:36:46',
	'COBOL\\COBOL.kwd': '2001/11/09 19:54:20',
	'CORBA\\corba.kwd': '1999/10/05 08:56:48',
	'CSS\\CSS2-GPL.txt': '2006/07/10 19:24:22',
	'CSS\\CSS2-readme.txt': '2006/09/15 11:06:06',
	'CSS\\CSS2.input.kwd': '2006/09/15 11:06:14',
	'CSS\\CSS2.khp': '2006/09/15 10:54:16',
	'CSS\\CSS2.kwd': '2006/09/15 10:59:10',
	'D\\d.kwd': '2007/10/19 21:50:20',
	'gnuplot\\gnuplot.kwd': '2011/02/28 20:32:22',
	'HSP\\HSP.col': '2005/05/23 18:08:06',
	'HSP\\HSP.KHP': '2002/08/01 09:45:30',
	'HSP\\HSP.KWD': '2005/05/23 17:56:26',
	'HSP\\HSP.otl': '2005/05/23 18:31:48',
	'HSP\\HSP.rkw': '2005/05/23 18:31:30',
	'HSP\\HSP.RL': '2002/08/01 09:05:26',
	'HTML\\html.kwd': '2012/06/09 19:28:55',
	'HTML\\html5.kwd': '2012/06/09 19:29:07',
	'HTML\\mathml2.kwd': '2002/12/04 04:59:22',
	'Java\\java.kwd': '2003/03/08 23:33:36',
	'JavaScript\\ecmascript.kwd': '2005/02/17 00:04:56',
	'JavaScript\\JavaScript.kwd': '2004/09/03 18:34:48',
	'Macro\\S_MAC.KWD': '2002/09/24 18:38:30',
	'MortScript\\MortScript-readme.txt': '2009/03/31 15:06:46',
	'MortScript\\MortScript.col': '2009/04/25 7:36:38',
	'MortScript\\MortScript.hkn': '2009/04/10 2:53:16',
	'MortScript\\MortScript.khp': '2009/04/04 16:49:50',
	'MortScript\\MortScript.kwd': '2009/03/28 10:29:46',
	'MortScript\\MortScript.rkw': '2009/04/10 2:53:04',
	'MortScript\\MortScript.rule': '2009/03/28 10:28:14',
	'MortScript\\MortScript2.kwd': '2009/03/31 14:33:42',
	'MovableType\\movabletype.kwd': '2007/10/19 21:56:42',
	'OPTIMA\\optima-readme.txt': '2007/09/20 20:45:38',
	'OPTIMA\\optima.rkw': '2007/09/20 20:30:14',
	'Pascal\\pascal.kwd': '1999/10/13 09:23:48',
	'Perl\\PERL.COL': '2002/07/08 22:46:18',
	'Perl\\Perl.kwd': '2006/05/16 01:51:34',
	'Perl\\Perl.rkw': '2002/07/08 22:46:22',
	'Perl\\Perlvar.kwd': '2000/02/10 21:33:38',
	'PHP\\php.khp': '2002/12/02 20:55:08',
	'PHP\\PHP.KWD': '2002/12/02 21:00:30',
	'PIC_CCS\\ccs_commands.kwd': '2008/08/19 15:27:14',
	'PIC_CCS\\PIC_CCS-C.kwd': '2008/08/19 14:02:52',
	'PIC_CCS\\PIC_CCS-C.txt': '2008/08/19 23:21:30',
	'PLI\\pli.kwd': '2001/12/03 18:14:30',
	'PLSQL\\plsql.kwd': '1999/06/02 15:59:34',
	'PPA\\PPA.KWD': '2002/09/23 21:16:40',
	'PPA\\S_MACPPA.DIC': '2002/09/24 18:38:30',
	'PPA\\S_MACPPA.KHP': '2002/09/24 18:38:30',
	'PPMCK\\ppmckc.col': '2004/11/19 21:14:30',
	'PPMCK\\ppmckc.rkw': '2004/11/19 21:13:30',
	'PukiWiki\\pukiwiki-readme.txt': '2007/05/09 09:29:08',
	'PukiWiki\\pukiwiki.col': '2007/05/09 09:06:02',
	'PukiWiki\\pukiwiki.kwd': '2007/05/09 09:23:12',
	'PukiWiki\\pukiwiki.rkw': '2007/05/09 09:05:40',
	'Python\\python_2.5.kwd': '2007/02/08 03:22:54',
	'RTF\\rtf.kwd': '2001/06/21 21:17:00',
	'Ruby\\Ruby-readme.txt': '2005/09/23 11:13:24',
	'Ruby\\Ruby.col': '2005/09/23 10:52:04',
	'Ruby\\Ruby.hkn': '2005/08/27 15:15:02',
	'Ruby\\Ruby.rkw': '2005/09/23 11:06:56',
	'Ruby\\Ruby.rule': '2005/08/27 05:31:02',
	'Ruby\\Ruby1.kwd': '2005/08/27 15:10:34',
	'Ruby\\Ruby2.kwd': '2005/08/27 15:11:12',
	'Ruby\\Ruby3.kwd': '2005/08/27 15:12:54',
	'Ruby\\Ruby4.kwd': '2005/08/27 15:12:50',
	'Shell\\SHELL.kwd': '2001/11/09 19:54:44',
	'SystemC\\systemc_2_0_1.kwd': '2007/10/19 21:49:30',
	'T-SQL\\tsql.kwd': '2001/07/09 20:41:00',
	'T-SQL\\tsql2.kwd': '2001/07/09 20:41:20',
	'Tcl\\tcl.kwd': '2005/02/01 17:09:22',
	'TeX\\TeX.kwd': '2001/11/05 18:46:30',
	'TTL\\TTL.KHP': '2002/07/24 20:17:02',
	'TTL\\TTL.KWD': '2002/07/24 20:17:02',
	'TTL\\TTL.RL': '2002/07/24 20:17:02',
	'UWSC\\uwsc.dic': '2005/02/26 20:28:50',
	'UWSC\\uwsc.kwd': '2005/02/26 20:52:18',
	'UWSC\\uwsc.rl': '2005/02/26 14:20:08',
	'UWSC\\uwsc2.kwd': '2005/02/26 20:51:06',
	'UWSC\\uwsc41.kwd': '2007/08/25 13:23:24',
	'UWSC\\uwsc42.kwd': '2007/08/25 13:40:12',
	'VB\\VB.KWD': '2000/12/07 23:02:32',
	'VB\\vb2.kwd': '2000/12/07 23:03:02',
	'VB\\vb_asp.kwd': '2001/11/15 09:07:40',
	'VBScript\\VBScript.kwd': '2001/11/15 09:16:50',
	'Verilog\\verilog.kwd': '2005/08/21 21:40:40',
	'WSH\\wsh.kwd': '2005/01/31 15:42:28',
	'XUL\\xul-attr.kwd': '2007/10/19 21:48:48',
	'XUL\\xul.kwd': '2007/10/19 21:48:46'
};

WScript.Echo('タイムスタンプの設定を行います');

var cwd = WScript.ScriptFullName.replace(/[^\\]+$/, '');
for (var k in dates) {
	touch(cwd + 'sakura\\keyword\\' + k, dates[k]);
}
if (message != '') {
	WScript.Echo(message);
}

WScript.Echo('タイムスタンプの設定が完了しました');
