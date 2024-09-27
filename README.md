# dRomeEmpire 
`drone swarm control project`

## Prerequisites
1. **Docker** either [`Desktop`](https://www.docker.com/products/docker-desktop/) or [`Engine`](https://docs.docker.com/engine/install/) will do. 
<i>P.S. use Engine, trust me</i>
2. **Visual Studio Code**. **Clion** is also an option, but you'll have to configure it yourself :)
3. StingBee's Vision module ver. 1.4

## Installation

`1.` Install **Dev Containers** extension in VS Code

`2.`  Clone this repo using provided credentials
```bash
git clone https://github.com/Raspy-Py/DroneSwarm.git
```

`3.` Open the project in VS Code and click `Reopen in container` in the right bottom corner.

`4.` Put archive with [vision SDK](https://gitlab.hard-tech.org.ua/external/mirror-vision-rv1126-buildroot/-/jobs/2141/artifacts/file/artifacts_sdk/vision-sdk.tar.gz) inside `install/` folder. DO NOT RENAME IT.

`5.` Reconfigure enviroment
```bash
tar -xf install/vision-sdk.tar.gz -C /opt
/opt/vision-sdk/relocate-sdk.sh
source /opt/vision-sdk/environment-setup
```

`6.` Run `./setup.sh` script

`7.` Install **CMake Tools** extension in VS Code (inside the container). It will be installed internally.

`8.` Try building [test application](https://gitlab.hard-tech.org.ua/external/vision-rv1126-helloworld-app.git).

## Deployment

`1.` Deploy to the device:
```bash
scp <path_to_bin> root@192.168.55.1:/tmp/
```

`2.` Connect to the device via ssh:
```bash
ssh root@192.168.55.1
```
password : 123qwe

`3.` Launch the thing:
```bash
./tmp/<your_bin>
```
