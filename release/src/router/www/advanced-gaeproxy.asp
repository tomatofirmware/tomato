<!DOCTYPE HTML PUBLIC '-//W3C//DTD HTML 4.0//EN'>
<!--
Tomato GUI
Copyright (C) 2013 Hyzoom bwq518
http://www.polarcloud.com/tomato/
For use with Tomato Firmware only.
No part of this file may be used without permission.
-->
<html>
<head>
<meta http-equiv='content-type' content='text/html;charset=utf-8'>
<meta name='robots' content='noindex,nofollow'>
<title>[TomatoUSB] 高级设置：GAE代理服务器设置</title>
<link rel='stylesheet' type='text/css' href='tomato.css'>
<link rel='stylesheet' type='text/css' href='hyzoom.css'>
<script type='text/javascript' src='tomato.js'></script>
<script type='text/javascript'>
//	<% nvram("gaeproxy_enable,gaeproxy_sleep,gaeproxy_check,gaeproxy_check_time,gaeproxy_binary,gaeproxy_binary_custom,gaeproxy_binary_download,gaeproxy_listen_ip,gaeproxy_listen_port,gaeproxy_listen_web_username,gaeproxy_listen_web_password,gaeproxy_listen_username,gaeproxy_listen_password,gaeproxy_listen_transparent_enable,gaeproxy_pac_enable,gaeproxy_pac_file_enable,geproxy_pac_file,gaeproxy_pac_https_mode,gaeproxy_gae_enable,gaeproxy_gae_appid,gaeproxy_gae_password,gaeproxy_gae_listen,gaeproxy_gae_profile,gaeproxy_gae_max_threads,gaeproxy_gae_fetch_mode,gaeproxy_hosts_enable,gaeproxy_hosts_dns,gaeproxy_hosts_resolve,gaeproxy_autorange_enable,gaeproxy_autorange_maxsize,gaeproxy_autorange_waitsize,gaeproxy_autorange_bufsize,gaeproxy_proxy_enable,gaeproxy_proxy_host,gaeproxy_proxy_port,gaeproxy_proxy_username,gaeproxy_proxy_password,gaeproxy_useragent_enable,gaeproxy_useragent_match,gaeproxy_useragent_rules,gaeproxy_useragent_string,gaeproxy_autoupload_enable,gaeproxy_autoupload_gmail,gaeproxy_autoupload_password,gaeproxy_paas_enable,gaeproxy_paas_password,gaeproxy_paas_listen,gaeproxy_paas_fetchserver,gaeproxy_paas_proxy_enable,gaeproxy_paas_proxy,http_lanport,wl_unit,http_id,web_mx,web_pb"); %>

