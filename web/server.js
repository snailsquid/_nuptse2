const express = require('express');
const http = require('http');
const { WebSocketServer } = require('ws');
const path = require('path');
const { insertSession, getLeaderboard } = require('./db');

const app = express();
const server = http.createServer(app);
const wss = new WebSocketServer({ server });

const PORT = process.env.PORT || 8081;

app.use(express.json());
app.use(express.static(path.join(__dirname, 'public')));

function broadcast(data) {
  const msg = JSON.stringify(data);
  for (const ws of wss.clients) {
    if (ws.readyState === 1) ws.send(msg);
  }
}

app.post('/api/score', (req, res) => {
  const { device_id, bpm, delta, spo2, cal_avg } = req.body;
  if (!device_id || bpm == null || delta == null) {
    return res.status(400).json({ error: 'missing fields' });
  }
  const result = insertSession({ device_id, bpm, delta, spo2, cal_avg });
  const leaderboard = getLeaderboard(50);
  broadcast({ type: 'leaderboard', data: leaderboard });
  res.json({ ok: true, id: result.lastInsertRowid });
});

app.get('/api/leaderboard', (_req, res) => {
  const data = getLeaderboard(50);
  res.json(data);
});

server.listen(PORT, () => {
  console.log(`Leaderboard running on http://0.0.0.0:${PORT}`);
});
