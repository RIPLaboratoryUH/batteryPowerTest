import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import math

testData = pd.read_csv('testData.csv',skiprows=0)
testData["Power_M"] = testData["Torque estimate [NM]"] * 2 * math.pi * testData["Vel estimate [rev/s]"]
testData["Power_M Avg"] = testData["Power_M"].rolling(window=2).mean()
testData["Power_M"] = testData["Power_M Avg"] 
testData["Power_E"] = testData["VBUS DC Current [A]"]*testData["VBUS DC Voltage [V]"]
testData["Power Avg"] = testData["Power_E"].rolling(window=2).mean()
testData["Power_E"] = testData["Power Avg"]


testData["Time since start"] = (testData["Time Since Start [microseconds]"] / 1000000 - 10)
indexPower_M = testData[ (testData['Power_M'] < 0) ].index
testData.drop(indexPower_M, inplace=True)
indexPower_E = testData[ (testData['Power_E'] < 0) ].index
testData.drop(indexPower_E, inplace=True)
#testData.head(20)

myData16 = testData[testData["nodeID"]==16]
myArray = myData16.loc[:,("Time since start")].diff()
myData16.loc[:,("deltaT")] = myArray
myArray = myData16.loc[:,("deltaT")] * myData16.loc[:,("Power_M")]
myData16["Energy"] = myArray
myArray = myData16.loc[:,("Energy")].cumsum()
myData16["Energy Sum"] = myArray

myDataE = testData[testData["nodeID"]==16]
myArray = myDataE.loc[:,("Time since start")].diff()
myDataE.loc[:,("deltaT")] = myArray
myArray = myDataE.loc[:,("deltaT")] * myDataE.loc[:,("Power_E")]
myDataE["Energy"] = myArray
myArray = myDataE.loc[:,("Energy")].cumsum()
myDataE["Energy Sum"] = myArray

#myData19 = testData[testData["nodeID"]==19]
#myData19["deltaT"] = myData19["Time since start"].diff()
#myData19["Energy"] = myData19["deltaT"]*myData19["Power"]
#myData19["Energy Sum"] = myData19["Energy"].cumsum()


fig, (ax1, ax3) = plt.subplots(2)

color = 'tab:red'
ax1.set_xlabel('Time since Start [s]')
ax1.set_ylabel('Instantaneous Power [Watts]', color=color)
ax1.plot(myData16["Time since start"],myData16["Power_M"], label="Power_M", color=color)
ax1.tick_params(axis='y', labelcolor=color)

ax2 = ax1.twinx()  # instantiate a second Axes that shares the same x-axis

color = 'tab:blue'
ax2.set_ylabel('Total Energy Used [J]', color=color)  # we already handled the x-label with ax1
ax2.plot(myData16["Time since start"], myData16["Energy Sum"], color=color)
ax2.tick_params(axis='y', labelcolor=color)

color = 'tab:red'
ax3.set_xlabel('Time since Start [s]')
ax3.set_ylabel('Instantaneous Power [Watts]', color=color)
ax3.plot(myDataE["Time since start"],myDataE["Power_E"], label="Power_E", color=color)
ax3.tick_params(axis='y', labelcolor=color)
ax4 = ax3.twinx()  # instantiate a second Axes that shares the same x-axis

color = 'tab:blue'
ax4.set_ylabel('Total Energy Used [J]', color=color)  # we already handled the x-label with ax1
ax4.plot(myDataE["Time since start"], myDataE["Energy Sum"], color=color)
ax4.tick_params(axis='y', labelcolor=color)

fig.tight_layout()  # otherwise the right y-label is slightly clipped
ax1.set_title("Power and Energy of Port Motor (Mechanical)")
ax3.set_title("Power and Energy of Port Motor (Electrical)")
plt.show()





