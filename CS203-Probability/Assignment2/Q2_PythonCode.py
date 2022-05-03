import random
import matplotlib.pyplot as plt
import math

# Depending on the denseness of the graph, you can change step parameter to decide how many times(k) you want to generate sample.
# In answer sheet, the graph plotted is for step = 1 for mean and step = 2 for median.Thats why we get dense graph

def mean_sampler(N):
  nhash_k = []
  step = 10 #define how many times you want to sample
  for k in range(1,1001,step):
      sample = []
      for i in range(0,k):
          sample.append(random.randint(1,N+1))
      
      #calculate mean
      mean = sum(sample)/k
      n_hash = 2*mean -1 # calculate n from mean
      nhash_k.append(n_hash)

  print("N: ",N)
  x = [i for i in range(1,1001,step)]
  y = nhash_k
  plt.plot(x,y,label="Mean")
  plt.legend()
  # plt.show()
  return

def median_sampler(N):
  if(N%2==0):
    N += 1
  
  nhash_k = []
  r = 1001
  step =10
  print("N: " ,N)
  for k in range(1,r,step):
    sample = []
    for i in range(0,k):
      sample.append(random.randint(1,N+1))
    sample.sort()
    median = sample[int((len(sample)-1)/2)]
    n_hash = 2*median -1
    nhash_k.append(n_hash)
 
  x = [i for i in range(1,r,step)]
  y = nhash_k
  plt.plot(x,y,label="Median")
  plt.axhline(y=N, color='black', linestyle='-',label =N)
  plt.legend()
  plt.show()
  return

for j in range(0,3):
  N = random.randint(10000,1000000)
  mean_sampler(N)
  median_sampler(N)