var proxygui_link = '&nbsp;&nbsp;<a href="http://' + location.hostname +':8086/" target="_blank"><i>[点击此处打开智能代理内置配置界面]</i></a>';
var autoupload_log_link = '&nbsp;&nbsp;<a href="http://' + location.hostname + ':' + 80 + '/ext/autoupload.txt" target="_blank"><i>[点击此处打开上载日志]</i></a>';
function verifyFields(focused, quiet)
{
var ok = 1;
var a = E('_f_gaeproxy_enable').checked;
var b = E('_f_gaeproxy_check').checked;
var c = E('_f_gaeproxy_pac_enable').checked;
var d = E('_f_gaeproxy_gae_enable').checked;
var e = E('_f_gaeproxy_hosts_enable').checked;
var g = E('_f_gaeproxy_autorange_enable').checked;
var h = E('_f_gaeproxy_proxy_enable').checked;
var i = E('_f_gaeproxy_autoupload_enable').checked;
var j = E('_f_gaeproxy_useragent_enable').checked;
var k = E('_f_gaeproxy_pac_file_enable').checked;
var l = E('_f_gaeproxy_paas_enable').checked;
var m = E('_f_gaeproxy_paas_proxy_enable').checked;
E('_f_gaeproxy_check').disabled = !a;
E('_gaeproxy_check_time').disabled = !a || !b;
E('_gaeproxy_binary').disabled = !a;
E('_gaeproxy_binary_custom').disabled = !a;
E('_gaeproxy_binary_download').disabled = !a;
E('_gaeproxy_sleep').disabled = !a;
E('_gaeproxy_listen_ip').disabled = !a;
E('_gaeproxy_listen_port').disabled = !a;
E('_gaeproxy_listen_web_username').disabled = !a;
E('_gaeproxy_listen_web_password').disabled = !a;
E('_gaeproxy_listen_username').disabled = !a;
E('_gaeproxy_listen_password').disabled = !a;
E('_f_gaeproxy_listen_transparent_enable').disabled = 1;
E('_f_gaeproxy_pac_enable').disabled = !a;
E('_f_gaeproxy_pac_file_enable').disabled = !a;
E('_gaeproxy_pac_file').disabled = !a || !c || !k;
E('_gaeproxy_pac_https_mode').disabled = !a || !c;
E('_f_gaeproxy_gae_enable').disabled = !a;
E('_gaeproxy_gae_appid').disabled = !a || !d;
E('_gaeproxy_gae_password').disabled = !a || !d;
E('_gaeproxy_gae_listen').disabled = !a || !d;
E('_gaeproxy_gae_profile').disabled = !a || !d;
E('_gaeproxy_gae_max_threads').disabled = !a || !d;
E('_gaeproxy_gae_fetch_mode').disabled = !a || !d;
E('_f_gaeproxy_hosts_enable').disabled = !a;
E('_gaeproxy_hosts_dns').disabled = !a || !e;
E('_gaeproxy_hosts_resolve').disabled = !a || !e;
E('_f_gaeproxy_autorange_enable').disabled = !a;
E('_gaeproxy_autorange_maxsize').disabled = !a || !g;
E('_gaeproxy_autorange_waitsize').disabled = !a || !g;
E('_gaeproxy_autorange_bufsize').disabled = !a || !g;
E('_f_gaeproxy_proxy_enable').disabled = !a;
E('_gaeproxy_proxy_host').disabled = !a || !h;
E('_gaeproxy_proxy_port').disabled = !a || !h;
E('_gaeproxy_proxy_username').disabled = !a || !h;
E('_gaeproxy_proxy_password').disabled = !a || !h;
E('_f_gaeproxy_useragent_enable').disabled = !a;
E('_gaeproxy_useragent_match').disabled = !a || !j
E('_gaeproxy_useragent_rules').disabled = !a || !j
E('_gaeproxy_useragent_string').disabled = !a || !j
E('_f_gaeproxy_autoupload_enable').disabled = !a;
E('_gaeproxy_autoupload_gmail').disabled = !a || !i;
E('_gaeproxy_autoupload_password').disabled = !a || !i;
E('_f_gaeproxy_paas_enable').disabled = !a;
E('_gaeproxy_paas_password').disabled = !a || !l;
E('_gaeproxy_paas_listen').disabled = !a || !l;
E('_gaeproxy_paas_fetchserver').disabled = !a || !l;
E('_f_gaeproxy_paas_proxy_enable').disabled = !a || !l;
E('_gaeproxy_paas_proxy').disabled = !a || !l || !m;
if (!v_port('_gaeproxy_listen_port', quiet)) ok = 0;
if (!v_port('_gaeproxy_gae_listen', quiet)) ok = 0;
if (!v_port('_gaeproxy_paas_listen', quiet)) ok = 0;
var p = (E('_gaeproxy_binary').value == 'custom');
elem.display('_gaeproxy_binary_custom', p && a);
var q = (E('_gaeproxy_binary').value == 'download');
elem.display('_gaeproxy_binary_download', q && a);
return ok;
}
function save()
{
if (verifyFields(null, 0)==0) return;
var fom = E('_fom');
fom.gaeproxy_enable.value =          E('_f_gaeproxy_enable').checked ? 1 : 0;
fom.gaeproxy_check.value =           E('_f_gaeproxy_check').checked ? 1 : 0;
fom.gaeproxy_pac_enable.value =      E('_f_gaeproxy_pac_enable').checked ? 1 : 0;
fom.gaeproxy_pac_file_enable.value = E('_f_gaeproxy_pac_file_enable').checked ? 1 : 0;
fom.gaeproxy_gae_enable.value =      E('_f_gaeproxy_gae_enable').checked ? 1 : 0;
fom.gaeproxy_hosts_enable.value =    E('_f_gaeproxy_hosts_enable').checked ? 1 : 0;
fom.gaeproxy_autorange_enable.value =E('_f_gaeproxy_autorange_enable').checked ? 1 : 0;
fom.gaeproxy_proxy_enable.value =    E('_f_gaeproxy_proxy_enable').checked ? 1 : 0;
fom.gaeproxy_autoupload_enable.value=E('_f_gaeproxy_autoupload_enable').checked ? 1 : 0;
fom.gaeproxy_useragent_enable.value =E('_f_gaeproxy_useragent_enable').checked ? 1 : 0;
fom.gaeproxy_listen_transparent_enable.value =E('_f_gaeproxy_listen_transparent_enable').checked ? 1 : 0;
fom.gaeproxy_paas_enable.value =     E('_f_gaeproxy_paas_enable').checked ? 1 : 0;
fom.gaeproxy_paas_proxy_enable.value=E('_f_gaeproxy_paas_proxy_enable').checked ? 1 : 0;
if (fom.gaeproxy_enable.value == 0) {
fom._service.value = 'gaeproxy-stop';
}
else {
fom._service.value = 'gaeproxy-restart'; 
}
form.submit('_fom', 1);
}
</script>
</head>
<body onLoad="init()">
<form id='_fom' method='post' action='tomato.cgi'>
<table id='container' cellspacing=0>
<tr><td colspan=2 id='header'>
<div class='title'>Tomato</div>
<div class='version'>1.28.0115 MIPSR2-20131204.Hyzoom.RT_AC K26AC USB 16M-AIO-AR-TR-PY-GAE-64K</div>
</td></tr>
<tr id='body'><td id='navi'><script type='text/javascript'>navi()</script></td>
<td id='content'>
<div id='ident'>TomatoUSB</div>
<input type='hidden' name='_nextpage' value='advanced-gaeproxy.asp'>
<input type='hidden' name='_service' value='gaeproxy-restart'>
<input type='hidden' name='gaeproxy_enable'>
<input type='hidden' name='gaeproxy_check'>
<input type='hidden' name='gaeproxy_pac_enable'>
<input type='hidden' name='gaeproxy_gae_enable'>
<input type='hidden' name='gaeproxy_hosts_enable'>
<input type='hidden' name='gaeproxy_autorange_enable'>
<input type='hidden' name='gaeproxy_proxy_enable'>
<input type='hidden' name='gaeproxy_autoupload_enable'>
<input type='hidden' name='gaeproxy_useragent_enable'>
<input type='hidden' name='gaeproxy_listen_transparent_enable'>
<input type='hidden' name='gaeproxy_pac_file_enable'>
<input type='hidden' name='gaeproxy_paas_enable'>
<input type='hidden' name='gaeproxy_paas_proxy_enable'>
<!-- Menu Section Begin -->
<div class='section-title'>基本设置<script>W(proxygui_link);</script></div>
<div class='section' id='config-section'>
<script type='text/javascript'>
createFieldTable('', [
{ title: '启用智能代理', name: 'f_gaeproxy_enable', type: 'checkbox', value: nvram.gaeproxy_enable == 1, suffix: ' <small>*</small>' },
{ title: '启动文件startup.py位于', multi: [
{ name: 'gaeproxy_binary', type: 'select', options: [
['internal','内置 (.../gaeproxy/local/startup.py)'],
['optware','外置 (/mnt/opt/wallproxy/local/startup.py)'],
['download','下载'],
['custom','自定义'] ], value: nvram.gaeproxy_binary, suffix: ' <small>*</small> ' },
{ name: 'gaeproxy_binary_download', type: 'text', maxlen: 128, size: 50, value: nvram.gaeproxy_binary_download },
{ name: 'gaeproxy_binary_custom', type: 'text', maxlen: 50, size: 50, value: nvram.gaeproxy_binary_custom }
] },
{ title: '启用守护进程', name: 'f_gaeproxy_check', type: 'checkbox', value: nvram.gaeproxy_check == 1, suffix: ' <small>*</small>' },
{ title: '检测时间间隔', indent: 2, name: 'gaeproxy_check_time', type: 'text', maxlen: 5, size: 7, value: nvram.gaeproxy_check_time, suffix: ' <small>分钟 (范围: 1 - 55; 缺省: 5)</small>' },
{ title: '启动延迟', name: 'gaeproxy_sleep', type: 'text', maxlen: 5, size: 7, value: nvram.gaeproxy_sleep, suffix: ' <small>秒 (范围: 1 - 60; 默认: 10)</small>' },
{ title: '智能代理监听IP地址', name: 'gaeproxy_listen_ip', type: 'text', maxlen: 15, size: 17, value: nvram.gaeproxy_listen_ip, suffix: ' <small>*</small>' },
{ title: '智能代理监听端口', name: 'gaeproxy_listen_port', type: 'text', maxlen: 5, size: 7, value: nvram.gaeproxy_listen_port, suffix: ' <small>*</small>' },
{ title: '访问代理用户名', name: 'gaeproxy_listen_username', type: 'text', maxlen: 32, size: 15, value: nvram.gaeproxy_listen_username },
{ title: '访问代理密码', name: 'gaeproxy_listen_password', type: 'password', maxlen: 32, size: 15, value: nvram.gaeproxy_listen_password },
{ title: 'Web界面访问用户名', name: 'gaeproxy_listen_web_username', type: 'text', maxlen: 32, size: 15, value: nvram.gaeproxy_listen_web_username },
{ title: 'Web界面访问密码', name: 'gaeproxy_listen_web_password', type: 'password', maxlen: 32, size: 15, value: nvram.gaeproxy_listen_web_password },
{ title: '启用透明代理', name: 'f_gaeproxy_listen_transparent_enable', type: 'checkbox', value: nvram.gaeproxy_listen_transparent_enable == 1 }
]);  
</script>
<ul>
<li><b>启用智能代理</b> - 注意! - 如果你的路由器只有32MB内存，你必须启用交换内存.
<li><b>启动文件Startup.py位于</b> - 若选择'自定义', 必须输入含startup.py的完整路径；若选择'下载'，须输入HTTP或FTP链接地址.
<li><b>启用智能代理</b> - 注意! - 如果你的路由器只有32MB内存，你必须启用交换内存.
<li><b>启用守护进程</b> - 如果启用，则每隔一段时间检测智能代理进程，若进程崩溃则重新启动之.
<li><b>访问代理用户名/密码</b> - 本地端共享给他人使用时为防止盗用而设置用户名和密码.
<li><b>Web界面访问用户名/密码</b> - 通过Web界面配置智能代理参数时，可以设置用户名和密码以加强安全性.
<li><b>启用透明代理</b> - 截获80端口的数据转发至智能代理的端口，其行为类似透明代理.
</ul>
</div>
<div class='section-title'>GAE 设置</div>
<div class='section' id='config-section'>
<script type='text/javascript'>
createFieldTable('', [
{ title: '启用 GAE', name: 'f_gaeproxy_gae_enable', type: 'checkbox', value: nvram.gaeproxy_gae_enable == 1, suffix: ' <small>*</small>' },
{ title: 'GAE APPID', name: 'gaeproxy_gae_appid', type: 'text', maxlen: 512, size: 64, value: nvram.gaeproxy_gae_appid, suffix: ' <small>最大appid数量为10.</small>' },
{ title: 'GAE 监听端口', name: 'gaeproxy_gae_listen', type: 'text', maxlen: 5, size: 7, value: nvram.gaeproxy_gae_listen, suffix: ' <small>*</small>' },
{ title: 'GAE 访问密码', name: 'gaeproxy_gae_password', type: 'password', maxlen: 32, size: 15, value: nvram.gaeproxy_gae_password },
{ title: 'GAE 访问策略', name: 'gaeproxy_gae_profile', type: 'select', options: [ ['google_hk','google_hk*'], ['google_cn','google_cn'], ['google_ipv6','google_ipv6'] ], value: nvram.gaeproxy_gae_profile, suffix: ' ' },
{ title: '同资源最大线程数', name: 'gaeproxy_gae_max_threads', type: 'text', maxlen: 10, size: 7, value: nvram.gaeproxy_gae_max_threads, suffix: ' <small>(范围: 0 - 10; 缺省: 3)</small>' },
{ title: '数据缓冲方式', name: 'gaeproxy_gae_fetch_mode', type: 'select', options: [ ['0','平滑缓冲'], ['1','部分缓冲*'], ['2','无缓冲'] ], value: nvram.gaeproxy_gae_fetch_mode, suffix: ' ' }
]);  
</script>
<ul>
<li><b>最大线程数</b> - 分段下载时每个资源的最大线程数(0 按照appid个数自动确定, 最大10).
<li><b>数据缓冲方式</b> - 0平滑缓冲，但无法确保稳定性；1或2可确保稳定性，但无法平滑缓冲；建议在网络稳定时设置0，不稳定时设置1.
</ul>
</div>
<div class='section-title'>PAC自动代理设置</div>
<div class='section' id='config-section'>
<script type='text/javascript'>
createFieldTable('', [
{ title: '启用PAC自动代理', name: 'f_gaeproxy_pac_enable', type: 'checkbox', value: nvram.gaeproxy_pac_enable == 1, suffix: ' <small>*</small>' },
{ title: '启用PAC文件', multi: [
{ name: 'f_gaeproxy_pac_file_enable', type: 'checkbox', value: nvram.gaeproxy_pac_file_enable == 1, suffix: '  ' },
{ name: 'gaeproxy_pac_file', type: 'text', maxlen: 256, size: 64, value: nvram.gaeproxy_pac_file } ] },
{ title: 'HTTPS 连接方式', name: 'gaeproxy_pac_https_mode', type: 'select', options: [ ['0','全直连,证书非伪造'], ['1','全走HTTPS,证书全伪造*'], ['2','根据ruleslist/iplist,部分直连,证书部分伪造'] ], value: nvram.gaeproxy_pac_https_mode, suffix: ' ' }
]);  
</script>
<ul>
<li><b>PAC 文件名</b> - 若设置PAC文件名，工作在浏览器PAC模式，自动更新PAC（不依赖GUI），但智能代理将失效.
</ul>
</div>
<div class='section-title'>自动分段下载</div>
<div class='section' id='config-section'>
<script type='text/javascript'>
createFieldTable('', [
{ title: '启用自动分段下载', name: 'f_gaeproxy_autorange_enable', type: 'checkbox', value: nvram.gaeproxy_autorange_enable == 1, suffix: ' <small>*</small>' },
{ title: '每段最大长度', name: 'gaeproxy_autorange_maxsize', type: 'text', maxlen: 10, size: 8, value: nvram.gaeproxy_autorange_maxsize, suffix: ' <small>字节 (范围：0 - 99999999；缺省：1000000)</small>' },
{ title: '首次读取长度', name: 'gaeproxy_autorange_waitsize', type: 'text', maxlen: 10, size: 8, value: nvram.gaeproxy_autorange_waitsize, suffix: ' <small>字节 (范围：0 - 99999999；缺省：1000000)</small>' },
{ title: '后续每次读写长度', name: 'gaeproxy_autorange_bufsize', type: 'text', maxlen: 10, size: 8, value: nvram.gaeproxy_autorange_bufsize, suffix: ' <small>字节 (范围：0 - 1024000；缺省：8192)</small>' }
]);  
</script>
</div>
<div class='section-title'>PAAS服务器设置</div>
<div class='section' id='config-section'>
<script type='text/javascript'>
createFieldTable('', [
{ title: '启用PAAS服务器', name: 'f_gaeproxy_paas_enable', type: 'checkbox', value: nvram.gaeproxy_paas_enable == 1, suffix: ' <small>默认：不启用</small>' },
{ title: '防盗用密码', name: 'gaeproxy_paas_password', type: 'password', maxlen: 32, size: 15, value: nvram.gaeproxy_paas_password },
{ title: 'PAAS监听端口', name: 'gaeproxy_paas_listen', type: 'text', maxlen: 5, size: 7, value: nvram.gaeproxy_paas_listen },
{ title: '服务端地址', name: 'gaeproxy_paas_fetchserver', type: 'text', maxlen: 512, size: 64, value: nvram.gaeproxy_paas_fetchserver },
{ title: '启用上级代理', multi: [
{ name: 'f_gaeproxy_paas_proxy_enable', type: 'checkbox', value: nvram.gaeproxy_paas_proxy_enable == 1, suffix: '  ' },
{ name: 'gaeproxy_paas_proxy', type: 'text', maxlen: 128, size: 64, value: nvram.gaeproxy_paas_proxy } ] }
]);  
</script>
</div>
<div class='section-title'>主机Hosts规则</div>
<div class='section' id='config-section'>
<script type='text/javascript'>
createFieldTable('', [
{ title: '启用 Hosts规则', name: 'f_gaeproxy_hosts_enable', type: 'checkbox', value: nvram.gaeproxy_hosts_enable == 1, suffix: ' <small>IPv6的用户应禁用.*</small>' },
{ title: 'DNS', name: 'gaeproxy_hosts_dns', type: 'text', maxlen: 128, size: 32, value: nvram.gaeproxy_hosts_dns },
{ title: '远程DNS解析的域名', name: 'gaeproxy_hosts_resolve', type: 'text', maxlen: 1024, size: 100, value: nvram.gaeproxy_hosts_resolve }
]);  
</script>
</div>
<div class='section-title'>上级代理设置</div>
<div class='section' id='config-section'>
<script type='text/javascript'>
createFieldTable('', [
{ title: '启用上级代理', name: 'f_gaeproxy_proxy_enable', type: 'checkbox', value: nvram.gaeproxy_proxy_enable == 1, suffix: ' <small> * 如果通过上级代理上网，请启用.</small>' },
{ title: '上级代理IP地址', name: 'gaeproxy_proxy_host', type: 'text', maxlen: 15, size: 17, value: nvram.gaeproxy_proxy_host, suffix: ' <small>*</small>' },
{ title: '上级代理端口', name: 'gaeproxy_proxy_port', type: 'text', maxlen: 5, size: 7, value: nvram.gaeproxy_proxy_port, suffix: ' <small>*</small>' },
{ title: '上级代理用户名', name: 'gaeproxy_proxy_username', type: 'text', maxlen: 32, size: 15, value: nvram.gaeproxy_proxy_username },
{ title: '上级代理密码', name: 'gaeproxy_proxy_password', type: 'password', maxlen: 32, size: 15, value: nvram.gaeproxy_proxy_password }
]);  
</script>
</div>
<div class='section-title'>用户代理设置</div>
<div class='section' id='config-section'>
<script type='text/javascript'>
createFieldTable('', [
{ title: '启用用户代理重写', name: 'f_gaeproxy_useragent_enable', type: 'checkbox', value: nvram.gaeproxy_useragent_enable == 1 },
{ title: '匹配正则表达式', name: 'gaeproxy_useragent_match', type: 'text', maxlen: 128, size: 32, value: nvram.gaeproxy_useragent_match },
{ title: 'URL匹配规则', name: 'gaeproxy_useragent_rules', type: 'text', maxlen: 128, size: 32, value: nvram.gaeproxy_useragent_rules },
{ title: '改写后的用户代理', name: 'gaeproxy_useragent_string', type: 'text', maxlen: 256, size: 100, value: nvram.gaeproxy_useragent_string }
]);  
</script>
</div>
<div class='section-title'>GAE自动上载服务器设置<script>W(autoupload_log_link);</script></div>
<div class='section' id='config-section'>
<script type='text/javascript'>
createFieldTable('', [
{ title: '启用自动上载至GAE服务器', name: 'f_gaeproxy_autoupload_enable', type: 'checkbox', value: nvram.gaeproxy_autoupload_enable == 1 },
{ title: '关联的GMail账号', name: 'gaeproxy_autoupload_gmail', type: 'text', maxlen: 64, size: 64, value: nvram.gaeproxy_autoupload_gmail },
{ title: 'GAE 访问密码', name: 'gaeproxy_autoupload_password', type: 'password', maxlen: 32, size: 15, value: nvram.gaeproxy_autoupload_password }
]);  
</script>
<ul>
<li><b>启用自动上载服务器</b> - 若启用，则根据设置自动上传至每个appid，但是每次启用只上传一次.
<li><b>注意:</b> - 请耐心等待5分钟以完成上载，你可以查看日志以确定上传状态.
</ul>
</div>
<!-- Menu Section End -->
</td></tr>
</form>
<tr><td id='footer' colspan=2>
<form>
<span id='footer-msg'></span>
<input type='button' value='保存设置' id='save-button' onclick='save()'>
<input type='button' value='取消设置' id='cancel-button' onclick='reloadPage();'>
</form>
</td></tr>
</table>
<script type='text/javascript'>verifyFields(null, 1);</script>
</body>
</html>

