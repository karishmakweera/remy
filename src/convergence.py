import os
import subprocess
import argparse
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.lines import Line2D
import matplotlib.animation as animation
from matplotlib.widgets import RadioButtons
from matplotlib.widgets import Button

class SubplotAnimation(animation.TimedAnimation):
    def __init__(self, fig, ax, data):
        self.t = []
        self.x = []
        self.y = []
        for line in data.split("\n"):
        	words = line.split()
        	if (len(words) != 3):
        		print line
        		continue
        	if not self.t or self.t[-1] != float(words[1]): 
        		self.t.append(float(words[1]))
        	if words[0] == '0':
        		self.x.append(float(words[2]))
        	else:
        		self.y.append(float(words[2]))

        self.line1 = Line2D([], [], color='black', marker='o')
        ax.add_line(self.line1)
        ax.set_xlim(0, 80)
        ax.set_ylim(0, 80)

        animation.TimedAnimation.__init__(self, fig, interval=1, blit=False)

    def _draw_frame(self, framedata):
        i = framedata
        self.line1.set_data(self.x[:i], self.y[:i])
        self.line1.set_color('black')

        self._drawn_artists = [self.line1]

    def new_frame_seq(self):
        return iter(range(len(self.t)))

    def _init_draw(self):
        lines = [self.line1]
        for l in lines:
            l.set_data([], [])

if __name__ == "__main__":		
    fig, ax = plt.subplots()
    plt.subplots_adjust(left=0.3)

    parser = argparse.ArgumentParser()
    parser.add_argument("remycc", type=str, help="RemyCC file(s) to run")
    parser.add_argument("-l", "--link", type=float, default=100,
        help="Link speed in Mbps")
    parser.add_argument("-s", "--signal", type=int, default=0,
        help="Index of signal to visualize:" + 
        " {0:send_ewma, 1:receive_ewma, 2:rtt_ratio 3:slow_rec}")
    args = parser.parse_args()
    path = os.path.abspath("rat-plotter")
    cmd = ["/." + path, "if=" + args.remycc, "export=" + str(args.signal), 
           "link_ppt=" + str(args.link / 10.0), "on=1000000", "of=0"]
    p = subprocess.Popen(cmd, stderr=subprocess.PIPE)
    err = p.communicate()
    ani = SubplotAnimation(fig, ax, err[1])
    plt.show()

