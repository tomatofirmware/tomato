<!DOCTYPE HTML PUBLIC '-//W3C//DTD HTML 4.0//EN'>
<!--
Tomato GUI
Copyright (C) 2006-2010 Jonathan Zarate
http://www.polarcloud.com/tomato/
For use with Tomato Firmware only.
No part of this file may be used without permission.
-->
<html>
<head>
<meta http-equiv='content-type' content='text/html;charset=utf-8'>
<meta name='robots' content='noindex,nofollow'>
<title>[<% ident(); %>] Status: Wireless Noise Floor</title>
<link rel='stylesheet' type='text/css' href='tomato.css'>
<% css(); %>
<script type='text/javascript' src='tomato.js'></script>
<!-- JavaScript Libraries: For Chart, where possible use hosted libraries to save router space ... like Google Developers -->
<!-- HighCharts / HighStock requires FrameWork (jQuery, for example) -->
<!-- NOTE: have to load over https, or https pages fail (but http pages OK with loading JavaScript over https ... so https only!) -->
<script type='text/javascript' src='https://ajax.googleapis.com/ajax/libs/jquery/2.1.4/jquery.min.js'></script>
<script type='text/javascript' src='https://code.highcharts.com/stock/highstock.js'></script>
<script type='text/javascript' src='https://code.highcharts.com/modules/data.js'></script>
<script type='text/javascript' src='https://code.highcharts.com/modules/exporting.js'></script>
<script type='text/javascript' src='https://code.highcharts.com/modules/no-data-to-display.js'></script>
<style type='text/css'>
#txt {
width: 550px;
white-space: nowrap;
}
#bwm-controls {
text-align: right;
margin-right: 5px;
margin-top: 5px;
float: right;
visibility: hidden;
}
</style>
<script type='text/javascript'>
// This is key - execute nvram command to get needed nvram parameters (additional needed ones inside quotes).
// Basic (nvram) info needed for run-time refresh to work (POST)!
//<% nvram(''); %>
function wlChartNew() {
	// Get File ... note that csv not handled quite right (mime type), so just make a .bin file -> works!
	$.get('ext/wlnoise.bin', function(csv) {
		// Before setting up chart, get local timezone offset (to include below) -> and set Global option for HighCharts
		var myDate = new Date()
		var myTZOffset = myDate.getTimezoneOffset();
		Highcharts.setOptions({
			global: {
				timezoneOffset: myTZOffset
			}
		});

		// Before setting HighCharts Options ... if csv is empty (file missing), insert headers - so "No Data to Display" works!
		if (csv==""){
			csv = "DateTime,eth1,eth2"
		}
		// Set up HighCharts (OK, actually HighStock) Options -> process full history (csv) file on page load
		var options = {
			chart: {
				renderTo: 'chart',
				zoomType: 'x',
				type: 'line'
			},
			title: {
				text: 'Wireless Noise Floor'
			},
			xAxis: {
				type: 'datetime',
				title: {
					text: 'Date / Time'
				}		
			},
			yAxis: {
				title: {
					text: 'Noise Floor (dBm)'
				}
			},
			legend: {
				enabled: true,
				layout: 'horizontal',
				align: 'right',
				verticalAlign: 'top',
				floating: true,
				y: 35,
				borderWidth: 1
			},
			lang : {
				noData: "No Data to Display (wlnoise running?)"
			}, 
			series: [{
				lineWidth: 0.5,
				//color: '#FF0000',
				tooltip: {
					valueDecimals: 0
				}
			},{
				lineWidth: 0.5,
				//color: '#FF0000',
				tooltip: {
					valueDecimals: 0
				}
			}],
			data: {
				csv: csv
			},
			rangeSelector : {
				buttons : [{
					type : 'minute',
					count : 30,
					text : '30m'
				}, {
					type : 'hour',
					count : 1,
					text : '1h'
				}, {
					type : 'hour',
					count : 6,
					text : '6h'
				}, {
					type : 'hour',
					count : 12,
					text : '12h'
				}, {
					type : 'day',
					count : 1,
					text : '1d'
				}, {
					type : 'all',
					count : 1,
					text : 'All'
				}],
				selected : 4,
				inputEnabled : false
			}		
		};
		var myHighChart = new Highcharts.StockChart(options);
	});
}

var dateLast = -1;
var ref = new TomatoRefresh('update.cgi', '', 0, 'status_wlnoise_refresh');
ref.refresh = function(text)
{
	// On Page Refresh, just get the last entry, to speed processing / minimize CPU load (this file is just the last output)
	// For this to work, and not miss Wireless Noise samples, make sure the page refresh is less than the wlnoise sample rate
	$.get('ext/wllast.bin', function(csv) {
		var items = csv.split(',');
		var rowdata = [];
		$.each(items, function(itemNo, item) {
			rowdata.push(parseFloat(item));
		});
		// Only update / add to chart if new data -> avoid duplicate samples
		if (rowdata[0] != dateLast) {
			dateLast = rowdata[0];
			var myHighChart = $('#chart').highcharts();
			myHighChart.series[0].addPoint([rowdata[0],rowdata[1]]);
			myHighChart.series[1].addPoint([rowdata[0],rowdata[2]]);
		}
	})
}
function init()
{
wlChartNew();
ref.initPage(3000, 3);
}
</script>
</head>
<body onload='init()'>
<form>
<table id='container' cellspacing=0>
<tr><td colspan=2 id='header'>
<div class='title'>Tomato</div>
<div class='version'>Version <% version(); %> by arrmo</div>
</td></tr>
<tr id='body'><td id='navi'><script type='text/javascript'>navi()</script></td>
<td id='content'>
<div id='ident'><% ident(); %></div>
<div id='rstats'>
<div id='tab-area'></div>

<div id="chart" style="min-width: 310px; height: 400px; margin: 0 auto"></div>
</div>
<br>
<br>
</td></tr>
<tr><td id='footer' colspan=2><script type='text/javascript'>genStdRefresh(1,0,'ref.toggle()');</script></td></tr>
</table>
</form>
</body>
</html>
