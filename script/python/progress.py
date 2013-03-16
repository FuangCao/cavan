PROGRESS_BAR_LENGTH_HALF = 30
PROGRESS_BAR_LENGTH = PROGRESS_BAR_LENGTH_HALF * 2

class CavanProgressBar:
	def __init__(self, total):
		self.init(total)

	def init(self, total):
		self.total = total
		self.current = 0
		self.update()

	def update(self):
		if self.total == 0:
			fill = PROGRESS_BAR_LENGTH
			percent = 100
		else:
			fill = self.current * PROGRESS_BAR_LENGTH / self.total
			percent = self.current * 100 / self.total

		left = right = ""
		for weight in range(0, PROGRESS_BAR_LENGTH_HALF):
			if weight < fill:
				left += "H"
			else:
				left += "="

		for weight in range(PROGRESS_BAR_LENGTH_HALF, PROGRESS_BAR_LENGTH):
			if weight < fill:
				right += "H"
			else:
				right += "="

		text = "[%s %d%% %s] [%d/%d]" % (left, percent, right, self.current, self.total)
		if SVN_REPO_DEBUG:
			print text
		else:
			sys.stdout.write(text + "\r")
			sys.stdout.flush()

	def add(self, count = 1):
		self.current += count
		self.update()

	def finish(self):
		if self.current < self.total:
			self.current = self.total
			self.update()

		if not SVN_REPO_DEBUG:
			print
