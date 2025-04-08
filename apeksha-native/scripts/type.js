import { parentPort } from 'worker_threads';
import clipboardy from 'clipboardy';
import { io } from 'socket.io-client';
import { keyboard, Key } from '@nut-tree-fork/nut-js';
keyboard.config.autoDelayMs = 0;

var socket = io("ws://localhost:5569");

socket.on("connect", () => {
    socket.emit("module_connect", "type");
    socket.emit("speech::start");
});

let previousText = "";
let typing = false;

socket.on("speech::transcript::partial", async (msg) => {
    console.log("partial transcript", msg);
    await typeText(msg);
});

socket.on("speech::transcript::final", async (msg) => {
    console.log("final transcript", msg);
    clipboardy.writeSync(msg);
    await typeText(msg);
    socket.disconnect();
    process.exit();
});

async function typeText(text) {
    if (!typing) {
        typing = true;
        for (let i = 0; i < previousText.length; i++) {
            await keyboard.type(Key.Backspace);
        }
        // Type new text
        await keyboard.type(text);
        previousText = text;
        typing = false;
        return;
    }
    else{
        return;
    }
}