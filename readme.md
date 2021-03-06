# Auto Arg Parse 
# Fast and easy C++ argument parser 

* Automatic validation of program arguments,
*  Automatic generation of usage instructions,
*  Type safe conversion from string input to desired type.

## Features
*  Validation of mandatory and optional arguments,
*  Validation of mandatory and optional flags,
*  Arbitrary nesting, flags may themselves have mandatory and optional flags or arguments,
* Type safe, (built in and  user defined) conversions from string input to required type,
*  Add constraints to arguments,
*  Make a set of flags exclusive,


## Coming soon

*  Variable number of arguments,
*  More built in conversions (other number formats, file path->stream conversion, etc.)


#Example Usage

## Setting up:
### Code:

```c++
#include <iostream>
#include "autoArgParse/argParser.h"
using namespace std;
using namespace AutoArgParse;

ArgParser argParser;
//declare flags and args here

int main(const int argc, const char** argv) {
//or here 
//then 
    argParser.validateArgs(argc, argv);
}
```

### Output:
If all flags and args are validated, proceed as normal, otherwise,an error is printed with the usage information (see below).

## Mandatory flag
### Code:
```c++
// Let's Add a mandatory flag speed
auto& speedFlag = argParser.add<Flag>("--speed", Policy::MANDATORY,
                                             "Specify the speed limit.");
```
### Output:

```
$./testProg 
Error: Missing mandatory argument(s). valid option(s) are:  --speed
Successfully parsed:  ./testProg
...
```

## Optional flag 
### Code:
````c++
// Let's add an optional flag -p for power
auto& powerFlag =
    argParser.add<Flag>("-p", Policy::OPTIONAL, "Use power.");
```

### Output:
```
$./testProg  -a
Error: Unexpected argument: -a
Valid option(s):  [-p]
...
$./testProg 
okay, -p is optional
$./testProg  -p
Okay, -p is optional and has been used
```

## Nesting, flags may themselves take flags and arguments
Give -p a mandatory integer argument, type safe conversion from string to int will automatically be used.

### Code:
```c++
// Let's add an optional flag -p for power.
//This time, -p is of type ComplexFlag.
//Allows nesting
auto& powerFlag =
    argParser.add<ComplexFlag>("-p", Policy::OPTIONAL, "Specify power output.");

// give -p a mandatory argument, an integer. Type safe conversion will be used
auto& powerArg =
    powerFlag.add<Arg<int>>("number_watts", Policy::MANDATORY,
                            "An integer representing the number of watts.");

```

### Output:
```
$./testProg -p 
Error: Missing mandatory argument(s).  Valid option(s) are:  number_watts
Successfully parsed:  ./testProg -p
...
$./testProg -p fudge
Error: Could not parse argument: number_watts
Could not interpret "fudge" as an integer.
...
```

## Add constraint to argument:
Power must be in range 0-50 inclusive.

## Code: modified from above
```c++

// Let's add an optional flag -p for power
auto& powerFlag =
    argParser.add<ComplexFlag>("-p", Policy::OPTIONAL, "Specify power output.");

// give -p a mandatory argument, an integer. Typesafe conversion will be used
auto& powerArg = powerFlag.add<Arg<int>>(
    "number_watts", Policy::MANDATORY,
    "An integer representing the number of watts in the range 0..50.",
    chain(Converter<int>(), IntRange(0, 50, true, true)));
```

### Output:
```
$./testProg -p 55 
Error: Could not parse argument: number_watts
Expected value to be between 0(inclusive) and 50(inclusive).
...
```

## Exclusive flags:

### Code:
```c++
auto& speedFlag = argParser.add<ComplexFlag>("--speed", Policy::MANDATORY,
                                             "Specify the speed.");

// Add three exclusive flags, fast, medium slow
// no need to provide descriptions, these are self explanatory
auto& slow = speedFlag.add<Flag>("slow", Policy::MANDATORY, "");
auto& medium = speedFlag.add<Flag>("medium", Policy::MANDATORY, "");
auto& fast = speedFlag.add<Flag>("fast", Policy::MANDATORY, "");
auto forceExclusive = speedFlag.makeExclusive("slow", "medium", "fast");
```

### Output:
```
$ ./testProg --speed 
Error: Missing mandatory argument(s). valid option(s) are:  slow, medium, fast
Successfully parsed:  ./testProg --speed
...
$ ./testProg --speed fast slow
Error: The following arguments are exclusive and may not be used in conjunction: fast|medium|slow
Successfully parsed:  ./testProg --speed
...
```

## Add user defined constraint to argument
### Code:
```

auto& fileFlag = argParser.add<ComplexFlag>("--file", Policy::MANDATORY,
                                            "Read the specified file.");
//passing a lambda function which validates the argument.
auto& file = fileFlag.add<Arg<std::fstream>>(
    "file_path", Policy::MANDATORY, "Path to an existing file.",
    [](const std::string& arg, std::fstream& stream) {
        stream.open(arg);
        if (!stream.good()) {
            throw ErrorMessage("File " + arg + " does not exist.");
        }
    });

