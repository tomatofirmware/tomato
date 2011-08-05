<!DOCTYPE HTML PUBLIC '-//W3C//DTD HTML 4.0//EN'>
<!--
	Tomato GUI
	FTP Server - !!TB

	For use with Tomato Firmware only.
	No part of this file may be used without permission.
-->
<html>
<head>
<meta http-equiv='content-type' content='text/html;charset=utf-8'>
<meta name='robots' content='noindex,nofollow'>
<title>[<% ident(); %>] <% translate("NAS"); %>: <% translate("FTP Server"); %></title>
<link rel='stylesheet' type='text/css' href='tomato.css'>
<link rel='stylesheet' type='text/css' href='color.css'>
<script type='text/javascript' src='tomato.js'></script>

<!-- / / / -->
<style tyle='text/css'>
#aft-grid {
	width: 99%;
}
#aft-grid .co1,
#aft-grid .co2 {
	width: 40%;
}
#aft-grid .co3 {
	width: 20%;
}
</style>

<style type='text/css'>
textarea {
	width: 98%;
	height: 5em;
}
</style>

<script type='text/javascript' src='debug.js'></script>

<script type='text/javascript'>

//	<% nvram("ftp_enable,ftp_super,ftp_anonymous,ftp_dirlist,ftp_port,ftp_max,ftp_ipmax,ftp_staytimeout,ftp_rate,ftp_anonrate,ftp_anonroot,ftp_pubroot,ftp_pvtroot,ftp_custom,ftp_users,ftp_sip,ftp_limit,log_ftp"); %>

ftplimit = nvram.ftp_limit.split(',');
if (ftplimit.length != 3) ftplimit = [0,3,60];

var aftg = new TomatoGrid();

aftg.exist = function(f, v)
{
	var data = this.getAllData();
	for (var i = 0; i < data.length; ++i) {
		if (data[i][f] == v) return true;
	}
	return false;
}

aftg.existName = function(name)
{
	return this.exist(0, name);
}

aftg.sortCompare = function(a, b) {
	var da = a.getRowData();
	var db = b.getRowData();
	var r = cmpText(da[this.sortColumn], db[this.sortColumn]);
	return this.sortAscending ? r : -r;
}

aftg.verifyFields = function(row, quiet)
{
	var f, s;
	f = fields.getAll(row);

	ferror.clear(f[0]);
	ferror.clear(f[1]);

	if (!v_length(f[0], quiet, 1)) return 0;

	s = f[0].value.trim().replace(/\s+/g, ' ');
	if (s.length > 0) {
		if (s.search(/^[a-zA-Z0-9_\-]+$/) == -1) {
			ferror.set(f[0], '<% translate("Invalid user name. Only characters "A-Z 0-9 - _" are allowed."); %>', quiet);
			return 0;
		}
		if (this.existName(s)) {
			ferror.set(f[0], '<% translate("Duplicate user name"); %>.', quiet);
			return 0;
		}
		f[0].value = s;
	}

	if (!v_length(f[1], quiet, 1)) return 0;
	if (!v_nodelim(f[1], quiet, '<% translate("Password"); %>', 1)) return 0;

	return 1;
}

aftg.resetNewEditor = function() {
	var f;

	f = fields.getAll(this.newEditor);
	ferror.clearAll(f);

	f[0].value = '';
	f[1].value = '';
	f[2].selectedIndex = 0;
}

aftg.setup = function()
{
	this.init('aft-grid', 'sort', 50, [
		{ type: 'text', maxlen: 50 },
		{ type: 'password', maxlen: 50, peekaboo: 1 },
		{ type: 'select', options: [['Read/Write', '<% translate("Read/Write"); %>'],['Read Only', '<% translate("Read Only"); %>'],['View Only', '<% translate("View Only"); %>'],['Private', '<% translate("Private"); %>']] }
	]);
	this.headerSet(['<% translate("User Name"); %>', '<% translate("Password"); %>', '<% translate("Access"); %>']);

	var s = nvram.ftp_users.split('>');
	for (var i = 0; i < s.length; ++i) {
		var t = s[i].split('<');
		if (t.length == 3) {
			this.insertData(-1, t);
		}
	}

	this.showNewEditor();
	this.resetNewEditor();
}

