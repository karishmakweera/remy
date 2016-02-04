import os
from matplotlib import pyplot as plt
import matplotlib.ticker as mtick
import numpy as np

labels = ["1%", "2%", "4%", "8%", "16%", "32%", "64%", "100%"]

def write_to_file(data, f):
	for d in data:
		f.write(str(d) + " ")
	f.write("\n")

def analyze():
	errors = []
	for i in range(len(labels) - 1):
		errors.append([])
	
	for filename in os.listdir(rootdir):
		f = open(rootdir + filename, 'r')
		scores = []
		last_t = 0
		last_x = 0
		for line in f.readlines():
			words = line.split()
			scores.append(float(words[1]))
			
		if len(scores) == len(labels):
			final_score = scores[-1]
			for i in range(len(scores) - 1):
				errors[i].append(abs((scores[i] - final_score) / final_score) * 100)
			
	cdf = open("cdf.txt", "w")
	for i in range(len(labels) - 1):
		sorted_data = np.sort(errors[i])
		yvals = np.arange(len(sorted_data)) / float(len(sorted_data))
		plt.plot(sorted_data,yvals, label=labels[i]+" simulation time")
		write_to_file(sorted_data, cdf)
		write_to_file(yvals, cdf)
	cdf.close()
		
	plt.xlim(0, 30)
	plt.xlabel("Percent Error from Final Score")
	plt.ylabel("CDF")
	plt.legend(loc=4)
	fmt = "%.0f%%"
	xticks = mtick.FormatStrFormatter(fmt)
	plt.gca().xaxis.set_major_formatter(xticks)
	plt.show()

		
rootdir = 'eval_score/'
analyze()
