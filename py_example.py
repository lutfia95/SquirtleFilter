import sys
import resource
sys.path.append("./build")
import squirtlefilter
import time
import psutil
import os

def get_memory_mb():
    process = psutil.Process(os.getpid())
    return process.memory_info().rss / (1024 * 1024)

def run_test():
    numberOfTestingElements = 200_000_000
    numberOfHashFunctions = 5
    falsePositiveRate = 0.001

    print("[INFO] Running insertion test...")
    print(f"[INFO] Python Bloom Filter Stat:")
    print(f"Max Elements size: {numberOfTestingElements}")
    print(f"Number of hash functions: {numberOfHashFunctions}")
    print(f"False positive rate: {falsePositiveRate}")

    bf = squirtlefilter.SquirtleFilter(numberOfTestingElements, falsePositiveRate, numberOfHashFunctions)

    #start_mem = get_memory_mb()
    start_mem = resource.getrusage(resource.RUSAGE_SELF).ru_maxrss
    start_time = time.time()

    bf.insert("MAKQSMKAREVKRVALADKYFAKRAELKAIISDVNASDEDRWNAVLKLQTLPRDSSPSRQRNRCRQTGRPHGFLRKFGLSRIKV")
    bf.insert("KAIISDVNASDEDRWNAVLKLQTLPRDSSPSRQRNRCRQT")
    bf.insert("RQTGRPHGFLRKFGLS")
    bf.insert("FVNILM[UNIMOD:35]VDG")
    bf.insert("FVNILMMXMVDG")
    bf.insert("GSELLAKFVNILMMXMVDGKKSTAESIVYSALETLAQRSGKS")

    for i in range(numberOfTestingElements):
        bf.insert(f"RQTGRPHGFLRKFGL{i}")

    end_time = time.time()
    #end_mem = get_memory_mb()
    end_mem = resource.getrusage(resource.RUSAGE_SELF).ru_maxrss
    print(f"[INFO] Insertion Done. Time: {end_time - start_time:.2f}s, Peak RSS (approx): {end_mem - start_mem:.2f} MB")

    print("[INFO] Running lookup test...")
    start_time = time.time()
    positives = 0
    negatives = 0
    start_mem = resource.getrusage(resource.RUSAGE_SELF).ru_maxrss
    for i in range(numberOfTestingElements):
        if bf.contains(f"RQTGRPHGFLRKFGL{i}"):
            positives += 1
        else:
            negatives += 1

    for i in range(numberOfTestingElements // 2):
        if bf.contains(f"RQTGRPHFLRKFGL{i}"):
            positives += 1
        else:
            negatives += 1

    end_time = time.time()
    end_mem = resource.getrusage(resource.RUSAGE_SELF).ru_maxrss
    print(f"[LOOKUP TEST] Total: {positives + negatives}, Positives: {positives}, Negatives: {negatives}")
    print(f"[INFO] Lookup Done. Time: {end_time - start_time:.2f}s, Peak RSS (approx): {end_mem - start_mem:.2f} MB")

if __name__ == "__main__":
    
    run_test()
