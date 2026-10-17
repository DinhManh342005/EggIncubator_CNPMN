#ifndef WEB_PAGE_H
#define WEB_PAGE_H

#include <Arduino.h>

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="vi">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
  <title>Smart Incubator Pro - Hệ Thống Ấp Trứng Thông Minh</title>
  <link href="https://fonts.googleapis.com/css2?family=Plus+Jakarta+Sans:wght@400;500;600;700;800&display=swap" rel="stylesheet">
  <style>
    :root {
      --bg: #0b132b;
      --card: rgba(28, 37, 65, 0.8);
      --card-border: rgba(255, 255, 255, 0.08);
      --primary: #38bdf8;
      --accent: #f43f5e;
      --green: #10b981;
      --orange: #f59e0b;
      --purple: #a855f7;
      --text: #f8fafc;
      --subtext: #94a3b8;
    }
    * { box-sizing: border-box; margin: 0; padding: 0; font-family: 'Plus Jakarta Sans', sans-serif; -webkit-tap-highlight-color: transparent; }
    body {
      background: var(--bg); color: var(--text); padding: 14px; min-height: 100vh;
      display: flex; flex-direction: column; align-items: center; transition: box-shadow 0.3s ease;
    }
    body.alarm-flash { animation: screenFlash 0.8s ease-in-out 3 forwards; }
    @keyframes screenFlash {
      0%, 100% { box-shadow: inset 0 0 0 0 rgba(244, 63, 94, 0); }
      50% { box-shadow: inset 0 0 35px 12px rgba(244, 63, 94, 0.85); }
    }
    .container { width: 100%; max-width: 480px; display: flex; flex-direction: column; gap: 14px; }
    
    /* Cảnh báo Banner */
    .alert-banner {
      display: none; background: rgba(244, 63, 94, 0.2); border: 1px solid var(--accent);
      border-radius: 14px; padding: 12px 14px; color: #fff; font-size: 13px; font-weight: 600;
      align-items: center; gap: 10px; animation: slideDown 0.3s ease;
    }
    .alert-banner.show { display: flex; }
    .alert-icon { font-size: 22px; }
    
    /* Header */
    .header { text-align: center; margin: 4px 0 2px 0; }
    .header-tag { display: inline-flex; align-items: center; gap: 6px; font-size: 11px; font-weight: 700; color: var(--primary); background: rgba(56, 189, 248, 0.12); padding: 4px 10px; border-radius: 20px; border: 1px solid rgba(56, 189, 248, 0.25); margin-bottom: 6px; text-transform: uppercase; letter-spacing: 0.5px; }
    .header h1 { font-size: 21px; font-weight: 800; color: #fff; letter-spacing: -0.3px; display: flex; align-items: center; justify-content: center; gap: 8px; }
    .header p { color: var(--subtext); font-size: 12px; margin-top: 3px; }
    
    /* Thẻ Đang Ấp Trứng - Trọng Tâm */
    .active-egg-card {
      background: linear-gradient(135deg, rgba(30, 41, 59, 0.9) 0%, rgba(15, 23, 42, 0.95) 100%);
      border: 1px solid rgba(56, 189, 248, 0.3); border-radius: 18px; padding: 16px;
      box-shadow: 0 8px 25px rgba(0, 0, 0, 0.35); position: relative; overflow: hidden;
    }
    .active-egg-card::before {
      content: ""; position: absolute; top: -50px; right: -50px; width: 120px; height: 120px;
      background: radial-gradient(circle, rgba(56, 189, 248, 0.25) 0%, rgba(56, 189, 248, 0) 70%);
      border-radius: 50%; pointer-events: none;
    }
    .egg-header-row { display: flex; align-items: center; justify-content: space-between; margin-bottom: 12px; }
    .egg-main-info { display: flex; align-items: center; gap: 12px; }
    .egg-avatar { font-size: 38px; line-height: 1; filter: drop-shadow(0 4px 8px rgba(0,0,0,0.4)); }
    .egg-name-box h3 { font-size: 16px; font-weight: 700; color: #fff; }
    .egg-name-box p { font-size: 11px; color: var(--subtext); margin-top: 1px; }
    
    .stage-badge {
      display: inline-flex; align-items: center; gap: 5px; font-size: 11px; font-weight: 700;
      padding: 5px 10px; border-radius: 20px; text-transform: uppercase; letter-spacing: 0.3px;
    }
    .stage-badge.incubating {
      background: rgba(16, 185, 129, 0.18); border: 1px solid rgba(16, 185, 129, 0.45); color: #34d399;
    }
    .stage-badge.hatching {
      background: rgba(245, 158, 11, 0.2); border: 1px solid rgba(245, 158, 11, 0.6); color: #fbbf24;
      animation: pulseStage 2s infinite ease-in-out;
    }
    @keyframes pulseStage {
      0%, 100% { box-shadow: 0 0 0 0 rgba(245, 158, 11, 0); }
      50% { box-shadow: 0 0 10px 2px rgba(245, 158, 11, 0.4); }
    }
    
    .target-stats-bar {
      background: rgba(15, 23, 42, 0.7); border-radius: 12px; padding: 10px 14px;
      display: flex; justify-content: space-around; align-items: center; border: 1px solid rgba(255, 255, 255, 0.05);
    }
    .target-stat-item { text-align: center; }
    .target-stat-item span { font-size: 11px; color: var(--subtext); display: block; margin-bottom: 2px; }
    .target-stat-item b { font-size: 15px; font-weight: 700; color: var(--primary); }
    
    /* Grid Cảm Biến */
    .grid { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; }
    .card { background: var(--card); backdrop-filter: blur(14px); border: 1px solid var(--card-border); border-radius: 16px; padding: 14px; box-shadow: 0 4px 20px rgba(0,0,0,0.25); }
    .card-title { font-size: 11px; font-weight: 600; color: var(--subtext); text-transform: uppercase; margin-bottom: 4px; display: flex; align-items: center; gap: 6px; }
    .card-value { font-size: 26px; font-weight: 700; color: #fff; display: flex; align-items: baseline; gap: 4px; }
    .card-value span { font-size: 13px; color: var(--subtext); font-weight: 400; }
    .temp-danger { color: var(--accent) !important; text-shadow: 0 0 10px rgba(244, 63, 94, 0.5); }
    .temp-cold { color: #60a5fa !important; }
    
    /* Tiến độ chu kỳ */
    .progress-track { width: 100%; height: 7px; background: rgba(255,255,255,0.08); border-radius: 4px; margin-top: 10px; overflow: hidden; }
    .progress-bar { height: 100%; width: 0%; background: linear-gradient(90deg, var(--primary), var(--green)); border-radius: 4px; transition: width 0.4s ease; }
    .day-action-row { display: flex; align-items: center; justify-content: space-between; margin-top: 10px; }
    .day-quick-btn {
      background: rgba(255,255,255,0.06); border: 1px solid rgba(255,255,255,0.12); color: #fff;
      padding: 5px 10px; border-radius: 8px; font-size: 11px; font-weight: 600; cursor: pointer; transition: 0.2s;
    }
    .day-quick-btn:hover { background: rgba(56, 189, 248, 0.2); border-color: var(--primary); }

    /* Panel */
    .panel { background: var(--card); border: 1px solid var(--card-border); border-radius: 16px; padding: 16px; display: flex; flex-direction: column; gap: 12px; }
    .panel-title { font-size: 12px; font-weight: 700; color: var(--primary); text-transform: uppercase; letter-spacing: 0.5px; display: flex; align-items: center; gap: 6px; }
    
    /* Switch Grid Rơ-le */
    .switch-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; }
    .switch-btn {
      padding: 14px 12px; border-radius: 14px; border: 1px solid var(--card-border);
      background: rgba(255, 255, 255, 0.03); color: var(--subtext); font-size: 12px;
      font-weight: 600; display: flex; flex-direction: column; align-items: center;
      gap: 7px; cursor: pointer; transition: all 0.25s ease;
    }
    .switch-btn .dev-icon { font-size: 22px; }
    .switch-btn.active-heater { background: rgba(245, 158, 11, 0.15); border-color: rgba(245, 158, 11, 0.6); color: var(--orange); box-shadow: 0 4px 14px rgba(245, 158, 11, 0.2); }
    .switch-btn.active-fan { background: rgba(56, 189, 248, 0.15); border-color: rgba(56, 189, 248, 0.6); color: var(--primary); box-shadow: 0 4px 14px rgba(56, 189, 248, 0.2); }
    .status-dot { width: 8px; height: 8px; border-radius: 50%; background: #64748b; }
    .active-heater .status-dot { background: var(--orange); box-shadow: 0 0 8px var(--orange); }
    .active-fan .status-dot { background: var(--primary); box-shadow: 0 0 8px var(--primary); }
    
    /* Menu Chọn Loại Trứng */
    .custom-select-box { position: relative; width: 100%; }
    .dropdown-btn {
      width: 100%; padding: 13px 15px; border-radius: 14px;
      border: 1px solid rgba(56, 189, 248, 0.35); background: #172554;
      color: #fff; font-size: 14px; font-weight: 600; display: flex;
      justify-content: space-between; align-items: center; cursor: pointer;
      box-shadow: 0 4px 12px rgba(0, 0, 0, 0.2); transition: all 0.2s ease;
    }
    .dropdown-arrow {
      width: 16px; height: 16px; transition: transform 0.25s ease;
      fill: none; stroke: var(--primary); stroke-width: 2.5; stroke-linecap: round; stroke-linejoin: round;
    }
    .dropdown-btn.open .dropdown-arrow { transform: rotate(180deg); }
    .dropdown-menu {
      position: absolute; top: calc(100% + 6px); left: 0; width: 100%;
      background: #0f172a; border: 1px solid rgba(255,255,255,0.15); border-radius: 14px;
      box-shadow: 0 10px 25px rgba(0,0,0,0.7); display: none; flex-direction: column;
      overflow: hidden; z-index: 100;
    }
    .dropdown-menu.show { display: flex; animation: slideDown 0.2s ease; }
    @keyframes slideDown { from { opacity: 0; transform: translateY(-8px); } to { opacity: 1; transform: translateY(0); } }
    .dropdown-item {
      padding: 12px 14px; display: flex; align-items: center; justify-content: space-between;
      color: #cbd5e1; font-size: 13px; font-weight: 500; border-bottom: 1px solid rgba(255,255,255,0.04);
      cursor: pointer; transition: 0.15s;
    }
    .dropdown-item:last-child { border-bottom: none; }
    .dropdown-item:hover, .dropdown-item:active { background: rgba(56, 189, 248, 0.12); color: #fff; }
    .dropdown-item.active { background: rgba(56, 189, 248, 0.2); color: var(--primary); font-weight: 700; }
    .check-icon { display: none; width: 16px; height: 16px; stroke: var(--primary); stroke-width: 2.5; fill: none; }
    .dropdown-item.active .check-icon { display: block; }
    
    .egg-info-box {
      background: rgba(15, 23, 42, 0.6); border-radius: 12px; padding: 12px;
      display: grid; grid-template-columns: 1fr 1fr; gap: 8px; font-size: 12px;
      border: 1px dashed rgba(255,255,255,0.12);
    }
    .egg-info-item span { color: var(--subtext); display: block; margin-bottom: 2px; }
    .egg-info-item b { color: #fff; font-size: 13px; }
    
    /* Nút Bấm */
    .btn {
      width: 100%; padding: 13px; border-radius: 12px; border: none; font-weight: 700;
      font-size: 13px; cursor: pointer; transition: all 0.15s ease; color: #fff;
      display: inline-flex; align-items: center; justify-content: center; gap: 6px;
    }
    .btn:active { transform: scale(0.98); }
    .btn-apply { background: #0284c7; box-shadow: 0 4px 12px rgba(2, 132, 199, 0.3); }
    .btn-apply:hover { background: #0369a1; }
    .btn-edit { background: rgba(255,255,255,0.08); border: 1px solid rgba(255,255,255,0.15); color: #fff; }
    .btn-edit:hover { background: rgba(255,255,255,0.14); }
    .btn-stop { background: var(--accent); box-shadow: 0 4px 12px rgba(244, 63, 94, 0.3); }
    .btn-run { background: var(--green); box-shadow: 0 4px 12px rgba(16, 185, 129, 0.3); }
    .btn-row { display: flex; gap: 10px; }
    
    /* Modal Chỉnh Sửa */
    .modal-overlay {
      position: fixed; top: 0; left: 0; width: 100%; height: 100%;
      background: rgba(0, 0, 0, 0.8); backdrop-filter: blur(8px);
      display: none; justify-content: center; align-items: center; z-index: 999; padding: 16px;
    }
    .modal-box {
      background: #1e293b; border: 1px solid rgba(255,255,255,0.15); border-radius: 20px;
      width: 100%; max-width: 440px; padding: 20px; display: flex; flex-direction: column; gap: 14px;
      box-shadow: 0 10px 30px rgba(0,0,0,0.6); animation: popIn 0.25s cubic-bezier(0.16, 1, 0.3, 1);
    }
    @keyframes popIn { from { transform: scale(0.92); opacity: 0; } to { transform: scale(1); opacity: 1; } }
    .modal-title { font-size: 16px; font-weight: 700; color: #fff; display: flex; justify-content: space-between; align-items: center; }
    .modal-form-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; }
    .field-group label { display: block; font-size: 11px; font-weight: 600; color: var(--subtext); margin-bottom: 4px; }
    .field-group input {
      width: 100%; border-radius: 10px; border: 1px solid rgba(255,255,255,0.12);
      background: #0f172a; color: #fff; padding: 10px; font-size: 13px; outline: none;
    }
    .field-group input:focus { border-color: var(--primary); }
    .modal-actions { display: flex; gap: 8px; margin-top: 6px; }
    
    /* Ghi chú Còi & An Toàn */
    .guide-box {
      background: rgba(15, 23, 42, 0.5); border-radius: 14px; padding: 12px 14px;
      border: 1px solid rgba(255,255,255,0.06); font-size: 11px; color: var(--subtext);
      display: flex; flex-direction: column; gap: 6px;
    }
    .guide-box-title { color: #cbd5e1; font-weight: 700; display: flex; align-items: center; gap: 6px; }
    .guide-item { display: flex; align-items: flex-start; gap: 6px; line-height: 1.4; }
    .theme-toggle {
      margin-top: 10px; padding: 8px 12px; border: 1px solid var(--card-border);
      border-radius: 10px; background: var(--card); color: var(--text);
      font-size: 12px; font-weight: 700; cursor: pointer; transition: all 0.2s ease;
    }
    .theme-toggle:hover { border-color: var(--primary); }

    body[data-theme="light"] {
      --bg: #e9eef3;
      --card: #f4f6f8;
      --card-border: rgba(51, 65, 85, 0.12);
      --primary: #28749a;
      --text: #263746;
      --subtext: #667887;
      background: var(--bg);
    }
    body[data-theme="light"] .header h1,
    body[data-theme="light"] .egg-name-box h3,
    body[data-theme="light"] .card-value,
    body[data-theme="light"] .egg-info-item b,
    body[data-theme="light"] .modal-title,
    body[data-theme="light"] .guide-box-title { color: var(--text); }
    body[data-theme="light"] .active-egg-card {
      background: linear-gradient(135deg, #f6f8fa 0%, #e5edf2 100%);
      border-color: rgba(40, 116, 154, 0.22);
      box-shadow: 0 8px 25px rgba(41, 59, 73, 0.09);
    }
    body[data-theme="light"] .target-stats-bar { background: rgba(232, 238, 243, 0.82); border-color: rgba(51, 65, 85, 0.08); }
    body[data-theme="light"] .card { box-shadow: 0 4px 16px rgba(41, 59, 73, 0.07); }
    body[data-theme="light"] .progress-track { background: rgba(51, 65, 85, 0.1); }
    body[data-theme="light"] .day-quick-btn { background: #edf1f4; border-color: rgba(51, 65, 85, 0.14); color: var(--text); }
    body[data-theme="light"] .switch-btn { background: rgba(244, 246, 248, 0.85); }
    body[data-theme="light"] .dropdown-btn { background: #edf2f5; color: var(--text); }
    body[data-theme="light"] .dropdown-menu { background: #f3f5f7; border-color: rgba(51, 65, 85, 0.14); box-shadow: 0 10px 25px rgba(41, 59, 73, 0.16); }
    body[data-theme="light"] .dropdown-item { color: #3d5261; border-bottom-color: rgba(51, 65, 85, 0.07); }
    body[data-theme="light"] .dropdown-item:hover,
    body[data-theme="light"] .dropdown-item:active { color: #263746; }
    body[data-theme="light"] .egg-info-box { background: rgba(235, 240, 244, 0.9); border-color: rgba(51, 65, 85, 0.15); }
    body[data-theme="light"] .btn-edit { background: #e1e8ed; border-color: #cbd6de; color: #263746; }
    body[data-theme="light"] .btn-edit:hover { background: #d5e0e7; }
    body[data-theme="light"] .modal-box { background: #f2f5f7; border-color: rgba(51, 65, 85, 0.15); box-shadow: 0 10px 30px rgba(41, 59, 73, 0.18); }
    body[data-theme="light"] .field-group input { background: #e9eef2; border-color: rgba(51, 65, 85, 0.16); color: #263746; }
    body[data-theme="light"] .guide-box { background: rgba(239, 243, 246, 0.9); border-color: rgba(51, 65, 85, 0.09); }
  </style>
</head>
<body>
  <div class="container">
    
    <!-- Cảnh Báo Nguy Hiểm -->
    <div class="alert-banner" id="alertBanner">
      <span class="alert-icon">🚨</span>
      <div id="alertText">CẢNH BÁO: Môi trường ngoài ngưỡng an toàn cho trứng!</div>
    </div>

    <!-- Header -->
    <div class="header">
      <div class="header-tag">✨ IoT Smart Incubator System</div>
      <h1>🥚 MÁY ẤP TRỨNG THÔNG MINH</h1>
      <p>Giám sát vi khí hậu & Tự động điều chỉnh giai đoạn</p>
      <button class="theme-toggle" id="themeToggle" type="button" onclick="toggleTheme()" aria-pressed="false">
        ☀️ Giao diện sáng
      </button>
    </div>

    <!-- Thẻ Đang Ấp Trứng Hiện Tại -->
    <div class="active-egg-card">
      <div class="egg-header-row">
        <div class="egg-main-info">
          <div class="egg-avatar" id="curEggIcon">🥚</div>
          <div class="egg-name-box">
            <h3 id="curEggName">Trứng Gà Ta / Gà Nòi</h3>
            <p id="curEggDesc">Hồ sơ ấp chuẩn sinh học</p>
          </div>
        </div>
        <div class="stage-badge incubating" id="curStageBadge">
          🌱 Ấp Phôi
        </div>
      </div>
      <div class="target-stats-bar">
        <div class="target-stat-item">
          <span>Mục Tiêu Nhiệt</span>
          <b id="curTargetTemp">37.5°C</b>
        </div>
        <div style="width:1px; height:24px; background:rgba(255,255,255,0.1);"></div>
        <div class="target-stat-item">
          <span>Mục Tiêu Ẩm</span>
          <b id="curTargetHumi">60%</b>
        </div>
        <div style="width:1px; height:24px; background:rgba(255,255,255,0.1);"></div>
        <div class="target-stat-item">
          <span>Giai Đoạn Nở Từ</span>
          <b id="curHatchFromDay">Ngày 19</b>
        </div>
      </div>
    </div>

    <!-- Thẻ Cảm Biến Thời Gian Thực -->
    <div class="grid">
      <div class="card">
        <div class="card-title">🌡️ Nhiệt độ hiện tại</div>
        <div class="card-value"><span id="temp">--</span><span>°C</span></div>
      </div>
      <div class="card">
        <div class="card-title">💧 Độ ẩm hiện tại</div>
        <div class="card-value"><span id="humi">--</span><span>%</span></div>
      </div>
    </div>

    <!-- Thẻ Tiến Độ Chu Kỳ Ấp -->
    <div class="card">
      <div style="display:flex; justify-content:space-between; align-items:center;">
        <div class="card-title" style="margin:0;">📅 Tiến độ chu kỳ ấp</div>
        <div style="font-size:13px; font-weight:800; color:var(--primary);" id="txtDayPercent">--%</div>
      </div>
      <div class="card-value" style="margin-top:6px;">
        Ngày <span id="day" style="color:var(--primary); margin:0 4px;">--</span> 
        <span id="maxDay">/ 21 ngày</span>
      </div>
      <div class="progress-track"><div class="progress-bar" id="dayBar"></div></div>
      <div class="day-action-row">
        <span style="font-size:11px; color:var(--subtext);">Tự động chuyển ngày qua đồng hồ RTC</span>
        <button class="day-quick-btn" onclick="promptSetDay()">✏️ Đổi ngày ấp</button>
      </div>
    </div>

    <!-- Bảng Rơ-le Điều Khiển -->
    <div class="panel">
      <div class="panel-title">⚡ ĐIỀU KHIỂN RƠ-LE THỦ CÔNG</div>
      <div class="switch-grid">
        <button id="btnHeater" class="switch-btn" onclick="toggleDevice('heater')">
          <span class="dev-icon">💡</span>
          <div class="status-dot"></div>
          <span>ĐÈN SƯỞI: <b id="txtHeater">OFF</b></span>
        </button>
        <button id="btnFan" class="switch-btn" onclick="toggleDevice('fan')">
          <span class="dev-icon">🌀</span>
          <div class="status-dot"></div>
          <span>QUẠT GIÓ: <b id="txtFan">OFF</b></span>
        </button>
      </div>
    </div>

    <!-- Bảng Chọn Profile Trứng & Chỉnh Sửa -->
    <div class="panel">
      <div class="panel-title">🎛️ DANH SÁCH 7 HỒ SƠ ẤP & CHẾ ĐỘ TEST</div>
      <div class="custom-select-box">
        <div class="dropdown-btn" id="customDropdownBtn" onclick="toggleDropdown()">
          <span id="selectedEggLabel">🥚 Trứng Gà Ta / Gà Nòi</span>
          <svg class="dropdown-arrow" viewBox="0 0 24 24"><polyline points="6 9 12 15 18 9"></polyline></svg>
        </div>
        <div class="dropdown-menu" id="customDropdownMenu"></div>
      </div>
      <div class="egg-info-box" id="infoBox"></div>
      <div class="btn-row">
        <button class="btn btn-apply" onclick="applyCurrentProfile()">🚀 Áp dụng hồ sơ</button>
        <button class="btn btn-edit" onclick="openEditModal()">⚙️ Chỉnh sửa</button>
      </div>
    </div>

    <!-- Bắt Đầu / Tạm Dừng Máy -->
    <div class="btn-row">
      <button class="btn btn-run" onclick="setRun(1)">▶️ Bắt đầu chu kỳ</button>
      <button class="btn btn-stop" onclick="setRun(0)">⏸️ Tạm dừng máy</button>
    </div>

    <!-- Hướng Dẫn Quy Ước Còi Báo -->
    <div class="guide-box">
      <div class="guide-box-title">🔔 QUY ƯỚC CÒI BÁO THÔNG MINH</div>
      <div class="guide-item"><span>•</span> <span><b>Mặc định:</b> Còi TẮT hoàn toàn trong quá trình ấp bình thường.</span></div>
      <div class="guide-item"><span>•</span> <span><b>1 Bíp ngắn:</b> Xác nhận bấm phím vật lý hoặc đổi lệnh trên Web.</span></div>
      <div class="guide-item"><span>•</span> <span><b>Tít dồn dập (nhanh):</b> Quá nhiệt nguy hiểm (&ge;38.3°C hoặc vượt ngưỡng 0.8°C). Quạt tự động xả nhiệt.</span></div>
      <div class="guide-item"><span>•</span> <span><b>Tít dài (chậm):</b> Quá lạnh (&le;35.0°C). Đèn tự động kích sưởi.</span></div>
      <div class="guide-item"><span>•</span> <span><b>Tít kép ngắt quãng:</b> Cảnh báo độ ẩm lệch chuẩn nguy hiểm (&lt;45% giai đoạn cận nở hoặc &gt;88%).</span></div>
    </div>

  </div>

  <!-- Modal Chỉnh Sửa Chi Tiết -->
  <div class="modal-overlay" id="editModal">
    <div class="modal-box">
      <div class="modal-title">
        <span id="modalEggName">⚙️ Tùy chỉnh thông số sinh học</span>
        <span onclick="closeEditModal()" style="cursor:pointer; color:var(--subtext); font-size:22px;">✕</span>
      </div>
      <div class="modal-form-grid">
        <div class="field-group"><label>Nhiệt độ ấp phôi (°C)</label><input type="number" step="0.1" id="m_tAp"></div>
        <div class="field-group"><label>Độ ẩm ấp phôi (%)</label><input type="number" id="m_hAp"></div>
        <div class="field-group"><label>Nhiệt độ cận nở (°C)</label><input type="number" step="0.1" id="m_tNo"></div>
        <div class="field-group"><label>Độ ẩm cận nở (%)</label><input type="number" id="m_hNo"></div>
        <div class="field-group"><label>Tổng ngày chu kỳ</label><input type="number" id="m_days"></div>
        <div class="field-group"><label>Ngày chuyển khay nở</label><input type="number" id="m_hatchDay"></div>
      </div>
      <div class="modal-actions">
        <button class="btn btn-apply" onclick="saveAndApplyCustom()">💾 Lưu & Kích hoạt</button>
        <button class="btn btn-edit" onclick="closeEditModal()">Hủy</button>
      </div>
    </div>
  </div>

  <script>
    const EGG_DB = [
      { icon: "🥚", name: "Trứng Gà Ta / Gà Nòi", tAp: 37.5, hAp: 60, tNo: 37.0, hNo: 70, days: 21, hatchDay: 19 },
      { icon: "🦆", name: "Trứng Vịt Cỏ / Vịt Xiêm", tAp: 37.8, hAp: 65, tNo: 37.1, hNo: 75, days: 28, hatchDay: 25 },
      { icon: "🐦", name: "Trứng Chim Cút Siêu Trứng", tAp: 37.5, hAp: 55, tNo: 37.0, hNo: 65, days: 17, hatchDay: 15 },
      { icon: "🕊️", name: "Trứng Bồ Câu Pháp", tAp: 38.0, hAp: 58, tNo: 37.3, hNo: 68, days: 18, hatchDay: 16 },
      { icon: "🪿", name: "Trứng Ngỗng Sư Tử", tAp: 37.7, hAp: 68, tNo: 37.0, hNo: 80, days: 30, hatchDay: 27 },
      { icon: "🦚", name: "Chim Trĩ Đỏ Khoang Cổ", tAp: 37.6, hAp: 60, tNo: 37.0, hNo: 70, days: 24, hatchDay: 21 },
      { icon: "🧪", name: "Chế Độ Test Hệ Thống (Mô Phỏng)", tAp: 27.0, hAp: 60, tNo: 28.0, hNo: 65, days: 3, hatchDay: 2 }
    ];
    let currentSelectedIdx = 0, heaterState = 0, fanState = 0, isAlarming = false;

    function setTheme(theme, persist = false) {
      const isLight = theme === 'light';
      document.body.dataset.theme = isLight ? 'light' : 'dark';
      const button = document.getElementById('themeToggle');
      button.innerText = isLight ? '🌙 Giao diện tối' : '☀️ Giao diện sáng';
      button.setAttribute('aria-pressed', String(isLight));

      if (persist) {
        try {
          localStorage.setItem('incubator-theme', isLight ? 'light' : 'dark');
        } catch (error) {
          console.warn('Không thể lưu tùy chọn giao diện:', error);
        }
      }
    }

    function toggleTheme() {
      setTheme(document.body.dataset.theme === 'light' ? 'dark' : 'light', true);
    }

    function initCustomDropdown() {
      let menu = document.getElementById('customDropdownMenu'), html = '';
      EGG_DB.forEach((p, idx) => {
        html += `<div class="dropdown-item ${idx === 0 ? 'active' : ''}" onclick="selectEggProfile(${idx})">
          <span>${p.icon} ${p.name}</span>
          <svg class="check-icon" viewBox="0 0 24 24"><polyline points="20 6 9 17 4 12"></polyline></svg>
        </div>`;
      });
      menu.innerHTML = html;
    }

    function toggleDropdown() {
      document.getElementById('customDropdownBtn').classList.toggle('open');
      document.getElementById('customDropdownMenu').classList.toggle('show');
    }

    function selectEggProfile(idx) {
      currentSelectedIdx = idx;
      let p = EGG_DB[idx];
      document.getElementById('selectedEggLabel').innerText = `${p.icon} ${p.name}`;
      document.querySelectorAll('.dropdown-item').forEach((item, i) => item.classList.toggle('active', i === idx));
      toggleDropdown();
      renderProfileInfo();
    }

    document.addEventListener('click', function(e) {
      let box = document.querySelector('.custom-select-box');
      if (box && !box.contains(e.target)) {
        document.getElementById('customDropdownBtn').classList.remove('open');
        document.getElementById('customDropdownMenu').classList.remove('show');
      }
    });

    function renderProfileInfo() {
      let p = EGG_DB[currentSelectedIdx];
      document.getElementById('infoBox').innerHTML = `
        <div class="egg-info-item"><span>Nhiệt độ ấp phôi:</span><b>${p.tAp}°C</b></div>
        <div class="egg-info-item"><span>Độ ẩm ấp phôi:</span><b>${p.hAp}%</b></div>
        <div class="egg-info-item"><span>Nhiệt độ cận nở:</span><b>${p.tNo}°C</b></div>
        <div class="egg-info-item"><span>Độ ẩm cận nở:</span><b>${p.hNo}%</b></div>
        <div class="egg-info-item"><span>Tổng chu kỳ:</span><b>${p.days} Ngày</b></div>
        <div class="egg-info-item"><span>Chuyển khay nở:</span><b>Ngày thứ ${p.hatchDay}</b></div>`;
    }

    function triggerScreenAlert() {
      if (isAlarming) return;
      isAlarming = true;
      document.body.classList.add('alarm-flash');
      setTimeout(() => {
        document.body.classList.remove('alarm-flash');
        setTimeout(() => { isAlarming = false; }, 8000);
      }, 2400);
    }

    let isFetching = false;

    function updateData() {
      if (isFetching) return;
      isFetching = true;

      fetch('/api/data', { cache: 'no-store' })
        .then(r => {
          if (!r.ok) throw new Error("HTTP " + r.status);
          return r.json();
        })
        .then(d => {
          let tempEl = document.getElementById('temp');
          if (typeof d.temp === 'number') {
            tempEl.innerText = d.temp.toFixed(1);
          }
          if (d.humi !== undefined) document.getElementById('humi').innerText = d.humi;
          if (d.day !== undefined) document.getElementById('day').innerText = d.day;
          
          // Cập nhật loại trứng đang ấp thực tế trên máy
          let activeProfileIdx = (typeof d.profile === 'number' && d.profile >= 0 && d.profile < EGG_DB.length) ? d.profile : currentSelectedIdx;
          let curEgg = EGG_DB[activeProfileIdx];
          document.getElementById('curEggIcon').innerText = curEgg.icon;
          document.getElementById('curEggName').innerText = curEgg.name;
          document.getElementById('curHatchFromDay').innerText = 'Ngày ' + curEgg.hatchDay;
          document.getElementById('maxDay').innerText = `/ ${curEgg.days} ngày`;

          // Giai đoạn đang ấp (0: Ấp phôi, 1: Cận nở)
          let stageBadge = document.getElementById('curStageBadge');
          let isHatchingStage = (d.stage === 1) || (d.day >= curEgg.hatchDay);
          if (isHatchingStage) {
            stageBadge.className = 'stage-badge hatching';
            stageBadge.innerHTML = '🐣 Cận Nở (Tự Hạ Nhiệt & Tăng Ẩm)';
          } else {
            stageBadge.className = 'stage-badge incubating';
            stageBadge.innerHTML = '🌱 Ấp Phôi (Phát Triển)';
          }

          // Mục tiêu nhiệt độ / độ ẩm đang áp dụng
          let curT = (typeof d.targetTemp === 'number') ? d.targetTemp.toFixed(1) : (isHatchingStage ? curEgg.tNo.toFixed(1) : curEgg.tAp.toFixed(1));
          let curH = (typeof d.targetHumi === 'number') ? d.targetHumi : (isHatchingStage ? curEgg.hNo : curEgg.hAp);
          document.getElementById('curTargetTemp').innerText = curT + '°C';
          document.getElementById('curTargetHumi').innerText = curH + '%';

          // Cảnh báo môi trường
          let banner = document.getElementById('alertBanner');
          let alertText = document.getElementById('alertText');
          let targetMax = parseFloat(curT) + 0.8;
          let targetMin = parseFloat(curT) - 1.5;

          if (d.temp === 0 || d.humi === 0) {
            tempEl.classList.add('temp-danger');
            tempEl.classList.remove('temp-cold');
            alertText.innerHTML = `❌ <b>LỖI CẢM BIẾN!</b> Không nhận được tín hiệu DHT11. Hãy kiểm tra dây nối!`;
            banner.classList.add('show');
          } else if (d.temp >= targetMax || d.temp >= 38.3) {
            tempEl.classList.add('temp-danger');
            tempEl.classList.remove('temp-cold');
            alertText.innerHTML = `🚨 <b>QUÁ NHIỆT NGUY CẤP (${d.temp.toFixed(1)}°C)!</b> Còi dồn dập, quạt đang xả nhiệt!`;
            banner.classList.add('show');
            triggerScreenAlert();
          } else if (d.temp <= targetMin) {
            tempEl.classList.remove('temp-danger');
            tempEl.classList.add('temp-cold');
            alertText.innerHTML = `❄️ <b>QUÁ LẠNH (${d.temp.toFixed(1)}°C)!</b> Còi kêu chậm, đèn sưởi đang kích nhiệt!`;
            banner.classList.add('show');
            triggerScreenAlert();
          } else if ((isHatchingStage && d.humi < 45) || d.humi > 88) {
            tempEl.classList.remove('temp-danger');
            tempEl.classList.remove('temp-cold');
            alertText.innerHTML = `💧 <b>CẢNH BÁO ĐỘ ẨM (${d.humi}%)!</b> Lệch ngưỡng an toàn cho sự sống của phôi!`;
            banner.classList.add('show');
            triggerScreenAlert();
          } else {
            tempEl.classList.remove('temp-danger');
            tempEl.classList.remove('temp-cold');
            banner.classList.remove('show');
          }

          // Tiến độ ngày
          let totalDays = curEgg.days;
          let percent = Math.min(100, Math.round(((d.day || 1) / totalDays) * 100));
          document.getElementById('txtDayPercent').innerText = percent + '%';
          document.getElementById('dayBar').style.width = percent + '%';

          // Trạng thái Rơ-le
          heaterState = d.light; fanState = d.fan;
          let btnH = document.getElementById('btnHeater'), txtH = document.getElementById('txtHeater');
          let btnF = document.getElementById('btnFan'), txtF = document.getElementById('txtFan');
          btnH.className = heaterState ? 'switch-btn active-heater' : 'switch-btn';
          txtH.innerText = heaterState ? 'ON' : 'OFF';
          btnF.className = fanState ? 'switch-btn active-fan' : 'switch-btn';
          txtF.innerText = fanState ? 'ON' : 'OFF';
        })
        .catch(err => {
          console.warn('Lỗi đọc dữ liệu /api/data:', err);
        })
        .finally(() => {
          isFetching = false;
          setTimeout(updateData, document.hidden ? 2000 : 500);
        });
    }

    function toggleDevice(dev) {
      if (dev === 'heater') { heaterState = heaterState ? 0 : 1; fetch('/api/toggle?device=heater&val=' + heaterState); }
      else if (dev === 'fan') { fanState = fanState ? 0 : 1; fetch('/api/toggle?device=fan&val=' + fanState); }
      updateData();
    }

    function applyCurrentProfile() { 
      let p = EGG_DB[currentSelectedIdx];
      fetch('/api/profile?val=' + currentSelectedIdx)
        .then(r => {
          if (r.ok) alert('✅ Đã kích hoạt hồ sơ: ' + p.icon + ' ' + p.name);
          else alert('Lỗi máy chủ: HTTP ' + r.status);
        })
        .catch(e => alert('Lỗi kết nối: ' + e));
    }

    function promptSetDay() {
      let cur = document.getElementById('day').innerText;
      let newDay = prompt("Nhập số ngày ấp hiện tại (1 - 45):", cur);
      if (newDay !== null && newDay.trim() !== "") {
        let val = parseInt(newDay.trim());
        if (!isNaN(val) && val >= 1 && val <= 45) {
          fetch('/api/set_day?val=' + val)
            .then(r => {
              if (r.ok) {
                alert('✅ Đã cập nhật ngày ấp thành: Ngày ' + val);
                updateData();
              } else alert('Lỗi cài đặt ngày: HTTP ' + r.status);
            })
            .catch(e => alert('Lỗi kết nối: ' + e));
        } else {
          alert('Số ngày không hợp lệ! Vui lòng nhập từ 1 đến 45.');
        }
      }
    }

    function openEditModal() {
      let p = EGG_DB[currentSelectedIdx];
      document.getElementById('modalEggName').innerText = '⚙️ Chỉnh: ' + p.icon + ' ' + p.name;
      document.getElementById('m_tAp').value = p.tAp; document.getElementById('m_hAp').value = p.hAp;
      document.getElementById('m_tNo').value = p.tNo; document.getElementById('m_hNo').value = p.hNo;
      document.getElementById('m_days').value = p.days; document.getElementById('m_hatchDay').value = p.hatchDay;
      document.getElementById('editModal').style.display = 'flex';
    }

    function closeEditModal() { document.getElementById('editModal').style.display = 'none'; }

    function saveAndApplyCustom() {
      let p = EGG_DB[currentSelectedIdx];
      p.tAp = parseFloat(document.getElementById('m_tAp').value); p.hAp = parseInt(document.getElementById('m_hAp').value);
      p.tNo = parseFloat(document.getElementById('m_tNo').value); p.hNo = parseInt(document.getElementById('m_hNo').value);
      p.days = parseInt(document.getElementById('m_days').value); p.hatchDay = parseInt(document.getElementById('m_hatchDay').value);
      renderProfileInfo(); closeEditModal();
      fetch(`/api/custom_profile?tAp=${p.tAp}&hAp=${p.hAp}&tNo=${p.tNo}&hNo=${p.hNo}&days=${p.days}&hatch=${p.hatchDay}`)
        .then(r => {
          if (r.ok) alert('✅ Đã lưu & gửi thông số sang máy ấp thành công!');
          else alert('Lỗi máy chủ: HTTP ' + r.status);
        })
        .catch(e => alert('Lỗi kết nối: ' + e));
    }

    function setRun(state) { 
      fetch('/api/run?val=' + state)
        .then(r => {
          if (r.ok) alert(state ? '▶️ Hệ thống đã bắt đầu chu trình ấp!' : '⏸️ Hệ thống đã tạm dừng!');
          else alert('Lỗi máy chủ: HTTP ' + r.status);
        })
        .catch(e => alert('Lỗi kết nối: ' + e));
    }

    let savedTheme = null;
    try {
      savedTheme = localStorage.getItem('incubator-theme');
    } catch (error) {
      console.warn('Không thể đọc tùy chọn giao diện:', error);
    }
    setTheme(savedTheme === 'light' ? 'light' : 'dark');
    initCustomDropdown(); renderProfileInfo(); updateData();
  </script>
</body>
</html>
)rawliteral";

#endif