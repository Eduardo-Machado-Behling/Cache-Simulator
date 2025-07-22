# Cache-Simulator
Simulates a cache using a series of addresses as input

# Build
Compilação padrão:
```sh
$ cmake -S . -B build
$ cmake --build build

# GUI
$ ./bin/cache_simulator 4 4 4 F 0 bin_100.bin 

# Headless
$ ./bin/cache_simulator 4 4 4 F 0 bin_100.bin 
```

Compilação com Docker caso haja problema
```
$ docker-compose up --build
```