import "./style/main.scss";

import sharedJson from '../shared/shared.gen.json';
const {enums, constants} = sharedJson;
const {Commands} = enums;

import {makeDiv} from "./js/utils.js";

const root = makeDiv({
	parent: document.body,
	class: 'root'
});

const imgContainer = makeDiv({
	parent: root,
	class: 'img-container ut-flex-v'
});

const savedImg = makeDiv('img', {
	parent: imgContainer,
	class: 'saved-img',
	src: "http://cat.lapinozz.com/saved-photo"
});

const streamImg = makeDiv('img', {
	parent: imgContainer,
	class: 'stream-img'
});

/*
const cameraButtons = makeDiv({
	parent: root,
	class: 'camera-buttons'
});
*/

const wifiContainer = makeDiv({
	parent: root,
	class: 'wifi-container ut-flex-v',
});

const wifiTop = makeDiv({
	parent: wifiContainer,
	class: 'ut-text-input-container wifi-status-bar',
});

const wifiStatusLabel = makeDiv({
	parent: wifiTop,
	class: 'ut-label',
	text: 'Status: '
});

const wifiStatus = makeDiv({
	parent: wifiTop,
	class: 'ut-label',
	text: 'Connected'
});

const wifiListToggle = makeDiv('input', {
	parent: wifiTop,
	class: 'connect',
	type: 'button',
	value: 'Wifi List',
	onclick: () => toggleWifiList()
});

const wifiList = makeDiv({
	parent: wifiContainer,
	class: 'wifi-list ut-flex-v ut-flex-expand',
	contentEditable: 'false',
});

const wifiSSIDContainer = makeDiv({
	parent: wifiContainer,
	class: 'wifi-connect-container ut-text-input-container'
});

const wifiSSIDLabel = makeDiv({
	parent: wifiSSIDContainer,
	class: 'ut-label',
	text: "SSID"
});

const wifiSSIDInput = makeDiv('input', {
	parent: wifiSSIDContainer,
	//class: 'secret-input',
	//type: 'password'
});

const wifiPasswordContainer = makeDiv({
	parent: wifiContainer,
	class: 'wifi-connect-container ut-text-input-container'
});

const wifiPasswordLabel = makeDiv({
	parent: wifiPasswordContainer,
	class: 'ut-label',
	text: "Password"
});

const wifiPasswordInput = makeDiv('input', {
	parent: wifiPasswordContainer,
	//class: 'secret-input',
	type: 'password'
});

const wifiConnectButton = makeDiv('input', {
	parent: wifiPasswordContainer,
	class: 'connect',
	type: 'button',
	value: "Connect",
	onclick: () => wifiConnect()
});

const dispensingEventsContainer = makeDiv({
	parent: root,
	class: 'dispensing-events-container ut-flex-h'
});

const secretContainer = makeDiv({
	parent: root,
	class: 'secret-container ut-text-input-container'
});

const secretLabel = makeDiv({
	parent: secretContainer,
	class: 'secret-label ut-label',
	text: "Code"
});

const secretInput = makeDiv('input', {
	parent: secretContainer,
	class: 'secret-input',
	type: 'password'
});

const connectButton = makeDiv('input', {
	parent: secretContainer,
	class: 'connect',
	type: 'button',
	value: "Connect"
});

const logDiv = makeDiv({
	parent: root,
	class: 'log ut-readonly ut-flex-expand',
	readOnly: true,
	contentEditable: false
});

const commandContainer = makeDiv({
	parent: root,
	class: 'command-container ut-text-input-container'
});

const commandLabel = makeDiv({
	parent: commandContainer,
	class: 'command-label ut-label',
	text: "Command"
});

const commandInput = makeDiv('input', {
	parent: commandContainer,
	class: 'command-input ut-flex-expand'
});

const commandSend = makeDiv('input', {
	parent: commandContainer,
	class: 'command-send',
	type: 'button',
	value: "Send"
});

secretInput.value = localStorage.getItem('secret') || '';
secretInput.onchange = () => 
{
	localStorage.setItem('secret', secretInput.value);
};

secretInput.onkeydown = (e) =>
{
	if(e.keyCode == 13)
	{
		connect();
	}
};

commandInput.onkeydown = (e) =>
{
	if(e.keyCode == 13)
	{
		onCommand();
	}
};

commandSend.onclick = () => onCommand();

function log(str, color)
{
	const msg = makeDiv({
		parent: logDiv,
		class: 'log-msg',
		text: str,
		style: 'color: ' + color 
	});

	logDiv.scrollTop = logDiv.scrollHeight;
}

