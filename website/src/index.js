import "./style/main.scss";


import {makeDiv} from "./js/utils.js";

const root = makeDiv({
	parent: document.body,
	class: 'root ut-flex-h'
});

const rootLeft = makeDiv({
	parent: root,
	class: 'root-left ut-flex-child ut-flex-v ut-flex-align-center'
});

const rootRight = makeDiv({
	parent: root,
	class: 'root-right ut-flex-child ut-flex-v ut-flex-align-center'
});

const cameraImg = makeDiv({
	parent: rootLeft,
	class: 'camera-img'
});

const cameraButtons = makeDiv({
	parent: rootLeft,
	class: 'camera-buttons'
});

const secretContainer = makeDiv({
	parent: rootRight,
	class: 'secret-container'
});

const secretLabel = makeDiv({
	parent: secretContainer,
	class: 'secret-label'
});

const secretInput = makeDiv('input', {
	parent: secretContainer,
	class: 'secret-input'
});

console.log('dwadwdwaa')
