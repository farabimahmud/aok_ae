import pandas as pd 
raw_logfile = "booksim/output.log"
output_datafile = "newdata.csv"
datapoints_to_collect = [0.0025, 0.05, 0.075, 0.0791666, 0.0812499, 0.0770833, 0.079166, 0.0875, 0.1]
discarded_set = set()
def main():
  datapoint = 0
  data = []
  with open(raw_logfile,'r') as f:
    for line in f.readlines():
      if line.startswith("Worst Case"):
        datapoint += 1
        line = line.strip()
        tokens = line.split(",")
        src_dest_ir = tokens[1].split('-')
        tokens = [int(x) for x in tokens[2:-1]]
        src = src_dest_ir[0]
        dest = src_dest_ir[1]
        ir = float(src_dest_ir[2][:-1])
        if ir in datapoints_to_collect:
          tokens = [ir] + tokens
          # print(tokens)
          data.append(tokens)
        else:
          # print(ir) 
          discarded_set.add(ir)
  print(discarded_set)
  df = pd.DataFrame(data)
  df.to_csv(output_datafile, index=None,header=None)


if __name__ == "__main__":
  main()