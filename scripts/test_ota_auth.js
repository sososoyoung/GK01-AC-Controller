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
  vm.runInNewContext(handler, {
    document: {getElementById: id => elements[id]}, XMLHttpRequest: XHR,
    FormData: class {append() {}},
    btoa: s => Buffer.from(s, 'binary').toString('base64'),
    unescape, encodeURIComponent, setTimeout() {}, location: {reload() {}},
    otaSupported: true, otaFileOk: fileOk, otaValidateFile() {},
  });
  submit({preventDefault() {}});
  return {elements, sent};
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
console.log('PASS OTA auth: empty/wrong password, network failure, timeout, preflight before upload, UTF-8 credentials, retry');
