#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <cmath>
#include <iomanip>

#include "StopClock.hpp"
#include <sys/resource.h>
#include <sys/time.h>

#include "include/SquirtleFilter.h"

// Source: https://github.com/JensUweUlrich/ReadBouncer/blob/master/src/main/main.cpp
double cputime(void)
{
	struct rusage r;
	getrusage(RUSAGE_SELF, &r);
	return r.ru_utime.tv_sec + r.ru_stime.tv_sec + 1e-6 * (r.ru_utime.tv_usec + r.ru_stime.tv_usec);
}

/**
 * Returns the peak (maximum so far) resident set size (physical
 * memory use) measured in bytes, or zero if the value cannot be
 * determined on this OS.
 */
long getPeakRSS(void)
{
	struct rusage r;
	getrusage(RUSAGE_SELF, &r);
	return r.ru_maxrss * 1024;
}

uint64_t computeSingleBFSize(uint64_t numberOfElements, double falsePositiveRate, uint8_t numberOfHashFunctions){

    uint64_t BinSizeBits = ceil(-1 / (pow(1 - pow((double) falsePositiveRate, 1.0 / (double) numberOfHashFunctions), 1.0 / ((double) (numberOfHashFunctions * numberOfElements))) - 1));
    return BinSizeBits;
}


void runTest(){

    StopClock squirtleFilterInsetionCheck;
    squirtleFilterInsetionCheck.start();

    std::cout << "[INFO] Running insertion test..." << '\n';
    std::cerr << "[INFO-DEV] C++ version: " << __cplusplus << std::endl;
    uint64_t numberOfTestingElements {200000000};
    uint8_t numberOfHashFunctions {5};
    double falsePositiveRate {0.001};
    auto BFMB = (double)(computeSingleBFSize(numberOfHashFunctions, falsePositiveRate, numberOfHashFunctions) * 1)/ static_cast< double >( 8388608u );
    auto BFGB = BFMB / 1024.0;

    std::cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<"<< std::endl;
    std::cout << "[INFO-DEV] Bloom Filter Stat   : "<< std::endl;
    std::cout << "Max Elements size                 : " << unsigned(numberOfTestingElements) << std::endl;
    std::cout << "BF size in MB                 : " <<  BFMB << " MBytes"<< std::endl;
    std::cout << "BF size in GB                 : " << std::fixed << std::setprecision(2) << BFGB << " GBytes" << std::endl;
    std::cout << "Number of hash functions       : " << unsigned(numberOfHashFunctions) << std::endl;
    std::cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<"<< std::endl;

    BloomFilter bf((numberOfTestingElements), falsePositiveRate, numberOfHashFunctions);
    bf.insert("MAKQSMKAREVKRVALADKYFAKRAELKAIISDVNASDEDRWNAVLKLQTLPRDSSPSRQRNRCRQTGRPHGFLRKFGLSRIKV");
    bf.insert("KAIISDVNASDEDRWNAVLKLQTLPRDSSPSRQRNRCRQT");
    bf.insert("RQTGRPHGFLRKFGLS");
    bf.insert("FVNILM[UNIMOD:35]VDG");
    bf.insert("FVNILMMXMVDG");
    /*
    @solved!
    BF will not acccept any modifications style! possible input would be to convert as marker:
    Like adding MXM for [UNIMOD:35]
    FVNILM[UNIMOD:35]VDG --> FVNILMXMVDG
    */
    bf.insert("GSELLAKFVNILMMXMVDGKKSTAESIVYSALETLAQRSGKS");

    std::cout << "\"FVNILM[UNIMOD:35]VDG\" -> " << (bf.contains("FVNILM[UNIMOD:35]VDG") ? "possibly in set" : "not in set") << std::endl;
    //std::cout << "\"FVNILMMXMVDG\" -> " << (bf.contains("FVNILMMXMVDG") ? "possibly in set" : "not in set") << std::endl;
    std::cout << "\"RQTGRPHGFLRKFGLL\" -> " << (bf.contains("RQTGRPHGFLRKFGLL") ? "possibly in set" : "not in set") << std::endl; // Last letter is an mutation

    for (int i = 0; i < numberOfTestingElements; ++i) {
        std::string key = "RQTGRPHGFLRKFGL" + std::to_string(i);
        bf.insert(key);
    }
    std::cout << "Inserted additional elements: " << numberOfTestingElements << std::endl;
    std::cout << "\"RQTGRPHGFLRKFGLL100\" -> " << (bf.contains("RQTGRPHGFLRKFGLL100") ? "possibly in set" : "not in set") << std::endl;
    std::cout << "\"RQTGRPHGFLRKFGL100\" -> " << (bf.contains("RQTGRPHGFLRKFGL100") ? "possibly in set" : "not in set") << std::endl;
    std::cout << "\"nonexistent\" -> " << (bf.contains("nonexistent") ? "possibly in set" : "not in set") << std::endl;
    squirtleFilterInsetionCheck.end();

    size_t peakSize = getPeakRSS();
	int peakSizeMByte = (int)(peakSize / (1024 * 1024));
    std::cout << '\n';
	std::cout << "*********************** SquirtleFilter Insetion Usage Report ******" << std::endl;
	std::cout << "* Real time : " << squirtleFilterInsetionCheck.elapsed() << " sec         " << std::endl;
	std::cout << "* CPU time  : " << cputime() << " sec                    " << std::endl;
	std::cout << "* Peak RSS  : " << peakSizeMByte << " MByte              " << std::endl;
	std::cout << "**********************************************************" << std::endl;


    std::cout << "[INFO] Running lookup tests..." << '\n';
    StopClock squirtleFilterLookupCheck;
    squirtleFilterLookupCheck.start();

    uint64_t positiveCases = 0;
    uint64_t negativeCases = 0;

    for (uint64_t i = 0; i < numberOfTestingElements; ++i) {
        std::string key = "RQTGRPHGFLRKFGL" + std::to_string(i);
        if (bf.contains(key)) {
            ++positiveCases;
        } else {
            ++negativeCases;
        }
    }

    for (uint64_t i = 0; i < numberOfTestingElements/2; ++i) {
        std::string key = "RQTGRPHFLRKFGL" + std::to_string(i); // one aa missed #RQTSGRPHFLRKFGL
        if (bf.contains(key)) {
            ++positiveCases;
        } else {
            ++negativeCases;
        }
    }


    uint64_t totalLookups = positiveCases + negativeCases;

    std::cout << "[LOOKUP TEST] Total: " << totalLookups
              << ", Positives: " << positiveCases
              << ", Negatives: " << negativeCases
              << std::endl;

    
    squirtleFilterLookupCheck.end();
    size_t peakSizeLookup = getPeakRSS();
	int peakSizeMByteLookup = (int)(peakSizeLookup / (1024 * 1024));
    std::cout << '\n';
	std::cout << "*********************** SquirtleFilter Lookup Usage Report ******" << std::endl;
	std::cout << "* Real time : " << squirtleFilterLookupCheck.elapsed() << " sec         " << std::endl;
	std::cout << "* CPU time  : " << cputime() << " sec                    " << std::endl;
	std::cout << "* Peak RSS  : " << peakSizeMByteLookup << " MByte              " << std::endl;
	std::cout << "**********************************************************" << std::endl;
}

