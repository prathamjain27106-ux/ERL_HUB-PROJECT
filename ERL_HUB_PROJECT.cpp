// ERL-HUB: Post-Earthquake Multi-Agency Emergency Relief Logistics Hub
#include <iostream>
#include <iomanip>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <exception>
#include <string>
#include <cstdio>
#include <cmath>
#include <utility>
#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;

// ==================== UNIT I: Namespaces and call semantics ====================
namespace ReliefOps {
namespace Logistics {
    class IncidentCommander;

    inline double calculateDistance(double x1, double y1, double x2, double y2) {
        const double dx = x2 - x1;
        const double dy = y2 - y1;
        return sqrt(dx * dx + dy * dy);
    }

    inline double calculatePayload(double unitWeight, int quantity) {
        if (unitWeight < 0 || quantity < 0)
            throw invalid_argument("Weight and quantity must be non-negative.");
        return unitWeight * quantity;
    }
}

namespace Inventory {
class ReliefCargo {
    friend class ReliefOps::Logistics::IncidentCommander;

    string name;
    int quantity;
    double unitWeight;
    int priority; // 1 = normal, 5 = most urgent

public:
    ReliefCargo(string n = "Unspecified", int q = 0, double w = 0.0, int p = 1)
        : name(n), quantity(q), unitWeight(w), priority(p) {
        if (n.empty() || q < 0 || w < 0.0 || p < 1 || p > 5)
            throw invalid_argument("Invalid cargo name, quantity, weight, or priority.");
    }

    string getName() const { return name; }
    int getQuantity() const { return quantity; }
    double getUnitWeight() const { return unitWeight; }
    int getPriority() const { return priority; }
    double totalWeight() const { return ReliefOps::Logistics::calculatePayload(unitWeight, quantity); }

    ReliefCargo& operator+=(int amount) {
        if (amount < 0) throw invalid_argument("Restock amount cannot be negative.");
        quantity += amount;
        return *this;
    }

    ReliefCargo& operator-=(int amount) {
        if (amount < 0 || amount > quantity)
            throw out_of_range("Insufficient cargo available for dispatch.");
        quantity -= amount;
        return *this;
    }

    bool operator==(const ReliefCargo& other) const {
        return name == other.name && unitWeight == other.unitWeight;
    }

    friend ostream& operator<<(ostream& out, const ReliefCargo& cargo) {
        out << left << setw(18) << cargo.name
            << setw(12) << cargo.quantity
            << setw(14) << cargo.unitWeight
            << cargo.priority;
        return out;
    }

    friend istream& operator>>(istream& in, ReliefCargo& cargo) {
        // Input format: one-word-name quantity unitWeight priority
        string newName;
        int newQuantity, newPriority;
        double newWeight;
        if (!(in >> newName >> newQuantity >> newWeight >> newPriority)) return in;
        if (newName.empty() || newQuantity < 0 || newWeight < 0.0 ||
            newPriority < 1 || newPriority > 5) {
            in.setstate(ios::failbit);
            return in;
        }
        cargo.name = newName;
        cargo.quantity = newQuantity;
        cargo.unitWeight = newWeight;
        cargo.priority = newPriority;
        return in;
    }
};
}
}

namespace ReliefOps { namespace Logistics {
    // Overload 1: ordinary quantity allocation.
    int allocateSupply(Inventory::ReliefCargo& cargo, int quantity) {
        if (quantity < 0) throw invalid_argument("Allocation quantity cannot be negative.");
        const int dispatched = min(quantity, cargo.getQuantity());
        cargo -= dispatched; // call by reference: original object is updated
        return dispatched;
    }

    // Overload 2: allocation allowed only for urgent cargo and urgent mission.
    int allocateSupply(Inventory::ReliefCargo& cargo, int quantity, int emergencyPriority) {
        if (emergencyPriority < 1 || emergencyPriority > 5)
            throw invalid_argument("Emergency priority must be between 1 and 5.");
        if (emergencyPriority >= 4 && cargo.getPriority() >= 4)
            return allocateSupply(cargo, quantity);
        return 0;
    }

