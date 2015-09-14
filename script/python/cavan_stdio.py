def pr_red_info(message):
	print "\033[31m%s\033[0m" % message

def pr_green_info(message):
	print "\033[32m%s\033[0m" % message

def pr_bold_info(message):
	print "\033[1m%s\033[0m" % message

def pr_err_info(message):
	pr_red_info(message)

def pr_warn_info(message):
	print "\033[35m%s\033[0m" % message
