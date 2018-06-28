// JavaScript code for the BLE Scan example app.

// Application object.
var app = {};

// Device list.
app.devices = {};

// UI methods.
app.ui = {};

// Timer that updates the device list and removes inactive
// devices in case no devices are found by scan.
app.ui.updateTimer = null;

//smoothie chart data vis
//var chart = new SmoothieChart({minValue: 0, maxValue: 20});
var chart = new SmoothieChart({
	//timestampFormatter: SmoothieChart.timeFormatter, 
	millisPerPixel: 30,
	minValue: 0, 
	maxValue: 3,
	grid: { strokeStyle:'rgb(155, 155, 155)', fillStyle:'rgb(0, 0, 0)', lineWidth: 1, millisPerLine: 1000, verticalSections: 6, },
  	labels: { disabled:false, showIntermediateLabels: true, fontSize:12, fillStyle:'#ffffff' }
  });

var line_data = new TimeSeries();


function initializeChart()
{
	console.log("init chart");
	chart.streamTo(document.getElementById("chart-canvas"), 500 /*delay*/);
	chart.addTimeSeries(line_data, { strokeStyle:'rgb(0, 255, 0)', fillStyle:'rgba(0, 255, 0, 0.2)', lineWidth:3 });
}

app.initialize = function()
{

	document.addEventListener(
		'deviceready',
		function() { evothings.scriptsLoaded(app.onDeviceReady) },
		false);
};

app.onDeviceReady = function()
{
	// Here you can update the UI to say that
	// the device (the phone/tablet) is ready
	// to use BLE and other Cordova functions.
	initializeChart();
};

// Start the scan. Call the callback function when a device is found.
// Format:
//   callbackFun(deviceInfo, errorCode)
//   deviceInfo: address, rssi, name
//   errorCode: String
app.startScan = function(callbackFun)
{
	app.stopScan();

	evothings.ble.startScan(
		function(device)
		{
			// Report success. Sometimes an RSSI of +127 is reported.
			// We filter out these values here.
			//if (device.rssi <= 0)
			if (device.address == 'CA:03:75:64:65:E2')
			{
				callbackFun(device, null);
			}
		},
		function(errorCode)
		{
			// Report error.
			callbackFun(null, errorCode);
		},
		{ serviceUUIDs: ['0000a000-0000-1000-8000-00805f9b34fb'] } //filter out everything but smartpill
	);
};

// Stop scanning for devices.
app.stopScan = function()
{
	evothings.ble.stopScan();
};

// Called when Start Scan button is selected.
app.ui.onStartScanButton = function()
{
	app.startScan(app.ui.deviceFound, { serviceUUIDs: ['0000a000-0000-1000-8000-00805f9b34fb'] }); 
	app.ui.displayStatus('Scanning...');
	app.ui.updateTimer = setInterval(app.ui.displayDeviceList, 500);
};

// Called when Stop Scan button is selected.
app.ui.onStopScanButton = function()
{
	app.stopScan();
	app.devices = {};
	app.ui.displayStatus('Scan Paused');
	app.ui.displayDeviceList();
	clearInterval(app.ui.updateTimer);
};

// Called when a device is found.
app.ui.deviceFound = function(device, errorCode)
{
	console.log('Found device:' + JSON.stringify(device.advertisementData));
	if (device)
	{
		// Set timestamp for device (this is used to remove
		// inactive devices).
		device.timeStamp = Date.now();

		// Insert the device into table of found devices.
		app.devices[device.address] = device;
	}
	else if (errorCode)
	{
		app.ui.displayStatus('Scan Error: ' + errorCode);
	}
};

// Display the device list.
app.ui.displayDeviceList = function()
{
	console.log("display");
	// Clear device list.
	$('#found-devices').empty();

	var timeNow = Date.now();

	$.each(app.devices, function(key, device)
	{

			// Map the RSSI value to a width in percent for the indicator.
			var pillData = (3.3 * ( parseInt(device.advertisementData.kCBAdvDataLocalName) ) / 1021).toFixed(2);
			var rssiWidth = 100; // Used when RSSI is zero or greater.
			if (device.rssi < -100) { rssiWidth = 0; }
			else if (device.rssi < 0) { rssiWidth = 100 + device.rssi; }

			// Create tag for device data.
			var element = $(
				'<li>'
				+	'<strong>bio-galvanic cell voltage: ' + pillData + 'v</strong><br />'
				// Do not show address on iOS since it can be confused
				// with an iBeacon UUID.
				+	'address: ' + device.address + '<br />'
				+	'signal strength: ' + device.rssi + '<br />' 
				+ 	'<div style="background:rgb(225,0,0);height:20px;width:'
				+ 		(rssiWidth - 10) + '%;"></div>'
				+ '</li>'
			);

			$('#found-devices').append(element);

			//update chart

			var now = Date.now();
			line_data.append(now, pillData);
			console.log("timestamp: " + now + "  data: " + pillData );  //log because time gets big


	});


};

// Display a status message
app.ui.displayStatus = function(message)
{
	$('#scan-status').html(message);
};

app.initialize();