    class IncidentCommander {
    public:
        // Friend class can directly access ReliefCargo private members.
        void emergencyOverride(Inventory::ReliefCargo& cargo, int quantity) {
            if (quantity < 0) throw invalid_argument("Override quantity cannot be negative.");
            const int dispatched = min(quantity, cargo.quantity);
            cargo.quantity -= dispatched;
            cout << "Emergency override: " << dispatched << " units of "
                 << cargo.name << " released for critical triage.\n";
        }
    };
}}

// ==================== UNIT II: Encapsulation and dynamic memory ====================
class DistributionDepot {
    string depotName;
    vector<ReliefOps::Inventory::ReliefCargo> stock;
    // Maps cargo names to vector positions for fast inventory lookup.
    unordered_map<string, size_t> stockIndex;
    static int depotCount;

public:
    explicit DistributionDepot(const string& name) : depotName(name) {
        if (name.empty()) throw invalid_argument("Depot name cannot be empty.");
        ++depotCount;
    }

    ~DistributionDepot() { --depotCount; }
    DistributionDepot(const DistributionDepot&) = delete;
    DistributionDepot& operator=(const DistributionDepot&) = delete;

    const string& getName() const { return depotName; }
    static int getDepotCount() { return depotCount; }

    void addCargo(const ReliefOps::Inventory::ReliefCargo& cargo) {
        unordered_map<string, size_t>::const_iterator found = stockIndex.find(cargo.getName());
        if (found != stockIndex.end()) {
            ReliefOps::Inventory::ReliefCargo& existing = stock.at(found->second);
            if (!(existing == cargo))
                throw invalid_argument("Cargo name already exists with a different unit weight.");
            existing += cargo.getQuantity();
        } else {
            stock.push_back(cargo);
            try {
                stockIndex[cargo.getName()] = stock.size() - 1;
            } catch (...) {
                stock.pop_back();
                throw;
            }
        }
    }

    // Controlled operations preserve encapsulation.
    void restockCargo(const string& cargoName, int quantity) {
        ReliefOps::Inventory::ReliefCargo& cargo = findCargo(cargoName);
        cargo += quantity;
    }

    int dispatchCargo(const string& cargoName, int quantity) {
        ReliefOps::Inventory::ReliefCargo& cargo = findCargo(cargoName);
        const int dispatched = min(quantity, cargo.getQuantity());
        cargo -= dispatched;
        return dispatched;
    }

    // Return-by-reference demonstration, but read-only to outside callers.
    const ReliefOps::Inventory::ReliefCargo& getCargo(const string& cargoName) const {
        unordered_map<string, size_t>::const_iterator found = stockIndex.find(cargoName);
        if (found == stockIndex.end()) throw out_of_range("Cargo not found in depot.");
        return stock.at(found->second);
    }

    void display() const {
        cout << "\nDepot: " << depotName << "\n";
        cout << left << setw(18) << "Cargo" << setw(12) << "Quantity"
             << setw(14) << "Unit weight" << "Priority\n";
        cout << string(55, '-') << '\n';
        for (size_t i = 0; i < stock.size(); ++i) cout << stock[i] << '\n';
    }

private:
    ReliefOps::Inventory::ReliefCargo& findCargo(const string& cargoName) {
        unordered_map<string, size_t>::const_iterator found = stockIndex.find(cargoName);
        if (found == stockIndex.end()) throw out_of_range("Cargo not found in depot.");
        return stock.at(found->second);
    }
};
int DistributionDepot::depotCount = 0;

class TransportVehicle {
    string vehicleId;
    ReliefOps::Inventory::ReliefCargo* manifest;
    int manifestSize;
    static int operationalVehicles;
    static double dispatchedTonnage;

public:
    explicit TransportVehicle(const string& id, int size = 0)
        : vehicleId(id), manifest(nullptr), manifestSize(0) {
        if (id.empty() || size < 0) throw invalid_argument("Invalid vehicle ID or manifest size.");
        if (size > 0) manifest = new ReliefOps::Inventory::ReliefCargo[size];
        manifestSize = size;
        ++operationalVehicles;
    }

