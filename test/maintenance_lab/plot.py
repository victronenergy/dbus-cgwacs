#!/usr/bin/python
import matplotlib.pyplot as plt
import subprocess

out = subprocess.check_output('../build-maintenance_lab-Desktop_4_8_3_desktop-Debug/maintenance_lab', stderr=subprocess.STDOUT)
#spl = [x.split() for x in out.split('\n') if x.strip() != '']
#t = [x[0] for x in spl]
#soc = [x[1] for x in spl]
#soc_limit = [x[2] for x in spl]
#p = plt.plot(t, soc, '-', t, soc_limit, '-')

spl = [x.split() for x in out.split('\n') if x.strip() != '']
t = [x[0] for x in spl]
min_soc = [x[1] for x in spl]
max_soc = [x[2] for x in spl]
soc_limit = [x[3] for x in spl]
total_load = [x[4] for x in spl]
total_power = [x[5] for x in spl]
charge_power = [x[6] for x in spl]

mis, = plt.plot(t, min_soc, '-', label='min SoC (%)')
mas, = plt.plot(t, max_soc, '-', label='max SoC (%)')
sl, = plt.plot(t, soc_limit, '-', label='SoC limit (%)')
plt.plot([min(t), max(t)], [10, 10])
plt.plot([min(t), max(t)], [80, 80])
plt.xlabel('Time (Days)')
plt.ylabel('SoC (%)')
plt.legend(handles=[mis, mas, sl])

#p = plt.plot(\
#    t, min_soc, '-', \
#    t, max_soc, '-', \
#    t, soc_limit, '-', \
#    # t, total_load, '-', \
#    # t, total_power, '-', \
#    # t, charge_power, '-', \
#    [min(t), max(t)], [80, 80], \
#    [min(t), max(t)], [10, 10])

plt.show()
