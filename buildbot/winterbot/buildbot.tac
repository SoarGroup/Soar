
from twisted.application import service
from buildbot.slave.bot import BuildSlave

basedir = r'/home/voigtjr/soar/buildbot/winterbot'
host = 'localhost'
port = 9989
slavename = 'winterbot'
passwd = '4Xyplz'
keepalive = 600
usepty = 1
umask = None

application = service.Application('buildslave')
s = BuildSlave(host, port, slavename, passwd, basedir, keepalive, usepty,
               umask=umask)
s.setServiceParent(application)

