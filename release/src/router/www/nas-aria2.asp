<!DOCTYPE HTML PUBLIC '-//W3C//DTD HTML 4.0//EN'>
<!--
	Tomato RAF Aria2 GUI
	Copyright (C) 2013 Hyzoom, bwq518@gmail.com
	http://openlinksys.info
	For use with Tomato RAF Firmware only.
	No part of this file may be used without permission.
-->
<html>
<head>
<meta http-equiv='content-type' content='text/html;charset=utf-8'>
<meta name='robots' content='noindex,nofollow'>
<title>[<% ident(); %>] Nas: Aria2 Client</title>
<link rel='stylesheet' type='text/css' href='tomato.css'>
<link rel='stylesheet' type='text/css' href='color.css'>
<script type='text/javascript' src='tomato.js'></script>
<style type='text/css'>
textarea {
 width: 98%;
 height: 15em;
}
</style>
<script type='text/javascript'>
//	<% nvram("aria2_enable,aria2_sleep,aria2_check,aria2_check_time,aria2_binary,aria2_binary_custom,aria2_custom,aria2_enable_rpc,aria2_rpc_allow_origin_all,aria2_rpc_listen_all,aria2_rpc_listen_port,aria2_event_poll,aria2_rpc_user,aria2_rpc_passwd,aria2_max_concurrent_downloads,aria2_continue,aria2_max_tries,aria2_retry_wait,aria2_max_connection_per_server,aria2_min_split_size,aria2_split,aria2_max_overall_download_limit,aria2_max_download_limit,aria2_max_overall_upload_limit,aria2_max_upload_limit,aria2_lowest_speed_limit,aria2_referer,aria2_input_file,aria2_save_session,aria2_usb_enable,aria2_dlroot,aria2_dir,aria2_disk_cache,aria2_enable_mmap,aria2_file_allocation,aria2_bt_enable_lpd,aria2_bt_tracker,aria2_bt_max_peers,aria2_bt_require_crypto,aria2_follow_torrent,aria2_listen_port,aria2_enable_dht,aria2_dht_listen_port,aria2_enable_peer_exchange,aria2_user_agent,aria2_peer_id_prefix,aria2_seed_ratio,aria2_force_save,aria2_bt_hash_check_seed,aria2_bt_seed_unverified,aria2_bt_save_metadata,aria2_save_session_interval,http_lanport"); %>

var btgui_link = '&nbsp;&nbsp;<a href="http://' + location.hostname + ':' + nvram.http_lanport + '/yaaw/index.html" target="_blank"><i>[Click here to open Aria2 GUI]</i></a>';
//	<% usbdevices(); %>
var usb_disk_list = new Array();
function refresh_usb_disk()
{
	var i, j, k, a, b, c, e, s, desc, d, parts, p;
	var partcount;
	list = [];
	for (i = 0; i < list.length; ++i) {
		list[i].type = '';
		list[i].host = '';
		list[i].vendor = '';
		list[i].product = '';
		list[i].serial = '';
		list[i].discs = [];
		list[i].is_mounted = 0;
	}
	for (i = usbdev.length - 1; i >= 0; --i) {
		a = usbdev[i];
		e = {
			type: a[0],
			host: a[1],
			vendor: a[2],
			product: a[3],
			serial: a[4],
			discs: a[5],
			is_mounted: a[6]
		};
		list.push(e);
	}
	partcount = 0;
	for (i = list.length - 1; i >= 0; --i) {
		e = list[i];
		if (e.discs) {
			for (j = 0; j <= e.discs.length - 1; ++j) {
				d = e.discs[j];
				parts = d[1];
				for (k = 0; k <= parts.length - 1; ++k) {
					p = parts[k];					
					if ((p) && (p[1] >= 1) && (p[3] != 'swap')) {
						usb_disk_list[partcount] = new Array();
						usb_disk_list[partcount][0] = p[2];
						usb_disk_list[partcount][1] = 'Partition ' + p[0] + ' mounted on '+p[2]+' (' + p[3]+ ' - ' + doScaleSize(p[6])+ ' available, total ' + doScaleSize(p[5]) + ')';
						partcount++;
					}
				}
			}
		}
	}
	list = [];
}


