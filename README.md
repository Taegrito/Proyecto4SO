# Proyecto 4: Copy-On-Write y MiniDB
# Integrantes
* Ana Isabella Gómez García
* Samuel Quintero Quintero
* Ashly Robayo Parra
# Descripción del proyecto
Este proyecto extiende el sistema operativo xv6-riscv mediante la
implementación de mecanismos de administración eficiente de memoria
basados en Copy-on-Write (CoW), estadísticas del uso de memoria y
un sistema de almacenamiento de snapshots para una aplicación MiniDB.
# Funcionalidades
## Parte A: Copy-on-Write
Se modifica el mecanismo de creación de procesos para compartir páginas de memoria entre el proceso padre y el proceso hijo.

Las páginas que originalmente son escribibiles, se marcan con `PTE_COW` y se eliminan sus permisos de escritura temporalmente.
Cuando un proceso intenta escribir sobre una página compartida, se genera un fallo de página.
También se adapta copyout() para resolver correctamente las escrituras
sobre páginas marcadas como CoW.

## Parte B: cowstat
Es una syscall para ver información de la memoria Copy-on-Write. El programa le pasa la dirección de un struct y el kernel la llena con freepages (páginas físicas libres en todo el sistema), sharedpages (páginas físicas compartidas por más de un proceso), cowfaults (fallos CoW atendidos para el proceso que llama) y cowcopies (de los fallos, cuántos copiaron la página).

Cuando un programa lo llama el kernel cuenta las páginas libres y compartidas, toma los contadores del proceso que llama y escribe los cuatro valores en la memoria del programa.

- `kernel/sysproc.c` arma el struct y devuelve con `copyout()`.
- `kernel/kalloc.c` cuenta páginas libres y compartidas.
- `kernel/cowstat.h` tiene el struct.
- `kernel/kalloc.c` agrega las funciones para contar páginas.
- `kernel/proc.c/h` agregan los contadores por proceso y los ponen en 0 al crear un proceso.
- `kernel/vm.c` hace los `cowfaults` y `cowcopies` del proceso y actualiza los contadores.
  
minidb.c la usa para medir la memoria libre y los fallos del snapshot

## Parte C: MiniDB


Solo se modificó `user/minidb.c`. Se completaron
`guardar_snapshot()` y `snapshot()`, y se agregó la función auxiliar
`bits_en_uno()`, que cuenta las páginas marcadas por `pgaccess()`.

### Consulta paralela (`minidb consulta K`) y mapa (`minidb mapa`)

minidb snapshot M` guarda en `snap.db` una copia consistente de la tabla de 8.192 cuentas mientras el padre sigue aplicando M transferencias

`consulta` crea K trabajadores con `fork()`, cada uno procesa un rango de
cuentas y envía su resultado al padre por un pipe unico. Como los
trabajadores solo leen la tabla, CoW nunca copia sus páginas: por eso la
memoria libre casi no baja después de los `fork()`.

## que pasos sigue
1. El padre calcula el checksum y el saldo total de la tabla, y limpia los bits de acceso de sus 64 páginas con `pgaccess()`.
2. El padre hace `fork()` y no espera al hijo.
3. El hijo escribe `snap.db`: un `struct snap_encabezado` (con el checksum previo al fork) seguido de las 8.192 cuentas, escritas directamente desde la tabla y sin buffer intermedio.
4. El padre aplica las M transferencias (semilla 4). Rechaza las que tienen origen igual a destino o saldo insuficiente; las demás mueven el monto y aumentan `nmov` en ambas cuentas.
5. El padre reporta las transferencias aplicadas, las páginas tocadas (`pgaccess()`) y los fallos y copias CoW de la fase de transferencias (diferencia de `cowstat()`).
6. El padre espera al hijo, relee `snap.db` y verifica el encabezado, el checksum de las cuentas guardadas y la conservación del saldo total. Imprime `snapshot: OK` solo si todo coincide.



# Archivos principales
| Archivo | Descripción |
|---|---|
| kernel/kalloc.c | Administra las páginas físicas y referencias. |
| kernel/vm.c | Maneja las tablas de páginas y mecanismo CoW. |
| kernel/trap.c | Atención de fallos de página. |
| kernel/proc.c | Crea libera de procesos. |
| kernel/proc.h | Estructura de los procesos. |
| kernel/cowstat.h | Estructura de estadísticas CoW. |
| kernel/sysproc.c | Implementa llamadas al sistema. |
| user/cowtest.c | Pruebas del mecanismo CoW. |
| user/minidb.c | Aplicación de demostración MiniDB. |
| user/minidb.h | Estructuras y funciones auxiliares de MiniDB. |

# Compilación y ejecución
Desde la raíz del proyecto, para iniciar xv6:
```bash
make qemu
```
Una vez dentro de la consola, se pueden ejecutar las siguientes pruebas
## Pruebas de Copy-on-Write
```text
cowtest
```
## Pruebas de tablas de páginas
```text
pgtbltest
```
## Consultas de MiniDb
```text
minidb consulta 1
minidb consulta 2
minidb consulta 3
minidb consulta 8
```
## Consultas de snapshots
```text
minidb snapshot 2000
minidb snapshot 0
```
## Visualización del mapa de memoria
```text
minidb snapshot 2000
minidb snapshot 0
```
## Pruebas generales de xv6
```text
usertests -q
```
# Declaración de uso de IA
Se utilizó ChatGPT como herramienta de apoyo  para la detección y correción de errores
