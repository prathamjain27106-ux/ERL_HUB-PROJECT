# ERL-HUB: Post-Earthquake Multi-Agency Emergency Relief Logistics Hub

A C++11 Object-Oriented Programming (OOP) project that models a regional
logistics hub coordinating emergency supplies and transport vehicles
after a major earthquake.

ERL-HUB demonstrates how C++ namespaces, classes, encapsulation, dynamic
memory, operator overloading, inheritance, runtime polymorphism,
exception handling, STL containers, templates, and binary file I/O can
work together in a disaster-relief logistics scenario.

## Project Overview

After a major earthquake, relief agencies must coordinate limited
supplies and multiple transport modes while responding to damaged
routes, urgent medical needs, and changing inventory levels.

This project provides a simplified, educational model of that
environment. It includes:

-   A depot inventory for relief cargo such as water tanks, ration kits,
    and trauma kits.
-   Normal and emergency-priority supply allocation.
-   A commander emergency override for critical triage.
-   Transport vehicles with dynamically allocated cargo manifests.
-   Aerial, ground, and amphibious vehicle types.
-   Route evaluation through base-class pointers.
-   Priority-based dispatch and sorting.
-   Exception demonstrations for overweight cargo, insufficient fuel,
    and damaged corridors.
-   Binary saving and restoration of transit manifest audit records.

> **Scope:** This is an academic OOP demonstration, not a production
> emergency-response or fleet-management system. It does not connect to
> live GPS, real dispatch services, or operational disaster-response
> infrastructure.

## Features

### Unit I --- Core Foundations and Call Semantics

-   Logical namespaces: `ReliefOps::Logistics` and
    `ReliefOps::Inventory`.
-   Overloaded `allocateSupply()` functions for standard allocation and
    emergency-priority allocation.
-   Call-by-reference updates to cargo objects.
-   A read-only reference returned by depot cargo lookup.
-   Inline distance and payload calculation functions.
-   `IncidentCommander` as a friend class of `ReliefCargo`, allowing a
    controlled emergency override.

### Unit II --- Encapsulation, Dynamic Memory, and Operator Overloading

-   `ReliefCargo`, `DistributionDepot`, and `TransportVehicle` classes.
-   Private inventory data with controlled depot operations.
-   `std::vector` for depot stock and manifests/audit collections.
-   `std::unordered_map` as a cargo-name-to-inventory-position index.
-   Static depot and fleet analytics.
-   Dynamically allocated vehicle manifests with deep-copy construction
    and copy-and-swap assignment.
-   Overloaded operators:
    -   `+=` to restock cargo.
    -   `-=` to deduct dispatched cargo.
    -   `==` to compare cargo identity.
    -   `<<` and `>>` for formatted cargo output and input.

### Unit III --- Inheritance, Composition, and Polymorphism

-   Abstract base class: `ReliefVehicle`.
-   Derived classes: `AerialTransport` and `GroundConvoy`.
-   `AmphibiousCraft` combines both derived transport types.
-   Virtual inheritance resolves the diamond inheritance relationship.
-   Composition models a vehicle having a `GPSBeacon` and an
    `EmergencyLifeSupportUnit`.
-   Runtime polymorphism evaluates routes through `ReliefVehicle*`
    base-class pointers.
-   A virtual destructor supports safe destruction through base
    pointers.

### Unit IV --- Streams, Persistence, Exception Safety, and STL

-   Custom exceptions derived from `std::exception`:
    -   `CargoOverweightException`
    -   `FuelDepletionException`
    -   `DamagedCorridorException`
-   Templated `PriorityDispatchQueue<T>` for priority-based dispatch.
-   `std::vector` and `std::unordered_map` containers.
-   `UrgentCargoFirst` custom sorting functor.
-   `manifestHeader` user-defined output stream manipulator.
-   Binary transit-log persistence using `std::ofstream` and
    `std::ifstream`.
-   Temporary-file writing followed by replacement of the destination
    file, with platform-specific handling.

## Technology Stack

-   **Language:** C++
-   **Language standard:** C++11
-   **Standard library:** STL (`vector`, `unordered_map`, algorithms,
    streams, exceptions)
-   **Build tools:** GCC, Clang, or a compatible C++11 compiler

No third-party libraries are required.

## Repository Contents

``` text
.
├── ERL_HUB_Project_Corrected.cpp   # Main C++ source code
├── README.md                       # Project documentation
└── transit_log.bin                 # Generated at runtime; do not commit
```

The binary log is created in the program's current working directory
when the application runs. It is a generated runtime file and is not
required to compile the project.

## Getting Started

### 1. Clone the repository

``` bash
git clone https://github.com/prathamjain27106-ux/ERL_HUB-PROJECT.git
cd ERL_HUB_PROJECT
```

