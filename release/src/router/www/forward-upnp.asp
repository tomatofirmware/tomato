<!DOCTYPE HTML PUBLIC '-//W3C//DTD HTML 4.0//EN'>
<!--
	Tomato GUI
	Copyright (C) 2006-2008 Jonathan Zarate
	http://www.polarcloud.com/tomato/

	For use with Tomato Firmware only.
	No part of this file may be used without permission.
-->
<html>
<head>
<meta http-equiv='content-type' content='text/html;charset=utf-8'>
<meta name='robots' content='noindex,nofollow'>
<title>[<% ident(); %>] Forwarding: UPnP</title>
<link rel='stylesheet' type='text/css' href='tomato.css'>
<link rel='stylesheet' type='text/css' href='color.css'>
<script type='text/javascript' src='tomato.js'></script>

<!-- / / / -->

<style type='text/css'>
#upnp-grid .co1, #upnp-grid .co2 {
	width: 12%;
}
#upnp-grid .co3 {
	width: 15%;
}
#upnp-grid .co4 {
	width: 8%;
}
#upnp-grid .co5 {
	width: 53%;
}
</style>

<script type='text/javascript' src='debug.js'></script>

<script type='text/javascript'>

/* REMOVE-BEGIN
	!!TB - added miniupnpd settings, removed upnp_mnp
REMOVE-END */
//	<% nvram("upnp_enable,upnp_nat_pmp_enable,upnp_clean_ruleset_enable,upnp_secure_mode,upnp_clean_ruleset_interval,upnp_clean_ruleset_threshold"); %>
// <% upnpinfo(); %>

function submitDelete(proto, port)
{
	form.submitHidden('upnp.cgi', {
		remove_ext_proto: proto,
		remove_ext_port: port,
		_redirect: 'forward-upnp.asp' });	
}

function deleteData(data)
{
	if (!confirm('Delete ' + data[4] + '? [' + data[0] + '->' + data[1] + ' ' + data[2] + ']')) return;
	submitDelete((data[3] == 'TCP') ? 6 : 17, data[0]);
}

var ug = new TomatoGrid();

ug.onClick = function(cell) {
	deleteData(cell.parentNode.getRowData());
}

ug.rpDel = function(e) {
	deleteData(PR(e).getRowData());
}

	
ug.setup = function() {
	this.init('upnp-grid', 'sort delete');
	this.headerSet(['External', 'Internal', 'Internal Address', 'Protocol', 'Description']);
	ug.populate();
}

ug.populate = function() {
	var i, j, r, row;

	if ((nvram.upnp_enable != 0) || (nvram.upnp_nat_pmp_enable != 0)) {
		for (i = 0; i < upnp_data.length; ++i) {
			r = upnp_data[i];
			if (r.length != 6) continue;
			row = this.insertData(-1, [r[2],r[3],r[4],r[1],r[5]]);
			
			if (!r[0]) {
				for (j = 0; j < 5; ++j) {
					elem.addClass(row.cells[j], 'disabled');
				}
			}
			for (j = 0; j < 5; ++j) {
				row.cells[j].title = 'Click to delete';
			}
		}
	}
	this.sort(4);
}

function deleteAll()
{
	if (!confirm('Delete all entries?')) return;
	submitDelete('0', '0');
}

function verifyFields(focused, quiet)
{
/* REMOVE-BEGIN
	!!TB - miniupnp
REMOVE-END */
	var enable = E('_f_upnp_enable').checked ||
		E('_f_upnp_nat_pmp_enable').checked;
	var bc = E('_f_upnp_clean_ruleset_enable').checked;

	E('_f_upnp_clean_ruleset_enable').disabled = (enable == 0);
	E('_f_upnp_secure_mode').disabled = (enable == 0);
	E('_upnp_clean_ruleset_interval').disabled = (enable == 0) || (bc == 0);
	E('_upnp_clean_ruleset_threshold').disabled = (enable == 0) || (bc == 0);
	elem.display(PR(E('_upnp_clean_ruleset_interval')), (enable != 0) && (bc != 0));
	elem.display(PR(E('_upnp_clean_ruleset_threshold')), (enable != 0) && (bc != 0));

	if ((enable != 0) && (bc != 0)) {
		if (!v_range('_upnp_clean_ruleset_interval', quiet, 60, 65535)) return 0;
		if (!v_range('_upnp_clean_ruleset_threshold', quiet, 0, 9999)) return 0;
	}
	else {
		ferror.clear(E('_upnp_clean_ruleset_interval'));
		ferror.clear(E('_upnp_clean_ruleset_threshold'));
	}
	
	return 1;
}