    // Deep-copy constructor.
    TransportVehicle(const TransportVehicle& other)
        : vehicleId(other.vehicleId), manifest(nullptr), manifestSize(0) {
        if (other.manifestSize > 0)
            manifest = new ReliefOps::Inventory::ReliefCargo[other.manifestSize];
        manifestSize = other.manifestSize;
        for (int i = 0; i < manifestSize; ++i) manifest[i] = other.manifest[i];
        ++operationalVehicles;
    }

    // Copy-and-swap assignment gives strong exception safety.
    TransportVehicle& operator=(const TransportVehicle& other) {
        if (this != &other) {
            TransportVehicle temporary(other);
            swap(vehicleId, temporary.vehicleId);
            swap(manifest, temporary.manifest);
            swap(manifestSize, temporary.manifestSize);
        }
        return *this;
    }

    ~TransportVehicle() {
        delete[] manifest;
        --operationalVehicles;
    }

    void setItem(int index, const ReliefOps::Inventory::ReliefCargo& cargo) {
        if (index < 0 || index >= manifestSize) throw out_of_range("Manifest index is invalid.");
        manifest[index] = cargo;
    }

    double getManifestWeight() const {
        double total = 0.0;
        for (int i = 0; i < manifestSize; ++i) total += manifest[i].totalWeight();
        return total;
    }

    void dispatch() {
        const double total = getManifestWeight();
        dispatchedTonnage += total / 1000.0;
        cout << "\nVehicle " << vehicleId << " dispatched " << total << " kg of relief cargo.\n";
    }

    void showManifest() const {
        cout << "\nManifest for " << vehicleId << ":\n";
        for (int i = 0; i < manifestSize; ++i) cout << manifest[i] << '\n';
    }

    static int getOperationalVehicles() { return operationalVehicles; }
    static double getDispatchedTonnage() { return dispatchedTonnage; }
};
int TransportVehicle::operationalVehicles = 0;
double TransportVehicle::dispatchedTonnage = 0.0;

// ==================== UNIT III: Inheritance and polymorphism ====================
class GPSBeacon {
    double latitude, longitude;
public:
    GPSBeacon(double lat = 0.0, double lon = 0.0) : latitude(lat), longitude(lon) {}
    void setPosition(double lat, double lon) { latitude = lat; longitude = lon; }
    void display() const { cout << "GPS: (" << latitude << ", " << longitude << ")\n"; }
};

class EmergencyLifeSupportUnit {
    int oxygenMinutes;
public:
    explicit EmergencyLifeSupportUnit(int minutes = 0) : oxygenMinutes(minutes) {
        if (minutes < 0) throw invalid_argument("Oxygen duration cannot be negative.");
    }
    void display() const { cout << "Life support: " << oxygenMinutes << " minutes\n"; }
};

class ReliefVehicle {
protected:
    string id;
    double fuel;
    GPSBeacon gps; // composition: vehicle HAS-A GPS beacon
    EmergencyLifeSupportUnit lifeSupport; // composition: vehicle HAS-A life-support unit
public:
    ReliefVehicle(const string& i, double f, double lat, double lon, int oxygen)
        : id(i), fuel(f), gps(lat, lon), lifeSupport(oxygen) {
        if (i.empty() || f < 0.0) throw invalid_argument("Invalid vehicle ID or fuel quantity.");
    }
    virtual void evaluateRoute() const = 0;
    virtual ~ReliefVehicle() {}
    double getFuel() const { return fuel; }
    void consumeFuel(double amount) {
        if (amount < 0.0) throw invalid_argument("Fuel consumption cannot be negative.");
        if (amount > fuel) throw runtime_error("Insufficient fuel.");
        fuel -= amount;
    }
    virtual void display() const {
        cout << "Vehicle ID: " << id << ", fuel: " << fuel << " L\n";
        gps.display();
        lifeSupport.display();
    }
};

class AerialTransport : virtual public ReliefVehicle {
public:
    AerialTransport(const string& i, double f, double lat, double lon, int oxygen)
        : ReliefVehicle(i, f, lat, lon, oxygen) {}
    void evaluateRoute() const override {
        cout << id << ": aerial route evaluation - checking air corridor and landing zone.\n";
    }
};

