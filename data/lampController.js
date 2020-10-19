var app = angular.module("myApp", ["ui.router"]);

app.config(function($stateProvider, $urlRouterProvider) {
    $stateProvider.state("home", {
        url: "/",
        templateUrl: "home.htm",
    });

    $stateProvider.state("settings", {
		url: "/settings",
		// controller: function ($scope, $state) {
		// 	$scope.$state = $state;
		// },
		controller: 'settingsController',
		templateUrl: "settings.htm",
		redirectTo: 'settings.vars',
	});

	$stateProvider.state("settings.vars", { url: "/vars", });
	$stateProvider.state("settings.wifi", { url: "/wifi", });
	$stateProvider.state("settings.mqtt", { url: "/mqtt", });
	$stateProvider.state("settings.firmware", { url: "/firmware", });
	
    $urlRouterProvider.otherwise('/');
});





app.controller('settingsController', function($scope, $state, $location, $http) {
	$scope.$state = $state;

	$scope.host = window.location.hostname;

	$scope.wifiScanResults = [];

	$scope.wifiScan = function(){
		$http.get("http://" + $scope.host + "/wifiStatus")
		.then(function(response) {
			$scope.wifiScanResults = response.data.wifiScan;
			$scope.wifiStatus = response.data.wifiStatus;
			$scope.ssid = response.data.ssid;
			$scope.localIP = response.data.localIP;
		});
	}

	$scope.newWiFiSsid = "";
	$scope.newWiFiPass = "";

	$scope.openNewWiFiModal = function(ssid){
		$scope.newWiFiSsid = ssid;
		$scope.newWiFiPass = "";
	}

	$scope.changeWiFiSettings = function(ssid, pass){
		$http.get("http://" + $scope.host + "/changeWiFiSettings?wifi_ssid=" + ssid + "&wifi_pass=" + pass)
		.then(function(response) {
			console.log(response.data);
		});
	}

	$scope.updateFirmware = function(){
		$http.get("http://" + $scope.host + "/updateFirmware")
		.then(function(response) {
			console.log(response.data);
		});
	}

	$scope.wifiScan();
	setInterval($scope.wifiScan, 10000);
});





app.controller('lampController', function($scope, $location, $http) {
	$scope.url = 'ws://'+window.location.hostname+':81/';

	$scope.lampState = {};
	$scope.lampState['lampState']; 
	$scope.lampState['presets'];

	$scope.lampState['brightness'];

	//$scope.lampState['hue'] = 0
	//$scope.lampState['saturation'] = 0


	$scope.selectedPreset = 0;


	$scope.connect = function(){
		console.log("connecting to " + $scope.url)
		$scope.ws = new WebSocket($scope.url); 
		console.log("after connection");
		//$scope.ws.binaryType = 'arraybuffer';
	}
	$scope.connect();
	
	$scope.ws.onmessage = function(evt){
		console.log(evt);
		if (evt.data.length === undefined){ //then it is the array buffer
			
			// let int16View = new Int16Array(evt.data);
			// $scope.lastDataTime = Date.now();
			// if ($scope.runMode == 1) {
			// 	$scope.data = int16View;
			// }
		} else {
			$scope.lampState = Object.assign($scope.lampState, JSON.parse(evt.data));
			//$scope.lampState.hue = $scope.lampState.activePattern.h / 2**16 * 360;
			//$scope.lampState.saturation = $scope.lampState.activePattern.s / 255 * 100;
		}
		$scope.$applyAsync();
	}

	$scope.sendWSMessage = function(){
		var toSend = $scope.lampState;
		//toSend.hue = toSend.hue; /// 360 * 2**16;
		//$scope.ws.send(JSON.stringify({lampState:$scope.lampState.lampState, hue:($scope.lampState.hue*2**16/360), saturation:($scope.lampState.saturation*255/100)})); 
		$scope.ws.send(JSON.stringify({lampState:$scope.lampState.lampState, hue:($scope.lampState.activePattern.h), saturation:($scope.lampState.activePattern.s)})); 
	}
	

	
	$scope.$watch("lampState", function() {
		if($scope.ws.readyState == 1) { //ws is open
			$scope.sendWSMessage();
		} else {
			console.log($scope.ws.readyState);
			//$scope.connect();
		}
		
	}, true);


	$scope.$watch("lampState.brightness", function() {
		if($scope.ws.readyState == 1) { //ws is open
			console.log("brightness");
			$scope.ws.send(JSON.stringify({brightness:$scope.lampState.brightness})); 
		} 		
	}, true);

	// $scope.$watch("lampState.color.hue", function() {
	// 	$scope.lampState.hue = $scope.lampState.color.hue ;
	// 	//$scope.ws.send(JSON.stringify($scope.lampState)); 
	// }, true);
	

	
	$scope.clickPresetBox = function(index){
		$scope.ws.send(JSON.stringify({'selectPreset':index})); 
	}

	$scope.savePreset = function(index){
		$scope.ws.send(JSON.stringify({'savePreset':index})); 
	}
	
	
	
	// var tick = function() {
	// 	if ((Date.now() - $scope.lastDataTime) < 1000) {
	// 		$scope.status = 1;
	// 	} else {
	// 		$scope.status = 0;
	// 	}
	// 	$scope.$applyAsync();
	// }
	// setInterval(tick, 1000);
});



app.filter('prettyPrintJson', function () {
	return function (obj) {
		return JSON.stringify(obj, null, 2)
	};
  });



app.filter('formatSeconds', function () {
  return function (seconds) {
	if (seconds > 60) {
		return Math.ceil(seconds/60) + " minutes";
	} else {
		return "1 minute";
	} 
  };
});


app.filter('uint16ToDeg', function () {
	return function (value) {
		return parseInt(value / 2**16 * 360 );
	};
  });
			