function verifyFields(focused, quiet)
{
	var ok = 1;

	var a = E('_f_aria2_enable_all').checked;
	var b = E('_f_aria2_enable_rpc').checked;
	var c = E('_f_aria2_rpc_allow_origin_all').checked;
	var d = E('_f_aria2_rpc_listen_all').checked;
	var e = E('_f_aria2_continue').checked;
	var f = E('_f_aria2_enable_mmap').checked;
	var g = E('_f_aria2_bt_enable_lpd').checked;
	var h = E('_f_aria2_bt_require_crypto').checked;
	var i = E('_f_aria2_enable_dht').checked;
	var j = E('_f_aria2_enable_peer_exchange').checked;
	var k = E('_f_aria2_force_save').checked;
	var l = E('_f_aria2_bt_hash_check_seed').checked;
	var m = E('_f_aria2_bt_seed_unverified').checked;
	var n = E('_f_aria2_bt_save_metadata').checked;
	var o = E('_f_aria2_check').checked;
	var u = E('_f_aria2_usb_enable').checked;
	
	E('_f_aria2_check').disabled = !a;
	E('_aria2_check_time').disabled = !a || !o;
	E('_aria2_sleep').disabled = !a;
	E('_aria2_binary').disabled = !a;
	E('_aria2_custom').disabled = !a;
	E('_f_aria2_enable_rpc').disabled = !a;
	E('_f_aria2_rpc_allow_origin_all').disabled = !a || !b;
	E('_f_aria2_rpc_listen_all').disabled = !a || !b;
	E('_aria2_rpc_listen_port').disabled = !a || !b;
	E('_aria2_event_poll').disabled = !a || !b;
	E('_aria2_rpc_user').disabled = !a || !b;
	E('_aria2_rpc_passwd').disabled = !a || !b;
	E('_aria2_max_concurrent_downloads').disabled = !a;
	E('_f_aria2_continue').disabled = !a;
	E('_aria2_max_tries').disabled = !a;
	E('_aria2_retry_wait').disabled = !a;
	E('_aria2_max_connection_per_server').disabled = !a;
	E('_aria2_min_split_size').disabled = !a;
	E('_aria2_split').disabled = !a;
	E('_aria2_max_overall_download_limit').disabled = !a;
	E('_aria2_max_download_limit').disabled = !a;
	E('_aria2_max_overall_upload_limit').disabled = !a;
	E('_aria2_max_upload_limit').disabled = !a;
	E('_aria2_lowest_speed_limit').disabled = !a;
	E('_aria2_referer').disabled = !a;
	E('_aria2_input_file').disabled = !a;
	E('_aria2_save_session').disabled = !a || !k;
	E('_f_aria2_usb_enable').disabled = !a;
	E('_aria2_dlroot').disabled = !a || !u;
	E('_aria2_dir').disabled = !a;
	E('_aria2_disk_cache').disabled = !a;
	E('_f_aria2_enable_mmap').disabled = !a;
	E('_aria2_file_allocation').disabled = !a;
	E('_f_aria2_bt_enable_lpd').disabled = !a;
	E('_aria2_bt_tracker').disabled = !a;
	E('_aria2_bt_max_peers').disabled = !a;
	E('_f_aria2_bt_require_crypto').disabled = !a;
	E('_f_aria2_follow_torrent').disabled = !a;
	E('_aria2_listen_port').disabled = !a;
	E('_f_aria2_enable_dht').disabled = !a;
	E('_aria2_dht_listen_port').disabled = !a || !i;
	E('_f_aria2_enable_peer_exchange').disabled = !a;
	E('_aria2_user_agent').disabled = !a;
	E('_aria2_peer_id_prefix').disabled = !a;
	E('_aria2_seed_ratio').disabled = !a;
	E('_f_aria2_force_save').disabled = !a;
	E('_f_aria2_bt_hash_check_seed').disabled = !a;
	E('_f_aria2_bt_seed_unverified').disabled = !a;
	E('_f_aria2_bt_save_metadata').disabled = !a;
	E('_aria2_save_session_interval').disabled = !a || !k;
	
	if (!v_port('_aria2_rpc_listen_port', quiet)) return 0;

	var p = (E('_aria2_binary').value == 'custom');
	elem.display('_aria2_binary_custom', p && a);

	elem.display('_aria2_dlroot', u && a);

	if (!v_length('_aria2_custom', quiet, 0, 2048)) ok = 0;

	var s = E('_aria2_custom');
	
	if (s.value.search(/enable-rpc=/) == 0)  {
		ferror.set(s, 'Cannot set "enable-rpc" option here. You can set it in Tomato GUI', quiet);
		ok = 0; }
	if (s.value.search(/rpc-allow-origin-all=/) == 0)  {
		ferror.set(s, 'Cannot set "rpc-allow-origin-all" option here. You can set it in Tomato GUI', quiet);
		ok = 0; }
	if (s.value.search(/rpc-listen-all=/) == 0)  {
		ferror.set(s, 'Cannot set "rpc-listen-all" option here. You can set it in Tomato GUI', quiet);
		ok = 0; }
	if (s.value.search(/rpc-listen-port=/) == 0)  {
		ferror.set(s, 'Cannot set "rpc-listen-port" option here. You can set it in Tomato GUI', quiet);
		ok = 0; }
	if (s.value.search(/event-poll=/) == 0)  {
		ferror.set(s, 'Cannot set "event-poll" option here. You can set it in Tomato GUI', quiet);
		ok = 0; }
	if (s.value.search(/rpc-user=/) == 0)  {
		ferror.set(s, 'Cannot set "rpc-user" option here. You can set it in Tomato GUI', quiet);
		ok = 0; }
	if (s.value.search(/rpc-passwd=/) == 0)  {
		ferror.set(s, 'Cannot set "rpc-passwd" option here. You can set it in Tomato GUI', quiet);
		ok = 0; }
	if (s.value.search(/max-concurrent-downloads=/) == 0)  {
		ferror.set(s, 'Cannot set "max-concurrent-downloads" option here. You can set it in Tomato GUI', quiet);
		ok = 0; }
	if (s.value.search(/continue=/) == 0)  {
		ferror.set(s, 'Cannot set "continue" option here. You can set it in Tomato GUI', quiet);
		ok = 0; }
	if (s.value.search(/max-connection-per-server=/) == 0)  {
		ferror.set(s, 'Cannot set "max-connection-per-server" option here. You can set it in Tomato GUI', quiet);
		ok = 0; }
	if (s.value.search(/min-split-size=/) == 0)  {
		ferror.set(s, 'Cannot set "min-split-size" option here. You can set it in Tomato GUI', quiet);
		ok = 0; }
	if (s.value.search(/split=/) == 0)  {
		ferror.set(s, 'Cannot set "split" option here. You can set it in Tomato GUI', quiet);
		ok = 0; }
	if (s.value.search(/max-overall-download-limit=/) == 0)  {
		ferror.set(s, 'Cannot set "max-overall-download-limit" option here. You can set it in Tomato GUI', quiet);
		ok = 0; }
	if (s.value.search(/max-download-limit=/) == 0)  {
		ferror.set(s, 'Cannot set "max-download-limit" option here. You can set it in Tomato GUI', quiet);
		ok = 0; }
	if (s.value.search(/max-overall-upload-limit=/) == 0)  {
		ferror.set(s, 'Cannot set "max-overall-upload-limit" option here. You can set it in Tomato GUI', quiet);
		ok = 0; }
	if (s.value.search(/max-upload-limit=/) == 0)  {
		ferror.set(s, 'Cannot set "max-upload-limit" option here. You can set it in Tomato GUI', quiet);
		ok = 0; }
	if (s.value.search(/lowest-speed-limit=/) == 0)  {
		ferror.set(s, 'Cannot set "lowest-speed-limit" option here. You can set it in Tomato GUI', quiet);
		ok = 0; }
	if (s.value.search(/referer=/) == 0)  {
		ferror.set(s, 'Cannot set "referer" option here. You can set it in Tomato GUI', quiet);
		ok = 0; }
	if (s.value.search(/input-file=/) == 0)  {
		ferror.set(s, 'Cannot set "input-file" option here. You can set it in Tomato GUI', quiet);
		ok = 0; }
	if (s.value.search(/save-session=/) == 0)  {
		ferror.set(s, 'Cannot set "save-session" option here. You can set it in Tomato GUI', quiet);
		ok = 0; }
	if (s.value.search(/dir=/) == 0)  {
		ferror.set(s, 'Cannot set "dir" option here. You can set it in Tomato GUI', quiet);
		ok = 0; }
	if (s.value.search(/disk-cache=/) == 0)  {
		ferror.set(s, 'Cannot set "disk-cache" option here. You can set it in Tomato GUI', quiet);
		ok = 0; }
	if (s.value.search(/enable-mmap=/) == 0)  {
		ferror.set(s, 'Cannot set "enable-mmap" option here. You can set it in Tomato GUI', quiet);
		ok = 0; }
	if (s.value.search(/file-allocation=/) == 0)  {
		ferror.set(s, 'Cannot set "file-allocation" option here. You can set it in Tomato GUI', quiet);
		ok = 0; }
	if (s.value.search(/bt-enable-lpd=/) == 0)  {
		ferror.set(s, 'Cannot set "bt-enable-lpd" option here. You can set it in Tomato GUI', quiet);
		ok = 0; }
	if (s.value.search(/bt-tracker=/) == 0)  {
		ferror.set(s, 'Cannot set "bt-tracker" option here. You can set it in Tomato GUI', quiet);
		ok = 0; }
	if (s.value.search(/bt-max-peers=/) == 0)  {
		ferror.set(s, 'Cannot set "bt-max-peers" option here. You can set it in Tomato GUI', quiet);
		ok = 0; }
	if (s.value.search(/bt-require-crypto=/) == 0)  {
		ferror.set(s, 'Cannot set "bt-require-crypto" option here. You can set it in Tomato GUI', quiet);
		ok = 0; }
	if (s.value.search(/follow-torrent=/) == 0)  {
		ferror.set(s, 'Cannot set "follow-torrent" option here. You can set it in Tomato GUI', quiet);
		ok = 0; }
	if (s.value.search(/listen-port=/) == 0)  {
		ferror.set(s, 'Cannot set "listen-port" option here. You can set it in Tomato GUI', quiet);
		ok = 0; }
	if (s.value.search(/enable-dht=/) == 0)  {
		ferror.set(s, 'Cannot set "enable-dht" option here. You can set it in Tomato GUI', quiet);
		ok = 0; }
	if (s.value.search(/enable-peer-exchange=/) == 0)  {
		ferror.set(s, 'Cannot set "enable-peer-exchange" option here. You can set it in Tomato GUI', quiet);
		ok = 0; }
	if (s.value.search(/user-agent=/) == 0)  {
		ferror.set(s, 'Cannot set "user-agent" option here. You can set it in Tomato GUI', quiet);
		ok = 0; }
	if (s.value.search(/peer-id-prefix=/) == 0)  {
		ferror.set(s, 'Cannot set "peer-id-prefix" option here. You can set it in Tomato GUI', quiet);
		ok = 0; }
	if (s.value.search(/seed-ratio=/) == 0)  {
		ferror.set(s, 'Cannot set "seed-ratio" option here. You can set it in Tomato GUI', quiet);
		ok = 0; }
	if (s.value.search(/force-save=/) == 0)  {
		ferror.set(s, 'Cannot set "force-save" option here. You can set it in Tomato GUI', quiet);
		ok = 0; }
	if (s.value.search(/bt-hash-check-seed=/) == 0)  {
		ferror.set(s, 'Cannot set "bt-hash-check-seed" option here. You can set it in Tomato GUI', quiet);
		ok = 0; }
	if (s.value.search(/bt-seed-unverified=/) == 0)  {
		ferror.set(s, 'Cannot set "bt-seed-unverified" option here. You can set it in Tomato GUI', quiet);
		ok = 0; }
	if (s.value.search(/bt-save-metadata=/) == 0)  {
		ferror.set(s, 'Cannot set "bt-save-metadata" option here. You can set it in Tomato GUI', quiet);
		ok = 0; }
	if (s.value.search(/save-session-interval=/) == 0)  {
		ferror.set(s, 'Cannot set "save-session-interval" option here. You can set it in Tomato GUI', quiet);
		ok = 0; }
		
	return ok;
}

