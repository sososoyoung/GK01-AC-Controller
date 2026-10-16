const {chromium}=require('playwright');
const fs=require('fs'),assert=require('assert'),path=require('path');
process.chdir(path.resolve(__dirname,'..'));
(async()=>{
const browser=await chromium.launch({headless:true});
let failSlaves=false,failCatalog=false,slavesGate=null,releaseSlaves;
const page=await browser.newPage({viewport:{width:390,height:844}}),errors=[],requests=[];
page.on('pageerror',e=>errors.push(e.message));
const version=fs.readFileSync('firmware/include/version.h','utf8').match(/#define IRAC_FIRMWARE_VERSION "([^"]+)"/)[1];
const html=fs.readFileSync('firmware/include/webui.h','utf8').split('R"rawliteral(')[1].split(')rawliteral"')[0];
await page.route('http://gk01.test/**',async route=>{
const req=route.request(),path=new URL(req.url()).pathname;
if(path==='/')return route.fulfill({contentType:'text/html',body:html});
if(req.method()==='POST')requests.push({path,body:req.postData()});
if(path==='/api/slaves'&&slavesGate)await slavesGate;
if((path==='/api/slaves'&&failSlaves)||(path==='/api/hvac/catalog'&&failCatalog))return route.fulfill({status:503,contentType:'application/json',body:'{}'});
let data={ok:true};
if(path==='/api/hvac/catalog')data=[{vendor:'GREE',models:[{id:0,name:'默认'},{id:1,name:'YAW1F'},{id:2,name:'YBOFB'}]},{vendor:'WAHIN',models:[{id:0,name:'默认'}]}];
if(path==='/api/hvac/state')data={vendor:'GREE',model:2,temp:25,mode:'Cool',fan:'Auto'};
if(path==='/api/wifi/status')data={mode:'ap',ssid:'IR-AC',ip:'10.1.1.1'};
if(path==='/api/system/info')data={firmwareVersion:version,mode:'ap',deviceName:'客厅主机',apMac:'00:11:22:33:44:55'};
if(path==='/api/ota/status')data={ok:true,current_rom:0,target_rom:1};
if(path==='/api/slaves')data={slaves:[]};
return route.fulfill({contentType:'application/json',body:JSON.stringify(data)});
});
await page.goto('http://gk01.test/');await page.waitForLoadState('networkidle');
assert.equal(await page.locator('#ac-model').inputValue(),'2');
assert.equal(await page.locator('#sys-version').textContent(),'v'+version);
for(const theme of ['light','dark']){await page.evaluate(t=>document.documentElement.dataset.theme=t,theme);await page.screenshot({path:'/tmp/gk01-ac-inline-'+theme+'.png',fullPage:true,animations:'disabled'})}
await page.getByRole('button',{name:'开机',exact:true}).click();
await page.waitForTimeout(100);assert(requests.find(r=>r.path==='/api/hvac'&&new URLSearchParams(r.body).get('model')==='2'));
await page.selectOption('#ac-v','WAHIN');assert.equal(await page.locator('#ac-model').inputValue(),'0');assert.equal(await page.locator('#ac-model option').count(),1);
await page.getByRole('button',{name:'电视',exact:true}).click();
assert.equal(await page.locator('#tv-buttons button').count(),11);
assert(await page.locator('#remote-ac').isHidden());
assert(await page.locator('#tv-buttons').isVisible());
await page.getByRole('button',{name:'电源',exact:true}).click();await page.waitForTimeout(100);
assert(requests.find(r=>r.path==='/api/tv/xiaomi'&&r.body==='key=power&target=LOCAL'));
await page.evaluate(()=>go('remote'));
await page.locator('#ir-import').setInputFiles('data/ir/xiaomi_tv.json');await page.waitForTimeout(100);
assert.equal(await page.locator('#remote-devices .grid-b').count(),11);
await page.locator('#remote-devices .grid-b').first().click();await page.waitForTimeout(100);
assert(requests.find(r=>r.path==='/api/send'&&new URLSearchParams(r.body).get('target')==='LOCAL'));
const tests=await page.evaluate(()=>{
let rejected=0;for(const signal of [{name:'x',raw:[-1]},{name:'x',raw:[70000]},{name:'x',raw:[1],frequency:0},{name:'x',raw:[0]},{name:'x',raw:Array(513).fill(1)}]){try{normalizeIrCodes({version:1,signals:[signal]})}catch{rejected++}}return rejected;
});assert.equal(tests,5);
await page.reload();await page.waitForLoadState('networkidle');await page.evaluate(()=>go('remote'));assert.equal(await page.locator('#remote-devices .grid-b').count(),11);
// Theme follows both the OS and the manual override; D-pad coordinates match a physical remote.
await page.evaluate(()=>go('ac'));await page.getByRole('button',{name:'电视',exact:true}).click();
const positions=await page.locator('.tv-dpad button').evaluateAll(buttons=>Object.fromEntries(buttons.map(b=>[b.dataset.key,{x:b.getBoundingClientRect().x,y:b.getBoundingClientRect().y}])));
assert(positions.up.y<positions.ok.y&&positions.down.y>positions.ok.y);
assert(positions.left.x<positions.ok.x&&positions.right.x>positions.ok.x);
assert.equal(positions.up.x,positions.ok.x);assert.equal(positions.down.x,positions.ok.x);
for(const systemTheme of ['light','dark']){
  await page.emulateMedia({colorScheme:systemTheme});
  for(const theme of [null,'light','dark']){
    await page.evaluate(theme=>{if(theme)document.documentElement.dataset.theme=theme;else document.documentElement.removeAttribute('data-theme')},theme);
    const expected=(theme||systemTheme)==='dark'?'rgb(255, 255, 255)':'rgb(28, 28, 30)';
    assert.equal(await page.locator('[data-key=home]').evaluate(e=>getComputedStyle(e).color),expected);
  }
}
for(const theme of ['light','dark']){
  await page.evaluate(theme=>document.documentElement.dataset.theme=theme,theme);
  await page.screenshot({path:'/tmp/gk01-tv-'+theme+'.png',fullPage:true,animations:'disabled'});
}
await page.setViewportSize({width:320,height:740});
assert(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth));
await page.setViewportSize({width:390,height:844});
await page.evaluate(()=>go('learn'));await page.locator('#btn-rec').click();
assert.equal(await page.locator('#learn-card').evaluate(e=>getComputedStyle(e).animationName),'none');
assert.equal(await page.locator('#btn-rec').evaluate(e=>getComputedStyle(e).transitionDuration),'0s');
await page.locator('#btn-rec').click();
// A held or failed slaves request never hides the current device and recovers on the next refresh.
slavesGate=new Promise(resolve=>releaseSlaves=resolve);
await page.evaluate(()=>go('devices'));
assert(await page.locator('.dev-card.master').isVisible());
assert.match(await page.locator('.dev-card.master').innerText(),/客厅主机/);
failSlaves=true;releaseSlaves();slavesGate=null;
await page.waitForFunction(()=>document.getElementById('dev-load-status').textContent.includes('从机列表读取失败'));
assert(await page.locator('.dev-card.master').isVisible());
failSlaves=false;await page.evaluate(()=>loadDevices());
await page.waitForFunction(()=>document.getElementById('dev-load-status').textContent.includes('暂无已配对从机'));
await page.screenshot({path:'/tmp/gk01-devices.png',fullPage:true,animations:'disabled'});
// A failed AC catalog is scoped to the AC panel and can be retried after navigation.
failCatalog=true;await page.reload();
await page.waitForFunction(()=>document.getElementById('ac-load-status').textContent.includes('读取失败'));
await page.evaluate(()=>go('devices'));
assert(!(await page.locator('#toast').innerText()).includes('空调'));
assert(await page.locator('.dev-card.master').isVisible());
failCatalog=false;await page.evaluate(()=>go('ac'));
await page.waitForFunction(()=>acStateReady);assert.equal(await page.locator('#ac-model').inputValue(),'2');
assert.deepEqual(errors,[]);
await browser.close();console.log('PASS browser: AC/TV switching, D-pad layout, six theme combinations, 320px viewport, no recording animation, slow/failed slaves and recovery, scoped AC failure/retry, model restore, TV/send/import persistence; no page errors');
})().catch(e=>{console.error(e);process.exit(1)});
