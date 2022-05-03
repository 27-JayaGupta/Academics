import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

iterations = 100000

#taking input
n = (int)(input("Number of samples generated(n): "))
m = (int)(input("Number of random variable(m): "))


#generating random sample
probs = [(float)(x) for x in input("Enter probabilities for 0 to m-1(enter m values separated by space, no need for the sum to be 1 , probabilities are noramlised in the next steps): ").split()]


#normalising probabilities if sum of probabilities is not 1 
#divinding each prbability by the sum of input probabilities received
sums = sum(probs)
prob = [x/sums for x in probs]
print(prob)


#calculating averages
averages = []
for i in range(0, iterations):   
    sample = []
    for j in range(0, n):
        sample.append(np.random.choice(np.arange(0, m), p = prob))
    average = (float)(sum(sample)/n)
    average = round(average, 1)   #rounding the average to one decimal position to preserve the nature of the graph
    averages.append(average)


#Plotting the graph
fig,ax = plt.subplots()
df = pd.DataFrame({"Numbers":averages})
df['Numbers'].value_counts().sort_index().plot(ax=ax, kind="bar")
plt.xlabel("Average Values across iterations")
plt.ylabel("Frequency")
plt.title("Frequency Distribution")
plt.show()