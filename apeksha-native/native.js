// const { openApp } = require("open");
import open, { openApp, apps } from 'open';
import fs from 'fs';
import jmespath from 'jmespath';
import { fork, spawn } from 'child_process';
import { io } from "socket.io-client";
import { loadConfig } from '../utils/config.mjs';
import { Worker } from 'worker_threads';
var socket = io("http://localhost:5569");
var forward_to_child = 0;
var script_worker;
var speech_status=0;
socket.on("connect",()=>{
    socket.emit("module_connect", "native");
    socket.on("speech::transcript::final",final_transcript_handler)
    socket.on("speech::transcript::partial",partial_transcript_handler)
    socket.on("speech::start",()=>{speech_status=1})
    socket.on("speech::end",(msg)=>(speech_status=0))
})




let appsdata = loadConfig("./native/apps.json");
let actionsdata = loadConfig("./native/actions.json");

function show_intermediate_state(str) {
    socket.emit("native_intermediate_state", str)
}

function loadappsconfig(){
    appsdata = JSON.parse(fs.readFileSync("./native/apps.json", "utf8", (err, jsonString) => {
        if (err) {
            console.log("File read failed:", err);
            return;
        }
    }));
}
function loadactionsconfig(){
    actionsdata = JSON.parse(fs.readFileSync("./native/actions.json", "utf8", (err, jsonString) => {
        if (err) {
            console.log("File read failed:", err);
            return;
        }
    }));
}

function final_transcript_handler(msg){
    if(forward_to_child == 1){
        script_worker.postMessage({message:"speech::transcript::final",data:msg})
    }
    else{
        command_handler(msg)
    }
}
function partial_transcript_handler(msg){
    if(forward_to_child == 1){
        script_worker.postMessage({message:"speech::transcript::partial",data:msg})
}
}


function script_handler(){
    forward_to_child = 1;
    
    script_worker.on("exit",(code)=>{
        console.log("Child exited with code: ", code)
        forward_to_child = 0;
        socket.emit("native::free")
        script_worker.terminate();
        script_worker = null;
    })
}




function command_handler(msg){
    socket.emit("native::busy","handling")
    console.log("Command recieved: ", msg);
    var command = msg.split(" ");
    
    let searchActionsDataResult;
    if((searchActionsDataResult = searchActionsData(command)) != null){
        if(searchActionsDataResult.command != null){
            spawn(searchActionsDataResult.command,searchActionsDataResult.arguments);
            socket.emit("native::free")
            
        }
        else if(searchActionsDataResult.url != null){
            open(searchActionsDataResult.url);
            socket.emit("native::free")
            
        }
        else if(searchActionsDataResult.script != null){
            if(searchActionsDataResult.blocking == true){
                script_worker = new Worker(searchActionsDataResult.script); 
                script_handler()
                
            }
            
        }
    }
    else if (command[0] == "open") {
        var openant = command.slice(1).join(" ")
        console.log("Opening " + openant);

        opener(openant.toLowerCase());
        socket.emit("native::free")

    }
    else if (command.join(" ") == "reload config") {
        appsdata = loadappsconfig();
        socket.emit("native::free")

    }
    else{
        socket.emit("native::free")
    }
}


function searchActionsData(searchString){
    const appArray = Object.values(actionsdata);
    const query = `[?contains(keywords, '${searchString.join(" ")}')] | [0]`;
    return jmespath.search(appArray, query);
}

function opener(searchString) {
    const appArray = Object.values(appsdata);

    const query = `[?contains(keywords, '${searchString}')] | [0]`;
    
    const result = jmespath.search(appArray, query);

    if (result==null) {
        console.log("No app found");
      }
    else if (result.app != null) {
        openApp(result.app);
    }
    else if (result.url != null) {
        open(result.url);
    }
    else if (result.command != null) {
        spawn(result.command);
    }
    else {
        console.log("No app found");
    }
}