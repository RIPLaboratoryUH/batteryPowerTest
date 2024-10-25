import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import math

testData = pd.read_csv('testData.csv',skiprows=0)
#testData["Power"] = testData["Torque estimate [NM]"] * 2 * math.pi * testData["Vel estimate [rev/s]"]
testData["Power"] = testData["VBUS DC Current [A]"]*testData["VBUS DC Voltage [V]"]
testData["Power Avg"] = testData["Power"].rolling(window=2).mean()
testData["Power"] = testData["Power Avg"]
testData["Time since start"] = (testData["Time Since Start [microseconds]"] / 1000000 - 10)
indexPower = testData[ (testData['Power'] < 0) ].index
testData.drop(indexPower, inplace=True)
testData.head(20)

myData16 = testData[testData["nodeID"]==16]
myArray = myData16.loc[:,("Time since start")].diff()
myData16.loc[:,("deltaT")] = myArray
myArray = myData16.loc[:,("deltaT")] * myData16.loc[:,("Power")]
myData16["Energy"] = myArray
myArray = myData16.loc[:,("Energy")].cumsum()
myData16["Energy Sum"] = myArray

myData19 = testData[testData["nodeID"]==19]
myData19["deltaT"] = myData19["Time since start"].diff()
myData19["Energy"] = myData19["deltaT"]*myData19["Power"]
myData19["Energy Sum"] = myData19["Energy"].cumsum()


fig, (ax1, ax3) = plt.subplots(2)

color = 'tab:red'
ax1.set_xlabel('Time since Start [s]')
ax1.set_ylabel('Instantaneous Power [Watts]', color=color)
ax1.plot(myData16["Time since start"],myData16["Power"], label="Power", color=color)
ax1.tick_params(axis='y', labelcolor=color)

ax2 = ax1.twinx()  # instantiate a second Axes that shares the same x-axis

color = 'tab:blue'
ax2.set_ylabel('Total Energy Used [J]', color=color)  # we already handled the x-label with ax1
ax2.plot(myData16["Time since start"], myData16["Energy Sum"], color=color)
ax2.tick_params(axis='y', labelcolor=color)

color = 'tab:red'
ax3.set_xlabel('Time since Start [s]')
ax3.set_ylabel('Instantaneous Power [Watts]', color=color)
ax3.plot(myData19["Time since start"],myData19["Power"], label="Power", color=color)
ax3.tick_params(axis='y', labelcolor=color)
ax4 = ax3.twinx()  # instantiate a second Axes that shares the same x-axis

color = 'tab:blue'
ax4.set_ylabel('Total Energy Used [J]', color=color)  # we already handled the x-label with ax1
ax4.plot(myData19["Time since start"], myData19["Energy Sum"], color=color)
ax4.tick_params(axis='y', labelcolor=color)

fig.tight_layout()  # otherwise the right y-label is slightly clipped
ax1.set_title("Power and Energy of Test Port Motor")
ax3.set_title("Power and Energy of Test Starboard Motor")
plt.show()