class GroundConvoy : virtual public ReliefVehicle {
public:
    GroundConvoy(const string& i, double f, double lat, double lon, int oxygen)
        : ReliefVehicle(i, f, lat, lon, oxygen) {}
    void evaluateRoute() const override {
        cout << id << ": ground route evaluation - checking road damage and fuel range.\n";
    }
};

// Hybrid vehicle resolves the diamond through virtual inheritance.
class AmphibiousCraft : public AerialTransport, public GroundConvoy {
public:
    AmphibiousCraft(const string& i, double f, double lat, double lon, int oxygen)
        : ReliefVehicle(i, f, lat, lon, oxygen),
          AerialTransport(i, f, lat, lon, oxygen),
          GroundConvoy(i, f, lat, lon, oxygen) {}
    void evaluateRoute() const override {
        cout << id << ": amphibious route evaluation - selecting land or water corridor.\n";
    }
};

// ==================== UNIT IV: Exceptions, STL, templates, streams ====================
class CargoOverweightException : public exception {
public:
    const char* what() const noexcept override { return "Cargo exceeds vehicle payload capacity."; }
};
class FuelDepletionException : public exception {
public:
    const char* what() const noexcept override { return "Insufficient fuel for the selected mission."; }
};
class DamagedCorridorException : public exception {
public:
    const char* what() const noexcept override { return "Selected transport corridor is damaged."; }
};

template <typename T>
class PriorityDispatchQueue {
    vector<T> items;
public:
    void push(const T& item) { items.push_back(item); }
    bool empty() const { return items.empty(); }
    T pop() {
        if (items.empty()) throw out_of_range("Dispatch queue is empty.");
        typename vector<T>::iterator highest = max_element(items.begin(), items.end(),
            [](const T& a, const T& b) { return a.getPriority() < b.getPriority(); });
        T result = *highest;
        items.erase(highest);
        return result;
    }
};

struct UrgentCargoFirst {
    bool operator()(const ReliefOps::Inventory::ReliefCargo& a,
                    const ReliefOps::Inventory::ReliefCargo& b) const {
        return a.getPriority() > b.getPriority();
    }
};

struct ManifestRow {
    string cargo;
    int quantity;
    double weight;
};

