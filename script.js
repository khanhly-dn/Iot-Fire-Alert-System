/* ============================================================
   IoT Sensor Monitor - script.js
   ============================================================ */

let ESP32_IP = '';
let prevLevel = '';
let pollingTimer = null;

function toast(msg, col) {
  const t = document.getElementById('toast');
  t.textContent = msg;
  t.style.borderColor = col || '#00b4d8';
  t.style.color = col || '#00b4d8';
  t.classList.add('show');
  setTimeout(() => t.classList.remove('show'), 2500);
}

function fmtUp(ms) {
  const s = Math.floor(ms / 1000);
  const m = Math.floor(s / 60);
  const h = Math.floor(m / 60);
  if (h > 0) return `${h}h ${m % 60}m`;
  return `${m}m ${s % 60}s`;
}

function applyLevel(lv) {
  const bn = document.getElementById('banner');
  const bi = document.getElementById('bi');
  const bt = document.getElementById('bt');
  const bs = document.getElementById('bs');
  if (lv === 'DANGER') {
    bn.className = 'danger';
    bi.innerHTML = '&#128680;';
    bt.textContent = 'NGUY HIEM!';
    bs.textContent = 'Nhiet do hoac do am vuot nguong!';
  } else if (lv === 'WARNING') {
    bn.className = 'warning';
    bi.innerHTML = '&#9888;';
    bt.textContent = 'CANH BAO';
    bs.textContent = 'Nhiet do / do am cao!';
  } else {
    bn.className = 'safe';
    bi.innerHTML = '&#9989;';
    bt.textContent = 'AN TOAN';
    bs.textContent = 'Moi thu binh thuong';
  }
}

function setIndicator(dotId, stateId, on) {
  const dot = document.getElementById(dotId);
  const st = document.getElementById(stateId);
  if (on) {
    dot.className = 'ind-dot on-danger';
    st.textContent = 'DANG HOAT DONG';
    st.style.color = '#ff1744';
  } else {
    dot.className = 'ind-dot off';
    st.textContent = 'TAT';
    st.style.color = '';
  }
}

function updateUI(d) {
  if (d.level !== prevLevel) {
    if (d.level === 'DANGER') toast('CANH BAO NGUY HIEM!', '#ff1744');
    else if (d.level === 'WARNING') toast('Canh bao nhiet do / do am!', '#ffd600');
    else if (prevLevel) toast('Da tro ve an toan', '#00e676');
    prevLevel = d.level;
  }

  applyLevel(d.level);

  document.getElementById('temp').textContent = d.temp.toFixed(1) + '°C';
  document.getElementById('tf').style.width = Math.min(d.temp / 50 * 100, 100) + '%';
  document.getElementById('humi').textContent = d.humi.toFixed(1) + '%';
  document.getElementById('hf').style.width = Math.min(d.humi, 100) + '%';

  document.getElementById('up').textContent = fmtUp(d.uptime || 0);
  document.getElementById('mx').textContent = d.maxTemp.toFixed(1) + '°C';
  document.getElementById('mn').textContent = d.minTemp.toFixed(1) + '°C';
  document.getElementById('rs').textContent = (d.rssi || '--') + ' dBm';

  setIndicator('led-dot', 'led-state', d.alert);
  setIndicator('buz-dot', 'buz-state', d.alert && !d.silenced);

  const dot = document.getElementById('dot');
  dot.style.background = '#00e676';
  dot.style.boxShadow = '0 0 8px #00e676';

  if (d.logs && d.logs.length > 0) {
    let html = '';
    d.logs.forEach(e => {
      const s = Math.floor(e.ts / 1000);
      const m = Math.floor(s / 60);
      const ts = `${m}m${(s % 60 < 10 ? '0' : '')}${s % 60}s`;
      html += `<div class="lr ${e.level}">
        <span class="lts">${ts}</span>
        <span class="lt">${e.temp.toFixed(1)}°C</span>
        <span class="lh">${e.humi.toFixed(1)}%</span>
        <span class="llv">${e.level}</span>
      </div>`;
    });
    document.getElementById('log-body').innerHTML = html;
  }

  console.log('DATA:', d);
}

function connErr(err) {
  console.log('LOI:', err);
  const dot = document.getElementById('dot');
  dot.style.background = '#ff1744';
  dot.style.boxShadow = '0 0 8px #ff1744';
  document.getElementById('conn-status').textContent = 'Mat ket noi!';
  document.getElementById('conn-status').style.color = '#ff1744';
  toast('Khong ket noi duoc ESP32!', '#ff1744');
}

function doConnect() {
  const raw = document.getElementById('ip-input').value.trim();
  if (!raw) { toast('Nhap IP di!', '#ffd600'); return; }

  ESP32_IP = raw.startsWith('http') ? raw.replace(/\/$/, '') : 'http://' + raw.replace(/\/$/, '');

  document.getElementById('conn-status').textContent = 'Dang ket noi...';
  document.getElementById('conn-status').style.color = '#ffd600';

  if (pollingTimer) clearInterval(pollingTimer);

  fetch(`${ESP32_IP}/status`)
    .then(r => r.json())
    .then(d => {
      document.getElementById('conn-status').textContent = 'Da ket noi';
      document.getElementById('conn-status').style.color = '#00e676';
      document.getElementById('ipinfo').textContent = 'Connected: ' + ESP32_IP;
      updateUI(d);
      startPolling();
      toast('OK ket noi!', '#00e676');
    })
    .catch(connErr);
}

function startPolling() {
  pollingTimer = setInterval(() => {
    fetch(`${ESP32_IP}/status`)
      .then(r => r.json())
      .then(updateUI)
      .catch(connErr);
  }, 2000);
}

function doTest() {
  if (!ESP32_IP) { toast('Chua nhap IP!', '#ffd600'); return; }
  fetch(`${ESP32_IP}/test`).then(() => toast('Test da kich hoat!', '#ff6b35')).catch(connErr);
}

function doSilence() {
  if (!ESP32_IP) { toast('Chua nhap IP!', '#ffd600'); return; }
  fetch(`${ESP32_IP}/silence`).then(() => toast('Buzzer da tat!', '#f59e0b')).catch(connErr);
}

function doReset() {
  if (!ESP32_IP) { toast('Chua nhap IP!', '#ffd600'); return; }
  fetch(`${ESP32_IP}/reset`).then(() => {
    toast('He thong da reset!', '#0ea5e9');
    prevLevel = '';
  }).catch(connErr);
}

document.addEventListener('DOMContentLoaded', () => {
  document.getElementById('ip-input').addEventListener('keydown', e => {
    if (e.key === 'Enter') doConnect();
  });
});
