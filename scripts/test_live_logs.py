#!/usr/bin/env python3
"""Compile the actual RAM queue, Print adapter and HTTP log handlers on a host."""
import os
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / 'firmware/src'


def main():
    with tempfile.TemporaryDirectory(prefix='gk01-logs-') as directory:
        tmp = Path(directory)
        (tmp / 'Arduino.h').write_text(r'''#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <string>
#define F(value) value
uint32_t millis();
class Print {
public:
 virtual ~Print() = default;
 virtual size_t write(uint8_t) = 0;
 size_t write(const char* s) { size_t n=0;while(*s)n+=write(uint8_t(*s++));return n; }
 void print(const char* s) { write(s); }
 void println(const char* s="") { write(s);write(uint8_t('\n')); }
 void printf(const char* format, ...) { char b[1024];va_list a;va_start(a,format);vsnprintf(b,sizeof(b),format,a);va_end(a);write(b); }
};
struct SerialPort { std::string output;void write(uint8_t c){output+=char(c);} };
extern SerialPort Serial;
''')
        source = (SRC / 'core/string_utils.cpp').read_text()
        start = source.index('String jsonEscape(')
        (tmp / 'escape.inc').write_text(source[start:source.index('\n}', start)+2])
        source = (SRC / 'web/web_server.cpp').read_text()
        start = source.index('namespace {\nvoid logResponse')
        helpers = source[start:source.index('\nvoid WebServer::registerApiRoutes()', start)]
        start = source.index('    server_.on("/api/logs/start"')
        routes = source[start:source.index('    server_.on("/",', start)]
        (tmp / 'routes.inc').write_text(helpers+'\nvoid registerLogs() {\n'+routes+'\n}')
        binary = tmp / 'live_logs_test'
        subprocess.run([os.environ.get('CXX','c++'), '-std=c++17', '-Wall', '-Wextra',
                        f'-I{tmp}', f'-I{SRC}', str(ROOT/'tests/host/live_log.cpp'),
                        str(SRC/'core/live_log.cpp'), str(SRC/'core/live_log_queue.cpp'),
                        '-o', str(binary)], check=True)
        subprocess.run([str(binary)], check=True)


if __name__ == '__main__':
    main()
