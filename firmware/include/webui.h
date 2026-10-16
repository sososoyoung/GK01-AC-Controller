#ifndef WEBUI_H
#define WEBUI_H
#include <Arduino.h>
const char INDEX_HTML[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="zh-CN">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<meta name="apple-mobile-web-app-capable" content="yes">
<meta name="apple-mobile-web-app-status-bar-style" content="default">
<meta name="apple-mobile-web-app-title" content="IR遥控">
<title>IR 遥控器</title>
<style>
:root{--bg:#f2f2f7;--card:#fff;--blue:#007AFF;--green:#34C759;--red:#FF3B30;
--orange:#FF9500;--purple:#AF52DE;--teal:#5AC8FA;--pink:#FF2D55;
--gray:#8e8e93;--gray3:#c7c7cc;--gray4:#d1d1d6;--gray5:#e5e5ea;--gray6:#f2f2f7;
--text:#1c1c1e;--text2:#3c3c43;--text3:#636366;--r:14px;--max:560px}
*{margin:0;padding:0;box-sizing:border-box;-webkit-tap-highlight-color:transparent}
body{font-family:-apple-system,BlinkMacSystemFont,"SF Pro Text","Helvetica Neue",sans-serif;
background:var(--bg);color:var(--text);min-height:100vh;min-height:100dvh;
-webkit-font-smoothing:antialiased}
.hdr{position:sticky;top:0;z-index:50;background:rgba(242,242,247,.92);
backdrop-filter:blur(20px);-webkit-backdrop-filter:blur(20px);
padding:12px 20px 10px;border-bottom:.5px solid var(--gray5);text-align:center}
.hdr h1{font-size:17px;font-weight:600}
nav.bar{position:fixed;bottom:0;left:0;right:0;background:rgba(249,249,249,.94);
backdrop-filter:blur(20px);-webkit-backdrop-filter:blur(20px);
border-top:.5px solid var(--gray4);display:flex;z-index:100;
padding-bottom:env(safe-area-inset-bottom)}
body{padding-bottom:80px}
nav.bar a{flex:1;display:flex;flex-direction:column;align-items:center;padding:6px 0 4px;
color:var(--gray);text-decoration:none;font-size:10px;font-weight:500;transition:color .15s;cursor:pointer}
nav.bar a.active{color:var(--blue)}
nav.bar a i{font-size:22px;line-height:1.2;margin-bottom:1px;font-style:normal}
.pg{display:none;padding:16px;max-width:var(--max);margin:0 auto}
.pg.active{display:block}
.pg-title{font-size:28px;font-weight:700;letter-spacing:-.5px;margin-bottom:14px}
.pg-sub{font-size:13px;color:var(--gray);margin-top:-10px;margin-bottom:14px}
.cd{background:var(--card);border-radius:var(--r);box-shadow:0 .5px 2px rgba(0,0,0,.06);margin-bottom:12px;overflow:hidden}
.cd-h{padding:10px 16px 4px;font-size:12px;text-transform:uppercase;color:var(--gray);font-weight:700;letter-spacing:.5px}
.cd-b{padding:8px 16px 12px}
.it{display:flex;align-items:center;justify-content:space-between;padding:11px 16px;min-height:44px;cursor:pointer}
.it+.it{border-top:.5px solid var(--gray6)}
.it span{font-size:16px}
.it .arr{color:var(--gray3)}
.it .arr::after{content:'';display:inline-block;width:7px;height:7px;
border-right:1.5px solid var(--gray3);border-bottom:1.5px solid var(--gray3);
transform:rotate(-45deg);margin-left:6px}
.b{display:inline-flex;align-items:center;justify-content:center;gap:5px;padding:11px 20px;
border-radius:12px;font-size:16px;font-weight:600;border:none;cursor:pointer;
transition:opacity .12s,transform .08s;-webkit-user-select:none;user-select:none}
.b:active{opacity:.55;transform:scale(.96)}
.b-bl{background:var(--blue);color:#fff}
.b-gn{background:var(--green);color:#fff}
.b-rd{background:var(--red);color:#fff}
.b-or{background:var(--orange);color:#fff}
.b-pp{background:var(--purple);color:#fff}
.b-ol{background:transparent;color:var(--blue);border:1.5px solid var(--blue)}
.b-gy{background:var(--gray5);color:var(--text)}
.b-fw{width:100%}
.seg{display:flex;background:var(--gray6);border-radius:10px;padding:2px;margin:6px 0}
.seg button{flex:1;padding:7px 2px;border:none;background:transparent;
font-size:13px;font-weight:500;color:var(--text3);border-radius:8px;cursor:pointer;transition:all .2s}
.seg button.active{background:#fff;color:var(--text);box-shadow:0 1px 3px rgba(0,0,0,.08)}
.temp{display:flex;align-items:center;justify-content:center;gap:20px;padding:12px 0}
.temp-v{font-size:64px;font-weight:200;letter-spacing:-3px;min-width:100px;text-align:center}
.temp-u{font-size:22px;font-weight:300;color:var(--gray)}
.temp-b{width:50px;height:50px;border-radius:50%;border:none;font-size:22px;font-weight:300;
cursor:pointer;background:var(--gray6);color:var(--text);transition:transform .08s}
.temp-b:active{transform:scale(.9)}
select{width:100%;padding:11px 16px;border-radius:10px;border:.5px solid var(--gray4);
font-size:16px;background:var(--card);color:var(--text);appearance:none;
background-image:url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' width='12' height='12'%3E%3Cpath d='M2 4l4 4 4-4' fill='none' stroke='%238e8e93' stroke-width='1.5'/%3E%3C/svg%3E");
background-repeat:no-repeat;background-position:right 14px center}
.grid{display:grid;grid-template-columns:repeat(3,1fr);gap:10px;padding:4px 0}
.grid-b{padding:13px 6px;border-radius:12px;border:1.5px solid var(--gray5);
background:var(--card);font-size:13px;font-weight:500;cursor:pointer;
display:flex;flex-direction:column;align-items:center;gap:3px;transition:all .12s}
.grid-b:active{background:var(--gray6);transform:scale(.95)}
.grid-b .ic{font-size:22px}
.toast{position:fixed;top:50%;left:50%;transform:translate(-50%,-50%) scale(.8);
background:rgba(0,0,0,.78);color:#fff;padding:12px 24px;border-radius:14px;
font-size:15px;font-weight:500;z-index:999;opacity:0;transition:all .2s;pointer-events:none;
text-align:center;max-width:260px}
.toast.show{opacity:1;transform:translate(-50%,-50%) scale(1)}
input[type=text],input[type=password],input[type=number]{width:100%;padding:11px 14px;border-radius:10px;
border:.5px solid var(--gray4);font-size:16px;background:var(--card);color:var(--text);outline:none}
input:focus{border-color:var(--blue)}
.raw-box{background:var(--gray6);border-radius:8px;padding:10px;font-family:"SF Mono",Menlo,monospace;
font-size:12px;word-break:break-all;max-height:80px;overflow-y:auto;margin:8px 0;color:var(--text3)}
.ota-grid{display:grid;grid-template-columns:1fr 1fr;gap:8px;margin-bottom:10px}
.ota-stat{background:var(--gray6);border-radius:8px;padding:9px 10px}
.ota-k{display:block;font-size:12px;color:var(--text3);margin-bottom:3px}
.ota-v{font-size:18px;font-weight:700;color:var(--text)}
.ota-guide{background:var(--gray6);border-left:3px solid var(--blue);border-radius:8px;
padding:10px 11px;font-size:13px;color:var(--text3);line-height:1.45;margin:8px 0}
.ota-guide b{color:var(--text);font-weight:700}.ota-list{margin:8px 0 0;padding:0;list-style:none}
.ota-list li{display:flex;gap:7px;margin:5px 0}.ota-list span{font-weight:700;min-width:54px}
.ota-list .yes span{color:var(--green)}.ota-list .no span{color:var(--red)}
.ota-file{border:1px dashed var(--gray4);border-radius:8px;padding:9px 10px;margin:10px 0 8px}
.ota-meta{font-size:12px;color:var(--text3);line-height:1.4;margin-top:4px}
.ota-ok{color:var(--green)}.ota-bad{color:var(--red)}
.ota-wait{color:var(--text3)}
.learn-pulse{animation:lp 1.5s infinite}
@keyframes lp{0%,100%{box-shadow:0 0 0 0 rgba(255,59,48,.25)}50%{box-shadow:0 0 0 10px rgba(255,59,48,0)}}
.pair-pulse{animation:pp 1.2s infinite}
@keyframes pp{0%,100%{box-shadow:0 0 0 0 rgba(0,122,255,.3)}50%{box-shadow:0 0 0 12px rgba(0,122,255,0)}}
.slave-dot{width:8px;height:8px;border-radius:50%;background:#34C759;display:inline-block;margin-right:8px}
.slave-row{display:flex;align-items:center;padding:8px 0;font-size:13px;border-bottom:.5px solid var(--gray6)}
.slave-row:last-child{border-bottom:none}
.slave-mac{flex:1;color:var(--gray)}
.slave-ip{color:var(--gray);font-size:12px;margin-right:8px}
.floor-h{font-size:14px;font-weight:600;color:var(--text2);padding:0 4px 8px;margin-top:4px}
.dev-grid{display:grid;grid-template-columns:repeat(2,1fr);gap:10px;margin-bottom:14px}
.dev-card{background:var(--card);border-radius:14px;padding:14px;display:flex;flex-direction:column;align-items:center;gap:4px;cursor:pointer;transition:transform .08s;box-shadow:0 .5px 2px rgba(0,0,0,.06)}
.dev-card:active{transform:scale(.96)}
.dev-card.master{border:2px solid var(--blue)}
.dev-icon{font-size:32px;line-height:1.2}
.dev-name{font-size:14px;font-weight:600}
.dev-sub{font-size:11px;color:var(--gray)}
.dev-status{font-size:10px;padding:1px 6px;border-radius:4px;font-weight:600}
.dev-online{background:rgba(52,199,89,.15);color:var(--green)}
.dev-offline{background:var(--gray6);color:var(--gray)}
.icon-pick{display:grid;grid-template-columns:repeat(6,1fr);gap:6px;margin:8px 0}
.icon-pick button{font-size:22px;padding:8px 0;border-radius:8px;border:1.5px solid var(--gray5);background:var(--card);cursor:pointer}
.icon-pick button.active{border-color:var(--blue);background:rgba(0,122,255,.08)}
.chip{padding:6px 12px;border-radius:16px;border:1.5px solid var(--gray5);background:var(--card);font-size:13px;font-weight:500;cursor:pointer;transition:all .12s}
.chip.active{background:var(--blue);color:#fff;border-color:var(--blue)}
.chip:active{transform:scale(.95)}
.hist-item{display:flex;align-items:center;padding:10px 16px;gap:10px}
.hist-item+.hist-item{border-top:.5px solid var(--gray6)}
.hist-info{flex:1;min-width:0}
.hist-proto{font-size:13px;font-weight:600}
.hist-bits{font-size:11px;color:var(--gray)}
.hist-raw{font-size:11px;color:var(--text3);white-space:nowrap;overflow:hidden;text-overflow:ellipsis}
.empty{padding:30px 16px;text-align:center;color:var(--gray);font-size:14px}
.badge{display:inline-block;padding:2px 7px;border-radius:6px;font-size:11px;font-weight:600}
.badge-gy{background:var(--gray6);color:var(--gray)}
.modal-bg{position:fixed;inset:0;background:rgba(0,0,0,.4);z-index:200;display:none;align-items:center;justify-content:center}
.modal-bg.show{display:flex}
.modal{background:var(--card);border-radius:16px;padding:24px;width:90%;max-width:320px}
.modal h3{font-size:18px;font-weight:600;margin-bottom:14px;text-align:center}
.modal input{margin-bottom:14px}
.modal .btns{display:flex;gap:10px}
.modal .btns button{flex:1}
@media(min-width:768px){
body{background:#e8e8ed}
.hdr{background:rgba(242,242,247,.88)}
.pg{padding:20px}
.pg-title{font-size:34px;letter-spacing:-.8px}
nav.bar{max-width:var(--max);left:50%;transform:translateX(-50%);border-radius:var(--r) var(--r) 0 0}
}
@media(prefers-color-scheme:dark){
:root{--bg:#000;--card:#1c1c1e;--blue:#0a84ff;--green:#30d158;--red:#ff453a;
--orange:#ff9f0a;--purple:#bf5af2;--teal:#64d2ff;--pink:#ff375f;
--gray:#8e8e93;--gray3:#48484a;--gray4:#3a3a3c;--gray5:#2c2c2e;--gray6:#1c1c1e;
--text:#fff;--text2:#ebebf5;--text3:#ababaf}
body{background:var(--bg)}
.hdr{background:rgba(0,0,0,.92);border-bottom-color:var(--gray5)}
nav.bar{background:rgba(28,28,30,.94);border-top-color:var(--gray4)}
input[type=text]{background:var(--card);color:var(--text)}
select{background:var(--card);color:var(--text)}
.seg{background:var(--gray5)}
.seg button.active{background:var(--gray4);color:var(--text)}
.raw-box{background:var(--gray5);color:var(--text3)}
.temp-b{background:var(--gray5)}
.grid-b{background:var(--card);border-color:var(--gray4)}
.b-gy{background:var(--gray5)}
}
.pg{animation:pgIn .22s cubic-bezier(.4,0,.2,1)}
@keyframes pgIn{from{opacity:0;transform:translateY(6px)}to{opacity:1;transform:none}}
.cd{transition:transform .12s}
.cd:active{transform:scale(.99)}
.empty{text-align:center;padding:36px 16px;color:var(--gray)}
.empty .em-ic{font-size:48px;margin-bottom:10px;opacity:.4}
.empty p{font-size:14px;margin-bottom:14px;line-height:1.5}
.prog{height:6px;background:var(--gray5);border-radius:3px;overflow:hidden;margin:8px 0;display:none}
.prog-bar{height:100%;background:var(--blue);width:0;transition:width .15s;border-radius:3px}
.prog-bar.err{background:var(--red)}
.prog-stat{font-size:13px;color:var(--text3);margin-top:4px;text-align:center}
.theme-btn{position:fixed;top:10px;right:12px;z-index:60;width:36px;height:36px;border-radius:50%;
border:none;background:rgba(0,0,0,.06);font-size:22px;cursor:pointer;line-height:1;padding:0;
display:flex;align-items:center;justify-content:center;transition:transform .12s}
.theme-btn{color:var(--text);background:var(--gray5)}
.theme-btn:active{transform:scale(.9)}

[data-theme=light]{--bg:#f2f2f7;--card:#fff;--blue:#007AFF;--green:#34C759;--red:#FF3B30;
--orange:#FF9500;--purple:#AF52DE;--teal:#5AC8FA;--pink:#FF2D55;
--gray:#8e8e93;--gray3:#c7c7cc;--gray4:#d1d1d6;--gray5:#e5e5ea;--gray6:#f2f2f7;
--text:#1c1c1e;--text2:#3c3c43;--text3:#636366}
[data-theme=dark]{--bg:#000;--card:#1c1c1e;--blue:#0a84ff;--green:#30d158;--red:#ff453a;
--orange:#ff9f0a;--purple:#bf5af2;--teal:#64d2ff;--pink:#ff375f;
--gray:#8e8e93;--gray3:#48484a;--gray4:#3a3a3c;--gray5:#2c2c2e;--gray6:#1c1c1e;
--text:#fff;--text2:#ebebf5;--text3:#ababaf}
body{background:var(--bg);color:var(--text)}
.hdr{background:color-mix(in srgb,var(--card) 88%,transparent);backdrop-filter:blur(20px);-webkit-backdrop-filter:blur(20px)}

/* Shared controls use the same tokens in system and manually selected themes. */
[hidden]{display:none!important}
nav.bar{background:var(--card)}
nav.bar a,.seg button,.b{min-height:44px}
.seg button.active{background:var(--card);color:var(--text)}
.seg{background:var(--gray5)}
label.field-label{display:block;font-size:13px;font-weight:600;color:var(--text2);margin:10px 0 6px}
input[type=text],input[type=password],input[type=number]{min-width:0;min-height:44px;background:var(--card);color:var(--text);border:1px solid var(--gray4)}
button:disabled,input:disabled{opacity:.5;cursor:default}
:focus-visible{outline:2px solid var(--blue);outline-offset:3px}
.password-field{display:flex;align-items:center;position:relative;margin-bottom:10px}
.password-field input{padding-right:66px;margin:0!important}
.password-field button{position:absolute;right:3px;top:2px;bottom:2px;min-width:58px;min-height:40px;border:0;border-radius:8px;background:transparent;color:var(--blue);font-size:13px;cursor:pointer}
.help{font-size:13px;line-height:1.5;color:var(--text3);margin:8px 0}
.subnav{margin:0 0 16px}
.log-view{height:250px;overflow:auto;padding:10px;border:1px solid var(--gray4);border-radius:10px;background:var(--gray6);color:var(--text2);font:11px/1.6 "SF Mono",Menlo,monospace;white-space:pre-wrap;overflow-wrap:anywhere;margin:10px 0}
.log-actions{display:flex;flex-wrap:wrap;gap:8px}.log-actions .b{flex:1;padding:10px 12px;font-size:14px}
.log-status{font-size:13px;color:var(--text3);margin:8px 0;overflow-wrap:anywhere}
.log-scroll{display:flex;align-items:center;gap:8px;min-height:44px;font-size:13px;color:var(--text3)}
.log-scroll input{width:20px;height:20px}
.ota-guide details{margin-top:8px}.ota-guide summary{cursor:pointer;min-height:44px;display:flex;align-items:center;color:var(--blue)}
.dev-name,.dev-sub,.it span{overflow-wrap:anywhere;min-width:0}
@media(prefers-reduced-motion:reduce){*,*::before,*::after{animation:none!important;transition:none!important}}
</style>
</head>
<body>
<div class="hdr"><h1>IR 遥控器</h1></div>
<div id="toast" class="toast"></div>
<div id="rename-modal" class="modal-bg">
<div class="modal">
<h3>重命名</h3>
<label class="field-label" for="rename-input">新名称</label><input type="text" id="rename-input" placeholder="新名称">
<div class="btns">
<button class="b b-gy" onclick="closeRename()">取消</button>
<button class="b b-bl" onclick="doRename()">确定</button>
</div>
</div>
</div>
<div id="pg-devices" class="pg">
<div class="pg-title">设备</div>
<div class="pg-sub">主机负责发指令；从机配对后会跟随主机执行。</div>
<div id="dev-floor-list"></div>
<div style="text-align:center;padding:8px">
<p class="help" id="dev-pair-help">主机开启配对后，60 秒内给另一台设备上电或重启。</p><button hidden class="b b-or b-fw" id="dev-pair-btn" onclick="togglePair()">配对新设备</button>
</div>
</div>

<div id="pg-ac" class="pg active">
<div class="pg-title">空调</div>
<div id="ac-target-wrap" style="display:none;margin-bottom:10px">
<div style="font-size:12px;font-weight:700;color:var(--gray);letter-spacing:.5px;margin-bottom:4px">控制目标</div>
<div id="ac-target-chips" style="display:flex;flex-wrap:wrap;gap:6px"></div>
</div>
<div class="cd"><div class="cd-h">品牌</div><div class="cd-b">
<select id="ac-v">
<optgroup label="常用">
<option value="GREE">格力</option>
<option value="WAHIN">华凌</option>
<option value="MIDEA">美的</option>
</optgroup>
<optgroup label="国产主流">
<option value="GREE112">格力 (112bit)</option>
<option value="MIDEA24">美的 (24bit)</option>
<option value="TECO">TECO</option>
<option value="HAIER_AC">海尔</option>
<option value="HAIER_AC_YRWO2">海尔 (YRW02)</option>
<option value="KELON">科龙</option>
<option value="KELON168">科龙 (168bit)</option>
<option value="TCL112AC">TCL</option>
<option value="TCL96AC">TCL (96bit)</option>
</optgroup>
<optgroup label="日系">
<option value="DAIKIN">大金</option>
<option value="DAIKIN128">大金 (128bit)</option>
<option value="DAIKIN2">大金2</option>
<option value="TOSHIBA_AC">东芝</option>
<option value="MITSUBISHI_AC">三菱</option>
<option value="MITSUBISHI112">三菱 (112bit)</option>
<option value="MITSUBISHI_HEAVY_88">三菱重工 (88bit)</option>
<option value="MITSUBISHI_HEAVY_152">三菱重工 (152bit)</option>
<option value="PANASONIC_AC">松下</option>
<option value="FUJITSU_AC">富士通</option>
<option value="SHARP_AC">夏普</option>
<option value="SANYO_AC">三洋</option>
<option value="HITACHI_AC">日立</option>
</optgroup>
<optgroup label="韩系/其他">
<option value="SAMSUNG_AC">三星</option>
<option value="LG">LG</option>
<option value="LG2">LG2</option>
<option value="ELECTRA_AC">Electra</option>
<option value="WHIRLPOOL_AC">惠而浦</option>
<option value="BOSCH144">博世</option>
<option value="CARRIER_AC64">Carrier</option>
<option value="YORK_AC">York</option>
</optgroup>
<optgroup label="万能">
<option value="COOLIX">Coolix 万能</option>
</optgroup>
</select>
</div></div>
<div class="cd"><div class="cd-h">模式</div><div class="cd-b">
<div class="seg" id="s-mode">
<button class="active" data-v="Cool">&#10052;&#65039; 制冷</button>
<button data-v="Heat">&#128293; 制热</button>
<button data-v="Auto">&#128260; 自动</button>
<button data-v="Dry">&#128167; 除湿</button>
<button data-v="Fan">&#127744; 送风</button>
</div></div></div>
<div class="cd"><div class="cd-h">温度</div><div class="cd-b">
<div class="temp">
<button class="temp-b" onclick="tAdj(-1)">&#8722;</button>
<div><span class="temp-v" id="t-val">26</span><span class="temp-u">&#176;C</span></div>
<button class="temp-b" onclick="tAdj(1)">+</button>
</div></div></div>
<div class="cd"><div class="cd-h">风速</div><div class="cd-b">
<div class="seg" id="s-fan">
<button class="active" data-v="Auto">自动</button>
<button data-v="Low">低</button>
<button data-v="Medium">中</button>
<button data-v="High">高</button>
</div></div></div>
<div style="display:flex;gap:10px">
<button class="b b-bl b-fw" onclick="acCmd('On')">开机</button>
<button class="b b-rd b-fw" onclick="acCmd('Off')">关机</button>
</div>
</div>

<div id="pg-learn" class="pg">
<div class="pg-title">遥控器</div><div class="seg subnav" aria-label="遥控功能"><button class="" onclick="go('remote')">已保存</button><button class="active" onclick="go('learn')">学习信号</button></div>
<div class="pg-sub">
<span class="badge badge-gy" id="learn-st">就绪</span>
</div>
<div class="cd" id="learn-card">
<div class="cd-h">捕获信号</div>
<div class="cd-b">
<div style="display:flex;gap:8px;margin-bottom:12px">
<button class="b b-rd b-fw" id="btn-rec" onclick="toggleRec()">&#9679; 开始学习</button>
</div>
<div id="rec-hint" style="display:none;font-size:14px;color:var(--text3);text-align:center;padding:8px">
&#128308; 正在监听... 把遥控器对着板子按键
</div>
<div id="latest-signal" style="display:none">
<div class="raw-box" id="raw-preview"></div>
<label class="field-label" for="sig-name">遥控按钮名称</label><input type="text" id="sig-name" placeholder="命名，如：开机、音量+">
<div style="display:flex;gap:8px;margin-top:10px">
<button class="b b-gn" style="flex:1" onclick="saveSig()">保存</button>
<button class="b b-ol" style="flex:1" onclick="replayLatest()">&#9654; 重放</button>
</div>
</div>
</div></div>
<div class="cd"><div class="cd-h">捕获历史</div>
<div id="hist-list"><div class="empty">还没有捕获信号</div></div></div>
<button class="b b-gy b-fw" style="font-size:14px" onclick="clearHist()">清除历史</button>
</div>
<div id="pg-remote" class="pg">
<div class="pg-title">遥控器</div><div class="seg subnav" aria-label="遥控功能"><button class="active" onclick="go('remote')">已保存</button><button class="" onclick="go('learn')">学习信号</button></div><div class="pg-sub">点击已保存的按钮发送信号</div>
<div id="remote-devices"><div class="empty">还没有保存的按钮<br>切换「学习信号」捕获并保存按钮</div></div>
</div>
<div id="pg-settings" class="pg"><div class="pg-title">设置</div>
<div class="seg subnav" aria-label="设置分组"><button class="active" data-settings="network" onclick="settingsPanel('network')">网络连接</button><button data-settings="maintenance" onclick="settingsPanel('maintenance')">维护诊断</button></div>
<div id="settings-network"><div class="cd" id="devmode-card" style="display:none"><div class="cd-h">工作模式</div><div class="cd-b">
<div class="seg" id="s-devmode">
<button data-v="auto" class="active">自动</button>
<button data-v="ap">独立主机</button>
<button data-v="slave">加入主机</button>
<button data-v="home">家庭网络</button>
</div>
<div style="font-size:13px;color:var(--text3);margin-top:8px" id="devmode-help">自动会根据配置选择家庭网络、主从组网或独立热点。</div>
<button class="b b-bl b-fw" style="margin-top:10px" onclick="saveForceMode()">保存并重启</button>
</div></div>
<div class="cd"><div class="cd-h">WiFi</div><div class="cd-b">
<div id="wifi-status" style="font-size:14px;color:var(--text3);margin-bottom:10px"></div>
<button class="b b-bl b-fw" style="margin-bottom:10px" onclick="scanWifi()">&#128269; 扫描WiFi</button>
<div id="wifi-list" style="display:none">
<div style="font-size:12px;color:var(--gray3);margin-bottom:6px">扫描列表仅含 2.4G 网络（设备不支持 5G）</div>
<div style="max-height:200px;overflow-y:auto;border:.5px solid var(--gray4);border-radius:10px;margin-bottom:10px" id="wifi-scan-results"></div>
<label class="field-label" for="wifi-ssid">家庭 WiFi 名称（仅支持 2.4G）</label><input type="text" id="wifi-ssid" placeholder="WiFi 名称（SSID）" style="margin-bottom:8px">
<label class="field-label" for="wifi-pass">家庭 WiFi 密码</label><input type="password" autocomplete="off" id="wifi-pass" placeholder="WiFi 密码" style="margin-bottom:10px">
<button class="b b-gn b-fw" onclick="connectWifi()">连接并重启</button>
</div>
<div id="wifi-forget-wrap" style="display:none;margin-top:10px">
<button class="b b-rd b-fw" style="font-size:14px" onclick="forgetWifi()">忘记网络（切回AP模式）</button>
</div>
</div></div>
<div class="cd"><div class="cd-h">热点配置</div><div class="cd-b">
<div style="font-size:13px;color:var(--text3);margin-bottom:8px">
配置 AP 热点名称和密码。从机使用相同配置自动组网，恢复出厂后回到默认值。</div>
<label class="field-label" for="ap-ssid">本设备热点名称</label><input type="text" id="ap-ssid" placeholder="热点名称（默认 IR-AC）" style="margin-bottom:8px">
<label class="field-label" for="ap-pass">本设备热点密码</label><input type="password" autocomplete="off" id="ap-pass" placeholder="热点密码（至少8位，默认 12345678）" style="margin-bottom:10px">
<button class="b b-bl b-fw" onclick="saveApConfig()">保存热点配置并重启</button>
</div></div>
<div class="cd"><div class="cd-h">MQTT</div><div class="cd-b">
<label class="field-label" for="mqtt-host">MQTT 服务器地址</label><input type="text" id="mqtt-host" placeholder="服务器地址（如 192.168.1.100）" style="margin-bottom:8px">
<div>
<label class="field-label" for="mqtt-port">端口</label><input type="number" inputmode="numeric" min="1" max="65535" id="mqtt-port" placeholder="端口" value="1883" style="flex:1">
<label class="field-label" for="mqtt-user">MQTT 用户名（可选）</label><input type="text" id="mqtt-user" placeholder="用户名（可选）" style="flex:1">
</div>
<label class="field-label" for="mqtt-pass">MQTT 密码（可选）</label><input type="password" autocomplete="off" id="mqtt-pass" placeholder="密码（可选）" style="margin-bottom:8px">
<label class="field-label" for="mqtt-topic">Topic 前缀</label><input type="text" id="mqtt-topic" placeholder="Topic 前缀" value="ir_ac" style="margin-bottom:10px">
<button class="b b-pp b-fw" onclick="saveMqtt()">保存MQTT配置并重启</button>
</div></div>
</div><div id="settings-maintenance" hidden><div class="cd"><div class="cd-h">固件更新 (OTA)</div><div class="cd-b">
<div class="ota-grid">
<div class="ota-stat"><span class="ota-k">当前运行</span><span class="ota-v" id="ota-rom-wrap">ROM <span id="ota-rom">-</span></span></div>
<div class="ota-stat"><span class="ota-k">准备写入</span><span class="ota-v" id="ota-target-wrap">ROM <span id="ota-target">-</span></span></div>
</div>
<div class="ota-guide" id="ota-guide">
<div id="ota-guide-main"><b>这里只能上传 Web OTA 应用镜像。</b>设备会自动写入备用分区，上传后重启验证。</div>
<ul class="ota-list">
<li class="yes"><span>可上传</span><div id="ota-allow">Release 里的 firmware-ota-*.bin，或本地 flash_images/ota.bin</div></li>
<li class="no"><span>不要传</span><div>combined.bin、rboot.bin、.pio/build/.../firmware.bin、旧版无 manifest 包</div></li>
</ul><details><summary>分区与镜像说明</summary><div>设备只写入备用 ROM；镜像必须通过板型、DOUT、checksum 与 manifest/CRC 校验。上传进度到 100% 后仍需等待设备校验结果。</div></details>
</div>
<form method="POST" action="/update" enctype="multipart/form-data" id="ota-form">
<div class="ota-file">
<input type="file" name="firmware" id="ota-file" accept=".bin" style="font-size:14px;width:100%" required>
<div class="ota-meta" id="ota-file-meta">未选择文件。选中文件后会先在浏览器里验头。</div>
</div>
<label class="field-label" for="ota-password">本设备热点密码</label>
<input type="password" id="ota-password" autocomplete="current-password" placeholder="请输入本设备热点密码" required style="margin:8px 0">
<div class="ota-meta">用于升级认证，用户名固定为 admin。请填写本设备的热点密码，上传前会先验证。</div>
<button type="submit" class="b b-or b-fw" id="ota-btn" disabled>选择 OTA 固件</button>
</form>
<div class="prog" id="ota-prog"><div class="prog-bar" id="ota-prog-bar"></div></div>
<div class="prog-stat" id="ota-prog-stat"></div>
<div id="ota-msg" class="ota-meta ota-wait"></div>
</div></div><div class="cd" id="logs-card"><div class="cd-h">实时日志</div><div class="cd-b">
<p class="help">仅查看当前设备连接后的应用日志，不保存历史。断开、离开此页或关闭屏幕后停止接收。</p>
<label for="logs-password" class="field-label">本设备热点密码</label>
<input type="password" id="logs-password" autocomplete="off" placeholder="请输入本设备热点密码">
<p class="help">用户名固定为 admin；请使用本设备热点密码，而非家庭 WiFi 密码。</p>
<div class="log-actions"><button class="b b-bl" id="logs-start" onclick="startLogs()">开始查看</button><button class="b b-gy" id="logs-pause" onclick="stopLogs('已暂停；继续查看会从当前时刻开始')" disabled>暂停</button><button class="b b-gy" onclick="clearLogs()">清空视图</button></div>
<div id="logs-status" class="log-status" role="status" aria-live="polite">未连接</div>
<div id="logs-view" class="log-view" role="region" aria-label="实时日志内容" tabindex="0"></div>
<label class="log-scroll"><input type="checkbox" id="logs-autoscroll" checked>自动滚动 · 最多显示 300 行</label>
</div></div>
<div class="cd"><div class="cd-h">浏览器遥控数据</div><div class="help" style="padding:0 16px">仅导入、导出或清除当前浏览器保存的遥控按钮和品牌，不包含设备网络配置。</div>
<div class="it" onclick="exportAll()"><span>导出遥控数据</span><span class="arr"></span></div>
<div class="it" onclick="importAll()"><span>导入遥控数据</span><span class="arr"></span></div>
<div class="it" onclick="clearAll()"><span style="color:var(--red)">清除浏览器遥控数据</span><span class="arr"></span></div>

</div>
<div class="cd"><div class="cd-h">本机按键</div><div class="cd-b">
<div style="font-size:13px;color:var(--text3);line-height:1.45">短按：格力制冷 26°C 一键开/关。长按 5 秒：恢复出厂设置。</div>
</div></div>
<div class="cd"><div class="cd-h">关于</div>
<div class="it"><span>版本</span><span style="color:var(--gray)">v3.0.0</span></div>
<div class="it"><span>MAC</span><span id="sys-mac" style="color:var(--gray)">--</span></div>
<div class="it"><span>设备</span><span style="color:var(--gray)">IR Mini V105</span></div>
<div class="it"><span>协议库</span><span style="color:var(--gray)">IRremoteESP8266</span></div>
</div>
<div class="cd"><div class="it" onclick="factoryReset()"><span style="color:var(--red)">恢复设备出厂设置</span><span class="arr"></span></div></div></div></div>
<nav class="bar">
<a class="active" data-page="ac" aria-current="page" href="#ac" onclick="go('ac');return false"><i>&#10052;&#65039;</i>空调</a>
<a data-page="remote" href="#remote" onclick="go('remote');return false"><i>&#127903;</i>遥控</a>
<a data-page="devices" href="#devices" onclick="go('devices');return false"><i>&#127968;</i>设备</a>
<a data-page="settings" href="#settings" onclick="go('settings');return false"><i>&#9881;</i>设置</a>
</nav>
<script>
var sigs=JSON.parse(localStorage.getItem('ir_sigs')||'[]');
var hist=[],recording=false,latestRaw='',latestProto='',latestBits=0,pollTimer=null;
var renameIdx=-1;
var otaTargetRom='-',otaSupported=false,otaBusy=false,otaValidation=0,otaTargetFile='flash_images/ota.bin';
fetch('/api/ota/status').then(function(r){return r.json()}).then(function(d){
  if(!d.ok||(!Number.isInteger(d.current_rom)&&d.ota_supported!==false))throw new Error('bad_status');
  otaSupported=d.ota_supported!==false;
  if(!otaSupported){
    var rw=$('ota-rom-wrap'),tw=$('ota-target-wrap'),g=$('ota-guide-main'),al=$('ota-allow'),b=$('ota-btn'),inp=$('ota-file');
    if(rw)rw.textContent='Factory';
    if(tw)tw.textContent='不可用';
    if(g)g.innerHTML='<b>当前是直刷 Factory 固件。</b>此版本不写入 rboot 分区，不能在这里 OTA。';
    if(al)al.textContent='Factory 模式无可上传文件；请通过 USB-TTL 刷写完整固件。';
    if(b){b.disabled=true;b.textContent='Factory 版不支持 OTA'}
    if(inp)inp.disabled=true;
    otaMsg('当前固件禁用了 Web OTA。','ota-wait');
    return;
  }
  var el=$('ota-rom');if(el)el.textContent=d.current_rom;
  otaTargetRom=(d.target_rom!==undefined)?d.target_rom:(d.current_rom===0?1:0);
  otaTargetFile='flash_images/ota.bin';
  var tg=$('ota-target');if(tg)tg.textContent=otaTargetRom;
  var al=$('ota-allow');if(al)al.textContent='Release 里的 firmware-ota-*.bin，或本地 '+otaTargetFile+'（rom0/rom1 为兼容别名）';
}).catch(function(){otaSupported=false;otaSetReady(false);otaMsg('无法读取固件分区状态，请刷新页面重试。','ota-bad')});
var otaFileOk=false;
function otaSize(n){return n>1048576?(n/1048576).toFixed(2)+' MB':Math.ceil(n/1024)+' KB'}
function otaMsg(t,cls){var m=$('ota-msg');if(m){m.textContent=t;m.className='ota-meta '+(cls||'ota-wait')}}
function otaMeta(t,cls){var m=$('ota-file-meta');if(m){m.textContent=t;m.className='ota-meta '+(cls||'ota-wait')}}
function otaLe32(a,o){return (a[o]|(a[o+1]<<8)|(a[o+2]<<16)|(a[o+3]<<24))>>>0}
var otaCrcTbl=null;
function otaCrc32(a,z,l){
  if(!otaCrcTbl){otaCrcTbl=[];for(var n=0;n<256;n++){var c=n;for(var k=0;k<8;k++)c=(c&1)?((c>>>1)^0xEDB88320):(c>>>1);otaCrcTbl[n]=c>>>0}}
  var c=0xFFFFFFFF;for(var i=0;i<a.length;i++){var v=(i>=z&&i<z+l)?0:a[i];c=(c>>>8)^otaCrcTbl[(c^v)&255]}return(~c)>>>0;
}
function otaAscii(a,o,l){var s='';for(var i=0;i<l;i++){var c=a[o+i];if(c===0)break;if(c<32||c>126)return null;s+=String.fromCharCode(c)}return s}
function otaRbootHeadAt(a,o){return a.length>=o+16&&a[o]===0xEA&&a[o+1]===0x04&&otaLe32(a,o+8)===0&&otaLe32(a,o+12)>0&&otaLe32(a,o+12)<0xFE000}
function otaRbootAt(a,o){return otaRbootHeadAt(a,o)&&a[o+2]===0x03}
function otaOldAppHeadAt(a,o){return a.length>=o+16&&a[o]===0xE9&&a[o+1]>=3&&a[o+1]<=16&&otaLe32(a,o+8)===0x40202010}
function otaOldAppAt(a,o){return otaOldAppHeadAt(a,o)&&a[o+2]===0x03}
function otaRbootInfo(a,o){
  if(!otaRbootHeadAt(a,o))return{ok:false,err:'bad_head'};
  if(a[o+2]!==0x03)return{ok:false,err:'bad_mode'};
  var entry=otaLe32(a,o+4),ilen=otaLe32(a,o+12),n=o+16+ilen;
  if(n+8>a.length)return{ok:false,err:'no_ram_header'};
  if(a[n]!==0xE9||a[n+2]!==0x03||otaLe32(a,n+4)!==entry)return{ok:false,err:'bad_ram_header'};
  var cnt=a[n+1];if(cnt<1||cnt>16)return{ok:false,err:'bad_section_count'};
  var pos=n+8,cs=0xEF;
  for(var i=0;i<cnt;i++){
    if(pos+8>a.length)return{ok:false,err:'no_section_header'};
    var addr=otaLe32(a,pos),len=otaLe32(a,pos+4);pos+=8;
    if(!len||(addr>=0x40200000&&addr<0x40300000)||pos+len>a.length)return{ok:false,err:'bad_section'};
    for(var j=0;j<len;j++)cs^=a[pos+j];
    pos+=len;
  }
  var cp=pos|15;if(cp>=a.length)return{ok:false,err:'no_checksum'};
  if(a[cp]!==cs)return{ok:false,err:'checksum'};
  if(a.length!==cp+1)return{ok:false,err:'trailing'};
  return{ok:true,irom:ilen,sections:cnt,manifest:16+ilen-64};
}
function otaManifestInfo(a,info){
  if(info.irom<64)return{ok:false,err:'missing_manifest'};
  var o=info.manifest;if(o<16||o+64>a.length)return{ok:false,err:'manifest_offset'};
  if(otaAscii(a,o,8)!=='IRACOTA1')return{ok:false,err:'missing_manifest'};
  if(a[o+8]!==1||a[o+9]!==1)return{ok:false,err:'manifest_version'};
  var target=a[o+10],mode=a[o+11],size=otaLe32(a,o+12),crc=otaLe32(a,o+16);
  var board=otaAscii(a,o+20,20),ver=otaAscii(a,o+40,20);
  if(mode!==0x03)return{ok:false,err:'manifest_mode'};
  if(size!==a.length)return{ok:false,err:'manifest_size'};
  if(board!=='GK01_IR_MINI_V105')return{ok:false,err:'board'};
  var tr=parseInt(otaTargetRom,10);if(target!==255&&!isNaN(tr)&&target!==tr)return{ok:false,err:'target'};
  var actual=otaCrc32(a,o+16,4);if(actual!==crc)return{ok:false,err:'crc'};
  return{ok:true,target:target,version:ver||'dev',crc:crc};
}
function otaSetReady(ok){otaFileOk=ok;var b=$('ota-btn');if(b){b.disabled=!ok||otaBusy;b.textContent=ok?'验证并上传到备用 ROM '+otaTargetRom:'选择 OTA 固件'}}
function otaValidateFile(file,done){
  if(otaBusy)return;
  var validation=++otaValidation;
  otaSetReady(false);
  if(!otaSupported){otaMeta('Factory 版不支持 Web OTA。','ota-bad');otaMsg('请用 USB-TTL 直刷 factory 固件。','ota-bad');done&&done(false);return}
  if(!file){otaMeta('未选择文件。选中文件后会先在浏览器里验头。','ota-wait');otaMsg('请选择 firmware-ota-*.bin 或 '+otaTargetFile,'ota-bad');done&&done(false);return}
  otaMeta('已选择：'+file.name+'（'+otaSize(file.size)+'）','ota-wait');
  if(file.size>0xFE000){otaMsg('文件超过 OTA 分区大小。请选择 firmware-ota-*.bin，不要上传 combined.bin。','ota-bad');done&&done(false);return}
  var rd=new FileReader();
  rd.onerror=function(){if(validation!==otaValidation)return;otaMsg('无法读取文件，请重新选择','ota-bad');done&&done(false)};
  rd.onload=function(){
    if(validation!==otaValidation||otaBusy)return;
    var a=new Uint8Array(rd.result);
    var info=otaRbootInfo(a,0);
    if(info.ok){
      var mf=otaManifestInfo(a,info);
      if(mf.ok){
        otaMeta('校验通过：IRACOTA1，板型匹配，CRC32 正确，版本 '+mf.version+'。','ota-ok');
        otaMsg('可以上传。设备将写入备用 ROM '+otaTargetRom+'，重启后稳定运行约 25 秒才确认升级。','ota-ok');
        otaSetReady(true);done&&done(true);return;
      }
      var em='OTA manifest 校验失败。';
      if(mf.err==='missing_manifest')em='这是旧版 rboot OTA 包，缺少 IRACOTA1 manifest/CRC。请上传 firmware-ota-*.bin。';
      else if(mf.err==='board')em='这个包不是 GK01 / IR Mini V105 固件，已拒绝。';
      else if(mf.err==='target')em='这个包声明的目标 ROM 与本次备用分区不一致，已拒绝。';
      else if(mf.err==='crc')em='OTA 包 CRC32 不匹配，文件可能损坏或不是本次发布产物。';
      else if(mf.err==='manifest_size')em='OTA manifest 中的镜像大小与文件大小不一致。';
      else if(mf.err==='manifest_mode')em='OTA manifest 不是 DOUT flash mode。';
      otaMsg(em,'ota-bad');
      otaMeta('校验失败：不会上传，也不会写入 flash。','ota-bad');
      done&&done(false);return;
    }
    if(otaRbootHeadAt(a,0)&&a[2]!==0x03){
      otaMsg('这个 OTA 镜像不是 DOUT flash mode。本板必须使用 DOUT，请重新构建或运行 prepare_flash.py。','ota-bad');
    }else
    if(otaOldAppAt(a,0)){
      otaMsg('这是旧版剥 eboot 镜像，会导致 rboot 启动异常。请重新运行 prepare_flash.py 后上传 '+otaTargetFile+'。','ota-bad');
    }else if(otaOldAppAt(a,0x1000)){
      otaMsg('这是 PlatformIO firmware.bin，前面带 eboot 头。请运行 prepare_flash.py 后上传 '+otaTargetFile+'。','ota-bad');
    }else if(otaRbootAt(a,0x2000)||otaOldAppAt(a,0x2000)){
      otaMsg('这是 combined.bin，首刷/救砖才写 0x0。OTA 请上传 firmware-ota-*.bin 或 '+otaTargetFile+'。','ota-bad');
    }else if(a.length>=16&&(a[0]===0xE9||a[0]===0xEA)){
      otaMsg('这是 ESP 镜像，但不是本项目 OTA 应用镜像。不要上传 rboot.bin 或其他 bootloader。','ota-bad');
    }else{
      otaMsg('文件格式不匹配；OTA 只接受 prepare_flash.py 生成的 rboot 应用镜像','ota-bad');
    }
    otaMeta('校验失败：不会上传，也不会写入 flash。','ota-bad');
    done&&done(false);
  };
  rd.readAsArrayBuffer(file);
}
(function(){
  var f=$('ota-form'),inp=$('ota-file');if(!f||!inp)return;
  inp.onchange=function(){otaValidateFile(inp.files&&inp.files[0])};
})();
function $(i){return document.getElementById(i)}
function toast(m){var t=$('toast');t.textContent=m;t.classList.add('show');setTimeout(function(){t.classList.remove('show')},1500)}
function esc(v){return String(v==null?'':v).replace(/[&<>"']/g,function(c){return {'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]})}
function jsq(v){return String(v==null?'':v).replace(/\\/g,'\\\\').replace(/'/g,"\\'").replace(/\r/g,' ').replace(/\n/g,' ')}
var currentPage='ac',currentSettings='network';
function resumePagePolls(){
  if(document.hidden||otaBusy)return;
  if(currentPage==='learn')startPoll();
  if(currentPage==='devices')startDevPoll();
}
function suspendPagePolls(){stopPoll();stopDevPoll()}
function go(p){
  if(otaBusy){toast('固件更新中，请等待结果');return}
  if(currentPage!==p)stopLogs('已断开：离开维护诊断');
  suspendPagePolls();currentPage=p;
  document.querySelectorAll('.pg').forEach(function(e){e.classList.remove('active')});
  $('pg-'+p).classList.add('active');
  document.querySelectorAll('nav.bar a').forEach(function(e){var active=e.dataset.page===(p==='learn'?'remote':p);e.classList.toggle('active',active);if(active)e.setAttribute('aria-current','page');else e.removeAttribute('aria-current')});
  if(p==='remote')renderRemote();
  if(p==='ac')loadAcTargets();
  resumePagePolls();
}
function settingsPanel(panel){
  if(otaBusy){toast('固件更新中，请等待结果');return}
  if(currentSettings!==panel)stopLogs('已断开：离开维护诊断');
  currentSettings=panel;
  $('settings-network').hidden=panel!=='network';$('settings-maintenance').hidden=panel!=='maintenance';
  document.querySelectorAll('[data-settings]').forEach(function(b){b.classList.toggle('active',b.dataset.settings===panel)});
}
document.addEventListener('visibilitychange',function(){if(document.hidden){suspendPagePolls();stopLogs('已断开：页面不可见')}else resumePagePolls()});
window.addEventListener('pagehide',function(){suspendPagePolls();stopLogs('已断开：页面已关闭')});
function segInit(id){
var bs=$(id).querySelectorAll('button');
bs.forEach(function(b){b.onclick=function(){bs.forEach(function(x){x.classList.remove('active')});b.classList.add('active');acCmd('On')}});
}
function segVal(id){return $(id).querySelector('.active').dataset.v}
segInit('s-mode');segInit('s-fan');
function tAdj(d){var e=$('t-val'),v=parseInt(e.textContent)+d;if(v<16)v=16;if(v>30)v=30;e.textContent=v;acCmd('On')}
function acCmd(pwr){
var tgt=getSelectedTargets();
var p='vendor='+$('ac-v').value+'&power='+pwr+'&mode='+segVal('s-mode')+
'&temp='+$('t-val').textContent+'&fan='+segVal('s-fan')+'&swing=Off&target='+encodeURIComponent(tgt);
fetch('/api/hvac',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:p})
.then(function(r){return r.json()}).then(function(d){toast(d.ok?'\u2713':'\u2717 失败')}).catch(function(){toast('\u2717 失败')});
}
function getSelectedTargets(){
  var chips=document.querySelectorAll('#ac-target-chips .chip.active');
  if(chips.length===0)return 'ALL';
  var ids=[];chips.forEach(function(c){ids.push(c.dataset.id)});
  return ids.join(',');
}
function setTargetAll(){
  document.querySelectorAll('#ac-target-chips .chip').forEach(function(c){
    c.classList.toggle('active',c.dataset.id==='ALL');
  });
  localStorage.setItem('ir_target','ALL');
}
var sv_=localStorage.getItem('ir_ac_v');if(sv_)$('ac-v').value=sv_;
$('ac-v').onchange=function(){localStorage.setItem('ir_ac_v',this.value)};
fetch('/api/hvac/state').then(function(r){return r.json()}).then(function(d){
  if(d.vendor){$('ac-v').value=d.vendor;localStorage.setItem('ir_ac_v',d.vendor)}
  if(d.temp)$('t-val').textContent=d.temp;
  if(d.mode){document.querySelectorAll('#s-mode button').forEach(function(b){b.classList.toggle('active',b.dataset.v===d.mode)})}
  if(d.fan){document.querySelectorAll('#s-fan button').forEach(function(b){b.classList.toggle('active',b.dataset.v===d.fan)})}
}).catch(function(){});
var captureGeneration=0;
function startPoll(){
  if(pollTimer||currentPage!=='learn'||document.hidden||otaBusy)return;
  $('learn-st').textContent='监听中';var generation=++captureGeneration;
  function capturePoll(){
    if(generation!==captureGeneration||currentPage!=='learn'||document.hidden||otaBusy)return;
    fetch('/api/capture').then(function(r){return r.json()}).then(function(d){
      if(generation===captureGeneration&&d.raw&&currentPage==='learn'&&!document.hidden&&!otaBusy)onIR(d);
    }).catch(function(){}).finally(function(){if(generation===captureGeneration&&currentPage==='learn'&&!document.hidden&&!otaBusy)pollTimer=setTimeout(capturePoll,500)});
  }
  pollTimer=setTimeout(capturePoll,500);
}
function stopPoll(){
  ++captureGeneration;clearTimeout(pollTimer);pollTimer=null;
  $('learn-st').textContent='已停止';
}
function onIR(d){
var raw=d.raw,proto=d.proto||'?',bits=d.bits||0;
if(!raw||raw===latestRaw)return;
latestRaw=raw;latestProto=proto;latestBits=bits;
hist.unshift({proto:proto,bits:bits,raw:raw,time:Date.now()});
if(hist.length>30)hist.length=30;
renderHist();
if(!recording)return;
$('latest-signal').style.display='block';
$('raw-preview').textContent=raw;
$('sig-name').value='';$('sig-name').focus();
}
function toggleRec(){
recording=!recording;
var b=$('btn-rec'),h=$('rec-hint'),ls=$('latest-signal');
if(recording){
b.textContent='\u25A0 停止';b.className='b b-gy b-fw';
h.style.display='block';$('learn-card').classList.add('learn-pulse');
startPoll();
}else{
b.textContent='\u25CF 开始学习';b.className='b b-rd b-fw';
h.style.display='none';ls.style.display='none';
$('learn-card').classList.remove('learn-pulse');
}
}
function saveSig(){
var name=$('sig-name').value.trim();
if(!name){toast('请命名');return}
if(!latestRaw){toast('没有信号');return}
sigs.push({name:name,raw:latestRaw,proto:latestProto,bits:latestBits});
localStorage.setItem('ir_sigs',JSON.stringify(sigs));
$('latest-signal').style.display='none';latestRaw='';
toast('\u2713 已保存：'+name);
}
function sendRaw(raw){
fetch('/api/send',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'raw='+encodeURIComponent(raw)})
.then(function(r){return r.json()}).then(function(d){toast(d.ok?'\u2713 已发送':'\u2717 失败')})
.catch(function(){toast('\u2717 失败')});
}
function replayLatest(){if(latestRaw)sendRaw(latestRaw)}
function renderHist(){
var el=$('hist-list');
if(!hist.length){el.innerHTML='<div class="empty">还没有捕获信号</div>';return}
var h='';
hist.forEach(function(s){
 h+='<div class="hist-item"><div class="hist-info"><div class="hist-proto">'+esc(s.proto)+
 ' <span class="badge badge-gy">'+esc(s.bits)+'b</span></div><div class="hist-raw">'+esc(s.raw.substring(0,50))+
 '</div></div><button class="b b-ol" style="padding:6px 12px;font-size:13px" onclick="sendRaw(\''+
 jsq(s.raw)+'\')">&#9654;</button></div>';
});
el.innerHTML=h;
}
function clearHist(){hist=[];renderHist()}
function renderRemote(){
var el=$('remote-devices');
if(!sigs.length){el.innerHTML='<div class="empty">还没有保存的按钮<br>切换「学习信号」捕获并保存按钮</div>';return}
var h='<div class="grid">';
sigs.forEach(function(s,i){
h+='<button class="grid-b" onclick="sendRaw(\''+
 jsq(s.raw)+'\')"><span class="ic">'+esc(s.icon||'\u26AB')+
 '</span>'+esc(s.name)+'</button>';
});
h+='</div>';
h+='<div style="margin-top:14px;display:flex;gap:8px">';
h+='<button class="b b-ol b-fw" style="font-size:14px" onclick="editMode()">&#9998;&#65039; 编辑</button></div>';
el.innerHTML=h;
}
var editing=false;
function editMode(){
editing=!editing;if(!editing){renderRemote();return}
var el=$('remote-devices'),h='<div class="grid">';
sigs.forEach(function(s,i){
h+='<div class="grid-b" onclick="editBtn('+i+')" style="cursor:pointer">'+
'<span class="ic" style="color:var(--blue)">&#9998;</span>'+esc(s.name)+'</div>';
});
h+='</div>';
h+='<div style="margin-top:10px;display:flex;gap:8px">';
sigs.forEach(function(s,i){
h+='<button class="b b-rd" style="flex:1;font-size:13px;padding:8px" onclick="delSig('+i+')">&#10005; '+esc(s.name)+'</button>';
if((i+1)%3===0)h+='</div><div style="margin-top:6px;display:flex;gap:8px">';
});
h+='</div>';
h+='<div style="margin-top:12px"><button class="b b-bl b-fw" onclick="editMode()">完成</button></div>';
el.innerHTML=h;
}
function editBtn(i){
renameIdx=i;
$('rename-input').value=sigs[i].name;
$('rename-modal').classList.add('show');
$('rename-input').focus();
}
function closeRename(){$('rename-modal').classList.remove('show');renameIdx=-1}
function doRename(){
if(renameIdx<0)return;
var n=$('rename-input').value.trim();
if(!n){toast('请输入名称');return}
sigs[renameIdx].name=n;
localStorage.setItem('ir_sigs',JSON.stringify(sigs));
closeRename();editMode();toast('\u2713 已重命名');
}
function delSig(i){
sigs.splice(i,1);localStorage.setItem('ir_sigs',JSON.stringify(sigs));editMode();
}
function exportAll(){
var d={sigs:sigs,ac:$('ac-v').value};
var b=new Blob([JSON.stringify(d,null,2)],{type:'application/json'});
var a=document.createElement('a');a.href=URL.createObjectURL(b);a.download='ir-remote.json';a.click();
}
function importAll(){
var inp=document.createElement('input');inp.type='file';inp.accept='.json';
inp.onchange=function(e){
var f=e.target.files[0];if(!f)return;
var r=new FileReader();r.onload=function(ev){
try{
var d=JSON.parse(ev.target.result);
if(d.sigs){sigs=d.sigs;localStorage.setItem('ir_sigs',JSON.stringify(sigs))}
if(d.ac){$('ac-v').value=d.ac;localStorage.setItem('ir_ac_v',d.ac)}
toast('\u2713 导入成功');renderRemote();
}catch(e){toast('\u2717 格式错误')}
};r.readAsText(f);
};inp.click();
}
function clearAll(){
if(!confirm('确定清除当前浏览器保存的遥控按钮和品牌？设备网络配置不会被清除。'))return;
localStorage.removeItem('ir_sigs');localStorage.removeItem('ir_ac_v');
sigs=[];renderRemote();toast('已清除');
}
renderRemote();
var STA_ERR={
  no_ap:'未找到该WiFi（请确认名称是路由器 2.4G 的 SSID，本设备不支持 5G）',
  auth_fail:'密码错误或被路由器拒绝（建议路由器加密方式设为 WPA2-PSK）',
  timeout:'连接超时（信号弱或路由器响应慢，可靠近路由器重试）',
  'save_failed':'配置写入或提交失败，请检查串口中的 [CFG]/[FS] 日志'
};
function staErrText(e){
  if(!e)return '';
  var code=e.split(':')[0];
  return STA_ERR[code]||e;
}
function loadWifiStatus(){
  fetch('/api/wifi/status').then(function(r){return r.json()}).then(function(d){
    var s=$('wifi-status');
    var modeTxt=d.mode==='sta'?'STA 模式':d.mode==='slave'?'从机模式':'AP 模式';
    var h='<b>'+modeTxt+'</b> &nbsp; SSID: '+esc(d.ssid)+' &nbsp; IP: '+esc(d.ip)+(d.rssi?' &nbsp; RSSI: '+esc(d.rssi):'');
    if(d.flash_real_kb){h+='<br><span style="color:var(--gray);font-size:12px">Flash: 真实 '+esc(d.flash_real_kb/1024)+'MB / 配置 '+esc(d.flash_size_kb/1024)+'MB</span>'}
    if(d.fs==='fail'){h+='<br><span style="color:var(--red)">文件系统异常：配置无法保存'+(d.flash_real_kb<4096?'（芯片仅 '+esc(d.flash_real_kb/1024)+'MB，固件按 4MB 布局）':'')+'</span>'}
    else if(d.mode==='ap'&&d.sta_ssid){h+='<br><span style="color:var(--gray)">已保存网络: '+esc(d.sta_ssid)+'</span>'}
    if(d.mode!=='sta'&&d.sta_error){h+='<br><span style="color:var(--red)">上次连接失败: '+esc(staErrText(d.sta_error))+'</span>'}
    s.innerHTML=h;
    if(d.mode==='sta'){$('wifi-forget-wrap').style.display='block'}
    if(d.mqtt){h+='<br><span style="color:var(--green)">MQTT: 已连接 ('+esc(d.mqtt_host)+')</span>';s.innerHTML=h}
    else if(d.mqtt_host){h+='<br><span style="color:var(--gray)">MQTT: 未连接</span>';s.innerHTML=h}
  }).catch(function(){});
}
loadWifiStatus();
function scanWifi(){
  var el=$('wifi-scan-results');el.innerHTML='<div style="padding:12px;text-align:center;color:var(--gray)">扫描中...</div>';
  $('wifi-list').style.display='block';
  fetch('/api/wifi/scan').then(function(r){return r.json()}).then(function(list){
    if(!list.length){el.innerHTML='<div style="padding:12px;text-align:center;color:var(--gray)">没有发现WiFi</div>';return}
    var h='';
    list.sort(function(a,b){return b.rssi-a.rssi});
    list.forEach(function(w){
      var q=w.rssi>-50?'●●●':w.rssi>-70?'●●○':'●○○';
      h+='<div class="it" onclick="pickWifi(\''+jsq(w.ssid)+'\')">'
        +'<span>'+esc(w.ssid)+' <span style="color:var(--gray);font-size:12px">'+q+' '+esc(w.rssi)+'dBm</span></span>'
        +(w.enc?'<span style="color:var(--gray);font-size:12px">&#128274;</span>':'')
        +'</div>';
    });
    el.innerHTML=h;
  }).catch(function(){el.innerHTML='<div style="padding:12px;text-align:center;color:var(--red)">扫描失败</div>'});
}
function pickWifi(ssid){$('wifi-ssid').value=ssid;$('wifi-pass').focus()}
function connectWifi(){
  var ssid=$('wifi-ssid').value.trim();if(!ssid){toast('请输入WiFi名称');return}
  toast('正在保存...');
  fetch('/api/wifi/connect',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},
    body:'ssid='+encodeURIComponent(ssid)+'&pass='+encodeURIComponent($('wifi-pass').value)})
  .then(function(r){return r.json()}).then(function(d){
    if(!d.ok){toast('保存失败: '+staErrText(d.error));return}
    toast('已保存，重启连接中（约1分钟）。若失败会回到此热点并显示原因');
  });
}
function forgetWifi(){
  if(!confirm('确定忘记网络？设备将重启为AP模式'))return;
  fetch('/api/wifi/forget',{method:'POST'}).then(function(){toast('重启中...')});
}
function loadMqttConfig(){
  fetch('/api/mqtt/config').then(function(r){return r.json()}).then(function(d){
    if(d.host)$('mqtt-host').value=d.host;
    if(d.port)$('mqtt-port').value=d.port;
    if(d.user)$('mqtt-user').value=d.user;
    if(d.topic)$('mqtt-topic').value=d.topic;
  }).catch(function(){});
}
loadMqttConfig();
function saveMqtt(){
  var host=$('mqtt-host').value.trim();if(!host){toast('请输入MQTT服务器地址');return}
  toast('保存中...');
  fetch('/api/mqtt/config',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},
    body:'host='+encodeURIComponent(host)+'&port='+$('mqtt-port').value
    +'&user='+encodeURIComponent($('mqtt-user').value)
    +'&pass='+encodeURIComponent($('mqtt-pass').value)
    +'&topic='+encodeURIComponent($('mqtt-topic').value)})
  .then(function(r){return r.json()}).then(function(d){toast(d.ok?'重启中...':'失败')});
}
function loadApConfig(){
  fetch('/api/ap/config').then(function(r){return r.json()}).then(function(d){
    if(d.ssid)$('ap-ssid').value=d.ssid;
    if(d.pass)$('ap-pass').value=d.pass;
  }).catch(function(){});
}
loadApConfig();
function saveApConfig(){
  var ssid=$('ap-ssid').value.trim();if(!ssid){toast('请输入热点名称');return}
  if(ssid.length>32){toast('名称不能超过32个字符');return}
  var pass=$('ap-pass').value;
  if(pass.length>0&&pass.length<8){toast('密码至少8位');return}
  toast('保存中...');
  fetch('/api/ap/config',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},
    body:'ssid='+encodeURIComponent(ssid)+'&pass='+encodeURIComponent(pass)})
  .then(function(r){return r.json()}).then(function(d){toast(d.ok?'重启中，请等待30秒...':'失败: '+(d.error||''))});
}
function factoryReset(){
  if(!confirm('确定恢复出厂设置？\n\n所有配置将被清除（WiFi、MQTT、热点配置）。\n设备将以默认 IR-AC / 12345678 启动。'))return;
  toast('恢复出厂中...');
  fetch('/api/factory/reset',{method:'POST'}).then(function(r){return r.json()}).then(function(d){toast(d.ok?'重启中...':'失败')}).catch(function(){toast('失败')});
}
function loadSysInfo(){
  fetch('/api/system/info').then(function(r){return r.json()}).then(function(d){
    if(d.mac)$('sys-mac').textContent=d.mac;
    var seg=$('s-devmode');if(seg){
      seg.querySelectorAll('button').forEach(function(b){
        var fm=d.forceMode||0;
        b.classList.toggle('active',(b.dataset.v==='auto'&&fm===0)||(b.dataset.v==='ap'&&fm===1)||(b.dataset.v==='slave'&&fm===2)||(b.dataset.v==='home'&&fm===3));
      });
    }
    updateDevModeHelp();
    $('devmode-card').style.display='';
  }).catch(function(){});
}
loadSysInfo();
function updateDevModeHelp(){
  var seg=$('s-devmode'),h=$('devmode-help');if(!seg||!h)return;
  var act=seg.querySelector('.active'),v=act?act.dataset.v:'auto';
  var map={
    auto:'自动会根据配置选择家庭网络、主从组网或独立热点。',
    ap:'独立主机会开启 IR-AC 热点，并负责发现和控制从机。',
    slave:'加入主机会连接同名 IR-AC 热点，只接收主机下发的红外指令。',
    home:'家庭网络会连接已保存的路由器 WiFi，用于 MQTT / Home Assistant。'
  };
  h.textContent=map[v]||map.auto;
}
function initDevModeSeg(){
  var seg=$('s-devmode');if(!seg)return;
  var bs=seg.querySelectorAll('button');
  bs.forEach(function(b){b.onclick=function(){bs.forEach(function(x){x.classList.remove('active')});b.classList.add('active');updateDevModeHelp()}});
}
initDevModeSeg();
function saveForceMode(){
  var seg=$('s-devmode');var v='auto';
  if(seg){var act=seg.querySelector('.active');if(act)v=act.dataset.v}
  toast('保存中...');
  fetch('/api/mode/force',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'mode='+v})
  .then(function(r){return r.json()}).then(function(d){
    if(d.ok)toast('重启中...');
    else if(d.error==='missing_sta_config')toast('请先配置家庭 WiFi');
    else toast('失败');
  }).catch(function(){toast('失败')});
}
function togglePair(){
  fetch('/api/slaves').then(function(r){return r.json()}).then(function(d){
    if(d.pairing){fetch('/api/pair/stop',{method:'POST'}).then(function(){toast('配对已停止');loadDevices()})}
    else{toast('配对已开启，给另一台设备上电或重启');fetch('/api/pair/start',{method:'POST'}).then(function(){loadDevices()})}
  });
}
var devPollTimer=null,devGeneration=0,devInFlight=false;
var ICONS={'ac':'❄️','bed':'🛏️','book':'📚','tv':'📺','light':'💡','kitchen':'🍳','door':'🚪','star':'⭐'};
function devPageActive(generation){return generation===devGeneration&&currentPage==='devices'&&!document.hidden&&!otaBusy}
function startDevPoll(){if(devPollTimer||currentPage!=='devices'||document.hidden||otaBusy)return;loadDevices()}
function stopDevPoll(){++devGeneration;clearTimeout(devPollTimer);devPollTimer=null;devInFlight=false}
function loadDevices(){
  if(currentPage!=='devices'||document.hidden||otaBusy||devInFlight)return;
  clearTimeout(devPollTimer);devPollTimer=null;devInFlight=true;var generation=devGeneration;
  return fetch('/api/slaves').then(function(r){return r.json()}).then(function(d){
    if(!devPageActive(generation))return;
    var el=$('dev-floor-list');if(!el)return;
    var floors={};
    d.slaves.forEach(function(s){
      var fl=s.floor||'未分组';
      if(!floors[fl])floors[fl]=[];
      floors[fl].push(s);
    });
    var h='';
    return fetch('/api/system/info').then(function(r2){return r2.json()}).then(function(info){
      if(!devPageActive(generation))return;
      h+='<div class="floor-h">'+(info.mode==='ap'?'本机（主机）':'当前设备')+'</div>';
      h+='<div class="dev-grid"><div class="dev-card master"><div class="dev-icon">'+(ICONS[info.deviceIcon||'ac']||'📡')+'</div>';
      h+='<div class="dev-name">'+esc(info.deviceName||'本机')+'</div>';
      h+='<div class="dev-sub">'+esc(info.apMac||info.mac||'')+'</div>';
      var role=info.mode==='ap'?'主机':info.mode==='slave'?'从机':'家庭网络';
      h+='<div class="dev-status dev-online">'+role+'</div></div></div>';
      $('dev-pair-btn').hidden=info.mode!=='ap';
      $('dev-pair-help').textContent=info.mode==='ap'?'开启配对后，60 秒内给另一台设备上电或重启。':'配对仅适用于独立主机模式。';
      Object.keys(floors).forEach(function(fl){
        h+='<div class="floor-h">'+esc(fl)+'</div><div class="dev-grid">';
        floors[fl].forEach(function(s){
          var ic=ICONS[s.icon]||'📦';
          var on=(s.ago||0)<30000;
          h+='<div class="dev-card" onclick="editDevice(\''+jsq(s.id)+'\')">';
          h+='<div class="dev-icon">'+ic+'</div>';
          h+='<div class="dev-name">'+esc(s.name||s.id)+'</div>';
          h+='<div class="dev-sub">'+esc(s.ip)+'</div>';
          h+='<div class="dev-status '+(on?'dev-online':'dev-offline')+'">'+(on?'在线':'离线')+'</div>';
          h+='</div>';
        });
        h+='</div>';
      });
      el.innerHTML=h;
      var pb=$('dev-pair-btn');
      if(pb){if(d.pairing){pb.textContent='配对中 ('+d.pairingLeft+'s)...';pb.classList.add('pair-pulse')}else{pb.textContent='配对新设备';pb.classList.remove('pair-pulse')}}
    });
  }).catch(function(){}).finally(function(){if(generation===devGeneration){devInFlight=false;if(devPageActive(generation))devPollTimer=setTimeout(loadDevices,3000)}});
}
function loadAcTargets(){
  fetch('/api/slaves').then(function(r){return r.json()}).then(function(d){
    var wrap=$('ac-target-chips');if(!wrap)return;
    var saved=localStorage.getItem('ir_target')||'ALL';
    var h='<button class="chip '+(saved==='ALL'?'active':'')+'" data-id="ALL" onclick="toggleChip(this)">所有设备</button>';
    d.slaves.forEach(function(s){
      var nm=s.name||s.id;
      var sel=saved!=='ALL'&&saved.indexOf(s.id)>=0?'active':'';
      h+='<button class="chip '+sel+'" data-id="'+esc(s.id)+'" onclick="toggleChip(this)">'+(ICONS[s.icon]||'📦')+' '+esc(nm)+'</button>';
    });
    wrap.innerHTML=h;
    $('ac-target-wrap').style.display=d.slaves.length>0?'':'none';
  }).catch(function(){});
}
function toggleChip(el){
  if(el.dataset.id==='ALL'){
    document.querySelectorAll('#ac-target-chips .chip').forEach(function(c){c.classList.toggle('active',c===el)});
  }else{
    el.classList.toggle('active');
    var allChip=document.querySelector('#ac-target-chips .chip[data-id="ALL"]');
    if(allChip&&el.classList.contains('active'))allChip.classList.remove('active');
    var anySel=document.querySelectorAll('#ac-target-chips .chip.active').length;
    if(anySel===0&&allChip)allChip.classList.add('active');
  }
  localStorage.setItem('ir_target',getSelectedTargets());
}
var st=localStorage.getItem('ir_target');
var editDevId='';
function editDevice(id){
  editDevId=id;
  fetch('/api/slaves').then(function(r){return r.json()}).then(function(d){
    var dev=d.slaves.find(function(s){return s.id===id});if(!dev)return;
    var icons=Object.keys(ICONS);
    var icHtml=icons.map(function(k){return '<button data-ic="'+k+'" onclick="pickIcon(\''+k+'\')" class="'+(dev.icon===k?'active':'')+'">'+ICONS[k]+'</button>'}).join('');
    $('rename-modal').innerHTML='<div class="modal"><h3>编辑设备</h3>'+
    '<div style="font-size:13px;color:var(--gray);margin-bottom:4px">'+esc(dev.mac)+'<br>'+esc(dev.ip)+'</div>'+
    '<div class="icon-pick" id="icon-pick">'+icHtml+'</div>'+
    '<input type="text" id="dev-name-input" placeholder="设备名称" value="'+esc(dev.name||'')+'">'+
    '<input type="text" id="dev-floor-input" placeholder="楼层/分组（如 3楼）" value="'+esc(dev.floor||'')+'">'+
    '<div style="display:flex;gap:8px;margin-top:10px">'+
    '<button class="b b-bl" style="flex:1" onclick="saveDevConfig()">保存</button>'+
    '<button class="b b-gy" onclick="closeRename()">取消</button></div>'+
    '<div style="margin-top:10px;border-top:.5px solid var(--gray6);padding-top:10px">'+
    '<button class="b b-or b-fw" style="font-size:14px" onclick="rebootDev()">远程重启此设备</button>'+
    '<button class="b b-rd b-fw" style="font-size:14px;margin-top:6px" onclick="disconnectDev()">断开此设备</button>'+
    '</div></div>';
    $('rename-modal').classList.add('show');
  });
}
function pickIcon(ic){
  document.querySelectorAll('#icon-pick button').forEach(function(b){b.classList.toggle('active',b.dataset.ic===ic)});
}
function saveDevConfig(){
  var nm=$('dev-name-input').value.trim();
  var fl=$('dev-floor-input').value.trim();
  var ic='';document.querySelectorAll('#icon-pick button.active').forEach(function(b){ic=b.dataset.ic});
  if(!ic)ic='star';
  var body='id='+editDevId+'&cmd=name&val='+encodeURIComponent(nm);
  fetch('/api/slave/config',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:body}).then(function(){
    return fetch('/api/slave/config',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'id='+editDevId+'&cmd=icon&val='+ic});
  }).then(function(){
    return fetch('/api/slave/config',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'id='+editDevId+'&cmd=floor&val='+encodeURIComponent(fl)});
  }).then(function(){
    toast('已发送配置');closeRename();loadDevices();
  }).catch(function(){toast('失败')});
}
function rebootDev(){
  if(!confirm('确定远程重启此设备？'))return;
  fetch('/api/slave/config',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'id='+editDevId+'&cmd=reboot'})
  .then(function(r){return r.json()}).then(function(d){toast(d.ok?'重启指令已发送':'失败')});
}
function disconnectDev(){
  if(!confirm('确定断开此设备？它将切换为独立主机模式。'))return;
  fetch('/api/slave/config',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'id='+editDevId+'&cmd=disconnect'})
  .then(function(r){return r.json()}).then(function(d){toast(d.ok?'断开指令已发送':'失败');closeRename()});
}
(function(){
  var saved=localStorage.getItem('ir_theme');
  if(saved)document.documentElement.setAttribute('data-theme',saved);
  var b=document.createElement('button');
  b.className='theme-btn';b.innerHTML='\u25D1';b.title='\u5207\u6362\u4e3b\u9898';
  b.onclick=function(){
    var cur=document.documentElement.getAttribute('data-theme');
    var dark=cur==='dark'||(!cur&&matchMedia('(prefers-color-scheme:dark)').matches);
    var next=dark?'light':'dark';
    document.documentElement.setAttribute('data-theme',next);
    localStorage.setItem('ir_theme',next);
  };
  document.body.appendChild(b);
})();
// Live logs exist only in memory for this visible maintenance view.
var logsSession=0,logsCursor=0,logsAuth='',logsGeneration=0,logsTimer=null,logsController=null,logsConnecting=false,logsFailures=0;
function logsStatus(message){$('logs-status').textContent=message}
function logsRelease(session,authorization){
  if(!session||!authorization)return;
  fetch('/api/logs/stop',{method:'POST',cache:'no-store',keepalive:true,headers:{Authorization:authorization,'Content-Type':'application/x-www-form-urlencoded'},body:'session='+session}).catch(function(){});
}
function stopLogs(message){
  var session=logsSession,authorization=logsAuth;
  ++logsGeneration;clearTimeout(logsTimer);logsTimer=null;
  if(logsController)logsController.abort();logsController=null;
  logsSession=0;logsAuth='';logsConnecting=false;logsFailures=0;
  $('logs-password').value='';$('logs-password').type='password';
  var toggle=$('logs-password-toggle');if(toggle){toggle.textContent='显示';toggle.setAttribute('aria-pressed','false')}
  $('logs-start').disabled=false;$('logs-start').textContent='开始查看';$('logs-pause').disabled=true;
  if(message)logsStatus(message);
  logsRelease(session,authorization);
}
function clearLogs(){$('logs-view').textContent=''}
function appendLog(text){
  var view=$('logs-view'),line=document.createElement('div');line.textContent=text;view.appendChild(line);
  while(view.children.length>300)view.removeChild(view.firstChild);
  if($('logs-autoscroll').checked)view.scrollTop=view.scrollHeight;
}
function logsFetch(url,options,generation){
  var controller=new AbortController(),timedOut=false;logsController=controller;
  var timeout=setTimeout(function(){timedOut=true;controller.abort()},3500);
  options=options||{};options.signal=controller.signal;options.cache='no-store';
  return fetch(url,options).then(function(r){
    if(!r.ok){var e=new Error('http');e.status=r.status;throw e}
    return r.json();
  }).catch(function(e){if(timedOut)e.logTimeout=true;throw e}).finally(function(){clearTimeout(timeout);if(logsController===controller)logsController=null});
}
function logsError(error,generation){
  if(generation!==logsGeneration)return;
  var message=error.status===401||error.status===403?'认证失败：请输入本设备当前的热点密码':error.status===409?'会话失效：已超时、设备重启或被其他页面替换，请重新开始':error.status===503?'设备忙：固件更新期间停止日志，请稍后重新开始':error.logTimeout?'请求超时，请检查设备连接后重试':'连接失败，请检查设备连接后重试';
  stopLogs(message);
}
function pollLogs(generation){
  if(generation!==logsGeneration||!logsSession||document.hidden||otaBusy)return;
  logsFetch('/api/logs?session='+logsSession+'&after='+logsCursor,{headers:{Authorization:logsAuth}},generation).then(function(d){
    if(generation!==logsGeneration)return;
    if(!d.ok||!Array.isArray(d.entries)||!Number.isInteger(d.next))throw new Error('bad_response');
    if(d.dropped)appendLog('[提示] 设备缓冲已满，丢失 '+d.dropped+' 条日志');
    d.entries.forEach(function(entry){if(Number.isInteger(entry.seq)&&((entry.seq-logsCursor)|0)>0){appendLog('['+(entry.uptime/1000).toFixed(3)+'s] '+String(entry.text));logsCursor=entry.seq}});
    logsCursor=d.next;logsFailures=0;logsStatus('接收中 · 仅当前连接期间的日志');
    logsTimer=setTimeout(function(){pollLogs(generation)},500);
  }).catch(function(error){
    if(generation!==logsGeneration)return;
    if(error.status||++logsFailures>=3){logsError(error,generation);return}
    logsStatus((error.logTimeout?'请求超时':'网络中断')+'，正在重试（'+logsFailures+'/3）');
    logsTimer=setTimeout(function(){pollLogs(generation)},Math.min(2000,500*Math.pow(2,logsFailures-1)));
  });
}
function startLogs(){
  if(logsConnecting||logsSession)return;
  if(currentPage!=='settings'||currentSettings!=='maintenance'||document.hidden||otaBusy)return;
  var input=$('logs-password');if(!input.value){logsStatus('请输入本设备热点密码');input.focus();return}
  logsAuth='Basic '+btoa(unescape(encodeURIComponent('admin:'+input.value)));input.value='';
  var generation=++logsGeneration;logsConnecting=true;logsFailures=0;
  $('logs-start').disabled=true;$('logs-pause').disabled=false;logsStatus('连接中...');
  logsFetch('/api/logs/start',{method:'POST',headers:{Authorization:logsAuth}},generation).then(function(d){
    if(generation!==logsGeneration)return;
    if(!d.ok||!Number.isInteger(d.session)||!d.session||!Number.isInteger(d.cursor))throw new Error('bad_response');
    logsSession=d.session;logsCursor=d.cursor;logsConnecting=false;logsStatus('接收中 · 从当前时刻开始');pollLogs(generation);
  }).catch(function(error){logsError(error,generation)});
}
(function(){
  document.querySelectorAll('input[type=password]').forEach(function(input){
    var wrap=document.createElement('div');wrap.className='password-field';input.parentNode.insertBefore(wrap,input);wrap.appendChild(input);
    var button=document.createElement('button');button.type='button';button.id=input.id+'-toggle';button.textContent='显示';button.setAttribute('aria-controls',input.id);button.setAttribute('aria-pressed','false');button.setAttribute('aria-label','显示或隐藏'+(document.querySelector('label[for="'+input.id+'"]').textContent));
    button.onclick=function(){var show=input.type==='password';input.type=show?'text':'password';button.textContent=show?'隐藏':'显示';button.setAttribute('aria-pressed',String(show))};wrap.appendChild(button);
  });
})();

(function(){
  var f=document.getElementById('ota-form');if(!f)return;
  f.addEventListener('submit',function(e){
    e.preventDefault();
    if(otaBusy)return;
    var fi=document.getElementById('ota-file');
    if(!fi||!fi.files[0])return;
    if(!otaSupported||!otaFileOk){otaValidateFile(fi.files[0]);return}
    var password=document.getElementById('ota-password');
    if(!password.value){password.focus();return}
    var authorization='Basic '+btoa(unescape(encodeURIComponent('admin:'+password.value)));
    var file=fi.files[0];
    otaBusy=true;++otaValidation;fi.disabled=true;password.disabled=true;
    suspendPagePolls();stopLogs('OTA 暂停：固件更新期间停止日志');
    function finishOta(){authorization='';password.value='';otaBusy=false;fi.disabled=false;password.disabled=false;btn.disabled=!otaFileOk;btn.textContent='验证并上传固件';resumePagePolls()}
    var prog=document.getElementById('ota-prog'),bar=document.getElementById('ota-prog-bar');
    var stat=document.getElementById('ota-prog-stat'),btn=document.getElementById('ota-btn');
    btn.disabled=true;btn.textContent='验证密码中...';
    prog.style.display='block';bar.style.width='0';bar.className='prog-bar';stat.textContent='正在验证设备热点密码...';
    var xhr=new XMLHttpRequest();
    xhr.timeout=120000;
    xhr.upload.onprogress=function(ev){if(ev.lengthComputable){
      var pct=Math.round(ev.loaded/ev.total*100);
      bar.style.width=pct+'%';stat.textContent='\u4e0a\u4f20\u4e2d '+pct+'%';
    }};
    xhr.upload.onload=function(){stat.textContent='上传已发送，设备正在校验，请稍候...'};
    xhr.onload=function(){
      if(xhr.status===200){
        stat.textContent='\u4e0a\u4f20\u5b8c\u6210\uff0c\u8bbe\u5907\u91cd\u542f\u4e2d...';
        authorization='';bar.style.width='100%';setTimeout(function(){location.reload()},10000);
      }else{
        bar.className='prog-bar err';
        stat.textContent=xhr.status===401||xhr.status===403?'认证失败，请检查设备热点密码后重试':'失败: '+(xhr.responseText||'未知错误').substring(0,80);
        finishOta();
      }
    };
    xhr.onerror=function(){
      bar.className='prog-bar err';stat.textContent='\u7f51\u7edc\u9519\u8bef';
      finishOta();
    };
    xhr.ontimeout=function(){
      bar.className='prog-bar err';stat.textContent='上传或校验超时，请检查设备串口日志和 WiFi 连接';
      finishOta();
    };
    var fd=new FormData();fd.append('firmware',file);
    var auth=new XMLHttpRequest();
    function authFailed(message){
      bar.className='prog-bar err';stat.textContent=message;
      finishOta();
    }
    auth.timeout=10000;
    auth.onload=function(){
      if(auth.status!==200){
        authFailed(auth.status===403||auth.status===401?'认证失败，请输入设备当前的热点密码（不是家庭 WiFi 密码）':'无法验证身份，请刷新页面后重试');
        return;
      }
      password.value='';
      btn.textContent='上传中...';stat.textContent='上传中 0%';
      xhr.open('POST','/update');
      xhr.setRequestHeader('Authorization',authorization);
      xhr.send(fd);
    };
    auth.onerror=function(){authFailed('认证请求失败，请检查设备连接')};
    auth.ontimeout=function(){authFailed('认证请求超时，请检查设备连接')};
    auth.open('GET','/api/ota/auth');
    auth.setRequestHeader('Authorization',authorization);
    auth.send();
  });
})();
</script>
</body></html>)rawliteral";
#endif
