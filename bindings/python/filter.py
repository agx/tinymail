#! /usr/bin/python
import sys
import re
import getopt

try:
	opts, args = getopt.getopt (sys.argv[1:], "e:h", ["help", "extra="])
except getopt.error, msg:
	sys.stdout.writeln (msg)
	sys.stdout.writeln ("for help use --help")
	sys.exit(2)

for o, a in opts:
	if o in ("-h", "--help"):
		sys.stdout.writeln ("--extra FILENAME")
		sys.exit(0)
	if o in ("-e", "--extra"):
		filename = a

things = []
extra = []
opened=True
tregex = re.compile('\(define-(.*)$')
cregex =  re.compile ('\(define-object\ (.*)Iface(.*)$')
f = open (filename, 'r')

for line in f:
	m = tregex.match(line)
	if m != None:
		things.append (m.groups ()[0])
	extra.append (line)

for line in sys.stdin:
	t = cregex.match(line)
	if t != None:
		part1 = t.groups ()[0]
		part2 = t.groups ()[1]
		line = "(define-interface " + part1 + "Iface" + part2 + "\n"

	m = tregex.match(line)
	if m != None:
		thing = m.groups ()[0]
		if thing in things:
			opened=False

	if opened:
		sys.stdout.write (line)
	else:
		if line == ')\n':
			opened=True

for line in extra:
	sys.stdout.write (line)
		