function verifyFields(focused, quiet)
{
	var a, b;
	var ok = 1;

	a = E('_ftp_enable').value;	
	b = E('_ftp_port');
	elem.display(PR(b), (a != 0));
	b = E('_f_ftp_sip');
	elem.display(PR(b), (a == 1));

	E('_f_limit').disabled = (a != 1);
	b = E('_f_limit').checked;
	elem.display(PR('_f_limit_hit'), PR('_f_limit_sec'), (a == 1 && b));
	E('_ftp_anonymous').disabled = (a == 0);
	E('_f_ftp_super').disabled = (a == 0);
	E('_f_log_ftp').disabled = (a == 0);
	E('_ftp_pubroot').disabled = (a == 0);
	E('_ftp_pvtroot').disabled = (a == 0);
	E('_ftp_anonroot').disabled = (a == 0);
	E('_ftp_dirlist').disabled = (a == 0);
	E('_ftp_max').disabled = (a == 0);
	E('_ftp_ipmax').disabled = (a == 0);
	E('_ftp_rate').disabled = (a == 0);
	E('_ftp_anonrate').disabled = (a == 0);
	E('_ftp_staytimeout').disabled = (a == 0);
	E('_ftp_custom').disabled = (a == 0);

	if (a != 0) {
		if (!v_port('_ftp_port', quiet || !ok)) ok = 0;
		if (!v_range('_ftp_max', quiet || !ok, 0, 12)) ok = 0;
		if (!v_range('_ftp_ipmax', quiet || !ok, 0, 12)) ok = 0;
		if (!v_range('_ftp_rate', quiet || !ok, 0, 99999)) ok = 0;
		if (!v_range('_ftp_anonrate', quiet || !ok, 0, 99999)) ok = 0;
		if (!v_range('_ftp_staytimeout', quiet || !ok, 0, 65535)) ok = 0;
		if (!v_length('_ftp_custom', quiet || !ok, 0, 2048)) ok = 0;
		if (!v_path('_ftp_pubroot', quiet || !ok, 0)) ok = 0;
		if (!v_path('_ftp_pvtroot', quiet || !ok, 0)) ok = 0;
		if (!v_path('_ftp_anonroot', quiet || !ok, 0)) ok = 0;
		if (a == 1 && b) {
			if (!v_range('_f_limit_hit', quiet || !ok, 1, 100)) ok = 0;
			if (!v_range('_f_limit_sec', quiet || !ok, 3, 3600)) ok = 0;
		}
	}

	if (a == 1) {
		b = E('_f_ftp_sip');
		if ((b.value.length) && (!_v_iptaddr(b, quiet || !ok, 15, 1, 1))) ok = 0;
		else ferror.clear(b);
	}

	return ok;
}

function save()
{
	if (aftg.isEditing()) return;
	if (!verifyFields(null, 0)) return;

	var fom = E('_fom');

	var data = aftg.getAllData();
	var r = [];
	for (var i = 0; i < data.length; ++i) r.push(data[i].join('<'));
	fom.ftp_users.value = r.join('>');

	fom.ftp_sip.value = fom.f_ftp_sip.value.split(/\s*,\s*/).join(',');
	fom.ftp_super.value = E('_f_ftp_super').checked ? 1 : 0;
	fom.log_ftp.value = E('_f_log_ftp').checked ? 1 : 0;

	fom.ftp_limit.value = (E('_f_limit').checked ? 1 : 0) +
		',' + E('_f_limit_hit').value + ',' + E('_f_limit_sec').value;

	form.submit(fom, 1);
}
</script>