function save()
{
  if (verifyFields(null, 0)==0) return;
  var fom = E('_fom');
  
  fom.aria2_enable.value               = E('_f_aria2_enable_all').checked ? 1 : 0;
  fom.aria2_check.value                = E('_f_aria2_check').checked ? 1 : 0;
  fom.aria2_enable_rpc.value           = E('_f_aria2_enable_rpc').checked ? 1 : 0;
  fom.aria2_rpc_allow_origin_all.value = E('_f_aria2_rpc_allow_origin_all').checked ? 1 : 0;
  fom.aria2_rpc_listen_all.value       = E('_f_aria2_rpc_listen_all').checked ? 1 : 0;
  fom.aria2_continue.value             = E('_f_aria2_continue').checked ? 1 : 0;
  fom.aria2_enable_mmap.value          = E('_f_aria2_enable_mmap').checked ? 1 : 0;
  fom.aria2_bt_enable_lpd.value        = E('_f_aria2_bt_enable_lpd').checked ? 1 : 0;
  fom.aria2_bt_require_crypto.value    = E('_f_aria2_bt_require_crypto').checked ? 1 : 0;
  fom.aria2_enable_dht.value           = E('_f_aria2_enable_dht').checked ? 1 : 0;
  fom.aria2_enable_peer_exchange.value = E('_f_aria2_enable_peer_exchange').checked ? 1 : 0;
  fom.aria2_force_save.value           = E('_f_aria2_force_save').checked ? 1 : 0;
  fom.aria2_bt_hash_check_seed.value   = E('_f_aria2_bt_hash_check_seed').checked ? 1 : 0;
  fom.aria2_bt_seed_unverified.value   = E('_f_aria2_bt_seed_unverified').checked ? 1 : 0;
  fom.aria2_bt_save_metadata.value     = E('_f_aria2_bt_save_metadata').checked ? 1 : 0;
  fom.aria2_follow_torrent.value       = E('_f_aria2_follow_torrent').checked ? 1 : 0;
  fom.aria2_usb_enable.value           = E('_f_aria2_usb_enable').checked ? 1 : 0;
	
  if (fom.aria2_enable.value == 0) {
  	fom._service.value = 'aria2-stop';
  }
  else {
  	fom._service.value = 'aria2-restart'; 
  }
  form.submit('_fom', 1);
}