function logLine(str, color)
{
	log(str + '\n', color);
}

function logIn(str, color)
{
	log('> ' + str + '\n', color);
}

function logOut(str, color)
{
	log('< ' + str + '\n', color);
}

console.log({sharedJson})

let ws;

let connected = false;
let connecting = false;

function updateConnected()
{
	connectButton.classList.toggle('disabled', connected || connecting);
	commandContainer.classList.toggle('disabled', !connected);
}
updateConnected();

const keepAlive = () =>
{
	if(ws)
	{
		ws.send("ping");
	}
};
setInterval(keepAlive, 2000);

const connect = () => 
{
	if(ws)
	{
		ws.onclose();
	}

	//const addr = "ws://home.lapinozz.com:4560/ws/"
	let addr = "ws://" + window.location.host + window.location.pathname + "ws/";
	
	if(window.webpackChunkwebpack_template)
	{
		addr = "ws://home.lapinozz.com:4560/ws/";
	}

	ws = new WebSocket(addr + secretInput.value);

	logLine('[WS] Conneting to: ' + addr + 'XXXXXXXXXXXX');

	connected = false;
	connecting = true;
	updateConnected();

	ws.onopen = function(e)
	{
		logLine('[WS] Connected', 'green');

		connected = true;
		connecting = false;
		updateConnected();
	};

	ws.onmessage = function(e)
	{
		const {data} = e;
		if(data instanceof Blob)
		{
			URL.revokeObjectURL(streamImg.src);
			streamImg.src = URL.createObjectURL(e.data);
		} 
		else
		{
			logIn(e.data);
			const args = e.data.split(',').map(a => parseInt(a));
			const cmd = args.splice(0,1)[0];

			if(cmd == Commands.ESP_SetDispensingSetting)
			{
				dispensingEvents = [];
				for(let x = 0; x < constants.DispensingEventMax; x++)
				{
					dispensingEvents[x] = [];
					for(let y = 0; y < enums.DispensingEventSettings._COUNT; y++)
					{
						dispensingEvents[x][y] = args[x * enums.DispensingEventSettings._COUNT + y];
					}
				}
				updateDispensingEvents();
			}
			else if(cmd == Commands.ESP_WifiList)
			{
				wifis = e.data.split(',').slice(1).join('').split('+').map(w => w.split('$'));
				updateWifiList();
			}
		}
	};

	ws.onclose = function(e = {})
	{
		logLine('[WS] Connection Closed' + (e.reason ? ': ' + e.reason : ''), e.wasClean ? '' : 'red');

		ws.onopen = undefined;
		ws.onmessage = undefined;
		ws.onclose = undefined;
		ws.close();
		ws = null;

		connected = false;
		connecting = false;

		updateConnected();
	};
}

