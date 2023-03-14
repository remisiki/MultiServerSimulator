import json
import os
import sys
import subprocess
import matplotlib.pyplot as plt

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

def runSim(
  configFilePath,
  policy = "fcfsLocal",
  iteration = 1000
  ):
  """Run simulation for all sets in the config file with a single policy
  @param configFilePath Path to config file
  @param policy Policy name, @see README.md -p option
  @param iteration Iteration count, @see README.md -n option. 1000 should be
  enough to check whether a number converges
  @return A list of simulation results
  """

  executableFilePath = os.path.abspath(os.path.join(os.path.dirname(__file__), os.pardir, "sim"))
  if (not os.path.isfile(executableFilePath)):
    raise Exception("Executable file not found")
  data = []

  with open(configFilePath, "r") as f:
    configs = json.load(f)
  for config in configs:
    opts = ""
    if (("processor" in config) and (config["processor"])):
      opts += " -n %d" % config["processor"]
    if (("jobTypeCnt" in config) and (config["jobTypeCnt"])):
      if (("arrivalRate" not in config) or ("serverNeeds" not in config)):
        raise Exception("arrivalRate and serverNeeds should be set together with jobTypeCnt")
      if ((len(config["arrivalRate"]) != config["jobTypeCnt"]) or (len(config["serverNeeds"]) != config["jobTypeCnt"])):
        raise Exception("arrivalRate and serverNeeds should be of length jobTypeCnt")
      arrivalRate = ",".join([str(x) for x in config["arrivalRate"]])
      serverNeeds = ",".join([str(x) for x in config["serverNeeds"]])
      opts += " -j %d -l %s -s %s" % (config["jobTypeCnt"], arrivalRate, serverNeeds)
    if (("regionCnt" in config) and (config["regionCnt"])):
      if ("serviceTime" not in config):
        raise Exception("serviceTime should be set together with regionCnt")
      serviceTime = [x for row in config["serviceTime"] for x in row]
      if (len(serviceTime) != (config["regionCnt"]*config["regionCnt"])):
        raise Exception("serviceTime should be of shape (regionCnt,regionCnt)")
      serviceTime = ",".join([str(x) for x in serviceTime])
      opts += " -r %d -a %s" % (config["regionCnt"], serviceTime)
    command = "%s%s -t %d -p %s" % (executableFilePath, opts, iteration, policy)
    print("Running %s" % command)
    process = subprocess.Popen([command], shell=True, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    output, err = process.communicate()
    if (process.returncode != 0):
      raise Exception("Error executing command `%s`" % command)
    else:
      data.append(float(output))

  return data

def main():
  # Config file path here
  configFilePath = os.path.abspath(os.path.join(os.path.dirname(__file__), "processorCnt.json"))
  # Run config file for each policy, modify to compare part of them
  policies = ["fcfsLocal", "fcfsCross", "fcfsCrossPart", "o3CrossPart"]
  datas = []
  for policy in policies:
    datas.append(runSim(configFilePath, policy))
  # TODO Not sure how to represent parameters that are arrays (like server
  # needs), just put set number here that needs to be explained explicitly.
  plot(range(10), datas, legends=policies, fileName="processorCnt.png")

if (__name__ == "__main__"):
  main()