function init()
{
}
</script>
</head>

<body onLoad="init()">
<table id='container' cellspacing=0>
<tr><td colspan=2 id='header'>
<div class='title'>Tomato</div>
<div class='version'>Version <% version(); %></div>
</td></tr>
<tr id='body'><td id='navi'><script type='text/javascript'>navi()</script></td>
<td id='content'>
<div id='ident'><% ident(); %></div>
<form id='_fom' method='post' action='tomato.cgi'>
<input type='hidden' name='_nextpage' value='nas-aria2.asp'>
<input type='hidden' name='_service' value='aria2-restart'>
<input type='hidden' name='aria2_enable'>
<input type='hidden' name='aria2_enable_rpc'>
<input type='hidden' name='aria2_check'>
<input type='hidden' name='aria2_rpc_allow_origin_all'>
<input type='hidden' name='aria2_rpc_listen_all'>
<input type='hidden' name='aria2_continue'>
<input type='hidden' name='aria2_enable_mmap'>
<input type='hidden' name='aria2_bt_enable_lpd'>
<input type='hidden' name='aria2_bt_require_crypto'>
<input type='hidden' name='aria2_follow_torrent'>
<input type='hidden' name='aria2_enable_dht'>
<input type='hidden' name='aria2_enable_peer_exchange'>
<input type='hidden' name='aria2_force_save'>
<input type='hidden' name='aria2_bt_hash_check_seed'>
<input type='hidden' name='aria2_bt_seed_unverified'>
<input type='hidden' name='aria2_bt_save_metadata'>
<input type='hidden' name='aria2_usb_enable'>

