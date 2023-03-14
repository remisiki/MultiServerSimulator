import json
import os
import sys
import subprocess
import matplotlib.pyplot as plt
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
    arrivalRate: List[int] = None,
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
    elif (
      ((arrivalRate is not None) and (len(arrivalRate) != jobTypeCnt)) or
      ((serverNeeds is not None) and (len(serverNeeds) != jobTypeCnt))
    ):
      raise Exception("arrivalRate and serverNeeds should be of length jobTypeCnt")
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
    if (self.jobTypeCnt is not None):
      arrivalRate = ",".join([str(x) for x in self.arrivalRate])
      serverNeeds = ",".join([str(x) for x in self.serverNeeds])
      opts += " -j %d -l %s -s %s" % (self.jobTypeCnt, arrivalRate, serverNeeds)
    if (self.regionCnt is not None):
      serviceTime = ",".join([str(x) for x in self.serviceTime])
      opts += " -r %d -a %s" % (self.regionCnt, serviceTime)
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

def main():
  # NOTE When choosing parameters, always choose carefully and start from
  # testing one config with small values (big values for processorCnt).
  # Escpecially for serviceNeeds, the simulation time will blow up very fast
  # and never ends. When not sure, test one config using executable file with
  # -v option to see the iteration speed.
  # Here is an example to test four policies, each over different processor
  # numbers {28, 32, 36, ..., 64}.
  policies = ["fcfsLocal", "fcfsCross", "fcfsCrossPart", "o3CrossPart"]
  processorCnts = list(range(28, 65, 4))
  datas = []
  for policy in policies:
    configs = []
    for n in processorCnts:
      # Iteration 1000 is enough to check whether a number converges
      config = Config(processorCnt=n, policy=policy, iteration=1000)
      configs.append(config)
    datas.append(runSim(configs))
  # TODO Not sure how to represent parameters that are arrays (like server
  # needs), just put set number here that needs to be explained explicitly.
  plot(range(len(processorCnts)), datas, legends=policies, fileName="processorCnt.png")

if (__name__ == "__main__"):
  main()
