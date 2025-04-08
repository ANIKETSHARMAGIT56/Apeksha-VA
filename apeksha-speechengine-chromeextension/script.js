// reference https://developer.mozilla.org/en-US/docs/Web/API/Web_Speech_API/Using_the_Web_Speech_API
var SpeechRecognition = SpeechRecognition || webkitSpeechRecognition
var recognition = new SpeechRecognition();
var trns = "";
recognition.interimResults = true;
recognition.lang = "en-US";
recognition.continuous = true;

// import ReconnectingWebSocket from 'reconnecting-websocket';
const ws = new ReconnectingWebSocket('ws://localhost:18315');
ws.onopen = () => {
  console.log('ws opened on browser')
  ws.send(JSON.stringify({
    "module::connect":{
        "name":"chrome-extension",
        "type":"speech engine"
    }
  }))
}
ws.onclose = function(e) {
    console.log('Socket is closed. Reconnect will be attempted in 1 second.', e.reason);
};

function emit_apeksha_event(event_name,event_data){
    ws.send(JSON.stringify({
        "event":{
            "name":event_name,
            "data":event_data
        }
    }))
}

ws.onmessage = (message) => {
    var event = JSON.parse(message.data).event
    if(event.name== "speech::start"){
        recognition.start();
        console.log("started listening")
    }    
    if(event.name== "speech::force_end"){
        recognition.stop();
        emit_apeksha_event("speech::end","force_ended")
    }

}

var res = "";
recognition.onresult = function(event) {
    res = event.results[0][0].transcript;
    document.querySelector("#textbox").value = res
    if (event.results[0].isFinal) {
        recognition.stop();
        console.log("final transcript", res)
    }
    else {
        console.log("partial transcript", res)
        emit_apeksha_event("speech::transcript::partial", res)
    }
    emit_apeksha_event("speech::transcript::interpretation",event.interpretation)
};

recognition.onaudiostart = function(){
    emit_apeksha_event("speech::started")
}
recognition.onspeechend = function() {
    recognition.stop();
}

recognition.onend = function(){
    emit_apeksha_event("speech::transcript::final", res)
    emit_apeksha_event("speech::end");
    res=""
}

function start_listening() {
    recognition.start();
}
recognition.onerror = (event) => {
    console.log(event.error)
    emit_apeksha_event("speech::error", event.error)
}
document.getElementById("start_listening").addEventListener("click", start_listening)