function save()
{
/* REMOVE-BEGIN
	!!TB - miniupnp
REMOVE-END */
	if (!verifyFields(null, 0)) return;

	var fom = E('_fom');
	var enable = E('_f_upnp_enable').checked;
	fom.upnp_enable.value = enable ? 1 : 0;

/* REMOVE-BEGIN
	!!TB - miniupnp
	fom.upnp_mnp.value = E('_f_upnp_mnp').checked ? 1 : 0;
REMOVE-END */
	fom.upnp_nat_pmp_enable.value = E('_f_upnp_nat_pmp_enable').checked ? 1 : 0;
	fom.upnp_clean_ruleset_enable.value = E('_f_upnp_clean_ruleset_enable').checked ? 1 : 0;
	fom.upnp_secure_mode.value = E('_f_upnp_secure_mode').checked ? 1 : 0;

/* REMOVE-BEGIN
	!!TB - miniupnp
	form.submit(fom, (enable == (nvram.upnp_enable != '0')));
REMOVE-END */
	form.submit(fom, (enable == (enable || E('_f_upnp_nat_pmp_enable').checked)));
}

function init()
{
	ug.recolor();	// opera
	if (ug.getDataCount() == 0) {
		E('upnp-delete-all').disabled = true;
	}
}

/* REMOVE-BEGIN
	!!TB - miniupnp
REMOVE-END */
function submit_complete()
{
	reloadPage();
}
</script>

</head>
<body onload='init()'>
<form id='_fom' method='post' action='tomato.cgi'>
<table id='container' cellspacing=0>
<tr><td colspan=2 id='header'>
	<div class='title'>Tomato</div>
	<div class='version'>Version <% version(); %></div>
</td></tr>
<tr id='body'><td id='navi'><script type='text/javascript'>navi()</script></td>
<td id='content'>
<div id='ident'><% ident(); %></div>

<!-- / / / -->

<input type='hidden' name='_nextpage' value='forward-upnp.asp'>
<input type='hidden' name='_service' value='upnp-restart'>

<input type='hidden' name='upnp_enable'>

/* REMOVE-BEGIN
	!!TB - miniupnp
<input type='hidden' name='upnp_mnp'>
REMOVE-END */
<input type='hidden' name='upnp_nat_pmp_enable'>
<input type='hidden' name='upnp_clean_ruleset_enable'>
<input type='hidden' name='upnp_secure_mode'>

<div class='section-title'>UPnP Forwarded Ports</div>
<div class='section'>
	<table id='upnp-grid' class='tomato-grid'></table>
	<div style='width: 100%; text-align: right'><input type='button' value='Delete All' onclick='deleteAll()' id='upnp-delete-all'> <input type='button' value='Refresh' onclick='javascript:reloadPage();'></div>
</div>

<div class='section-title'>Settings</div>
<div class='section'>
<script type='text/javascript'>
createFieldTable('', [
	{ title: 'Enable UPnP', name: 'f_upnp_enable', type: 'checkbox', value: (nvram.upnp_enable == '1') },

/* REMOVE-BEGIN
	!!TB - miniupnp
	{ title: 'Show In My Network Places',  name: 'f_upnp_mnp',  type: 'checkbox',  value: (nvram.upnp_mnp == '1') }
REMOVE-END */
	{ title: 'Enable NAT-PMP', name: 'f_upnp_nat_pmp_enable', type: 'checkbox', value: (nvram.upnp_nat_pmp_enable == '1') },
	{ title: 'Inactive Rules Cleaning', name: 'f_upnp_clean_ruleset_enable', type: 'checkbox', value: (nvram.upnp_clean_ruleset_enable == '1') },
	{ title: 'Cleaning Interval', indent: 2, name: 'upnp_clean_ruleset_interval', type: 'text', maxlen: 5, size: 7,
		suffix: ' <small>seconds</small>', value: nvram.upnp_clean_ruleset_interval },
	{ title: 'Cleaning Threshold', indent: 2, name: 'upnp_clean_ruleset_threshold', type: 'text', maxlen: 4, size: 7,
		suffix: ' <small>redirections</small>', value: nvram.upnp_clean_ruleset_threshold },
	{ title: 'Secure Mode', name: 'f_upnp_secure_mode', type: 'checkbox',
		suffix: ' <small>(when enabled, UPnP clients are allowed to add mappings only to their IP)</small>',
		value: (nvram.upnp_secure_mode == '1') }
]);
</script>
</div>


<!-- / / / -->

</td></tr>
<tr><td id='footer' colspan=2>
	<span id='footer-msg'></span>
	<input type='button' value='Save' id='save-button' onclick='save()'>
	<input type='button' value='Cancel' id='cancel-button' onclick='javascript:reloadPage();'>
</td></tr>
</table>
</form>
/* REMOVE-BEGIN
	!!TB - added verifyFields
REMOVE-END */
<script type='text/javascript'>ug.setup();verifyFields(null, 1);</script>
</body>
</html>