let dispensingEvents = [];
const updateDispensingEvents = () =>
{
	dispensingEventsContainer.innerHTML = '';

	const table = makeDiv('table', {
		parent: dispensingEventsContainer,
		class: 'dispensing-table'
	});

	const headers = makeDiv('tr', {
		parent: table
	});

	makeDiv('td', {
		parent: headers,
	});

	const dispensingSettings = Object.keys(enums.DispensingEventSettings).filter(e => e[0] != '_');

	for(const header of dispensingSettings)
	{
		if(header[0] == '_')
		{
			continue;
		}

		const headerEl = makeDiv('td', {
			parent: headers,
			class: 'dispensing-table-header-' + header,
			innerText: header
		});
	}

	let localDispensingEvents = [];

	const onChange = () =>
	{
		let different = false;

		for(let eventIndex = 0; eventIndex < dispensingEvents.length; eventIndex++)
		{
			for(let settingId = 0; settingId < dispensingSettings.length; settingId++)
			{
				different = different || (dispensingEvents[eventIndex][settingId] != localDispensingEvents[eventIndex][settingId]);
			}
		}

		setButton.classList.toggle('disabled', !different);
	};

	const set = () =>
	{
		for(let eventIndex = 0; eventIndex < dispensingEvents.length; eventIndex++)
		{
			for(let settingId = 0; settingId < dispensingSettings.length; settingId++)
			{
				const setting = dispensingEvents[eventIndex][settingId];
				const local = localDispensingEvents[eventIndex][settingId];
				if(setting != local)
				{
					dispensingEvents[eventIndex][settingId] = local;
					onCommand([Commands.ESP_SetDispensingSetting,eventIndex,settingId,local].join(','));					
				}
			}
		}

		onCommand(Commands.ESP_SaveDispensingSettings);

		onChange()
	};

	for(let eventIndex = 0; eventIndex < dispensingEvents.length; eventIndex++)
	{
		const event = dispensingEvents[eventIndex];
		localDispensingEvents[eventIndex] = [];

		const row = makeDiv('tr', {
			parent: table,
			class: 'dispensing-event-container'
		});

		const settingContainer = makeDiv('td', {
			parent: row,
			class: 'dispensing-setting-container',
			//innerText: `Dispensing ${eventIndex}`
		});

		for(let settingId = 0; settingId < dispensingSettings.length; settingId++)
		{
			const settingValue = event[settingId];
			localDispensingEvents[eventIndex][settingId] = settingValue;

			const settingContainer = makeDiv('td', {
				parent: row,
				class: 'dispensing-setting-container dispensing-table-' + dispensingSettings[settingId],
			});

			const settingInput = makeDiv('input', {
				parent: settingContainer,
				class: 'dispensing-setting-input'
			});

			if(settingId == enums.DispensingEventSettings.Time)
			{
				settingInput.type = 'time';
				settingInput.value = new Date(settingValue * 1000).toISOString().substring(11, 16);

				settingInput.onchange = () => {
					const value = settingInput.value.split(':').reverse().reduce((prev, curr, i) => prev + curr*Math.pow(60, i + 1), 0);
					localDispensingEvents[eventIndex][settingId] = value;
					onChange();
				}
			}
			else if(settingId == enums.DispensingEventSettings.Amount)
			{
				settingInput.type = 'number';
				settingInput.min = 0;
				settingInput.max = 5;
				settingInput.value = settingValue;

				settingInput.onchange = () => {
					const value = Math.min(100, Math.max(0, parseInt(settingInput.value)));
					localDispensingEvents[eventIndex][settingId] = value;
					onChange();
				}
			}
			else if(settingId == enums.DispensingEventSettings.Positions)
			{
				settingInput.remove();

				let checkboxs = [];

				const onCheckboxChange = () =>
				{
					let value = 0;
					
					for(const checkbox of checkboxs)
					{
						value = value << 1;

						if(checkbox.checked)
						{
							value += 1;
						}
					}

					localDispensingEvents[eventIndex][settingId] = value;

					onChange();
				};

				for(let x = 0; x < constants.PositionCount; x++)
				{
					let checkbox = makeDiv('input', {
						parent: settingContainer,
						class: 'dispensing-setting-input',
						type: 'checkbox',
						checked: (settingValue >> x) & 1
					});

					checkbox.onchange = onCheckboxChange;

					checkboxs.push(checkbox);
				}
			}
		}
	}

	const dispensingButtons = makeDiv({
		parent: dispensingEventsContainer,
		class: 'ut-flex-v',
		style: 'align-self:end',
	});

	const setButton = makeDiv('input', {
		type: 'button',
		parent: dispensingButtons,
		class: 'set-dispensing-events-button',
		value: 'Save',
		onclick: set
	});

	const dispenseButton = makeDiv('input', {
		type: 'button',
		parent: dispensingButtons,
		class: 'set-dispensing-events-button',
		value: 'Dispense',
		onclick: () => 	onCommand([Commands.ARD_Dispense, 1, 1].join(','))
	});

	onChange();
};

let wifis = [];
const updateWifiList = () =>
{
	wifiList.innerHTML = '';

	for(const wifi of wifis)
	{
		const [locked, ssid, rssi] = wifi;
		let div = makeDiv({
			parent: wifiList,
			class: 'wifi-element',
			text: ssid
		});

		div.onclick = () =>
		{
			wifiSSIDInput.value = ssid;
		};
	}
};

const toggleWifiList = () =>
{
	wifiList.innerHTML = '';

	makeDiv({
		parent: wifiList,
		class: 'wifi-spinner spinner',
	});
	
	root.classList.toggle('wifi-open');
	if(root.classList.contains('wifi-open'))
	{
		onCommand([Commands.ESP_WifiList].join(','));
	}
};

const wifiConnect = () =>
{
	const SSID = wifiSSIDInput.value;
	const password = wifiPasswordInput.value;

	onCommand(Commands.ESP_WifiConnect + ',' + SSID + '$' + password);

	toggleWifiList();
};

connectButton.onclick = connect;
connect();

const onCommand = (cmd = undefined) => 
{
	if(cmd === undefined)
	{
		cmd = commandInput.value;
		commandInput.value = '';
	}

	logOut(cmd);
	ws.send(cmd);
};

setInterval(() =>
{
	onCommand('7');
}, 1000);
