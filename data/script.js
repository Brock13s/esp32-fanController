// JavaScript Document

var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
let bscolor = "#cc0000";
let bcolor = "#d3d3d3";
let buttonstate = 0;
let stateData;

window.addEventListener('load', onload);
function initWebSocket(){
	console.log("trying to open a websocket connection...");
	websocket = new WebSocket(gateway);
	websocket.onopen = onOpen;
	websocket.onclose = onClose;
	websocket.onmessage = onMessage;
	websocket.onerror = onError;
}

function onload(event){
	initWebSocket();
	initButtons();
}

function onMessage(event){ // IF the user presses on one of the buttons and if it was sucessful the server replies back to the client.
	let text = event.data;
	let temperature = text.slice(3,5);
	let lstatus = text.charAt(2);


	if(text.startsWith("TEM")){
		document.getElementById("temperaturereading").innerHTML = temperature;
	}
	else if(text.startsWith("LS")){
		if(lstatus == "1"){
			
			document.getElementById("lightStateReading").innerHTML = "ON";
			document.getElementById("bulbPic").src = 'bulb-on';
		} else{
			document.getElementById("lightStateReading").innerHTML = "OFF";
			document.getElementById("bulbPic").src = 'bulb-off';
		}
	}
	 else{
		document.getElementById("state").innerHTML = text;
	}
}


function onError(event){
	console.log(event);
}

function onOpen(event){
	console.log('Connection opened');
}

function initButtons(){
	document.getElementById('btn_high').addEventListener('click', ()=>{
		websocket.send("FANCODE_HIGH");
	});
	document.getElementById('btn_med').addEventListener('click', ()=>{
		websocket.send("FANCODE_MED");
	});
	document.getElementById('btn_low').addEventListener('click', ()=>{
		websocket.send("FANCODE_LOW");
	});
	document.getElementById('btn_off').addEventListener('click', ()=>{
			websocket.send("FANCODE_OFF");
	});
	document.getElementById('btn_changecolor').addEventListener('click', ()=>{
		websocket.send("FANCODE_CHANGECOLOR");
	});
	document.getElementById('btn_lightonoff').addEventListener('click', ()=>{
		websocket.send("FANCODE_LIGHTONOFF");
	});
	
}

function onClose(event){
	console.log('Connection closed');
}