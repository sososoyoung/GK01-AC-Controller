const {chromium}=require('@playwright/test');
const fs=require('fs'),assert=require('assert'),path=require('path'),os=require('os');
process.chdir(path.resolve(__dirname,'..'));
(async()=>{
const browser=await chromium.launch({headless:true});
const page=await browser.newPage({viewport:{width:390,height:844}}),errors=[],requests=[];
page.on('pageerror',e=>errors.push(e.message));
const html=fs.readFileSync('firmware/include/webui.h','utf8').split('R"rawliteral(')[1].split(')rawliteral"')[0];
await page.route('http://gk01.test/**',async route=>{
const req=route.request(),path=new URL(req.url()).pathname;
if(path==='/')return route.fulfill({contentType:'text/html',body:html});
if(req.method()==='POST')requests.push({path,body:req.postData()});
let data={ok:true};
if(path==='/api/hvac/catalog')data=[{vendor:'GREE',models:[{id:0,name:'默认'},{id:1,name:'YAW1F'},{id:2,name:'YBOFB'}]},{vendor:'WAHIN',models:[{id:0,name:'默认'}]}];
if(path==='/api/hvac/state')data={vendor:'GREE',model:2,temp:25,mode:'Cool',fan:'Auto'};
if(path==='/api/wifi/status')data={mode:'ap',ssid:'IR-AC',ip:'10.1.1.1'};
if(path==='/api/slaves')data={slaves:[]};
return route.fulfill({contentType:'application/json',body:JSON.stringify(data)});
});
await page.goto('http://gk01.test/');await page.waitForLoadState('networkidle');
assert.equal(await page.locator('#ac-model').inputValue(),'2');
await page.getByRole('button',{name:'开机',exact:true}).click();
await page.waitForTimeout(100);assert(requests.find(r=>r.path==='/api/hvac'&&new URLSearchParams(r.body).get('model')==='2'));
await page.selectOption('#ac-v','WAHIN');assert.equal(await page.locator('#ac-model').inputValue(),'0');assert.equal(await page.locator('#ac-model option').count(),1);
await page.evaluate(()=>go('remote'));assert.equal(await page.locator('#tv-buttons button').count(),11);
await page.getByRole('button',{name:'电源',exact:true}).click();await page.waitForTimeout(100);
assert(requests.find(r=>r.path==='/api/tv/xiaomi'&&r.body==='key=power&target=LOCAL'));
await page.locator('#ir-import').setInputFiles('data/ir/xiaomi_tv.json');await page.waitForTimeout(100);
assert.equal(await page.locator('#remote-devices .grid-b').count(),11);
await page.locator('#remote-devices .grid-b').first().click();await page.waitForTimeout(100);
assert(requests.find(r=>r.path==='/api/send'&&new URLSearchParams(r.body).get('target')==='LOCAL'));
const tests=await page.evaluate(()=>{
let rejected=0;for(const signal of [{name:'x',raw:[-1]},{name:'x',raw:[70000]},{name:'x',raw:[1],frequency:0},{name:'x',raw:[0]},{name:'x',raw:Array(513).fill(1)}]){try{normalizeIrCodes({version:1,signals:[signal]})}catch{rejected++}}return rejected;
});assert.equal(tests,5);
await page.reload();await page.waitForLoadState('networkidle');await page.evaluate(()=>go('remote'));assert.equal(await page.locator('#remote-devices .grid-b').count(),11);
await page.screenshot({path:path.join(os.tmpdir(),'gk01-ir-remote.png'),fullPage:true,animations:'disabled'});assert.deepEqual(errors,[]);
await browser.close();console.log('PASS browser: model restore/reset, HVAC model request, 11 TV buttons/local target, import/replay/reload, malformed signal rejection; no page errors');
})().catch(e=>{console.error(e);process.exit(1)});
