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

	return line

def file_read_lines(filename):
	fp = open(filename, "r")
	if not fp:
		return []

	lines = fp.readlines()
	fp.close()

	return lines
