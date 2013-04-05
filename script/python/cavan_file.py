#!/usr/bin/python

def file_write_text(filename, text):
	fp = open(filename, "w", 0777)
	if not fp:
		return False

	fp.write(text)
	fp.close()

	return True

def file_read_line(filename):
	fp = open(filename, "r")
	if not fp:
		return None

	line = fp.readline()
	fp.close()

	if not line:
		return None

	return line.rstrip("\r\n\f")

def file_read_lines(filename):
	fp = open(filename, "r")
	if not fp:
		return []

	lines = fp.readlines()
	fp.close()

	return lines

def file_write_lines(filename, lines):
	fp = open(filename, "w")
	if not fp:
		return False

	fp.writelines(lines)
	fp.close()

	return True

def file_append_line(filename, line):
	fp = open(filename, "a")
	if not fp:
		return False

	fp.write(line + "\n")
	fp.close()

	return True

def file_append_lines(filename, lines):
	fp = open(filename, "a")
	if not fp:
		return False

	fp.writelines(lines)
	fp.close()

	return True
