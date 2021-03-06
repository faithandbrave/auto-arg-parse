#include <fstream>
#include <iostream>
#include "autoArgParse/argParser.h"
using namespace std;
using namespace AutoArgParse;

ArgParser argParser;

// Let's add an optional flag -p for power
auto& powerFlag =
    argParser.add<ComplexFlag>("-p", Policy::OPTIONAL, "Specify power output.");

// give -p a mandatory argument, an integer. Typesafe conversion will be used
auto& powerArg = powerFlag.add<Arg<int>>(
    "number_watts", Policy::MANDATORY,
    "An integer representing the number of watts.",
    chain(Converter<int>(), IntRange(0, 50, true, true)));

// Let's Add a mandatory flag speed
auto& speedFlag = argParser.add<ComplexFlag>("--speed", Policy::MANDATORY,
                                             "Specify the speed.");

// Add three exclusive flags, fast, medium slow
// no need to provide descriptions, these are self explanatory
auto& slow = speedFlag.add<Flag>("slow", Policy::MANDATORY, "");
auto& medium = speedFlag.add<Flag>("medium", Policy::MANDATORY, "");
auto& fast = speedFlag.add<Flag>("fast", Policy::MANDATORY, "");
auto forceExclusive = speedFlag.makeExclusive("slow", "medium", "fast");

auto& fileFlag = argParser.add<ComplexFlag>("--file", Policy::MANDATORY,
                                            "Read the specified file.");
auto& file = fileFlag.add<Arg<std::fstream>>(
    "file_path", Policy::MANDATORY, "Path to an existing file.",
    [](const std::string& arg, std::fstream& stream) {
        stream.open(arg);
        if (!stream.good()) {
            throw ErrorMessage("File " + arg + " does not exist.");
        }
    });
int main(const int argc, const char** argv) {
    argParser.validateArgs(argc, argv);
    // the above will print an error and exit if not all arguments were
    //     successfully parsed
    // otherwise, testing flags is easy

    if (powerFlag) {
        int power = powerArg.get();
        // already converted to integer
        cout << "Accepted power output of " << power << " W" << endl;
    }
    if (slow) {
        cout << "Running slowly." << endl;
    } else if (medium) {
        cout << "Running normally." << endl;
    } else if (fast) {
        cout << "running fast." << endl;
    }
}
