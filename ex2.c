#include <stdio.h>
#include <stdlib.h>
#define MAX 5 //massimo num di elem che compongono una diagonale
#define N 3 //numero di diagonali presenti nel programma
#define DD 10 //difficoltà diagonale max
#define DP 20 //difficoltà programma max

/*l'idea e' generare tutte le possibili diagonali che posso fare con disp con ripetizione di elementi, poi cercare la migliore combinazione di tre diagonali
  tramite le combinazioni con ripetizione di queste diagonali*/



//tipo elemento
typedef struct {
    char nome[100];
    int tipologia,dirIng,dirUsc,reqPrec,finale,diff;
    float val;
}elem_t;
//tipo diagonale
typedef struct {
    int nEl;
    int diffTot;
    float valoreTot;
    int vettEl[MAX];
}diag_t;

//funzioni per il tipo elem_t che gestiscono il vettore di elementi
int leggiElem(FILE *fp,elem_t *pEl);
elem_t* leggiFile(char* filename,int *nEl);
void stampaVetEl(elem_t *vEl,int nEl);
void distruggiVetEl(elem_t *vEl);
void stampaElemento(elem_t el);

//funzioni per gestire diagonali
diag_t* diagInit();
void distruggiDiag(diag_t *diag);
void stampaDiag(diag_t diag, elem_t *vEl);

void riallocaVettDiag(diag_t **pVetDiag,int* nMaxDiag);
diag_t* allocaVetDiag(int nMaxDiag);
void liberaVetDiag(diag_t *vetDiag);
//funzioni per generare tutte le possibili diagonali di un ginnasta
int checkCompatible(diag_t *p_diagTmp, elem_t *vetEl,int indexEl,int pos);
void disp_r(int pos,int k,diag_t **pVetDiag, diag_t* p_diagTmp,int *p_nMaxDiag,int* p_nDiag,elem_t *vetEl, int nEl);
int generaDiagonali(diag_t **p_vetDiag, diag_t* p_diagTmp,elem_t* vetEl, int nEl, int* p_nMaxDiag);
//funzioni per il programma
int checkSol(int *sol,diag_t **p_vetDiag, elem_t *vEl,float *scoreProgramma);
void comb_r(diag_t **p_vetDiag,elem_t *vetEl,int nDiag,int pos,int start,int *sol,int *bestSol,float *score,float *bestscore);
void stampaProgramma(int *sol,diag_t *vDiag, elem_t *vEl);
int main(){
    elem_t *vEl;
    int nDiag,nEl,nMaxDiag=1;
    diag_t *vetDiag,*diagTmp;
    diagTmp = diagInit();
    vetDiag = allocaVetDiag(nMaxDiag);
    vEl=leggiFile("elementi.txt",&nEl);
    //stampaVetEl(vEl,nEl);
    nDiag=generaDiagonali(&vetDiag,diagTmp,vEl,nEl,&nMaxDiag);
    int sol[N],bestSol[N];
    float score=0,bestScore=0;
    comb_r(&vetDiag,vEl,nDiag,0,0,sol,bestSol,&score,&bestScore);
    stampaProgramma(bestSol,vetDiag,vEl);
    printf("Con valore totale = %2f\n",bestScore);
    distruggiVetEl(vEl);
    liberaVetDiag(vetDiag);
    distruggiDiag(diagTmp);
}

void stampaProgramma(int *sol,diag_t *vDiag, elem_t *vEl){
    int i,k,indexEl;
    printf("<<<<< STAMPA PROGRAMMA >>>>>\n");
    for(i=0;i<N;i++){//per tutte le diagonali del programma
        printf("Diagonale #%d\n",i+1);
        for(k=0;k<vDiag[sol[i]].nEl;k++){//per tutti gli elem della diagonale
            indexEl=vDiag[sol[i]].vettEl[k];
            if (k!=vDiag[sol[i]].nEl -1) {
                printf("%s, ",vEl[indexEl].nome);
            }
            else{
                printf("%s\n",vEl[indexEl].nome);
            }
        }
    }
}

