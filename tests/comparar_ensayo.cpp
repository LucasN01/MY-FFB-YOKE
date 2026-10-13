#include "comparar_ensayo.h"
int main(int argc,char** argv) {
    if(argc!=2) { std::fprintf(stderr,"Uso: comparar_ensayo.exe <carpeta_ensayo>\n"); return 2; }
    return YokeEnsayo::run(argv[1]);
}
