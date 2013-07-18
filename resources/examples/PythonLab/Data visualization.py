import pylab as pl
import numpy as np
import pythonlab

""" XY chart """

years = np.linspace(1860, 2010, 16)
occurrence = np.array([0, .05e-5, .1e-5, .5e-5, 1e-5, 1.3e-5,
                       1.9e-5, 2.6e-5, 4.2e-5, 6e-5, 8e-5,
                       7.7e-5, 5.8e-5, 6.3e-5, 6.2e-5, 6.1e-5])*1e3

pl.figure()
pl.title('Occurrence of phrase in literature')
pl.plot(years, occurrence, '.-', color=(0.75,0,0), label="electromagnetic field")
pl.xlabel("year")
pl.ylabel("occurrence ($^{o}/_{oo}$)")
pl.legend(loc="lower right")
pl.grid(True)

chart_file = pythonlab.tempname("png")
pl.savefig(chart_file, dpi=60)
pl.close()

pythonlab.image(chart_file)

""" Pie chart """

labels = ["HDPE", "LDPE", "PP", "PS", "PVC", "PET", "other"]
fracs = [17, 31, 17, 15, 10, 5, 5]

pl.figure()
pl.title("Composition of Polymers in Waste")
pl.pie(fracs, labels=labels, autopct='$%1.0f\%%$')

chart_file = pythonlab.tempname("png")
pl.savefig(chart_file, dpi=60)
pl.close()

pythonlab.image(chart_file)

""" Bar chart """

month = ["January", "February", "March", "April"]
commits = [227, 298, 236, 113]

ind = np.arange(len(month))
width = 0.9

pl.figure()
pl.title("Number of commits in agros2d.git")
pl.bar(ind, commits, width, color=(0.0,0.0,0.75))
pl.xticks(ind+width/2.0, month)
pl.ylabel("Number of commits")
pl.grid(True)

chart_file = pythonlab.tempname("png")
pl.savefig(chart_file, dpi=60)
pl.close()

pythonlab.image(chart_file)