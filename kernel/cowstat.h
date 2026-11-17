#ifndef COWSTAT_H
#define COWSTAT_H

// Estadísticas de Copy-on-Write devueltas por la llamada al sistema
// cowstat(). La usan el kernel, cowtest y minidb.
// (Requiere que kernel/types.h se incluya antes, para uint64.)
struct cowstat {
  uint64 freepages;   // páginas físicas libres en todo el sistema
  uint64 sharedpages; // páginas físicas con más de una referencia
  uint64 cowfaults;   // fallos CoW atendidos para el proceso que llama
  uint64 cowcopies;   // de esos fallos, cuántos copiaron la página
};

#endif