//funzione che verifica se il programma soddisfa i tre seguenti vincoli:
//1. deve presentare un el acrob avanti e uno indietro nel corso del suo programma, non necessariamente nella stessa diagonale
//2. deve presentare almeno una diagonale con alemno due elem acrobatici in sequenza
//3. difficolta' totale del programma deve essere inferiore o uguale al valore costante DP
int checkSol(int *sol,diag_t **p_vetDiag, elem_t *vEl,float *scoreProgramma){
    int i,j,indEl;
    int avanti=0,indietro=0;
    int ok1=0,ok2=0,ok3=0;//sono i flag per i tre vincoli che un programma deve rispettare
    int diffProgramma=0;
    int c=0;


    int n,ind_rifUltimoEl,bonusDiag=-1;
    float valBonusDiag=-1,valDiag_i;
    int diagDiff8[3]={-1,-1,-1};
    //vedo se tra le tre diagonali che compongono il programma ce ne sta una che ha un elem che chiude la diag con diff >= 8
    //se ce ne stanno più di una vedo a quale è meglio applicare il bonus
    for (i=0; i<N; i++){
        n=(*p_vetDiag)[sol[i]].nEl -1;
        ind_rifUltimoEl = (*p_vetDiag)[sol[i]].vettEl[n];
        valDiag_i = (*p_vetDiag)[sol[i]].valoreTot;
        if (vEl[ind_rifUltimoEl].diff >= 8){
            if(valBonusDiag <  valDiag_i){//se tra le tre diag c'era già una a cui si poteva applicare il bonus vedo se è meglio applicarlo a questa diag
                valBonusDiag = valDiag_i;
                bonusDiag = i;
            }
        }
    }

    *scoreProgramma = 0;
    //per tutte le diag del programma
    for (i=0; i<N;i++){
        if (ok2!=1 && (*p_vetDiag)[sol[i]].nEl > 1){
            ok2=1;//secondo vincolo soddisfatto
        }
        if (ok1!=1) { //per tutti gli elem della diagonale in esame
            for (j = 0; j < (*p_vetDiag)[sol[i]].nEl; j++) {
                indEl = (*p_vetDiag)[sol[i]].vettEl[j];//salvo l'indice del jesimo elemento dell' iesima diag
                if (vEl[indEl].tipologia == 2) {
                    avanti = 1;//e' un el acrob avanti
                    if (avanti == indietro == 1) {
                        ok1 = 1;//mi segno che il primo vincolo e' soddisfatto
                    }
                }
                if (vEl[indEl].tipologia == 1) {
                    indietro = 1;//e' un elem acrob indietro
                    if (avanti == indietro == 1) {
                        ok1 = 1;//mi segno che il primo vincolo e' soddisfatto
                    }
                }
            }
        }
        if (i != bonusDiag) {
            *scoreProgramma += (*p_vetDiag)[sol[i]].valoreTot ;//calcolo il valore del programma
        }
        else{
            *scoreProgramma += (*p_vetDiag)[sol[i]].valoreTot * 1.5;//diag con bonus
        }
        diffProgramma+=(*p_vetDiag)[sol[i]].diffTot;//calcolo la diff tot del programma

    }

    if (diffProgramma <= DP){
        ok3=1;//terzo vincolo soddisfatto!
    }

    if(ok1==1 && ok2==1 && ok3==1){
        return 1;//programma che soddisfa tutti i vincoli!
    }
    return 0;//non soddisfa tutti i requisiti
}

void comb_r(diag_t **p_vetDiag,elem_t *vetEl,int nDiag,int pos,int start,int *sol,int *bestSol,float *score,float *bestscore){

    int i;

    if (pos==N){//terminazione trovate tre diagonali
        //controllo se la soluzione trovata rispetta i vincoli del problema
        if (checkSol(sol,p_vetDiag,vetEl,score)){
            if (*score > *bestscore){//se migliora la soluzione la salvo altrimenti vado avanti
                *bestscore = *score;
                for (int j = 0; j < N; j++) {
                    bestSol[j]=sol[j];
                }
                return;
            }

        }
        return;
    }

    for (i=start;i<nDiag;i++){//per tutte le diagonali create
        sol[pos]=i;//faccio la scelta e ricorro a pos+1
        comb_r(p_vetDiag,vetEl,nDiag,pos+1,start,sol,bestSol,score,bestscore);
        start++;
    }
}


//funzione wrapper che chiama iteramente disp_r per generare tutte le diagonali possibili con num di elem che
//la compongono k che va da 1 a MAX
int generaDiagonali(diag_t **p_vetDiag, diag_t* p_diagTmp,elem_t* vetEl, int nEl, int* p_nMaxDiag){
    int nDiag=0;
    int k;

    for (k=1; k<=MAX;k++){
        disp_r(0,k,p_vetDiag,p_diagTmp,p_nMaxDiag,&nDiag,vetEl,nEl);
    }
    //printf("Numero diagonali generate dalle disposizioni ripetute: %d\n",nDiag);
    return nDiag;
}

//funzione che controlla se l'elem che sto per inserire nella diagonale è compatibile
int checkCompatible(diag_t *p_diagTmp, elem_t *vetEl,int indexEl,int pos){

    if (pos==0){//devo aggiungere il primo el di una diag
        if(vetEl[indexEl].reqPrec == 0 && vetEl[indexEl].dirIng == 1 && vetEl[indexEl].diff <= DD){
            return 1;
        }
        else{
            return 0;
        }
    }
    else{
        if(vetEl[p_diagTmp->vettEl[pos-1]].finale == 0 && vetEl[p_diagTmp->vettEl[pos-1]].dirUsc == vetEl[indexEl].dirIng
        && (vetEl[indexEl].diff + p_diagTmp->diffTot) <= DD){
            return 1;
        }
        else{
            return 0;
        }
    }
}


