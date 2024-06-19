# Tutorial de Computación Paralela con OpenMP en C++

Este proyecto demuestra cómo utilizar OpenMP para realizar computación paralela en C++. El programa se encarga de procesar datos desde un archivo CSV utilizando múltiples hilos para mejorar el rendimiento.

## Requisitos previos

Para compilar y ejecutar este programa, necesitas tener instalado un compilador de C++ que admita OpenMP, como `g++`. Asegúrate también de tener el archivo de datos `pd.csv` en la misma carpeta donde se encuentra el código fuente.

## Clonar el repositorio

Puedes clonar este repositorio desde GitHub utilizando el siguiente comando:

```bash
git clone https://github.com/SmnZin/Proyecto_CompParal.git
cd Proyecto_CompParal
```
## Copiar .cpp
Tambien puedes crear un archivo vacio .cpp y pegar todo lo que esta en el archivo CompParal.cpp y seguir con el siguente paso 👇

## Compilar el programa
Para compilar el programa con g++ y OpenMP, ejecuta el siguiente comando en la consola:

```bash
g++ -fopenmp -o CompParal CompParal.cpp
```

## Ejecutar el programa
```bash
./CompParal.exe
```
Reemplaza CompParal.exe con el nombre del ejecutable que se haya creado en tu sistema operativo.