<div class='section-title'>Basic Settings</div>
<div class='section' id='config-section'>
<script type='text/javascript'>
	
refresh_usb_disk();

createFieldTable('', [
	{ title: 'Enable aria2 client', name: 'f_aria2_enable_all', type: 'checkbox', value: nvram.aria2_enable == 1, suffix: ' <small>*</small>' },
	{ title: 'Aria2c binary path', multi: [
		{ name: 'aria2_binary', type: 'select', options: [
			['internal','Internal (/usr/bin)'],
			['optware','Optware (/opt/bin)'],
			['custom','Custom'] ], value: nvram.aria2_binary, suffix: ' <small>*</small> ' },
		{ name: 'aria2_binary_custom', type: 'text', maxlen: 40, size: 40, value: nvram.aria2_binary_custom , suffix: ' <small>Not include "/aria2c"</small>' }
	] },
	{ title: 'Keep alive', name: 'f_aria2_check', type: 'checkbox', value: nvram.aria2_check == 1, suffix: ' <small>*</small>' },
	{ title: 'Check alive every', indent: 2, name: 'aria2_check_time', type: 'text', maxlen: 5, size: 7, value: nvram.aria2_check_time, suffix: ' <small>minutes (range: 1 - 55; default: 15)</small>' },
	{ title: 'Delay at startup', name: 'aria2_sleep', type: 'text', maxlen: 5, size: 7, value: nvram.aria2_sleep, suffix: ' <small>seconds (range: 1 - 60; default: 10)</small>' },
	{ title: 'Enable USB Partition', multi: [
		{ name: 'f_aria2_usb_enable', type: 'checkbox', value: nvram.aria2_usb_enable == 1, suffix: '  ' },
		{ name: 'aria2_dlroot', type: 'select', options: usb_disk_list, value: nvram.aria2_dlroot, suffix: ' '} ] },
	{ title: 'Download saved dir.', indent: 2, name: 'aria2_dir', type: 'text', maxlen: 50, size: 40, value: nvram.aria2_dir, suffix: ' <small>Directory name under mounted partition.</small>' },
	{ title: 'Continue download', name: 'f_aria2_continue', type: 'checkbox', value: nvram.aria2_continue == 1, suffix: ' <small>*</small>' },
	{ title: 'Max Tries', name: 'aria2_max_tries', type: 'text', maxlen: 16, size: 7, value: nvram.aria2_max_tries, suffix: ' <small>(range: 0 - 9999; default: 0 - unlimited)</small>' },
	{ title: 'Retry Wait Time', name: 'aria2_retry_wait', type: 'text', maxlen: 16, size: 7, value: nvram.aria2_retry_wait, suffix: ' <small>seconds (range: 0 - 3600; default: 10)</small>' },
	{ title: 'Referer (for v1.16+)', name: 'aria2_referer', type: 'text', maxlen: 1024, size: 15, value: nvram.aria2_referer },
	{ title: 'Disk cache size (for v1.16+)', name: 'aria2_disk_cache', type: 'text', maxlen: 16, size: 7, value: nvram.aria2_disk_cache, suffix: ' <small>( e.g. 10K, 5M, 1024K etc.; default: 0 - No disk cache)</small>' },
	{ title: 'Enable MMAP', name: 'f_aria2_enable_mmap', type: 'checkbox', value: nvram.aria2_enable_mmap == 1, suffix: ' <small>*</small>' },
	{ title: 'File allocation method', name: 'aria2_file_allocation', type: 'select', options: [ ['prealloc','Prealloc'], ['trunc','Trunc'], ['falloc','Falloc'], ['none','None*'] ], value: nvram.aria2_file_allocation, suffix: ' ' },
	{ title: 'Aria2 custom configuration', name: 'aria2_custom', type: 'textarea', value: nvram.aria2_custom }
]);
</script>
	<ul>
		<li><b>Enable aria2 client</b> - Caution! - If your router only has 32MB of RAM, you'll have to use swap.
		<li><b>Aria2 binary path</b> - Path to the directory containing aria2c etc. Not include program name "/aria2c"
		<li><b>Keep alive</b> - If enabled, aria2c will be checked at the specified interval and will re-launch after a crash.
	</ul>