void runTestHash(){

    StopClock squirtleFilterInsetionCheck;
    squirtleFilterInsetionCheck.start();

    std::cout << "[INFO] Running insertion test..." << '\n';
    std::cerr << "[INFO-DEV] C++ version: " << __cplusplus << std::endl;
    uint64_t numberOfTestingElements {200000000};
    uint8_t numberOfHashFunctions {5};
    double falsePositiveRate {0.001};
    auto BFMB = (double)(computeSingleBFSize(numberOfHashFunctions, falsePositiveRate, numberOfHashFunctions) * 1)/ static_cast< double >( 8388608u );
    auto BFGB = BFMB / 1024.0;

    std::cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<"<< std::endl;
    std::cout << "[INFO-DEV] Bloom Filter Stat   : "<< std::endl;
    std::cout << "Max Elements size                 : " << unsigned(numberOfTestingElements) << std::endl;
    std::cout << "BF size in MB                 : " <<  BFMB << " MBytes"<< std::endl;
    std::cout << "BF size in GB                 : " << std::fixed << std::setprecision(2) << BFGB << " GBytes" << std::endl;
    std::cout << "Number of hash functions       : " << unsigned(numberOfHashFunctions) << std::endl;
    std::cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<"<< std::endl;

    BloomFilter bf((numberOfTestingElements), falsePositiveRate, numberOfHashFunctions);
    bf.insert("MAKQSMKAREVKRVALADKYFAKRAELKAIISDVNASDEDRWNAVLKLQTLPRDSSPSRQRNRCRQTGRPHGFLRKFGLSRIKV");
    bf.insert("KAIISDVNASDEDRWNAVLKLQTLPRDSSPSRQRNRCRQT");
    bf.insert("RQTGRPHGFLRKFGLS");
    bf.insert("FVNILM[UNIMOD:35]VDG");
    bf.insert("FVNILMMXMVDG");
    /*
    @solved!
    BF will not acccept any modifications style! possible input would be to convert as marker:
    Like adding MXM for [UNIMOD:35]
    FVNILM[UNIMOD:35]VDG --> FVNILMXMVDG
    */
    bf.insert("GSELLAKFVNILMMXMVDGKKSTAESIVYSALETLAQRSGKS");

    std::cout << "\"FVNILM[UNIMOD:35]VDG\" -> " << (bf.contains("FVNILM[UNIMOD:35]VDG") ? "possibly in set" : "not in set") << std::endl;
    //std::cout << "\"FVNILMMXMVDG\" -> " << (bf.contains("FVNILMMXMVDG") ? "possibly in set" : "not in set") << std::endl;
    std::cout << "\"RQTGRPHGFLRKFGLL\" -> " << (bf.contains("RQTGRPHGFLRKFGLL") ? "possibly in set" : "not in set") << std::endl; // Last letter is an mutation

    for (int i = 0; i < numberOfTestingElements; ++i) {
        std::string key = "RQTGRPHGFLRKFGL" + std::to_string(i);
        bf.insert(key);
    }
    std::cout << "Inserted additional elements: " << numberOfTestingElements << std::endl;
    std::cout << "\"RQTGRPHGFLRKFGLL100\" -> " << (bf.contains("RQTGRPHGFLRKFGLL100") ? "possibly in set" : "not in set") << std::endl;
    std::cout << "\"RQTGRPHGFLRKFGL100\" -> " << (bf.contains("RQTGRPHGFLRKFGL100") ? "possibly in set" : "not in set") << std::endl;
    std::cout << "\"nonexistent\" -> " << (bf.contains("nonexistent") ? "possibly in set" : "not in set") << std::endl;
    squirtleFilterInsetionCheck.end();

    size_t peakSize = getPeakRSS();
	int peakSizeMByte = (int)(peakSize / (1024 * 1024));
    std::cout << '\n';
	std::cout << "*********************** SquirtleFilter Insetion Usage Report ******" << std::endl;
	std::cout << "* Real time : " << squirtleFilterInsetionCheck.elapsed() << " sec         " << std::endl;
	std::cout << "* CPU time  : " << cputime() << " sec                    " << std::endl;
	std::cout << "* Peak RSS  : " << peakSizeMByte << " MByte              " << std::endl;
	std::cout << "**********************************************************" << std::endl;


    std::cout << "[INFO] Running lookup tests..." << '\n';
    StopClock squirtleFilterLookupCheck;
    squirtleFilterLookupCheck.start();

    uint64_t positiveCases = 0;
    uint64_t negativeCases = 0;

    for (uint64_t i = 0; i < numberOfTestingElements; ++i) {
        std::string key = "RQTGRPHGFLRKFGL" + std::to_string(i);
        if (bf.contains(key)) {
            ++positiveCases;
        } else {
            ++negativeCases;
        }
    }

    for (uint64_t i = 0; i < numberOfTestingElements/2; ++i) {
        std::string key = "RQTGRPHFLRKFGL" + std::to_string(i); // one aa missed #RQTSGRPHFLRKFGL
        if (bf.contains(key)) {
            ++positiveCases;
        } else {
            ++negativeCases;
        }
    }


    uint64_t totalLookups = positiveCases + negativeCases;

    std::cout << "[LOOKUP TEST] Total: " << totalLookups
              << ", Positives: " << positiveCases
              << ", Negatives: " << negativeCases
              << std::endl;

    
    squirtleFilterLookupCheck.end();
    size_t peakSizeLookup = getPeakRSS();
	int peakSizeMByteLookup = (int)(peakSizeLookup / (1024 * 1024));
    std::cout << '\n';
	std::cout << "*********************** SquirtleFilter Lookup Usage Report ******" << std::endl;
	std::cout << "* Real time : " << squirtleFilterLookupCheck.elapsed() << " sec         " << std::endl;
	std::cout << "* CPU time  : " << cputime() << " sec                    " << std::endl;
	std::cout << "* Peak RSS  : " << peakSizeMByteLookup << " MByte              " << std::endl;
	std::cout << "**********************************************************" << std::endl;
}

