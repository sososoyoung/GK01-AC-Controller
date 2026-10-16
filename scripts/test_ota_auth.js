// Exercise the actual WebUI submit handler without a browser or a device.
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const vm = require('node:vm');
const source = fs.readFileSync(path.join(__dirname, '../firmware/include/webui.h'), 'utf8');
const script = source.split('<script>')[1].split('</script>')[0];
new vm.Script(script); // Validate the complete embedded script too.
const start = script.indexOf("(function(){\n  var f=document.getElementById('ota-form')");
assert(start >= 0);
const handler = script.slice(start, script.indexOf('})();', start) + 5);

function setup(password = 'test-password', fileOk = true) {
  const elements = Object.fromEntries([
    'ota-form', 'ota-file', 'ota-password', 'ota-prog', 'ota-prog-bar', 'ota-prog-stat', 'ota-btn',
  ].map(id => [id, {style: {}, value: '', focus() {this.focused = true;}}]));
  elements['ota-password'].value = password;
  elements['ota-file'].files = [{name: 'ota.bin'}];
  let submit;
  elements['ota-form'].addEventListener = (event, fn) => {submit = fn;};
  const sent = [];
  class XHR {
    constructor() {this.upload = {}; this.headers = {};}
    open(method, url) {this.method = method; this.url = url;}
    setRequestHeader(key, value) {this.headers[key] = value;}
    send(body) {this.body = body; sent.push(this);}
  }
  const context = {
    document: {getElementById: id => elements[id]}, XMLHttpRequest: XHR,
    FormData: class {append() {}},
    btoa: s => Buffer.from(s, 'binary').toString('base64'),
    unescape, encodeURIComponent, setTimeout() {}, location: {reload() {}},
    otaSupported: true, otaFileOk: fileOk, otaBusy: false, otaValidation: 0, otaValidateFile() {},
    suspendPagePolls() {context.suspended = true;}, stopLogs() {context.logsStopped = true;}, resumePagePolls() {context.resumed = true;},
  };
  vm.runInNewContext(handler, context);
  submit({preventDefault() {}});
  return {elements, sent, submit, context};
}

let t = setup('');
assert.equal(t.sent.length, 0);
assert.equal(t.elements['ota-password'].focused, true);
t = setup('test-password', false);
assert.equal(t.sent.length, 0, 'Unvalidated files must not start authentication or upload');
for (const status of [401, 403, 404, 500]) {
  t = setup();
  assert.equal(t.sent.length, 1);
  assert.equal(t.sent[0].url, '/api/ota/auth');
  assert.equal(t.sent[0].body, undefined);
  t.sent[0].status = status;
  t.sent[0].onload();
  assert.equal(t.sent.length, 1, 'Failed auth must not upload the firmware');
  assert.equal(t.elements['ota-btn'].disabled, false);
}
for (const event of ['onerror', 'ontimeout']) {
  t = setup();
  t.sent[0][event]();
  assert.equal(t.sent.length, 1);
  assert.equal(t.elements['ota-btn'].disabled, false);
}
for (const password of ['test-password', '热点密码123']) {
  t = setup(password);
  const auth = t.sent[0];
  auth.status = 200;
  auth.onload();
  assert.equal(t.sent.length, 2);
  const upload = t.sent[1];
  assert.equal(upload.method, 'POST');
  assert.equal(upload.url, '/update');
  assert.equal(upload.headers.Authorization, auth.headers.Authorization);
  assert.equal(Buffer.from(upload.headers.Authorization.slice(6), 'base64').toString(), 'admin:' + password);
  assert.equal(t.elements['ota-password'].value, '');
  upload.status = 401;
  upload.onload();
  assert.match(t.elements['ota-prog-stat'].textContent, /认证失败/);
  assert.equal(t.elements['ota-btn'].disabled, false);
}
t = setup();
t.submit({preventDefault() {}});
assert.equal(t.sent.length, 1, 'Duplicate submit must not start another preflight');
assert.equal(t.context.otaBusy, true);
assert.equal(t.elements['ota-file'].disabled, true);
assert.equal(t.elements['ota-password'].disabled, true);
assert.equal(t.context.suspended, true);
assert.equal(t.context.logsStopped, true);
t.sent[0].onerror();
assert.equal(t.context.otaBusy, false);
assert.equal(t.elements['ota-file'].disabled, false);
assert.equal(t.context.resumed, true);
// Actual asynchronous validator: a late FileReader for an old selection cannot
// overwrite the currently selected file's result, nor change an active upload.
const validationStart = script.indexOf('function otaValidateFile(');
const validationCode = script.slice(validationStart, script.indexOf('\n(function(){', validationStart));
const readers = [];
const validation = {
  otaBusy: false, otaValidation: 0, otaSupported: true, otaTargetRom: 1,
  otaTargetFile: 'ota.bin', Uint8Array,
  otaSetReady(ok) {validation.ready = ok;}, otaMeta() {}, otaMsg() {}, otaSize() {return '1KB';},
  otaRbootInfo() {return {ok:true};}, otaManifestInfo() {return {ok:true,version:'test'};},
  FileReader: class {constructor() {readers.push(this);this.result=new ArrayBuffer(16);} readAsArrayBuffer() {}},
};
vm.runInNewContext(validationCode, validation);
validation.otaValidateFile({name:'old.bin',size:16});
validation.otaValidateFile({name:'new.bin',size:16});
readers[0].onload();assert.equal(validation.ready, false);
readers[1].onload();assert.equal(validation.ready, true);
validation.otaValidateFile({name:'newer.bin',size:16});
validation.otaBusy=true;readers[2].onload();assert.equal(validation.ready,false);
console.log('PASS OTA auth: empty/wrong password, network failure, timeout, preflight before upload, UTF-8 credentials, retry, duplicate submit, background suspension, stale file validation');
