#!/bin/sh
cd `dirname $0`
WINEDEBUG=-all wine ./transaq_terminal.exe
