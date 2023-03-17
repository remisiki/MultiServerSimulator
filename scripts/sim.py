import json
import os
import sys
import subprocess
import matplotlib.pyplot as plt
import numpy as np
from typing import *

class Config:
  """A config object
  See README.md command line options of corresponding fields for details.
  """
  def __init__(
    self,
    policy: str = None,
    iteration: int = None,
    processorCnt: int = None,
    jobTypeCnt: int = None,
    arrivalRate: List[List[int]] = None,
    serverNeeds: List[int] = None,
    regionCnt: int = None,
    serviceTime: List[List[int]] = None
    ):
    self.policy = policy
    self.iteration = iteration
    self.processorCnt = processorCnt
    if (not (
      ((jobTypeCnt is not None) and (arrivalRate is not None) and (serverNeeds is not None)) or
      ((jobTypeCnt is None) and (arrivalRate is None) and (serverNeeds is None))
    )):
      raise Exception("arrivalRate and serverNeeds should be set together with jobTypeCnt")
    elif ((serverNeeds is not None) and (len(serverNeeds) != jobTypeCnt)):
      raise Exception("serverNeeds should be of length jobTypeCnt")
    elif (arrivalRate is not None):
      arrivalRate = [x for row in arrivalRate for x in row]
      if (
        ((regionCnt is None) and (len(arrivalRate) != 2*jobTypeCnt)) or
        ((regionCnt is not None) and (len(arrivalRate) != regionCnt*jobTypeCnt))
      ):
        raise Exception("arrivalRate should be of shape (regionCnt,jobTypeCnt)")
    self.jobTypeCnt = jobTypeCnt
    self.arrivalRate = arrivalRate
    self.serverNeeds = serverNeeds
    if (not (
      ((regionCnt is not None) and (serviceTime is not None)) or
      ((regionCnt is None) and (serviceTime is None))
    )):
      raise Exception("serviceTime should be set together with regionCnt")
    elif (serviceTime is not None):
      serviceTime = [x for row in serviceTime for x in row]
      if (len(serviceTime) != (regionCnt*regionCnt)):
        raise Exception("serviceTime should be of shape (regionCnt,regionCnt)")
    self.regionCnt = regionCnt
    self.serviceTime = serviceTime

  def toCommand(self) -> str:
    opts = ""
    if (self.processorCnt is not None):
      opts += " -n %d" % self.processorCnt
    if (self.policy is not None):
      opts += " -p %s" % self.policy
    if (self.iteration is not None):
      opts += " -t %d" % self.iteration
    if (self.regionCnt is not None):
      serviceTime = ",".join([str(x) for x in self.serviceTime])
      opts += " -r %d -a %s" % (self.regionCnt, serviceTime)
    if (self.jobTypeCnt is not None):
      arrivalRate = ",".join([str(x) for x in self.arrivalRate])
      serverNeeds = ",".join([str(x) for x in self.serverNeeds])
      opts += " -j %d -l %s -s %s" % (self.jobTypeCnt, arrivalRate, serverNeeds)
    return opts

def plot(
  x,
  ys,
  xLabel="Parameter set",
  yLabel="Average queue length",
  title="Plot",
  fileName="plot.png",
  legends=None
  ):
  fig = plt.figure(figsize=(12,9))
  plt.clf()
  for y in ys:
    plt.plot(x, y)
  plt.xlabel(xLabel)
  plt.ylabel(yLabel)
  plt.title(title)
  plt.legend(legends)
  plt.savefig(fileName)

