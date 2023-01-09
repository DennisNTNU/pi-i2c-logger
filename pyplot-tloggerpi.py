import sys
import os
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
original_figsize = plt.rcParams["figure.figsize"].copy()
import numpy as np
import datetime as dt

#import sys
#sys.path.insert(0, '/home/den/Documents/ntnu-phd/code/python/lib')
#import util
#ramfspath = util.ramfs.mkramfs()
#if ramfspath is None:
#	print('Could not create or mount ramfs')
#	exit()

#outputpath = '/home/pi/plots'
outputpath = os.getcwd() + '/plots'

try:
	os.mkdir(outputpath)
except FileExistsError:
	pass

################################################################################

if len(sys.argv) < 2:
	print("need arg: path to a csv file")
	exit()

inputfile_df = pd.read_csv(sys.argv[1])
inputfile = inputfile_df.values

inputfile = inputfile.astype('float32')

start_ts = inputfile[0,0]
print(f'{start_ts:.20f}')
print(round(start_ts))

start_time = dt.datetime.fromtimestamp(round(start_ts), dt.timezone.utc)
start_midnight = start_time.replace(hour=0, minute=0, second=0)

print(start_time)
print(start_midnight)

print(inputfile_df.columns)


start_index = 0
xlabel_timeformat = '%H:%M\n%Y-%m-%d'
while True:
	# add a day to start-date-midnight
	until_midnight = start_midnight + dt.timedelta(days=1)

	# convert to unix
	start_unix = round(start_midnight.timestamp())
	end_unix = round(until_midnight.timestamp())
	print(f'from {start_unix:d} to {end_unix:d}')

	# find index at which time is leq unix
	until_index = -1
	for i in range(start_index,inputfile.shape[0]):
		if inputfile[i,0] > end_unix:
			until_index = i
			break
	if until_index == -1:
		until_index = inputfile.shape[0]-1

	# plot range
	title = f'temp_{start_unix:d}-{end_unix:d}'
	plt.figure(1, figsize=[2.5*original_figsize[0], 2.5*original_figsize[1]])
	plt.clf()
	#plt.title(title)

	print('index range:', start_index, until_index)
	x = [dt.datetime.fromtimestamp(round(ts), dt.timezone.utc) for ts in inputfile[start_index:until_index, 0]]

	ax1 = plt.subplot2grid((4, 1), (0, 0), rowspan=3)
	ax2 = plt.subplot2grid((4, 1), (3, 0))

	for i in range(2,inputfile.shape[1]-1):
		ax1.plot(x, inputfile[start_index:until_index, i], linewidth=0.75, label=inputfile_df.columns[i])


	#ax = plt.gca()
	ax1.legend(loc='upper right')
	ax1.xaxis.set_major_formatter(mdates.DateFormatter(xlabel_timeformat))

	ax1.grid()


	ax2.plot(x, inputfile[start_index:until_index, 1], linewidth=0.75, label=inputfile_df.columns[1])
	ax2.grid()
	ax2.legend(loc='upper right')
	ax2.xaxis.set_major_formatter(mdates.DateFormatter(xlabel_timeformat))

	#ax1.xticks(rotation=20)
	#plt.xticks(rotation=20)
	plt.savefig(f'{outputpath:s}/{title:s}.svg', dpi=800.0)



	start_midnight = until_midnight
	start_index = until_index

	if until_index == inputfile.shape[0]-1:
		break;

# repeat


exit()

plt.figure()
for i in range(len(columns_to_plot)):
	print('plotting', inputfile_df.columns[columns_to_plot[i]])
	plt.plot(inputfile[::subsample,columns_to_plot[i]], linewidth='0.33')
plt.grid()



#plt.locator_params(axis='y', numticks=10)
#plt.locator_params(axis='y', nbins=10)
#plt.locator_params(numticks=12)
#plt.locator_params(nticks=12)

#ax = plt.gca()
#every_nth = 4000
#for n, label in enumerate(ax.xaxis.get_ticklabels()):
#	if n % every_nth != 0:
#		label.set_visible(False)

#ax = plt.gca()
#N = 10
#ymin, ymax = ax.get_ylim()
#custom_ticks = np.linspace(ymin, ymax, N, dtype=int)
#ax.set_yticks(custom_ticks)
#ax.set_yticklabels(custom_ticks)

#ax = plt.gca()
#ax.locator_params(axis='y', numticks=10)
#ax.locator_params(axis='y', nbins=10)
#ax.locator_params(numticks=12)
#ax.locator_params(nticks=12)


plt.show()