//disp con ripetizione di n elem a k a k per formare tutte le possibili diagonali
void disp_r(int pos,int k,diag_t **pVetDiag, diag_t* p_diagTmp,int *p_nMaxDiag,int* p_nDiag,elem_t *vetEl, int nEl){
    int i;

    if (pos == k){//terminazione la diag ha raggiunto il num di elem che la compongono = k

        //salvo la diagonale appena creata nel vettore di diagonali
        if((*p_nDiag) == (*p_nMaxDiag)-1){
            //ha raggiunto la dimensione max bisogna riallocare
            riallocaVettDiag(pVetDiag,p_nMaxDiag);
        }

        (*pVetDiag)[(*p_nDiag)] = *p_diagTmp;
        (*p_nDiag)++;
        return;
    }

    for (i=0; i < nEl;i++){//per tutti gli elementi
        //verifico se l'elem che sto esaminando è compatibile, quindi se posso aggiungerlo agli
        //elementi che compongono la mia diagonale
        if(checkCompatible(p_diagTmp,vetEl,i,pos)){
            //se compatibile faccio la scelta e successivamente ricorro a pos+1
            p_diagTmp->nEl++;p_diagTmp->valoreTot+=vetEl[i].val;p_diagTmp->diffTot+=vetEl[i].diff;
            p_diagTmp->vettEl[pos]=i;
            disp_r(pos+1,k,pVetDiag,p_diagTmp,p_nMaxDiag,p_nDiag,vetEl,nEl);
            //backtrack
            p_diagTmp->vettEl[pos]=-1;
            p_diagTmp->nEl--;p_diagTmp->valoreTot-=vetEl[i].val;p_diagTmp->diffTot-=vetEl[i].diff;
        }//altrimenti passa ad esaminare prox elem
    }
}

void liberaVetDiag(diag_t *vetDiag){
    free(vetDiag);
}

void riallocaVettDiag(diag_t **pVetDiag,int* nMaxDiag){
    *pVetDiag = realloc(*pVetDiag, ((*nMaxDiag)*2)*sizeof(diag_t)); //rialloco del doppio
    *nMaxDiag = (*nMaxDiag) *2;
}

diag_t* allocaVetDiag(int nMaxDiag){
    diag_t *vetDiag;
    vetDiag = malloc(nMaxDiag*sizeof(diag_t));
    if (vetDiag == NULL){
        printf("Errore alloc vetdiag\n");
        return NULL;
    }
    return vetDiag;
}

void stampaDiag(diag_t diag, elem_t *vEl){
    printf("Diagonale\n");
    printf("Valore:%f\tDifficolta':%d\n",diag.valoreTot,diag.diffTot);
    for (int i=0; i< diag.nEl; i++){
        if(i==diag.nEl -1){
            printf("%s\n",vEl[diag.vettEl[i]].nome);
        }
        else {
            printf("%s   ", vEl[diag.vettEl[i]].nome);
        }
    }
}

void distruggiDiag(diag_t *diag){
    free(diag);
}

diag_t* diagInit(){
    diag_t *diag;
    diag = malloc(sizeof(diag_t));
    if (diag == NULL){
        printf("Errore allocazione diagonale\n");
        return NULL;
    }
    diag->nEl = 0;
    diag->diffTot=0;
    diag->valoreTot=0;
    for (int i=0; i<MAX; i++) {
        diag->vettEl[i] = -1;
    }
    return diag;
}



void distruggiVetEl(elem_t *vEl){
    free(vEl);
}

void stampaElemento(elem_t el){

    printf("Nome: %s Tipologia: %d DirIng: %d DirUsc: %d reqPrec: %d Finale: %d Valore: %f Difficolta': %d\n",
           el.nome,el.tipologia,el.dirIng,el.dirUsc,el.reqPrec,el.finale,el.val,el.diff);

}
void stampaVetEl(elem_t *vEl,int nEl){
    for (int i = 0; i < nEl; i++) {
        printf("Elem #%d\n",i+1);
        printf("Nome: %s Tipologia: %d DirIng: %d DirUsc: %d reqPrec: %d Finale: %d Valore: %f Difficolta': %d\n",
               vEl[i].nome,vEl[i].tipologia,vEl[i].dirIng,vEl[i].dirUsc,vEl[i].reqPrec,vEl[i].finale,vEl[i].val,vEl[i].diff);

    }
}

elem_t* leggiFile(char* filename,int *nEl){
    elem_t *vEl;
    int i=0;
    FILE *fp= fopen(filename,"r");
    if (fp==NULL){
        printf("Errore apertura file\n");
        return NULL;
    }
    fscanf(fp,"%d",nEl);
    vEl = malloc((*nEl)*sizeof(elem_t));
    if(vEl == NULL){
        printf("Errore alloc vett di elem\n");
        return NULL;
    }
    while (leggiElem(fp,&vEl[i])!=-1){
        i++;
    }
    fclose(fp);
    return vEl;
}

int leggiElem(FILE *fp,elem_t *pEl){

    if(fscanf(fp,"%s%d%d%d%d%d%f%d",pEl->nome,&pEl->tipologia,&pEl->dirIng,&pEl->dirUsc,
              &pEl->reqPrec,&pEl->finale,&pEl->val,&pEl->diff) != 8){

        return -1;
    }
    return 1;
}