// Binary persistence. A complete temporary file is written first. std::rename
// replaces an existing destination atomically on POSIX systems; on platforms
// where replacement is not supported, the old file is deliberately preserved
// and an error is reported rather than deleting the valid original file.
void saveTransitLog(const vector<ManifestRow>& records, const string& filename) {
    const string temporaryName = filename + ".tmp";
    {
        ofstream out(temporaryName.c_str(), ios::binary | ios::trunc);
        if (!out) throw runtime_error("Unable to create temporary transit log.");
        const size_t count = records.size();
        out.write(reinterpret_cast<const char*>(&count), sizeof(count));
        for (size_t i = 0; i < records.size(); ++i) {
            if (records[i].cargo.size() > 1000 || records[i].quantity < 0 || records[i].weight < 0.0)
                throw invalid_argument("Invalid transit log record.");
            const size_t length = records[i].cargo.size();
            out.write(reinterpret_cast<const char*>(&length), sizeof(length));
            if (length > 0)
                out.write(records[i].cargo.data(), static_cast<streamsize>(length));
            out.write(reinterpret_cast<const char*>(&records[i].quantity), sizeof(records[i].quantity));
            out.write(reinterpret_cast<const char*>(&records[i].weight), sizeof(records[i].weight));
        }
        out.flush();
        if (!out) throw runtime_error("Error while writing transit log.");
    }

    bool replaced = false;
#ifdef _WIN32
    replaced = MoveFileExA(temporaryName.c_str(), filename.c_str(),
                           MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
#else
    replaced = std::rename(temporaryName.c_str(), filename.c_str()) == 0;
#endif
    if (!replaced) {
        std::remove(temporaryName.c_str());
        throw runtime_error("Could not replace transit log. Original file, if present, was preserved.");
    }
}

vector<ManifestRow> loadTransitLog(const string& filename) {
    vector<ManifestRow> records;
    ifstream in(filename.c_str(), ios::binary);
    if (!in) return records;

    size_t count = 0;
    in.read(reinterpret_cast<char*>(&count), sizeof(count));
    if (!in || count > 10000) throw runtime_error("Invalid transit log header.");

    for (size_t i = 0; i < count; ++i) {
        size_t length = 0;
        in.read(reinterpret_cast<char*>(&length), sizeof(length));
        if (!in || length == 0 || length > 1000)
            throw runtime_error("Corrupt cargo name in transit log.");

        ManifestRow row;
        row.cargo.resize(length);
        in.read(&row.cargo[0], static_cast<streamsize>(length));
        in.read(reinterpret_cast<char*>(&row.quantity), sizeof(row.quantity));
        in.read(reinterpret_cast<char*>(&row.weight), sizeof(row.weight));
        if (!in) throw runtime_error("Incomplete transit log record.");
        if (row.quantity < 0 || row.weight < 0.0)
            throw runtime_error("Invalid quantity or weight in transit log.");
        records.push_back(row);
    }

    // Reject unexpected trailing/corrupt bytes.
    char extra;
    if (in.read(&extra, 1)) throw runtime_error("Transit log contains unexpected trailing data.");
    return records;
}

// Conventional user-defined ostream manipulator.
ostream& manifestHeader(ostream& out) {
    out << left << setw(18) << "Cargo" << setw(12) << "Quantity"
        << "Total weight (kg)\n";
    out << string(48, '-') << '\n';
    return out;
}

int main() {
    try {
        using ReliefOps::Inventory::ReliefCargo;
        using ReliefOps::Logistics::allocateSupply;

        cout << "===== ERL-HUB: Emergency Relief Logistics Hub =====\n";

        DistributionDepot depot("North Relief Depot");
        depot.addCargo(ReliefCargo("WaterTanks", 100, 20.0, 5));
        depot.addCargo(ReliefCargo("RationKits", 250, 2.0, 3));
        // Duplicate cargo addition uses operator== to validate identity.
        depot.addCargo(ReliefCargo("WaterTanks", 10, 20.0, 5));
        depot.display();

        // Return-by-reference (const reference) and call-by-reference.
        const ReliefCargo& selectedCargo = depot.getCargo("WaterTanks");
        cout << "\nRead-only reference lookup: " << selectedCargo.getName()
             << " has " << selectedCargo.getQuantity() << " units.\n";

        cout << "Normal allocation from a working cargo object: ";
        ReliefCargo rationForMission("RationKits", 250, 2.0, 3);
        cout << allocateSupply(rationForMission, 20) << " ration kits.\n";

        cout << "Emergency-priority allocation: ";
        ReliefCargo waterForMission("WaterTanks", 100, 20.0, 5);
        cout << allocateSupply(waterForMission, 10, 5) << " water tanks.\n";

        // += and -= are exercised through controlled depot operations.
        depot.restockCargo("WaterTanks", 5);
        cout << "Depot dispatched " << depot.dispatchCargo("RationKits", 15)
             << " ration kits.\n";

        // Friend-class emergency override accesses private cargo members.
        ReliefOps::Logistics::IncidentCommander commander;
        commander.emergencyOverride(waterForMission, 3);

        // Inline payload and distance calculations.
        cout << "Payload calculation: "
             << ReliefOps::Logistics::calculatePayload(2.0, 50) << " kg.\n";
        cout << "Route distance: "
             << fixed << setprecision(2)
             << ReliefOps::Logistics::calculateDistance(0, 0, 3, 4) << " km.\n";

        // Demonstrate overloaded input and output stream operators.
        ReliefCargo receivedCargo;
        stringstream inputStream("TraumaKits 40 1.5 5");
        if (!(inputStream >> receivedCargo)) throw runtime_error("Cargo input parsing failed.");
        cout << "\nCargo received through operator>>: " << receivedCargo << '\n';
        stringstream outputStream;
        outputStream << receivedCargo;
        cout << "Cargo formatted through operator<<: " << outputStream.str() << '\n';

        // Dynamic manifest, deep copy constructor, assignment and destructor.
        TransportVehicle truck("TRUCK-01", 2);
        ReliefCargo waterManifest("WaterTanks", 50, 20.0, 5);
        ReliefCargo rationManifest("RationKits", 100, 2.0, 3);
        truck.setItem(0, waterManifest);
        truck.setItem(1, rationManifest);
        TransportVehicle backupTruck(truck); // deep-copy constructor
        TransportVehicle assignedTruck("TRUCK-02", 1);
        assignedTruck = truck; // deep-copy assignment operator
        truck.showManifest();
        truck.dispatch();

        // Generic priority queue and custom sorting functor.
        PriorityDispatchQueue<ReliefCargo> dispatchQueue;
        dispatchQueue.push(rationManifest);
        dispatchQueue.push(waterManifest);
        cout << "\nNext highest-priority cargo: " << dispatchQueue.pop().getName() << '\n';

        vector<ReliefCargo> urgentList;
        urgentList.push_back(rationManifest);
        urgentList.push_back(waterManifest);
        sort(urgentList.begin(), urgentList.end(), UrgentCargoFirst());
        cout << "\nPriority-sorted cargo:\n" << manifestHeader;
        for (size_t i = 0; i < urgentList.size(); ++i)
            cout << left << setw(18) << urgentList[i].getName()
                 << setw(12) << urgentList[i].getQuantity()
                 << urgentList[i].totalWeight() << '\n';

        // Inheritance, virtual base classes, composition and runtime polymorphism.
        AerialTransport helicopter("HELI-01", 500, 28.6, 77.2, 90);
        GroundConvoy convoy("CONVOY-01", 300, 28.7, 77.1, 0);
        AmphibiousCraft hybrid("HYBRID-01", 250, 28.5, 77.0, 60);
        vector<ReliefVehicle*> fleet;
        fleet.push_back(&helicopter);
        fleet.push_back(&convoy);
        fleet.push_back(&hybrid);
        cout << "\nRoute evaluations through base-class pointers:\n";
        for (size_t i = 0; i < fleet.size(); ++i) {
            fleet[i]->evaluateRoute();
            fleet[i]->display();
        }

        // Demonstrate all three custom exceptions with meaningful conditions.
        cout << "\nException-handling demonstrations:\n";
        try {
            const double payloadKg = truck.getManifestWeight();
            const double vehicleCapacityKg = 1000.0;
            if (payloadKg > vehicleCapacityKg) throw CargoOverweightException();
        } catch (const CargoOverweightException& e) {
            cout << "Cargo warning: " << e.what() << '\n';
        }

        try {
            const double requiredFuelLitres = 600.0;
            if (helicopter.getFuel() < requiredFuelLitres) throw FuelDepletionException();
            helicopter.consumeFuel(requiredFuelLitres);
        } catch (const FuelDepletionException& e) {
            cout << "Fuel warning: " << e.what() << '\n';
        }

        try {
            const bool corridorReportedDamaged = true; // received from incident assessment
            if (corridorReportedDamaged) throw DamagedCorridorException();
        } catch (const DamagedCorridorException& e) {
            cout << "Route warning: " << e.what() << '\n';
        }

        // Binary persistence: save and restore transit manifest audit records.
        vector<ManifestRow> logs;
        logs.push_back(ManifestRow{"WaterTanks", 50, 1000.0});
        logs.push_back(ManifestRow{"RationKits", 100, 200.0});
        saveTransitLog(logs, "transit_log.bin");
        vector<ManifestRow> restored = loadTransitLog("transit_log.bin");
        cout << "\nRestored binary transit log:\n" << manifestHeader;
        for (size_t i = 0; i < restored.size(); ++i)
            cout << left << setw(18) << restored[i].cargo
                 << setw(12) << restored[i].quantity << restored[i].weight << '\n';

        cout << "\nActive depots: " << DistributionDepot::getDepotCount()
             << "\nOperational vehicles: " << TransportVehicle::getOperationalVehicles()
             << "\nTotal dispatched: " << fixed << setprecision(2)
             << TransportVehicle::getDispatchedTonnage() << " metric tonnes\n";

        cout << "\nProject executed successfully.\n";
    }
    catch (const exception& e) {
        cerr << "Fatal error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
