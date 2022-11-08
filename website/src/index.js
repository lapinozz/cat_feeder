import "./style/main.scss";

import sharedH from 'raw-loader!../../shared/shared.h';

import {makeDiv} from "./js/utils.js";

const root = makeDiv({
	parent: document.body,
	class: 'root ut-flex-h'
});

const rootLeft = makeDiv({
	parent: root,
	class: 'root-left ut-flex-expand ut-flex-v ut-flex-align-center'
});

const rootRight = makeDiv({
	parent: root,
	class: 'root-right ut-flex-expand ut-flex-v'
});

const cameraImg = makeDiv('img', {
	parent: rootLeft,
	class: 'camera-img'
});

const cameraButtons = makeDiv({
	parent: rootLeft,
	class: 'camera-buttons'
});

const secretContainer = makeDiv({
	parent: rootRight,
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
	parent: rootRight,
	class: 'log ut-readonly ut-flex-expand',
	readOnly: true,
	contentEditable: false
});

const commandContainer = makeDiv({
	parent: rootRight,
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
	log('< ' + str + '\n', color);
}

function logOut(str, color)
{
	log('> ' + str + '\n', color);
}

console.log({sharedH})

let ws;

let connected = false;
let connecting = false;

function updateConnected()
{
	connectButton.classList.toggle('disabled', connected || connecting);
	commandContainer.classList.toggle('disabled', !connected);
}
updateConnected();

const connect = () => 
{
	if(ws)
	{
		ws.onclose();
	}

	const addr = "ws://192.168.0.120/ws/" + secretInput.value
	ws = new WebSocket(addr);

	logLine('[WS] Conneting to: ' + addr);

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
		logIn(e.data);
		cameraImg.src = URL.createObjectURL(e.data);
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

secretInput.onkeydown = (e) =>
{
    if(e.keyCode == 13)
    {
        connect();
    }
};

connectButton.onclick = connect;
connect();

const onCommand = () => 
{
	const command = commandInput.value;
	commandInput.value = '';

	logOut(command);
	ws.send(command);
};

commandInput.onkeydown = (e) =>
{
    if(e.keyCode == 13)
    {
        onCommand();
    }
};

commandSend.onclick = onCommand;