</div>
<div class='section-title'>RPC Remote Access<script>W(btgui_link);</script></div>
<div class='section'>
<script type='text/javascript'>
createFieldTable('', [
	{ title: 'Enable RPC', name: 'f_aria2_enable_rpc', type: 'checkbox', value: nvram.aria2_enable_rpc == 1, suffix: ' <small>*</small>' },
	{ title: 'RPC listening port', name: 'aria2_rpc_listen_port', type: 'text', maxlen: 5, size: 7, value: nvram.aria2_rpc_listen_port, suffix: ' <small>*</small>' },
	{ title: 'RPC allow origin all', name: 'f_aria2_rpc_allow_origin_all', type: 'checkbox', value: nvram.aria2_rpc_allow_origin_all == 1, suffix: ' <small>*</small>' },
	{ title: 'RPC listen all', name: 'f_aria2_rpc_listen_all', type: 'checkbox', value: nvram.aria2_rpc_listen_all == 1, suffix: ' <small>*</small>' },
	{ title: 'Event poll', name: 'aria2_event_poll', type: 'select', options: [ ['select','Select'], ['poll','Poll'], ['port','Port'], ['kqueue','KQueue'], ['epoll','EPoll'] ], value: nvram.aria2_event_poll, suffix: ' ' },
	{ title: 'RPC Username', name: 'aria2_rpc_user', type: 'text', maxlen: 32, size: 15, value: nvram.aria2_rpc_user },
	{ title: 'RPC Password', name: 'aria2_rpc_passwd', type: 'password', maxlen: 32, size: 15, value: nvram.aria2_rpc_passwd }
]);
</script>
	<ul>
		<li><b>RPC listening port</b> - Port used for rpc service. Make sure this port is not in use.
		<li><b>RPC Username/Password</b> - In yaaw, you should use <b>http://username:passwd@hostname:port/jsonrpc</b> to access GUI.
	</ul>