std::string roundToStr(double value, int decimals = 3) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(decimals) << value;
    return oss.str();
}

void ppmRoundedTest() {
    BloomFilter bf(100, 0.001, 5);

    // Insert rounded values as strings
    std::vector<double> inserted = {
        52344.2098,  // will be rounded
        52344.5000,
        60000.1234
    };

    for (double mass : inserted) {
        std::string rounded = roundToStr(mass, 3); // round to 3 decimals
        bf.insert(rounded);
        std::cout << "[INSERTED] " << rounded << std::endl;
    }

    // Search mass with 10 ppm tolerance
    double search_mass = 52344.2101;
    std::string expected = roundToStr(search_mass, 3);
    double ppm = 10.0;
    double tolerance = search_mass * ppm / 1e6;
    double step = 0.001;

    std::cout << "\n[INFO] Searching around: " << search_mass << " ± " << tolerance << " Da (rounded)\n";
    bool match_found = false;

    for (double delta = -tolerance; delta <= tolerance; delta += step) {
        double probe_mass = search_mass + delta;
        std::string probe = roundToStr(probe_mass, 3);
        //if (bf.contains(expected)) {
        //    std::cout << "[CONFIRMED] Exact rounded match: " << expected << '\n';
        //}
        if (bf.contains(probe) && probe == expected) {
            std::cout << " Match found at (rounded): " << probe << '\n';
            match_found = true;
        }
    }

    if (!match_found)
        std::cout << "No match found within tolerance.\n";

    // Negative test
    std::string bad_mass = roundToStr(99999.9999, 3);
    std::cout << "\nNegative test: " << bad_mass << " -> "
              << (bf.contains(bad_mass) ? "possibly in set" : "not in set") << std::endl;
}

