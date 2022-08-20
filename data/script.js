// JavaScript Document

var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
let bscolor = "#cc0000";
let bcolor = "#d3d3d3";
let buttonstate = 0;

window.addEventListener('load', onload);
function initWebSocket(){
	console.log("trying to open a websocket connection...");
	websocket = new WebSocket(gateway);
	websocket.onopen = onOpen;
	websocket.onclose = onClose;
}

function onload(event){
	initWebSocket();
	document.getElementById("btn_off").style.background = bscolor;
	initButtons();
	
}

function onOpen(event){
	console.log('Connection opened');
}

function initButtons(){
	document.getElementById('btn_high').addEventListener('click', ()=>{
		alert("Fan speed set to high");
		document.getElementById("btn_lightonoff").style.background = "";
		document.getElementById("btn_changecolor").style.background = "";
		document.getElementById("btn_off").style.background = "";
		document.getElementById("btn_low").style.background = "";
		document.getElementById("btn_med").style.background = "";
		document.getElementById("btn_high").style.background = bscolor;
		websocket.send("FANCODE_HIGH");
	});
	document.getElementById('btn_med').addEventListener('click', ()=>{
		alert("Fan speed set to medium");
		document.getElementById("btn_lightonoff").style.background = "";
		document.getElementById("btn_changecolor").style.background = "";
		document.getElementById("btn_off").style.background = "";
		document.getElementById("btn_low").style.background = "";
		document.getElementById("btn_med").style.background = bscolor;
		document.getElementById("btn_high").style.background = "";
		websocket.send("FANCODE_MED");
	});
	document.getElementById('btn_low').addEventListener('click', ()=>{
		alert("Fan speed set to medium");
		document.getElementById("btn_lightonoff").style.background = "";
		document.getElementById("btn_changecolor").style.background = "";
		document.getElementById("btn_off").style.background = "";
		document.getElementById("btn_low").style.background = bscolor;
		document.getElementById("btn_med").style.background = "";
		document.getElementById("btn_high").style.background = "";
		websocket.send("FANCODE_LOW");
	});
	document.getElementById('btn_off').addEventListener('click', ()=>{
		alert("Fan speed set to off");
		document.getElementById("btn_lightonoff").style.background = "";
			document.getElementById("btn_changecolor").style.background = "";
			document.getElementById("btn_off").style.background = bscolor;
			document.getElementById("btn_low").style.background = "";
			document.getElementById("btn_med").style.background = "";
			document.getElementById("btn_high").style.background = "";
			websocket.send("FANCODE_OFF");
	});
	document.getElementById('btn_changecolor').addEventListener('click', ()=>{
		alert("Ceiling fan light color has been changed");
		websocket.send("FANCODE_CHANGECOLOR");
	});
	document.getElementById('btn_lightonoff').addEventListener('click', ()=>{
		alert("Turned light on/off");
		websocket.send("FANCODE_LIGHTONOFF");
	});
	
}

function onClose(event){
	console.log('Connection closed');
	setTimeout(initWebSocket, 2000);
}