</div>
<div class='section-title'>Limits</div>
<div class='section'>
<script type='text/javascript'>
createFieldTable('', [
	{ title: 'Max concurrent downloads', name: 'aria2_max_concurrent_downloads', type: 'text', maxlen: 10, size: 7, value: nvram.aria2_max_concurrent_downloads, suffix: ' <small>(range: 1 - 100; default: 5)</small>' },
	{ title: 'Max connections per server', name: 'aria2_max_connection_per_server', type: 'text', maxlen: 10, size: 7, value: nvram.aria2_max_connection_per_server, suffix: ' <small>(range: 1 - 16; default: 5)</small>' },
	{ title: 'Minimum split size', name: 'aria2_min_split_size', type: 'text', maxlen: 20, size: 10, value: nvram.aria2_min_split_size, suffix: ' <small>(range: 1M - 1024M; default: 10M)</small>' },
	{ title: 'No of connections per file', name: 'aria2_split', type: 'text', maxlen: 10, size: 10, value: nvram.aria2_split, suffix: ' <small>(range: 1 - 100; default: 5)</small>' },
	{ title: 'Max overall download limit', name: 'aria2_max_overall_download_limit', type: 'text', maxlen: 16, size: 10, value: nvram.aria2_max_overall_download_limit, suffix: ' <small>( e.g. 10K, 5M, 1024K etc.; default: 0 - No limit)</small>' },
	{ title: 'Max speed per download', name: 'aria2_max_download_limit', type: 'text', maxlen: 16, size: 10, value: nvram.aria2_max_download_limit, suffix: ' <small>( e.g. 10K, 5M, 1024K etc.; default: 0 - No limit)</small>' },
	{ title: 'Max overall upload limit', name: 'aria2_max_overall_upload_limit', type: 'text', maxlen: 16, size: 10, value: nvram.aria2_max_overall_upload_limit, suffix: ' <small>( e.g. 10K, 5M, 1024K etc.; default: 0 - No limit)</small>' },
	{ title: 'Max speed per upload', name: 'aria2_max_upload_limit', type: 'text', maxlen: 16, size: 10, value: nvram.aria2_max_upload_limit, suffix: ' <small>( e.g. 10K, 5M, 1024K etc.; default: 0 - No limit)</small>' },
	{ title: 'Lowest speed limit', name: 'aria2_lowest_speed_limit', type: 'text', maxlen: 16, size: 10, value: nvram.aria2_lowest_speed_limit, suffix: ' <small>( e.g. 10K, 5M, 1024K etc.; default: 0 - No limit)</small>' }
]);
</script>
</div>
<div class='section-title'>BitTorrent Settings</div>
<div class='section'>
<script type='text/javascript'>
createFieldTable('', [
	{ title: 'BitTorrent Enable LPD', name: 'f_aria2_bt_enable_lpd', type: 'checkbox', value: nvram.aria2_bt_enable_lpd == 1, suffix: ' <small>*</small>' },
	{ title: 'BT enable DHT', name: 'f_aria2_enable_dht', type: 'checkbox', value: nvram.aria2_enable_dht == 1, suffix: ' <small>*</small>' },
	{ title: 'DHT listening port', name: 'aria2_dht_listen_port', type: 'text', maxlen: 50, size: 50, value: nvram.aria2_dht_listen_port, suffix: ' <small>default: 6881-6999</small>' },
	{ title: 'BT tracker announce', name: 'aria2_bt_tracker', type: 'text', maxlen: 256, size: 64, value: nvram.aria2_bt_tracker },
	{ title: 'Max peers per torrent', name: 'aria2_bt_max_peers', type: 'text', maxlen: 10, size: 7, value: nvram.aria2_bt_max_peers, suffix: ' <small>(range: 1 - 9999; default: 55)</small>' },
	{ title: 'BT Require Crypto', name: 'f_aria2_bt_require_crypto', type: 'checkbox', value: nvram.aria2_bt_require_crypto == 1, suffix: ' <small>*</small>' },
	{ title: 'Follow torrent', name: 'f_aria2_follow_torrent', type: 'checkbox', value: nvram.aria2_follow_torrent == 1, suffix: ' <small>*</small>' },
	{ title: 'BT listening port', name: 'aria2_listen_port', type: 'text', maxlen: 50, size: 50, value: nvram.aria2_listen_port, suffix: ' <small>*</small>' },
	{ title: 'BT enable peer exchange', name: 'f_aria2_enable_peer_exchange', type: 'checkbox', value: nvram.aria2_enable_peer_exchange == 1, suffix: ' <small>*</small>' },
	{ title: 'User agent', name: 'aria2_user_agent', type: 'text', maxlen: 64, size: 50, value: nvram.aria2_user_agent },
	{ title: 'Prefix of peer id', name: 'aria2_peer_id_prefix', type: 'text', maxlen: 64, size: 15, value: nvram.aria2_peer_id_prefix },
	{ title: 'Shared ratio', name: 'aria2_seed_ratio', type: 'text', maxlen: 64, size: 15, value: nvram.aria2_seed_ratio, suffix: ' <small>(range: 0.0 - 9999; default: 1.0)</small>' },
	{ title: 'Enable force save session', name: 'f_aria2_force_save', type: 'checkbox', value: nvram.aria2_force_save == 1, suffix: ' <small>*</small>' },
	{ title: 'Save session interval', indent: 2, name: 'aria2_save_session_interval', type: 'text', maxlen: 20, size: 7, value: nvram.aria2_save_session_interval, suffix: ' <small>seconds (range: 0 - 3600; default: 60)</small>' },
	{ title: 'Input URIs file', indent:2, name: 'aria2_input_file', type: 'text', maxlen: 50, size: 50, value: nvram.aria2_input_file },
	{ title: 'Session saved file', indent:2, name: 'aria2_save_session', type: 'text', maxlen: 50, size: 50, value: nvram.aria2_save_session },
	{ title: 'Enable BT hash check to seed', name: 'f_aria2_bt_hash_check_seed', type: 'checkbox', value: nvram.aria2_bt_hash_check_seed == 1, suffix: ' <small>*</small>' },
	{ title: 'Enable BT seed unverified', name: 'f_aria2_bt_seed_unverified', type: 'checkbox', value: nvram.aria2_bt_seed_unverified == 1, suffix: ' <small>*</small>' },
	{ title: 'Enable BT save metadata', name: 'f_aria2_bt_save_metadata', type: 'checkbox', value: nvram.aria2_bt_save_metadata == 1, suffix: ' <small>*</small>' }
]);
</script>
<ul>                                                                                                                                    
	<li><b>Input URIs file</b> - Full path for downloading URIs saved file. If leave blank, /mounted_sd_part/download_dir/aria2.session will be used.
	<li><b>Session saved file</b> - Full path for saving session. If leave blank, /mounted_sd_part/download_dir/aria2.session will be used.
	<li><b>BitTorrent listening port</b> - Port used for BitTorrent. Make sure this port is not in use.                             
</ul>                                                                                                                                   
</div>
</form>
</div>
</td></tr>
<tr><td id='footer' colspan=2>
 <form>
 <span id='footer-msg'></span>
 <input type='button' value='Save' id='save-button' onclick='save()'>
 <input type='button' value='Cancel' id='cancel-button' onclick='javascript:reloadPage();'>
 </form>
</div>
</td></tr>
</table>
<script type='text/javascript'>verifyFields(null, 1);</script>
</body>
</html>