void ppmDoubleTest() {
    BloomFilter bf(100, 0.001, 5);
    // Insert some exact fragment masses (as double)
    std::vector<double> inserted = {
        52344.2098,  // within 10 ppm
        52344.5000,  // outside 10 ppm
        60000.1234   // unrelated
    };

    for (double mass : inserted)
        bf.insert(mass);

    // Observed fragment mass
    double search_mass = 52344.2101;
    double ppm = 10.0;

    double tolerance = search_mass * ppm / 1e6; // 10 ppm window
    double step = 0.001; // scan resolution

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "[INFO] Searching around: " << search_mass << " ± " << tolerance << " Da\n";

    bool match_found = false;
    for (double delta = -tolerance; delta <= tolerance; delta += step) {
        double probe = search_mass + delta;
        if (bf.contains(probe)) {
            std::cout << " Match found at " << probe << "\n";
            match_found = true;
        }
    }

    if (!match_found)
        std::cout << "  No match found within tolerance.\n";

    // Negative test
    double bad_mass = 99999.999;
    std::cout << "\nNegative test: " << bad_mass << " -> "
              << (bf.contains(bad_mass) ? "possibly in set" : "not in set") << std::endl;
}

int main() {
    
    StopClock squirtleFilterUsageCheck;
    squirtleFilterUsageCheck.start();
    //runTest();
    ppmDoubleTest();
    std::cout << "Rounding test!" << std::endl;
    ppmRoundedTest();
    squirtleFilterUsageCheck.end();

    size_t peakSize = getPeakRSS();
	int peakSizeMByte = (int)(peakSize / (1024 * 1024));

	std::filesystem::path bin = std::filesystem::current_path();
	const std::string memoryUsageReport = bin.string() + "/memory.txt";

    std::ofstream memoryLog(memoryUsageReport);
	memoryLog << "*********************** SquirtleFilter Complete Usage Report ******" << '\n';
	memoryLog << "* Real time : " << squirtleFilterUsageCheck.elapsed() << " sec         " << '\n';
	memoryLog << "* CPU time  : " << cputime() << " sec                    " << '\n';
	memoryLog << "* Peak RSS  : " << peakSizeMByte << " MByte              " << '\n';
	memoryLog << "**********************************************************" << '\n';

	std::cout << '\n';
	std::cout << "*********************** SquirtleFilter Complete Usage Report ******" << std::endl;
	std::cout << "* Real time : " << squirtleFilterUsageCheck.elapsed() << " sec         " << std::endl;
	std::cout << "* CPU time  : " << cputime() << " sec                    " << std::endl;
	std::cout << "* Peak RSS  : " << peakSizeMByte << " MByte              " << std::endl;
	std::cout << "**********************************************************" << std::endl;

    return 0;
}