### 2. Compile

Using GCC:

``` bash
g++ -std=c++11 -Wall -Wextra -pedantic ERL_HUB_Project_Corrected.cpp -o erl_hub
```

Using Clang:

``` bash
clang++ -std=c++11 -Wall -Wextra -pedantic ERL_HUB_Project_Corrected.cpp -o erl_hub
```

On Windows with MinGW, the GCC command can be used from the MinGW
terminal. The source includes Windows-specific file-replacement handling
when compiled on Windows.

### 3. Run

Linux/macOS:

``` bash
./erl_hub
```

Windows:

``` powershell
.\erl_hub.exe
```

The program runs a set of built-in demonstrations and prints the results
to the terminal. No interactive input is required for the demonstration
run.

## Expected Demonstrations

When the program runs, it demonstrates:

1.  Creating a relief depot and adding cargo.
2.  Looking up stock using a read-only reference.
3.  Standard and emergency-priority cargo allocation.
4.  Restocking and dispatching supplies.
5.  A commander emergency override.
6.  Inline payload and route-distance calculations.
7.  Cargo stream extraction (`>>`) and insertion (`<<`).
8.  Dynamic vehicle manifests, deep copying, and assignment.
9.  Priority queue dispatch and functor-based sorting.
10. Aerial, ground, and amphibious route evaluation using polymorphism.
11. Handling overweight, fuel-depletion, and damaged-corridor
    exceptions.
12. Saving and restoring binary transit-log records.
13. Displaying depot, vehicle, and dispatched-tonnage statistics.

The exact output may vary slightly by compiler and platform.

## Binary Log Notes

The application writes a file named `transit_log.bin` in its current
working directory.

-   Records are written in binary form.
-   The program reads the records back and displays the restored
    entries.
-   A temporary file is written before replacing the destination log.
-   Replacement behavior is platform-dependent; if replacement fails,
    the program reports an error rather than deliberately deleting the
    existing log first.

**Portability note:** The binary format uses native C++ representations
such as `size_t`, `int`, and `double`. It is intended for this project
demonstration and should not be treated as a portable, long-term
interchange format across different architectures, compilers, or
endianness.

## Concepts Demonstrated

  OOP / C++ concept      Example in this project
  ---------------------- ------------------------------------------------
  Namespaces             `ReliefOps::Logistics`, `ReliefOps::Inventory`
  Function overloading   `allocateSupply()`
  Call by reference      Cargo allocation and emergency override
  Return by reference    `DistributionDepot::getCargo()`
  Friend class           `IncidentCommander`
  Encapsulation          Private cargo and depot data
  Operator overloading   `+=`, `-=`, `==`, `<<`, `>>`
  Static members         Depot count and vehicle analytics
  Dynamic memory         Vehicle manifest allocation
  Copy constructor       Deep-copy vehicle construction
  Copy assignment        Copy-and-swap assignment
  Inheritance            Aerial, ground, and amphibious vehicles
  Virtual inheritance    Shared `ReliefVehicle` base
  Composition            GPS beacon and life-support unit
  Runtime polymorphism   Virtual route evaluation
  Exception handling     Custom relief logistics exceptions
  STL containers         `vector`, `unordered_map`
  Templates              `PriorityDispatchQueue<T>`
  Functors               `UrgentCargoFirst`
  Stream manipulator     `manifestHeader`
  File handling          Binary transit-log save and restore

## Limitations

-   The project uses sample data embedded in `main()`; it is not a
    complete interactive logistics application.
-   Route checks and corridor damage status are simulated for
    demonstration.
-   GPS and life-support components are illustrative classes, not
    connected hardware.
-   Fuel, payload capacity, and dispatch calculations are simplified
    examples.
-   The binary log format is intended for local demonstration rather
    than cross-platform data exchange.
-   No authentication, networking, database, or multi-user
    synchronization is implemented.

## Future Improvements

Possible extensions include:

-   Add an interactive menu for depot and fleet operations.
-   Read cargo, vehicle, and incident data from configuration files.
-   Add route planning using real map or GIS data.
-   Add stronger validation and unit tests for inventory and dispatch
    operations.
-   Use a versioned, platform-independent file format for persistent
    records.
-   Add automated build and test workflows with GitHub Actions.

## Contributing

This repository is primarily an academic project. Suggestions, bug
reports, and improvements are welcome.

For changes: 1. Fork the repository. 2. Create a feature branch. 3. Make
and test your changes with a C++11-compatible compiler. 4. Submit a pull
request describing the change.

## License

No license has been specified yet. Unless a license is added to this
repository, all rights remain with the copyright holder and reuse,
modification, or redistribution is not automatically granted.

------------------------------------------------------------------------

**ERL-HUB --- Coordinating relief cargo and transport resources after a
disaster.**