</head>
<body>
<form id='_fom' method='post' action='tomato.cgi'>
<table id='container' cellspacing=0>
<tr><td colspan=2 id='header'>
	<div class='title'>Tomato</div>
	<div class='version'><% translate("Version"); %> <% version(); %></div>
</td></tr>
<tr id='body'><td id='navi'><script type='text/javascript'>navi()</script></td>
<td id='content'>
<div id='ident'><% ident(); %></div>

<!-- / / / -->

<input type='hidden' name='_nextpage' value='nas-ftp.asp'>
<input type='hidden' name='_service' value='ftpd-restart'>

<input type='hidden' name='ftp_super'>
<input type='hidden' name='log_ftp'>
<input type='hidden' name='ftp_users'>
<input type='hidden' name='ftp_sip'>
<input type='hidden' name='ftp_limit'>

<div class='section-title'><% translate("FTP Server Configuration"); %></div>
<div class='section'>
<script type='text/javascript'>
createFieldTable('', [
	{ title: '<% translate("Enable FTP Server"); %>', name: 'ftp_enable', type: 'select',
		options: [['0', '<% translate("No"); %>'],['1', '<% translate("Yes, WAN and LAN"); %>'],['2', '<% translate("Yes, LAN only"); %>']],
		value: nvram.ftp_enable },
	{ title: '<% translate("FTP Port"); %>', indent: 2, name: 'ftp_port', type: 'text', maxlen: 5, size: 7, value: fixPort(nvram.ftp_port, 21) },
	{ title: '<% translate("Allowed Remote"); %><br><% translate("Address(es)"); %>"); %>', indent: 2, name: 'f_ftp_sip', type: 'text', maxlen: 512, size: 64, value: nvram.ftp_sip,
		suffix: '<br><small>(<% translate("optional"); %>; <% translate("ex"); %>: "1.1.1.1", "1.1.1.0/24", "1.1.1.1 - 2.2.2.2" <% translate("or"); %> "me.example.com")</small>' },
	{ title: '<% translate("Anonymous Users Access"); %>', name: 'ftp_anonymous', type: 'select',
		options: [['0', '<% translate("Disabled"); %>'],['1', '<% translate("Read/Write"); %>'],['2', '<% translate("Read Only"); %>'],['3', '<% translate("Write Only"); %>']],
		value: nvram.ftp_anonymous },
	{ title: '<% translate("Allow Admin Login"); %>*', name: 'f_ftp_super', type: 'checkbox',
		suffix: ' <small><% translate("Allows users to connect with admin account"); %>.</small>',
		value: nvram.ftp_super == 1 },
	{ title: '<% translate("Log FTP requests and responses"); %>', name: 'f_log_ftp', type: 'checkbox',
		value: nvram.log_ftp == 1 }
]);
</script>
<small><br>*&nbsp;<% translate("Avoid using this option when FTP server is enabled for WAN. IT PROVIDES FULL ACCESS TO THE ROUTER FILE SYSTEM"); %>!</small>
</div>

<div class='section-title'><% translate("Directories"); %></div>
<div class='section'>
<script type='text/javascript'>
createFieldTable('', [
	{ title: '<% translate("Public Root Directory"); %>*', name: 'ftp_pubroot', type: 'text', maxlen: 256, size: 32,
		suffix: ' <small>(<% translate("for authenticated users access"); %>)</small>',
		value: nvram.ftp_pubroot },
	{ title: '<% translate("Private Root Directory"); %>**', name: 'ftp_pvtroot', type: 'text', maxlen: 256, size: 32,
		suffix: ' <small>(<% translate("for authenticated users access in private mode"); %>)</small>',
		value: nvram.ftp_pvtroot },
	{ title: '<% translate("Anonymous Root Directory"); %>*', name: 'ftp_anonroot', type: 'text', maxlen: 256, size: 32, 
		suffix: ' <small>(<% translate("for anonymous connection"); %>)</small>',
		value: nvram.ftp_anonroot },
	{ title: '<% translate("Directory Listings"); %>', name: 'ftp_dirlist', type: 'select',
		options: [['0', '<% translate("Enabled"); %>'],['1', '<% translate("Disabled"); %>'],['2', '<% translate("Disabled for Anonymous"); %>']],
		suffix: ' <small>(<% translate("always enabled for Admin"); %>)</small>',
		value: nvram.ftp_dirlist }
]);
</script>
<small>
<br>*&nbsp;&nbsp;<% translate("When no directory is specified, /mnt is used as a root directory"); %>.
<br>**&nbsp;<% translate("In private mode, the root directory is the directory under the 'Private Root Directory' with the name matching the name of the user"); %>.
</small>
</div>

