import clipboardy from 'clipboardy';
import { io } from 'socket.io-client';
keyboard.config.autoDelayMs = 0;

var socket = io("ws://localhost:5569");

socket.on("connect", () => {
    socket.emit("module_connect", "typing");
    socket.emit("speech::start");
});


socket.on("speech::transcript::final", async (msg) => {
    clipboardy.writeSync(msg);
    process.exit();
});