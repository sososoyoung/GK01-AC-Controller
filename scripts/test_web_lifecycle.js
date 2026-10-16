// Run real embedded functions with deferred requests to expose stale-response races.
const assert = require('node:assert/strict');
const fs = require('node:fs');
const vm = require('node:vm');
const source = fs.readFileSync(require('node:path').join(__dirname,'../firmware/include/webui.h'),'utf8').split('<script>')[1].split('</script>')[0];
function part(start,end){return source.slice(source.indexOf(start),source.indexOf(end,source.indexOf(start)));}
function setup(){
  const timers=new Map(),requests=[],elements={};let id=0;
  function element(){return {value:'',type:'password',textContent:'',children:[],style:{},classList:{add(){},remove(){},toggle(){}},setAttribute(){},removeAttribute(){},focus(){this.focused=true;},appendChild(e){this.children.push(e);},removeChild(e){this.children.splice(this.children.indexOf(e),1);},get firstChild(){return this.children[0];}};}
  function $(key){return elements[key]||(elements[key]=element());}
  const c={$, document:{hidden:false,createElement:element,querySelectorAll(){return []},addEventListener(){}},window:{addEventListener(){}},
    setTimeout(fn,delay){timers.set(++id,{fn,delay});return id;},clearTimeout(id){timers.delete(id);},
    fetch(url,options){let resolve,reject;const promise=new Promise((a,b)=>{resolve=a;reject=b;});requests.push({url,options,resolve,reject});return promise;},
    AbortController: class{constructor(){this.signal={};}abort(){this.aborted=true;}},
    btoa:s=>Buffer.from(s,'binary').toString('base64'),unescape,encodeURIComponent,
    otaBusy:false,pollTimer:null,toast(){},renderRemote(){},loadAcTargets(){},onIR(){c.captures++;},captures:0,
    esc:String,jsq:String,
  };
  vm.createContext(c);
  vm.runInContext(part("var currentPage='ac'",'function segInit'),c);
  vm.runInContext(part('var captureGeneration=0;','function onIR'),c);
  vm.runInContext(part('var devPollTimer=null','function loadAcTargets'),c);
  vm.runInContext(part('var logsSession=0','(function(){\n  document.querySelectorAll(\'input[type=password]\')'),c);
  $('logs-autoscroll').checked=true;
  return {c,$,requests,timers,run(delay){const found=[...timers].find(([,v])=>v.delay===delay);assert(found,`timer ${delay}`);timers.delete(found[0]);found[1].fn();}};
}
const flush=async()=>{for(let i=0;i<12;i++)await Promise.resolve();};
function response(request,data,status=200){request.resolve({ok:status===200,status,json:async()=>data});}
(async()=>{
  let t=setup();t.c.currentPage='settings';t.c.currentSettings='maintenance';
  t.c.startLogs();assert.match(t.$('logs-status').textContent,/密码/);assert.equal(t.requests.length,0);
  t.$('logs-password').value='secret';t.c.startLogs();t.c.startLogs();assert.equal(t.requests.length,1);
  assert.equal(t.$('logs-password').value,'');assert.match(t.requests[0].options.headers.Authorization,/^Basic /);
  response(t.requests[0],{},403);await flush();assert.match(t.$('logs-status').textContent,/认证失败/);
  t.$('logs-password').value='secret';t.c.startLogs();response(t.requests[1],{ok:true,session:1,cursor:0});await flush();
  assert.match(t.requests[2].url,/after=0/);
  response(t.requests[2],{ok:true,entries:Array.from({length:310},(_,i)=>({seq:i+1,uptime:100,text:'<img onerror=alert(1)>'})),next:310,dropped:0});await flush();
  assert.equal(t.$('logs-view').children.length,300);assert.match(t.$('logs-view').firstChild.textContent,/<img/);
  t.c.clearLogs();assert.equal(t.$('logs-view').textContent,'');
  t.run(500);const pending=t.requests.at(-1);t.c.stopLogs('暂停');const count=t.requests.length;
  response(pending,{ok:true,entries:[{seq:311,uptime:1,text:'late'}],next:311});await flush();
  assert.equal(t.requests.length,count);assert.equal(t.timers.size,0);assert.equal(t.c.logsAuth,'');assert.equal(t.c.logsSession,0);
  t=setup();t.c.currentPage='learn';t.c.startPoll();t.run(500);const old=t.requests[0];
  t.c.stopPoll();t.c.startPoll();t.run(500);assert.equal(t.requests.length,2);
  response(old,{raw:'stale'});await flush();assert.equal(t.c.captures,0);assert.equal(t.timers.size,0,'old request cannot create a second chain');
  response(t.requests[1],{raw:'fresh'});await flush();assert.equal(t.c.captures,1);assert.equal(t.timers.size,1);
  t=setup();t.c.currentPage='devices';t.c.startDevPoll();assert.equal(t.requests.length,1);
  t.c.stopDevPoll();t.c.currentPage='ac';response(t.requests[0],{slaves:[]});await flush();assert.equal(t.requests.length,1,'late slaves response must not query system info');
  t.c.currentPage='devices';t.c.startDevPoll();t.c.loadDevices();assert.equal(t.requests.length,2,'in-flight guard prevents overlap');
  response(t.requests[1],{slaves:[]});await flush();assert.equal(t.requests.length,3);
  t.c.otaBusy=true;t.c.stopDevPoll();response(t.requests[2],{mode:'ap'});await flush();assert.equal(t.timers.size,0,'OTA prevents late device response from scheduling');
  // An expired session terminates rather than repeatedly retrying authentication.
  t=setup();t.c.currentPage='settings';t.c.currentSettings='maintenance';t.$('logs-password').value='secret';t.c.startLogs();response(t.requests[0],{ok:true,session:1,cursor:0});await flush();response(t.requests[1],{},409);await flush();assert.match(t.$('logs-status').textContent,/会话失效/);assert.equal(t.c.logsAuth,'');
  // Three real request timeouts produce bounded backoff, then release credentials.
  t=setup();t.c.currentPage='settings';t.c.currentSettings='maintenance';t.$('logs-password').value='secret';t.c.startLogs();response(t.requests[0],{ok:true,session:1,cursor:0});await flush();
  for(let attempt=0;attempt<3;attempt++){
    const pending=t.requests.at(-1);t.run(3500);pending.reject(new Error('aborted'));await flush();
    if(attempt<2)t.run(attempt===0?500:1000);
  }
  assert.match(t.$('logs-status').textContent,/超时/);assert.equal(t.c.logsAuth,'');assert.equal(t.c.logsSession,0);
  // A late start response from a cancelled connection must not begin polling.
  t=setup();t.c.currentPage='settings';t.c.currentSettings='maintenance';t.$('logs-password').value='secret';t.c.startLogs();
  t.c.stopLogs('断开');const total=t.requests.length;response(t.requests[0],{ok:true,session:7,cursor:0});await flush();
  assert.equal(t.requests.length,total);assert.equal(t.c.logsSession,0);
  console.log('PASS Web lifecycle: log auth/300-line/text-only/pause/stale responses; capture and device generation/in-flight guards; OTA suspension; timeout/backoff/late start');
})().catch(e=>{console.error(e);process.exitCode=1;});