<div class='section-title'><% translate("Limits"); %></div>
<div class='section'>
<script type='text/javascript'>
createFieldTable('', [
	{ title: '<% translate("Maximum Users Allowed to Log in"); %>', name: 'ftp_max', type: 'text', maxlen: 5, size: 7,
		suffix: ' <small>(0 - <% translate("unlimited"); %>)</small>',
		value: nvram.ftp_max },
	{ title: '<% translate("Maximum Connections from the same IP"); %>', name: 'ftp_ipmax', type: 'text', maxlen: 5, size: 7,
		suffix: ' <small>(0 - <% translate("unlimited"); %>)</small>',
		value: nvram.ftp_ipmax },
	{ title: '<% translate("Maximum Bandwidth for Anonymous Users"); %>', name: 'ftp_anonrate', type: 'text', maxlen: 5, size: 7,
		suffix: ' <small>KB/s (0 - <% translate("unlimited"); %>)</small>',
		value: nvram.ftp_anonrate },
	{ title: '<% translate("Maximum Bandwidth for Authenticated Users"); %>', name: 'ftp_rate', type: 'text', maxlen: 5, size: 7,
		suffix: ' <small>KB/s (0 - <% translate("unlimited"); %>)</small>',
		value: nvram.ftp_rate },
	{ title: '<% translate("Idle Timeout"); %>', name: 'ftp_staytimeout', type: 'text', maxlen: 5, size: 7,
		suffix: ' <small><% translate("seconds"); %> (0 - <% translateno timeout"); %>)</small>',
		value: nvram.ftp_staytimeout },
	{ title: '<% translate("Limit Connection Attempts"); %>', name: 'f_limit', type: 'checkbox',
		value: ftplimit[0] != 0 },
	{ title: '', indent: 2, multi: [
		{ name: 'f_limit_hit', type: 'text', maxlen: 4, size: 6, suffix: '&nbsp; <small><% translate("every"); %></small> &nbsp;', value: ftplimit[1] },
		{ name: 'f_limit_sec', type: 'text', maxlen: 4, size: 6, suffix: '&nbsp; <small><% translate("seconds"); %></small>', value: ftplimit[2] }
	] }
]);
</script>
</div>

<div class='section-title'><% translate("Custom Configuration"); %></div>
<div class='section'>
<script type='text/javascript'>
createFieldTable('', [
	{ title: '<a href="http://vsftpd.beasts.org/vsftpd_conf.html" target="_new">Vsftpd</a><br><% translate("Custom Configuration"); %>', name: 'ftp_custom', type: 'textarea', value: nvram.ftp_custom }
]);
</script>
</div>

<div class='section-title'><% translate("User Accounts"); %></div>
<div class='section'>
	<table class='tomato-grid' cellspacing=1 id='aft-grid'></table>
	<script type='text/javascript'>aftg.setup();</script>
</div>

<!-- / / / -->

</td></tr>
<tr><td id='footer' colspan=2>
	<span id='footer-msg'></span>
	<input type='button' value='<% translate("Save"); %>' id='save-button' onclick='save()'>
	<input type='button' value='<% translate("Cancel"); %>' id='cancel-button' onclick='javascript:reloadPage();'>
</td></tr>
</table>
</form>
<script type='text/javascript'>verifyFields(null, 1);</script>
</body>
</html>
