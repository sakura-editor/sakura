// HHCtrls
function HHCtrlClose()
{
	document.writeln('<OBJECT id="aaHHCtlCloseWin" type="application/x-oleobject"');
	document.writeln(' classid="clsid:adb880a6-d8ff-11cf-9377-00aa003b7a11"> ');
	document.writeln(' <PARAM name="Command" value="Close"> ');
	document.writeln('</OBJECT> ');
	document.onkeypress= function () {
		if (window.event.keyCode == 27){
			aaHHCtlCloseWin.Click();
		}
	};
}

// void
function HHCtrlPopup()
{
	document.writeln('<OBJECT id="hhctrl" type="application/x-oleobject"');
	document.writeln(' classid="clsid:adb880a6-d8ff-11cf-9377-00aa003b7a11"');
	document.writeln(' codebase="hhctrl.ocx#Version=4,74,8793,0" width="0" height="0">');
	document.writeln('</OBJECT>');
}

function IsChm(){return (-1 !=(""+window.location.href).search(/\.chm::/i));}

if(IsChm()){
	HHCtrlClose();
}else{
	document.writeln('[<a href="index.html">–ÚŽŸ</a>]');
}

function hideDiv (targetId)
{
	if (document.getElementById) {
		target = document.getElementById( targetId );
		target.style.display = ( target.style.display == "none" ) ? "block" : "none";
	}
}