def runSim(configs: List[Config]) -> List[float]:
  """Run a list of configs and return a list of results
  """
  executableFilePath = os.path.abspath(os.path.join(os.path.dirname(__file__), os.pardir, "sim"))
  if (not os.path.isfile(executableFilePath)):
    raise Exception("Executable file not found")
  data = []
  for config in configs:
    opts = config.toCommand()
    command = executableFilePath + opts
    print("Running %s" % command)
    process = subprocess.Popen([command], shell=True, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    output, err = process.communicate()
    if (process.returncode != 0):
      raise Exception("Error executing command `%s`" % command)
    else:
      data.append(float(output))
  return data

policies = ["fcfsLocal", "fcfsCross", "fcfsCrossPart", "o3CrossPart"]

def test1():
  # NOTE When choosing parameters, always choose carefully and start from
  # testing one config with small values (big values for processorCnt).
  # Escpecially for serviceNeeds, the simulation time will blow up very fast
  # and never ends. When not sure, test one config using executable file with
  # -v option to see the iteration speed.
  # Here is an example to test four policies, each over different processor
  # numbers {28, 32, 36, ..., 64}.
  processorCnts = list(range(28, 65, 4))
  datas = []
  for policy in policies:
    configs = []
    for n in processorCnts:
      # Iteration 1000 is enough to check whether a number converges
      config = Config(processorCnt=n, policy=policy, iteration=1000)
      configs.append(config)
    datas.append(runSim(configs))
  return processorCnts, datas

def test2():
  arrivalRates = list(range(1, 21, 1))
  datas = []
  for policy in policies:
    configs = []
    for l in arrivalRates:
      config = Config(policy=policy, iteration=1000, jobTypeCnt=2, arrivalRate=[[l, 4], [l, 4]], serverNeeds=[1, 4])
      configs.append(config)
    datas.append(runSim(configs))
  return arrivalRates, datas

def test3():
  arrivalRates = list(range(1, 10, 1))
  datas = []
  for policy in policies:
    configs = []
    for l in arrivalRates:
      config = Config(policy=policy, iteration=1000, jobTypeCnt=2, arrivalRate=[[10, l], [10, l]], serverNeeds=[1, 4])
      configs.append(config)
    datas.append(runSim(configs))
  return arrivalRates, datas

def test4():
  serverNeeds = list(range(1, 4, 1))
  datas = []
  for policy in policies:
    configs = []
    for s in serverNeeds:
      config = Config(policy=policy, iteration=1000, jobTypeCnt=2, arrivalRate=[10, 4], serverNeeds=[s, 4])
      configs.append(config)
    datas.append(runSim(configs))
  return serverNeeds, datas

def test5():
  serverNeeds = list(range(2, 5, 1))
  datas = []
  for policy in policies:
    configs = []
    for s in serverNeeds:
      config = Config(policy=policy, iteration=1000, jobTypeCnt=2, arrivalRate=[10, 4], serverNeeds=[1, s])
      configs.append(config)
    datas.append(runSim(configs))
  return serverNeeds, datas

def test6():
  serviceTimes = list(range(2, 11, 1))
  datas = []
  for policy in policies:
    configs = []
    for a in serviceTimes:
      config = Config(policy=policy, iteration=1000, regionCnt=2, serviceTime=[[1, a], [a, 1]])
      configs.append(config)
    datas.append(runSim(configs))
  return serviceTimes, datas

def test7():
  regionCnts = list(range(2, 15, 2))
  datas = []
  for policy in policies:
    configs = []
    for r in regionCnts:
      m = [[4]*r for i in range(r)]
      for i in range(r):
        m[i][i] = 1
      config = Config(policy=policy, iteration=1000, regionCnt=r, serviceTime=m)
      configs.append(config)
    datas.append(runSim(configs))
  return regionCnts, datas

def test8():
  loads = np.linspace(0.1, 0.99, 10).tolist()
  datas = []
  for policy in policies:
    configs = []
    for load in loads:
      arrivalRate = [10, 4]
      arrivalRate[1] = np.random.randint(1, 6)
      arrivalRate[0] = np.random.randint(arrivalRate[1], 20)
      serverNeeds = [1, 4]
      serverNeeds[0] = np.random.randint(1, 3)
      serverNeeds[1] = np.random.randint(serverNeeds[0]+1, 5)
      minNeed = arrivalRate[0]*serverNeeds[0] + arrivalRate[1]*serverNeeds[1]
      processorCnt = int(minNeed*load)
      config = Config(policy=policy, iteration=1000, processorCnt=processorCnt, jobTypeCnt=2, arrivalRate=arrivalRate, serverNeeds=serverNeeds)
      configs.append(config)
    datas.append(runSim(configs))
  return loads, datas

def test9():
  arrivalRates = list(range(10, 31, 1))
  datas = []
  for policy in policies:
    configs = []
    for l in arrivalRates:
      config = Config(policy=policy, iteration=1000, processorCnt=100, jobTypeCnt=2, arrivalRate=[[10, 10], [l, 5]], serverNeeds=[1, 7])
      configs.append(config)
    datas.append(runSim(configs))
  return arrivalRates, datas

def main():
  x, ys = test9()
  plot(x, ys, legends=policies, fileName="img/inbalancedArrivalRate.png", xLabel="Arrival rate of small jobs")

if (__name__ == "__main__"):
  main()