```

### Output:
```
$ ./testProg  --file fudge_file 
Error: Could not parse argument: file_path
File fudge_file does not exist.

Successfully parsed:  ./testProg --file
...
```

## Usage information:
### Code:
If an error is reported, the usage information is printed out.  Otherwise, the information can be manually printed:
```c++
int main(const int argc, const char** argv) {
    argParser.printAllUsageInfo(cout, argv[0]);
}
```

### Output:
Taken from code src/exampleUsage.cpp
```
Usage: ./testProg [-p number_watts]  --speed slow|medium|fast  

Arguments:
    -p number_watts
    [optional] Specify power output.
            number_watts: An integer representing the number of watts.
        --speed slow|medium|fast 
    Specify the speed.
            
```

## Testing optional flags after validation:
### Code:
All flags and arguments are implicitly convertible to bool (true=parsed, false otherwise).  This can be used to test whether an optional flag has been parsed.
```c++
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

```


# Implementation FAQ:


##Speed:
The aim has been to make the validation of correct input fast.  Sometimes, this leads to slower error reporting. However, since finding an error usually leads to the program exiting, preference is given to speeding up the validating of valid input.  It is however still very doubtful that any speed differences will ever be noticed.

## Memory management:

*  As long as the `ArgParser` object is in scope, all flags and arguments shall remain constructed.  
* You should __never__ need to copy or copy-initialise a flag or argument object. Only maintain references where possible.  e.g. `auto& arg = ...` not `auto arg = ...`  
* You need not maintain a reference to a `flag`, `ComplexFlag` or  `Arg` object unless you wish to query its status. e.g. 
*     Test if an optional flag has been used,
*     Retrieve the value given as an argument.
*  This is therefore legal and sometimes recommended. 
    ```
    auto& someArg = argParser.add<ComplexFlag>("--flag", Policy::OPTIONAL, "")
    .add<Arg<int>>("someArg", Policy::MANDATORY, "");
    ```
    Since `someArg` is a mandatory argument on `--flag`, you can test if a value for `someArg` exists by testing, `if (someArg)`.  You need not test the flag itself, though that is of course up to you.

## Constructors and  Copy Constructors:
The argument types must be default constructible.  It need not be copyable.  If you are getting an error due to a missing copy constructor, first check that you are not copy-initialising arguments (`auto& arg = ...` not `auto arg = ...`).  If you are still getting an error, please report it as a bug.

## Built-in and User Defined Converters:

So far, parsing args of type `Arg<int>` or `Arg<std::string>` require no further work, they will trigger built-in converters.


However, if you wish to parse an argument of an unsupported type, you have two options:

The simplest option is to provide a fourth argument to the add<Arg<NewType>>() method, an object (e.g. lambda) that implements:
```
void operator()(const std::sotring& stringArgToParse, NewType& parsedValue);
```
In this function:
* `Newtype& parsedValue` has been default constructed and represents the parsed value.
* `const std::string& stringToParse` is the string argument given at the command line.
* You signal an error by calling `throw ErrorMessage("error here")`.  

Here is an example of parsing a  string file path into an fstream  object, reporting an error if the file does not exist:
```c++
auto& file = argParser.add<Arg<std::fstream>>(
    "file_path", Policy::MANDATORY, "Path to an existing file.",
    [](const std::string& arg, std::fstream& stream) {
        stream.open(arg);
        if (!stream.good()) {
            throw ErrorMessage("File " + arg + " does not exist.");
        }
    });
```

Alternatively, you may provide a default converter for all instances of `Arg<NewType>` by specialising the Converter object within the AutoArgParse namespace.

 ```
namespace AutoArgParse {
template <> struct Converter<NewType> {
	inline void operator()(const std::string& stringToParse,
			NewType& parsedValue) {
		...
	}
}
}
```

## Built-in and User Defined Constraints:


User defined constraints are specified exactly in the same method as converters (see above).  This allows constraints to be tested before or after parsing the string into the argument type, which ever is more efficient/convenient.  A satisfied constraint need not perform any additional actions But they may report an error via the same method as Converters `throw ErrorMessage("error message here");`.  So far, the `IntRange` constraint has been provided as a built-in (see end of section on chaining), more are coming soon.

## Chaining multiple converters or constraints together:

You may wish to apply multiple constraints to an argument, or perhaps apply a conversion followed by a constraint.  You can do this by calling the `Auto'ArgParse::chain(...)` function.  Below is an example where we use the built in conversion to int followed by a lambda function to verify that the int is within the correct range.

```c++
auto& intArg = argParser.add<Arg<int>>(
    "some_int", Policy::MANDATORY, "description",
    chain(Converter<int>(),
    [](const std::string& arg, int& value) {
        if (value < 0 || value > 50) {
            throw ErrorMessage("Integer is out of range.");
        }
    }));
```

For convenience an IntRange constraint has already been added:

```c++
auto& intArg = argParser.add<Arg<int>>(
    "some_int", Policy::MANDATORY, "description",
    chain(Converter<int>(), IntRange(0,50,true,true)));
```
