const Database = require('better-sqlite3');
const path = require('path');

const db = new Database(path.join(__dirname, 'leaderboard.db'));

db.exec(`
  CREATE TABLE IF NOT EXISTS sessions (
    id           INTEGER PRIMARY KEY AUTOINCREMENT,
    device_id    TEXT NOT NULL,
    bpm          INTEGER NOT NULL,
    delta        REAL NOT NULL,
    spo2         REAL NOT NULL DEFAULT 0,
    cal_avg      REAL NOT NULL DEFAULT 0,
    recorded_at  TEXT DEFAULT (datetime('now'))
  )
`);

function insertSession({ device_id, bpm, delta, spo2, cal_avg }) {
  const stmt = db.prepare(`
    INSERT INTO sessions (device_id, bpm, delta, spo2, cal_avg)
    VALUES (?, ?, ?, ?, ?)
  `);
  return stmt.run(device_id, bpm, delta, spo2 || 0, cal_avg || 0);
}

function getLeaderboard(limit = 50) {
  const stmt = db.prepare(`
    SELECT id, device_id, bpm, delta, spo2, cal_avg, recorded_at
    FROM sessions
    ORDER BY delta ASC, recorded_at DESC
    LIMIT ?
  `);
  return stmt.all(limit);
}

module.exports = { insertSession, getLeaderboard };
