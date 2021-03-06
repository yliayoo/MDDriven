/*angular.module('starter.services', ['ngResource'])

.factory('Session', function($resource) {
	return $resource('http://localhost:5000/sessions/:sessionId');
});*/

angular.module('services', [])

.factory('Devices', function() {
	//might use a resource here that returns a JSON array

	//fake testing data
	var devices = [];
	return {
		all: function() {
			return devices;
		},
		clean: function() {
			devices.splice(0, devices.length);
		},
		remove: function(device) {
			devices.splice(devices.indexOf(device), 1);
		},
		get: function(deviceId) {
			for (var i = 0; i < chats.length; i++) {
				if (devices[i].id === parseInt(deviceId)) { return devices[i]; }
			}
			return null;
		},
		add: function(device) {
			devices.push(device);
		}
	};
})

.factory('BLE', function($q, $interval, Devices) {
	var connected = null;
	var service = null;
	var characteristic = null;
	var dataQueue = [];

	var ret = {
		stringToArrayBuffer: function(str) {
			var ret = new Unit8Array(str.length);
			for (var i = 0; i < str.length; i++) {
				ret[i] = str.charCodeAt(i);
			}
			return ret.buffer;
		},
		isConnected: function() { return connected !== null; },
		scan: function() {
			Devices.clean();
			var deferred = $q.defer();
			if (connected) { ret.disconnect(); }
			ble.startScan([], 
				function(peripheral) { Devices.add(peripheral); },
				function(error) { deferred.reject(error); }
			);
			setTimeout(ble.stopScan, 5000, 
				function() { deferred.resolve(); },
				function() {
					console.log("stopScan failed");
					deferred.reject("Error stopping scan");
				}
			);
			return deferred.promise;
		},
		disconnect: function() {
			ble.disconnect(connected.id, function() {
				alert("Disconnected " + connected.id);
				console.log("Disconnected " + connected.id);
			});
			connected = null;
		},
		connect: function(deviceId) {
			var deferred = $q.defer();
			ble.connect(deviceId,
				function(peripheral) {
					connected = peripheral;
					deferred.resolve(peripheral);
					angular.forEach(connected.characteristics, function(c) {
						var props = c.properties;
						var hasWrite = props.indexOf("Write")>0 || props.indexOf("WriteWithoutResponse")>0;
						if (hasWrite) {
							characteristic = c.characteristic;
							service = c.service;
						}
					});
				},
				function(reason) {
					deferred.reject(reason);
				}
			);
			return deferred.promise;
		},
		sendData: function(data) {
			var deferred = $q.defer();
			ble.WriteWithoutResponse(connectedId, service, characteristic, data,
				function(response) {
					deferred.resolve(response);
				},
				function(response) {
					alert("send ko " + response);
					deferred.reject(response);
			});
			return deferred.promise;
		},
		addQueue: function(d) {
			if (dataQueue.length <= 3) { dataQueue.push(d); }
		}
	};

	$interval(function() {
		if (dataQueue.length>0 && connected!==null) {
			var d = dataQueue.shift();
			ret.sendData(d);
		}
	}, 250);
	return ret;
});










