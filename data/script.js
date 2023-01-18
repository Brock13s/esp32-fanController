// JavaScript Document

var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
let connectiontext;

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
	console.log("Watcha doing in the console trying to hack me! 0_0");
	initWebSocket();
	initButtons();
}

function onMessage(event){ // IF the user presses on one of the buttons and if it was sucessful the server replies back to the client.
	let text = event.data;
	let temperature = text.slice(3,5);
	 connectiontext = text.slice(3);
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

	else if(text.startsWith("DAY")){
		document.getElementById("connectionsText").innerHTML = connectiontext;
		let daysFoo = (connectiontext > "1") ? "days" : "day";
		console.log(daysFoo);
		document.getElementById("daysState").innerHTML = daysFoo;
	}
	 else{
		if(text=="FANCODE_HIGH"){
			document.getElementById("btn_high").style.backgroundColor = "green";
			document.getElementById("btn_med").style.backgroundColor = "";
			document.getElementById("btn_low").style.backgroundColor = "";
			document.getElementById("btn_off").style.backgroundColor = "";
			document.getElementById("btn_changecolor").style.backgroundColor = "";
			document.getElementById("btn_lightonoff").style.backgroundColor = "";
		} else if(text=="FANCODE_MED"){
			document.getElementById("btn_med").style.backgroundColor = "green";
			document.getElementById("btn_high").style.backgroundColor = "";
			document.getElementById("btn_low").style.backgroundColor = "";
			document.getElementById("btn_off").style.backgroundColor = "";
			document.getElementById("btn_changecolor").style.backgroundColor = "";
			document.getElementById("btn_lightonoff").style.backgroundColor = "";
		} else if(text=="FANCODE_LOW"){
			document.getElementById("btn_low").style.backgroundColor = "green";
			document.getElementById("btn_med").style.backgroundColor = "";
			document.getElementById("btn_high").style.backgroundColor = "";
			document.getElementById("btn_off").style.backgroundColor = "";
			document.getElementById("btn_changecolor").style.backgroundColor = "";
			document.getElementById("btn_lightonoff").style.backgroundColor = "";
		} else if(text=="FANCODE_OFF"){
			document.getElementById("btn_off").style.backgroundColor = "green";
			document.getElementById("btn_med").style.backgroundColor = "";
			document.getElementById("btn_low").style.backgroundColor = "";
			document.getElementById("btn_high").style.backgroundColor = "";
			document.getElementById("btn_changecolor").style.backgroundColor = "";
			document.getElementById("btn_lightonoff").style.backgroundColor = "";
		} else if(text=="FANCODE_CHANGECOLOR"){
			document.getElementById("btn_changecolor").style.backgroundColor = "green";
			document.getElementById("btn_med").style.backgroundColor = "";
			document.getElementById("btn_low").style.backgroundColor = "";
			document.getElementById("btn_off").style.backgroundColor = "";
			document.getElementById("btn_high").style.backgroundColor = "";
			document.getElementById("btn_lightonoff").style.backgroundColor = "";
		} else if(text=="FANCODE_LIGHTONOFF"){
			document.getElementById("btn_lightonoff").style.backgroundColor = "green";
			document.getElementById("btn_med").style.backgroundColor = "";
			document.getElementById("btn_low").style.backgroundColor = "";
			document.getElementById("btn_off").style.backgroundColor = "";
			document.getElementById("btn_changecolor").style.backgroundColor = "";
			document.getElementById("btn_high").style.backgroundColor = "";
		}
	}
}


function onError(event){
	console.log('Socket error: ', event.message, 'closing socket');
	websocket.close();
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
	console.log('Connection closed', event.reason);
	setTimeout(function(){window.location.reload();}, 5000);
}