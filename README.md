# Cache-Simulator
Simulates a cache using a series of addresses as input

# Build
### Compilação:
```sh
git clone --recursive https://github.com/Eduardo-Machado-Behling/Cache-Simulator 
cd Cache-Simulator

# --- Use somente um desses commandos
# compilar com GUI
cmake -S . -B build
# compilar sem GUI
cmake -S . -B build -DBUILD_GUI=OFF
# ---

cmake --build build

# GUI
./bin/cache_simulator 4 4 4 F 0 bin_100.bin 

# Headless
./bin/cache_simulator 4 4 4 F 1 bin_100.bin 
```
