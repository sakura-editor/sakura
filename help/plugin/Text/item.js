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

function IsChm(){return (-1 !=(""+window.location.href).search(/\.chm::/i));}

if(IsChm()){
	HHCtrlClose();
}else{
	document.writeln('[<a href="index.html">目次</a>]');
}

function hideDiv (targetId)
{
	if (document.getElementById) {
		target = document.getElementById( targetId );
		target.style.display = ( target.style.display == "none" ) ? "block" : "none";
	}
}
