// JavaScript Document

var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
window.addEventListener('load', onload);
function initWebSocket(){
	console.log("trying to open a websocket connection...");
	websocket = new WebSocket(gateway);
	websocket.onopen = onOpen;
	websocket.onclose = onClose;
}

function onload(event){
	//initWebSocket();
	initButtons();
}

function onOpen(event){
	console.log('Connection opened');
}

function initButtons(){
	document.getElementById('btn_high').addEventListener('click', ()=>{
		alert("Fan speed set to high");
	});
	document.getElementById('btn_med').addEventListener('click', ()=>{
		alert("Fan speed set to medium");
	});
	document.getElementById('btn_low').addEventListener('click', ()=>{
		alert("Fan speed set to medium");
	});
	document.getElementById('btn_off').addEventListener('click', ()=>{
		alert("Fan speed set to off");
	});
	document.getElementById('btn_changecolor').addEventListener('click', ()=>{
		alert("Ceiling fan light color has been changed");
	});
	document.getElementById('btn_lightonoff').addEventListener('click', ()=>{
		alert("Turned light on/off");
	});
	
}

function sendBtnCommand(){
	alert("test");
}

function onClose(event){
	console.log('Connection closed');
	setTimeout(initWebSocket, 2000);
}