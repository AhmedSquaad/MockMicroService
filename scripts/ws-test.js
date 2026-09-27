#!/usr/bin/env node
// Client WebSocket non-interactif pour tests d'intégration CI : contrairement à
// wscat (qui se ferme immédiatement si stdin n'est pas un TTY, comme dans un job
// GitLab CI), ce script écoute réellement les messages pendant la durée impartie.
'use strict';

const WebSocket = require('ws');

const [, , url, timeoutMsArg, expectedSubstring] = process.argv;
const timeoutMs = parseInt(timeoutMsArg || '8000', 10);

if (!url) {
  console.error('Usage: node ws-test.js <url> [timeoutMs] [expectedSubstring]');
  process.exit(2);
}

const ws = new WebSocket(url);
let matched = false;

const timer = setTimeout(() => {
  if (!matched) {
    console.error(`Timeout (${timeoutMs}ms) sans message correspondant reçu sur ${url}`);
    ws.close();
    process.exit(1);
  }
}, timeoutMs);

ws.on('open', () => {
  console.error(`Connecté à ${url}, en attente de message...`);
});

ws.on('message', (data) => {
  const text = data.toString();
  console.log(text);
  if (!expectedSubstring || text.includes(expectedSubstring)) {
    matched = true;
    clearTimeout(timer);
    ws.close();
    process.exit(0);
  }
});

ws.on('error', (err) => {
  console.error(`Erreur WebSocket: ${err.message}`);
  process.exit(1);
});
