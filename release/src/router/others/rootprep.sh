#!/bin/bash

ROOTDIR=$PWD

mkdir -p -m 0755 dev
mkdir -p -m 0755 proc
mkdir -p -m 0755 sys
mkdir -p -m 0755 tmp
mkdir -p -m 0755 jffs
mkdir -p -m 0755 cifs1
mkdir -p -m 0755 cifs2
mkdir -p -m 0755 opt

# !!TB
#mkdir -p -m 0755 mmc
mkdir -p -m 0755 usr/local
ln -sf /tmp/share usr/share
ln -sf /tmp/share usr/local/share

ln -sf tmp/mnt mnt
ln -sf tmp/var var
ln -sf tmp/etc etc
ln -sf tmp/home home
ln -sf tmp/home/root root
(cd usr && ln -sf ../tmp)

# !!TB
ln -sf /tmp/var/wwwext www/ext
ln -sf /tmp/var/wwwext www/user
ln -sf /www/ext/proxy.pac www/proxy.pac
ln -sf /www/ext/proxy.pac www/wpad.dat

# shibby
mkdir -p -m 0755 nas
mkdir -p -m 0755 bkp
mkdir -p -m 0777 tftpboot

# Hyzoom bwq518
#if [ -d "usr/lib/python2.7/site-packages/gaeproxy" ]; then
#   ln -sf /tmp/gaeproxy/proxy.ini usr/lib/python2.7/site-packages/gaeproxy/local/proxy.ini
#   ln -sf /tmp/gaeproxy/config.py usr/lib/python2.7/site-packages/gaeproxy/local/config.py
#   ln -sf /tmp/gaeproxy/pac usr/lib/python2.7/site-packages/gaeproxy/local/pac
#   ln -sf /tmp/gaeproxy/misc usr/lib/python2.7/site-packages/gaeproxy/local/misc
#   ln -sf /tmp/gaeproxy/cert usr/lib/python2.7/site-packages/gaeproxy/local/cert
#   ln -sf /tmp/gaeproxy/autoupload.py usr/lib/python2.7/site-packages/gaeproxy/server/autoupload.py
#   rm -f usr/lib/python2.7/site-packages/gaeproxy/server/python/app.yaml
#   ln -sf /tmp/gaeproxy/app.yaml usr/lib/python2.7/site-packages/gaeproxy/server/python/app.yaml
#fi
#if [ -d "usr/lib/python2.7/site-packages/goagent" ]; then
#   ln -sf /tmp/goagent/proxy.ini usr/lib/python2.7/site-packages/goagent/proxy.ini
#   ln -sf /tmp/goagent/proxy.pac usr/lib/python2.7/site-packages/goagent/proxy.pac
#   ln -sf /tmp/goagent/certs usr/lib/python2.7/site-packages/goagent/certs
#fi
# for debuging, bwq518
#rm -f www/advanced-gaeproxy.asp
#ln -sf /tmp/advanced-gaeproxy.asp www/advanced-gaeproxy.asp
